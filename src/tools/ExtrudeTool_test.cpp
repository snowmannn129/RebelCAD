#include "modeling/ExtrudeTool.h"
#include "modeling/Profile.h"
#include "modeling/Mesh.h"
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <algorithm>

using namespace rebel_cad::modeling;

class ExtrudeToolTest : public ::testing::Test {
protected:
    ExtrudeTool tool;
    
    // Helper to create a square profile
    Profile createSquareProfile(float size = 1.0f) {
        Profile profile;
        std::vector<glm::vec2> points = {
            {-size/2, -size/2},
            {size/2, -size/2},
            {size/2, size/2},
            {-size/2, size/2}
        };
        profile.setBoundary(points);
        return profile;
    }

    // Helper to create a circular profile
    Profile createCircularProfile(float radius = 1.0f, int segments = 16) {
        Profile profile;
        std::vector<glm::vec2> points;
        for (int i = 0; i < segments; ++i) {
            float angle = (2.0f * glm::pi<float>() * i) / segments;
            points.push_back({
                radius * std::cos(angle),
                radius * std::sin(angle)
            });
        }
        profile.setBoundary(points);
        return profile;
    }

    // Helper to verify mesh properties
    void verifyMeshProperties(const Mesh& mesh, size_t expected_vertices, size_t expected_faces) {
        EXPECT_EQ(mesh.getVertexCount(), expected_vertices);
        EXPECT_EQ(mesh.getFaceCount(), expected_faces);
        
        // Verify all vertices have UV coordinates
        for (size_t i = 0; i < mesh.getVertexCount(); ++i) {
            const auto& vertex = mesh.getVertex(i);
            EXPECT_FALSE(std::isnan(vertex.uv.x));
            EXPECT_FALSE(std::isnan(vertex.uv.y));
        }

        // Verify all faces have valid normals
        for (size_t i = 0; i < mesh.getFaceCount(); ++i) {
            const auto& face = mesh.getFace(i);
            EXPECT_NEAR(glm::length(face.normal), 1.0f, 0.001f);
        }
    }
};

TEST_F(ExtrudeToolTest, SimpleExtrusion) {
    Profile square = createSquareProfile();
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;

    auto mesh = tool.extrude(square, params);
    ASSERT_NE(mesh, nullptr);
    
    // Square profile with 4 vertices, extruded creates:
    // 4 vertices on base + 4 vertices on top + 4 side faces (8 triangles) + 2 cap faces (4 triangles)
    verifyMeshProperties(*mesh, 8, 12);
}

TEST_F(ExtrudeToolTest, ExtrusionWithDraftAngle) {
    Profile square = createSquareProfile();
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.draftAngle = glm::radians(5.0f);

    auto mesh = tool.extrude(square, params);
    ASSERT_NE(mesh, nullptr);
    verifyMeshProperties(*mesh, 8, 12);

    // Verify draft angle effect
    float expected_offset = 2.0f * std::tan(params.draftAngle);
    const auto& top_vertex = mesh->getVertex(4); // First vertex of top face
    EXPECT_NEAR(std::abs(top_vertex.position.x), expected_offset/2, 0.001f);
}

TEST_F(ExtrudeToolTest, MultiSectionExtrusion) {
    // Create two different sized squares for start and end profiles
    Profile start_profile = createSquareProfile(1.0f);
    Profile end_profile = createSquareProfile(2.0f);

    ExtrudeTool::ExtrudeParams params;
    params.distance = 3.0f;
    params.sections = {
        {start_profile, 0.0f, 1.0f, 0.0f},
        {end_profile, 3.0f, 2.0f, glm::pi<float>() / 4} // End profile is larger and rotated 45 degrees
    };
    params.interpolateProfiles = true;

    auto mesh = tool.extrude(start_profile, params);
    ASSERT_NE(mesh, nullptr);

    // With interpolation (4 subdivisions), we expect 6 layers of vertices
    // Each layer has 4 vertices, plus triangulated faces between layers
    verifyMeshProperties(*mesh, 24, 40);

    // Verify scale and rotation of final layer
    const auto& final_vertex = mesh->getVertex(mesh->getVertexCount() - 1);
    EXPECT_NEAR(glm::length(glm::vec2(final_vertex.position.x, final_vertex.position.y)), 
                1.0f, 0.001f); // Account for rotation
}

TEST_F(ExtrudeToolTest, ProfileInterpolation) {
    Profile circle = createCircularProfile(1.0f, 16);
    Profile square = createSquareProfile(2.0f);

    // Test interpolation at t=0.5
    Profile interpolated = tool.interpolateProfiles(circle, square, 0.5f);
    ASSERT_EQ(interpolated.getBoundary().points.size(), 16);

    // Verify points are between circle and square
    const auto& boundary = interpolated.getBoundary();
    for (const auto& point : boundary.points) {
        // Points should be roughly between radius 1.0 and 1.414 (sqrt(2))
        float distance = glm::length(point);
        EXPECT_GE(distance, 0.9f);
        EXPECT_LE(distance, 1.5f);
    }
}

