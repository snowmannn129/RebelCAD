#include <gtest/gtest.h>
#include "modeling/ExtrudeTool.h"
#include <glm/gtc/constants.hpp>

using namespace rebel_cad::modeling;

class ExtrudeToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<ExtrudeTool>();
        setupTestProfile();
    }

    void setupTestProfile() {
        // Create a simple square profile
        std::vector<glm::vec2> boundary = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        profile.setBoundary(boundary);

        // Add a circular hole
        std::vector<glm::vec2> hole;
        const float radius = 0.2f;
        const int segments = 16;
        for (int i = 0; i < segments; ++i) {
            float angle = (2.0f * glm::pi<float>() * i) / segments;
            hole.push_back({
                0.5f + radius * std::cos(angle),
                0.5f + radius * std::sin(angle)
            });
        }
        profile.addHole(hole);
    }

    std::unique_ptr<ExtrudeTool> tool;
    Profile profile;
};

TEST_F(ExtrudeToolTest, ValidatesProfile) {
    EXPECT_TRUE(tool->validateProfile(profile));

    // Test invalid profile (less than 3 points)
    Profile invalid_profile;
    std::vector<glm::vec2> invalid_boundary = {
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };
    invalid_profile.setBoundary(invalid_boundary);
    EXPECT_FALSE(tool->validateProfile(invalid_profile));
}

TEST_F(ExtrudeToolTest, BasicExtrusion) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify mesh properties
    EXPECT_GT(mesh->getVertexCount(), 0);
    EXPECT_GT(mesh->getFaceCount(), 0);
    EXPECT_TRUE(mesh->validate());
}

TEST_F(ExtrudeToolTest, DraftAngleExtrusion) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.draftAngle = glm::radians(5.0f);
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Store base vertices for comparison
    std::vector<glm::vec3> base_positions;
    const size_t base_vertex_count = 4; // Square profile has 4 corners
    for (size_t i = 0; i < base_vertex_count; ++i) {
        base_positions.push_back(mesh->getVertex(i).position);
    }
    
    // Get corresponding top vertices
    std::vector<glm::vec3> top_positions;
    const size_t vertex_count = mesh->getVertexCount();
    for (size_t i = vertex_count - base_vertex_count; i < vertex_count; ++i) {
        top_positions.push_back(mesh->getVertex(i).position);
    }
    
    // Verify draft angle effects
    for (size_t i = 0; i < base_vertex_count; ++i) {
        // Base vertices should remain unchanged at z=0
        EXPECT_FLOAT_EQ(base_positions[i].z, 0.0f);
        
        // Top vertices should be offset outward
        const float expected_offset = params.distance * std::tan(params.draftAngle);
        const glm::vec2 base_pos_2d(base_positions[i].x, base_positions[i].y);
        const glm::vec2 top_pos_2d(top_positions[i].x, top_positions[i].y);
        const glm::vec2 center_2d(0.5f, 0.5f); // Center of the square profile
        
        // Calculate distance from center to vertex in XY plane
        const float base_dist = glm::length(base_pos_2d - center_2d);
        const float top_dist = glm::length(top_pos_2d - center_2d);
        
        // Top distance should be greater than base distance by expected offset
        EXPECT_NEAR(top_dist - base_dist, expected_offset, 0.001f);
    }
    
    // Verify face normals
    for (size_t i = 0; i < mesh->getFaceCount(); ++i) {
        const auto& face = mesh->getFace(i);
        const float normal_z = face.normal.z;
        
        // Top and bottom faces should have vertical normals
        if (std::abs(normal_z) > 0.99f) {
            EXPECT_NEAR(std::abs(normal_z), 1.0f, 0.001f);
            EXPECT_NEAR(face.normal.x, 0.0f, 0.001f);
            EXPECT_NEAR(face.normal.y, 0.0f, 0.001f);
        }
        // Side faces should have tilted normals due to draft
        else {
            const float expected_tilt = params.draftAngle;
            const float actual_tilt = std::atan2(
                std::sqrt(face.normal.x * face.normal.x + face.normal.y * face.normal.y),
                std::abs(face.normal.z)
            );
            EXPECT_NEAR(actual_tilt, expected_tilt, 0.001f);
        }
    }
    
    // Verify mesh topology remains valid
    EXPECT_TRUE(mesh->validate());
    const auto& edges = mesh->getEdges();
    for (const auto& edge : edges) {
        EXPECT_LE(edge.adjacentFaces.size(), 2);
        EXPECT_GE(edge.adjacentFaces.size(), 1);
    }
}