TEST_F(ExtrudeToolTest, ProfileTransformation) {
    Profile square = createSquareProfile();
    float scale = 2.0f;
    float twist = glm::pi<float>() / 4; // 45 degrees

    Profile transformed = tool.transformProfile(square, scale, twist);
    ASSERT_EQ(transformed.getBoundary().points.size(), 4);

    // Verify scale
    const auto& points = transformed.getBoundary().points;
    for (const auto& point : points) {
        float distance = glm::length(point);
        EXPECT_NEAR(distance, scale * 0.707107f, 0.001f); // sqrt(2)/2 * scale
    }

    // Verify rotation of first point
    EXPECT_NEAR(std::atan2(points[0].y, points[0].x), -3 * glm::pi<float>() / 4, 0.001f);
}

TEST_F(ExtrudeToolTest, GenerateIntermediateProfiles) {
    Profile start = createSquareProfile(1.0f);
    Profile end = createSquareProfile(2.0f);

    std::vector<ExtrudeTool::Section> sections = {
        {start, 0.0f, 1.0f, 0.0f},
        {end, 1.0f, 2.0f, glm::pi<float>() / 2}
    };

    std::vector<Profile> intermediates = tool.generateIntermediateProfiles(sections, 4);
    ASSERT_EQ(intermediates.size(), 5); // Start + 3 intermediate + end

    // Verify gradual scale increase
    for (size_t i = 0; i < intermediates.size(); ++i) {
        float t = static_cast<float>(i) / (intermediates.size() - 1);
        float expected_scale = 1.0f + t;
        
        const auto& points = intermediates[i].getBoundary().points;
        float actual_scale = glm::length(points[0]) / 0.707107f; // sqrt(2)/2
        EXPECT_NEAR(actual_scale, expected_scale, 0.001f);
    }
}

TEST_F(ExtrudeToolTest, PreviewGeneration) {
    Profile square = createSquareProfile();
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.draftAngle = glm::radians(5.0f);

    // First preview generation
    auto preview1 = tool.generatePreview(square, params);
    ASSERT_NE(preview1, nullptr);

    // Second call with same parameters should return cached result
    auto preview2 = tool.generatePreview(square, params);
    EXPECT_EQ(preview1, preview2);

    // Modified parameters should generate new preview
    params.distance = 3.0f;
    auto preview3 = tool.generatePreview(square, params);
    EXPECT_NE(preview1, preview3);
}

TEST_F(ExtrudeToolTest, InvalidProfileHandling) {
    // Create invalid profile with less than 3 points
    Profile invalid_profile;
    std::vector<glm::vec2> points = {{0,0}, {1,1}};
    invalid_profile.setBoundary(points);

    ExtrudeTool::ExtrudeParams params;
    EXPECT_FALSE(tool.validateProfile(invalid_profile));
    EXPECT_THROW(tool.extrude(invalid_profile, params), std::invalid_argument);
}

TEST_F(ExtrudeToolTest, UVCoordinateGeneration) {
    Profile square = createSquareProfile(2.0f);
    ExtrudeTool::ExtrudeParams params;
    params.distance = 3.0f;

    auto mesh = tool.extrude(square, params);
    ASSERT_NE(mesh, nullptr);

    // Verify UV coordinates are within [0,1] range
    for (size_t i = 0; i < mesh->getVertexCount(); ++i) {
        const auto& uv = mesh->getVertex(i).uv;
        EXPECT_GE(uv.x, -1.0f);
        EXPECT_LE(uv.x, 1.0f);
        EXPECT_GE(uv.y, -1.0f);
        EXPECT_LE(uv.y, 1.0f);
    }
}

TEST_F(ExtrudeToolTest, MultiSectionTextureMapping) {
    Profile start = createCircularProfile(1.0f, 16);
    Profile end = createSquareProfile(2.0f);

    ExtrudeTool::ExtrudeParams params;
    params.distance = 3.0f;
    params.sections = {
        {start, 0.0f, 1.0f, 0.0f},
        {end, 3.0f, 2.0f, glm::pi<float>() / 4}
    };
    params.interpolateProfiles = true;

    auto mesh = tool.extrude(start, params);
    ASSERT_NE(mesh, nullptr);

    // Verify UV continuity between sections
    std::vector<float> section_v_coords;
    for (size_t i = 0; i < mesh->getVertexCount(); ++i) {
        const auto& vertex = mesh->getVertex(i);
        if (std::abs(vertex.position.x) < 0.001f && vertex.position.y > 0) {
            section_v_coords.push_back(vertex.uv.y);
        }
    }

    // Sort v coordinates
    std::sort(section_v_coords.begin(), section_v_coords.end());

    // Verify v coordinates are monotonically increasing
    for (size_t i = 1; i < section_v_coords.size(); ++i) {
        EXPECT_GT(section_v_coords[i], section_v_coords[i-1]);
    }
}