TEST_F(ExtrudeToolTest, DraftAngleExtrusionWithHoles) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.draftAngle = glm::radians(10.0f);
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify hole edges are properly drafted
    const float expected_offset = params.distance * std::tan(params.draftAngle);
    const float hole_radius = 0.2f;
    const float expected_top_radius = hole_radius + expected_offset;
    
    // Sample points around the hole at z=distance
    const int sample_count = 8;
    bool found_hole_edge = false;
    for (size_t i = 0; i < mesh->getVertexCount(); ++i) {
        const auto& vertex = mesh->getVertex(i);
        if (std::abs(vertex.position.z - params.distance) < 0.001f) {
            const float dist_from_center = glm::length(
                glm::vec2(vertex.position.x - 0.5f, vertex.position.y - 0.5f)
            );
            if (std::abs(dist_from_center - expected_top_radius) < 0.01f) {
                found_hole_edge = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_hole_edge);
    
    // Verify mesh remains manifold
    EXPECT_TRUE(mesh->validate());
}

TEST_F(ExtrudeToolTest, ZeroDraftAngle) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.draftAngle = 0.0f;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify side faces remain vertical
    for (size_t i = 0; i < mesh->getFaceCount(); ++i) {
        const auto& face = mesh->getFace(i);
        if (std::abs(face.normal.z) < 0.99f) {
            // Side faces should have zero x,y normal components
            EXPECT_NEAR(face.normal.x, 0.0f, 0.001f);
            EXPECT_NEAR(face.normal.y, 0.0f, 0.001f);
        }
    }
}

TEST_F(ExtrudeToolTest, ReverseSideExtrusion) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    params.reverseSide = true;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify extrusion direction
    const auto& top_vertex = mesh->getVertex(mesh->getVertexCount() - 1);
    EXPECT_LT(top_vertex.position.z, 0.0f);
}

TEST_F(ExtrudeToolTest, CustomDirectionExtrusion) {
    ExtrudeTool::ExtrudeParams params;
    params.direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
    params.distance = 2.0f;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify extrusion direction
    const auto& top_vertex = mesh->getVertex(mesh->getVertexCount() - 1);
    EXPECT_GT(top_vertex.position.x, 0.0f);
    EXPECT_GT(top_vertex.position.y, 0.0f);
    EXPECT_GT(top_vertex.position.z, 0.0f);
}

TEST_F(ExtrudeToolTest, PreviewGeneration) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    
    // First preview generation
    auto preview1 = tool->generatePreview(profile, params);
    ASSERT_NE(preview1, nullptr);
    
    // Second call with same parameters should return cached result
    auto preview2 = tool->generatePreview(profile, params);
    EXPECT_EQ(preview1, preview2);
    
    // Modified parameters should generate new preview
    params.distance = 3.0f;
    auto preview3 = tool->generatePreview(profile, params);
    EXPECT_NE(preview1, preview3);
}

TEST_F(ExtrudeToolTest, InvalidProfileHandling) {
    Profile invalid_profile;
    std::vector<glm::vec2> invalid_boundary = {
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };
    invalid_profile.setBoundary(invalid_boundary);
    
    ExtrudeTool::ExtrudeParams params;
    EXPECT_THROW(tool->extrude(invalid_profile, params), std::invalid_argument);
}

TEST_F(ExtrudeToolTest, UVCoordinateGeneration) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify UV coordinates
    const auto& vertex = mesh->getVertex(0);
    EXPECT_GE(vertex.uv.x, 0.0f);
    EXPECT_GE(vertex.uv.y, 0.0f);
    EXPECT_LE(vertex.uv.x, 1.0f);
    EXPECT_LE(vertex.uv.y, 1.0f);
}

TEST_F(ExtrudeToolTest, ValidatesSections) {
    ExtrudeTool::ExtrudeParams params;
    
    // Empty sections should be valid (falls back to simple extrusion)
    EXPECT_TRUE(tool->validateSections({}));

    // Single section should be valid
    ExtrudeTool::Section section1;
    section1.profile = profile;
    section1.height = 1.0f;
    EXPECT_TRUE(tool->validateSections({section1}));

    // Multiple sections with increasing heights should be valid
    ExtrudeTool::Section section2;
    section2.profile = profile;
    section2.height = 2.0f;
    EXPECT_TRUE(tool->validateSections({section1, section2}));

    // Non-increasing heights should be invalid
    ExtrudeTool::Section section3;
    section3.profile = profile;
    section3.height = 1.5f;  // Less than section2
    EXPECT_FALSE(tool->validateSections({section1, section2, section3}));

    // Sections with incompatible profiles should be invalid
    Profile different_profile;
    std::vector<glm::vec2> different_boundary = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f},
        {0.5f, 0.5f}  // Extra point
    };
    different_profile.setBoundary(different_boundary);
    
    ExtrudeTool::Section invalid_section;
    invalid_section.profile = different_profile;
    invalid_section.height = 3.0f;
    EXPECT_FALSE(tool->validateSections({section1, invalid_section}));
}

TEST_F(ExtrudeToolTest, ProfileInterpolation) {
    // Create two different profiles for interpolation
    Profile profile1;
    std::vector<glm::vec2> boundary1 = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    profile1.setBoundary(boundary1);

    Profile profile2;
    std::vector<glm::vec2> boundary2 = {
        {0.0f, 0.0f},
        {2.0f, 0.0f},
        {2.0f, 2.0f},
        {0.0f, 2.0f}
    };
    profile2.setBoundary(boundary2);

    // Test interpolation at t=0.5
    Profile interpolated = tool->interpolateProfiles(profile1, profile2, 0.5f);
    const auto& points = interpolated.getBoundary().points;
    
    // Verify interpolated points
    for (size_t i = 0; i < points.size(); ++i) {
        const glm::vec2 expected = (boundary1[i] + boundary2[i]) * 0.5f;
        EXPECT_NEAR(points[i].x, expected.x, 0.001f);
        EXPECT_NEAR(points[i].y, expected.y, 0.001f);
    }
}

TEST_F(ExtrudeToolTest, ProfileTransformation) {
    // Test scaling
    Profile scaled = tool->transformProfile(profile, 2.0f, 0.0f);
    const auto& scaled_points = scaled.getBoundary().points;
    
    // Verify scaling
    for (size_t i = 0; i < scaled_points.size(); ++i) {
        const glm::vec2& original = profile.getBoundary().points[i];
        const glm::vec2& transformed = scaled_points[i];
        EXPECT_NEAR(glm::length(transformed), 2.0f * glm::length(original), 0.001f);
    }

    // Test twisting
    const float twist_angle = glm::radians(90.0f);
    Profile twisted = tool->transformProfile(profile, 1.0f, twist_angle);
    const auto& twisted_points = twisted.getBoundary().points;
    
    // Verify rotation
    for (size_t i = 0; i < twisted_points.size(); ++i) {
        const glm::vec2& original = profile.getBoundary().points[i];
        const glm::vec2& transformed = twisted_points[i];
        
        // Length should be preserved
        EXPECT_NEAR(glm::length(transformed), glm::length(original), 0.001f);
        
        // Angle should be rotated by twist_angle
        if (glm::length(original) > 0.001f) {
            float original_angle = std::atan2(original.y, original.x);
            float transformed_angle = std::atan2(transformed.y, transformed.x);
            float angle_diff = transformed_angle - original_angle;
            if (angle_diff > glm::pi<float>()) angle_diff -= 2.0f * glm::pi<float>();
            if (angle_diff < -glm::pi<float>()) angle_diff += 2.0f * glm::pi<float>();
            EXPECT_NEAR(angle_diff, twist_angle, 0.001f);
        }
    }
}

TEST_F(ExtrudeToolTest, MultiSectionExtrusion) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 3.0f;
    
    // Create sections with different scales and twists
    ExtrudeTool::Section section1;
    section1.profile = profile;
    section1.height = 1.0f;
    section1.scale = 1.5f;
    section1.twist = glm::radians(45.0f);
    
    ExtrudeTool::Section section2;
    section2.profile = profile;
    section2.height = 2.0f;
    section2.scale = 2.0f;
    section2.twist = glm::radians(90.0f);
    
    params.sections = {section1, section2};
    params.interpolateProfiles = true;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify mesh properties
    EXPECT_GT(mesh->getVertexCount(), 0);
    EXPECT_GT(mesh->getFaceCount(), 0);
    EXPECT_TRUE(mesh->validate());
    
    // Verify intermediate layers were created
    bool found_intermediate = false;
    for (size_t i = 0; i < mesh->getVertexCount(); ++i) {
        const auto& vertex = mesh->getVertex(i);
        if (vertex.position.z > 0.0f && vertex.position.z < params.distance) {
            found_intermediate = true;
            break;
        }
    }
    EXPECT_TRUE(found_intermediate);
}

TEST_F(ExtrudeToolTest, PreviewCacheWithSections) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    
    ExtrudeTool::Section section;
    section.profile = profile;
    section.height = 1.0f;
    section.scale = 1.5f;
    section.twist = glm::radians(45.0f);
    
    params.sections = {section};
    
    // First preview generation
    auto preview1 = tool->generatePreview(profile, params);
    ASSERT_NE(preview1, nullptr);
    
    // Same parameters should return cached result
    auto preview2 = tool->generatePreview(profile, params);
    EXPECT_EQ(preview1, preview2);
    
    // Modified section parameters should generate new preview
    params.sections[0].scale = 2.0f;
    auto preview3 = tool->generatePreview(profile, params);
    EXPECT_NE(preview1, preview3);
}

TEST_F(ExtrudeToolTest, TopologyValidation) {
    ExtrudeTool::ExtrudeParams params;
    params.distance = 2.0f;
    
    auto mesh = tool->extrude(profile, params);
    ASSERT_NE(mesh, nullptr);
    
    // Verify mesh topology
    const auto& edges = mesh->getEdges();
    for (const auto& edge : edges) {
        // Each edge should have 1 or 2 adjacent faces
        EXPECT_GE(edge.adjacentFaces.size(), 1);
        EXPECT_LE(edge.adjacentFaces.size(), 2);
    }
}
