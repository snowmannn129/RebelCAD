#include "modeling/RevolveTool.h"
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

// Test helper class that exposes protected members
class TestableRevolveTool : public RevolveTool {
public:
    const RevolveToolImpl* GetImpl() const { return impl.get(); }
    RevolveToolImpl* GetImpl() { return impl.get(); }
};

class RevolveToolTest : public ::testing::Test {
protected:
    TestableRevolveTool tool;
    
    // Helper to create a rectangular profile
    std::vector<float> createRectProfile(float width = 1.0f, float height = 2.0f) {
        return {
            0.0f, 0.0f, 0.0f,      // Start at origin
            width, 0.0f, 0.0f,     // Width along X
            width, height, 0.0f,    // Up to height
            0.0f, height, 0.0f      // Back to origin X
        };
    }

    // Helper to create a semicircle profile
    std::vector<float> createSemiCircleProfile(float radius = 1.0f, int segments = 16) {
        std::vector<float> points;
        points.reserve((segments + 1) * 3);
        
        // Start at origin
        points.push_back(0.0f);
        points.push_back(0.0f);
        points.push_back(0.0f);
        
        for (int i = 0; i <= segments; ++i) {
            float angle = (glm::pi<float>() * i) / segments;
            points.push_back(radius * std::cos(angle));
            points.push_back(radius * std::sin(angle));
            points.push_back(0.0f);
        }
        
        return points;
    }
};

TEST_F(RevolveToolTest, BasicRevolve) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify mesh data was generated
    const auto* impl = tool.GetImpl();
    EXPECT_FALSE(impl->mesh_vertices.empty());
    EXPECT_FALSE(impl->mesh_normals.empty());
    EXPECT_FALSE(impl->mesh_uvs.empty());
    EXPECT_FALSE(impl->mesh_indices.empty());
    
    // Verify vertex count matches segments
    size_t vertices_per_profile = profile.size() / 3;
    size_t expected_vertices = vertices_per_profile * (params.segments + 1);
    EXPECT_EQ(impl->mesh_vertices.size() / 3, expected_vertices);
}

TEST_F(RevolveToolTest, PartialRevolve) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 180.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 16;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify segment count was adjusted for partial revolve
    const auto* impl = tool.GetImpl();
    size_t vertices_per_profile = profile.size() / 3;
    size_t expected_vertices = vertices_per_profile * (params.segments/2 + 1);
    EXPECT_EQ(impl->mesh_vertices.size() / 3, expected_vertices);
}

TEST_F(RevolveToolTest, MultiSectionRevolve) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    // Add two sections with different scales and twists
    RevolveSection section1;
    section1.angle = 120.0f;
    section1.scale = 1.5f;
    section1.twist = glm::radians(30.0f);
    section1.profile_vertices = profile;
    
    RevolveSection section2;
    section2.angle = 240.0f;
    section2.scale = 2.0f;
    section2.twist = glm::radians(60.0f);
    section2.profile_vertices = profile;
    
    params.sections = {section1, section2};
    params.interpolateSections = true;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify mesh was generated with sections
    const auto* impl = tool.GetImpl();
    EXPECT_FALSE(impl->mesh_vertices.empty());
    EXPECT_GT(impl->mesh_vertices.size(), profile.size() * params.segments);
}

TEST_F(RevolveToolTest, ProfileValidation) {
    // Test profile intersecting axis
    std::vector<float> invalid_profile = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  // This line lies on Y axis
        1.0f, 1.0f, 0.0f
    };
    
    EXPECT_EQ(tool.SetProfile(invalid_profile), RevolveTool::ErrorCode::InvalidMesh);
    
    // Test empty profile
    std::vector<float> empty_profile;
    EXPECT_EQ(tool.SetProfile(empty_profile), RevolveTool::ErrorCode::InvalidMesh);
    
    // Test valid profile
    auto valid_profile = createRectProfile(1.0f, 1.0f);
    EXPECT_EQ(tool.SetProfile(valid_profile), RevolveTool::ErrorCode::None);
}

TEST_F(RevolveToolTest, PreviewCaching) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    
    // Create mock graphics system for preview
    auto graphics = std::make_shared<RebelCAD::Graphics::GraphicsSystem>();
    
    // First preview should generate new mesh
    ASSERT_EQ(tool.Preview(graphics), RevolveTool::ErrorCode::None);
    const auto* impl = tool.GetImpl();
    auto initial_vertices = impl->preview_cache.mesh_vertices;
    
    // Second preview with same parameters should use cache
    ASSERT_EQ(tool.Preview(graphics), RevolveTool::ErrorCode::None);
    EXPECT_EQ(impl->preview_cache.mesh_vertices, initial_vertices);
    
    // Modified parameters should generate new preview
    params.angle = 180.0f;
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Preview(graphics), RevolveTool::ErrorCode::None);
    EXPECT_NE(impl->preview_cache.mesh_vertices, initial_vertices);
}

TEST_F(RevolveToolTest, UVMapping) {
    auto profile = createSemiCircleProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify UV coordinates
    const auto* impl = tool.GetImpl();
    const auto& uvs = impl->mesh_uvs;
    ASSERT_FALSE(uvs.empty());
    
    // Check UV range
    for (size_t i = 0; i < uvs.size(); i += 2) {
        float u = uvs[i];
        float v = uvs[i + 1];
        
        EXPECT_GE(u, 0.0f);
        EXPECT_LE(u, 1.0f);
        EXPECT_GE(v, 0.0f);
        EXPECT_LE(v, 1.0f);
    }
}

TEST_F(RevolveToolTest, ErrorHandling) {
    auto profile = createRectProfile();
    
    // Test invalid angle
    RevolveParams params;
    params.angle = -90.0f;
    EXPECT_EQ(tool.Configure(params), RevolveTool::ErrorCode::InvalidMesh);
    
    params.angle = 361.0f;
    EXPECT_EQ(tool.Configure(params), RevolveTool::ErrorCode::InvalidMesh);
    
    // Test invalid segments
    params.angle = 360.0f;
    params.segments = 3;  // Minimum is 4
    EXPECT_EQ(tool.Configure(params), RevolveTool::ErrorCode::InvalidMesh);
    
    // Test invalid axis
    params.segments = 32;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f);  // Zero length axis
    EXPECT_EQ(tool.Configure(params), RevolveTool::ErrorCode::InvalidMesh);
}

TEST_F(RevolveToolTest, SubtractiveRevolve) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    params.isSubtractive = true;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify mesh was generated
    const auto* impl = tool.GetImpl();
    EXPECT_FALSE(impl->mesh_vertices.empty());
}

TEST_F(RevolveToolTest, FullRevolutionOptimization) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    params.full_revolution = true;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // In a full revolution, first and last vertices should be the same
    const auto* impl = tool.GetImpl();
    size_t vertices_per_profile = profile.size() / 3;
    size_t last_profile_start = impl->mesh_vertices.size() - vertices_per_profile * 3;
    
    for (size_t i = 0; i < vertices_per_profile * 3; i++) {
        EXPECT_NEAR(impl->mesh_vertices[i], impl->mesh_vertices[last_profile_start + i], 0.001f);
    }
}

TEST_F(RevolveToolTest, CommonShapes) {
    // Test cylinder creation
    std::vector<float> cylinder_profile = {
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };
    
    ASSERT_EQ(tool.SetProfile(cylinder_profile), RevolveTool::ErrorCode::None);
    
    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Test cone creation
    std::vector<float> cone_profile = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };
    
    ASSERT_EQ(tool.SetProfile(cone_profile), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
}

TEST_F(RevolveToolTest, ProfilePointOrder) {
    // Test counter-clockwise profile (should succeed)
    std::vector<float> ccw_profile = {
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        2.0f, 1.0f, 0.0f,
        2.0f, 0.0f, 0.0f
    };
    
    ASSERT_EQ(tool.SetProfile(ccw_profile), RevolveTool::ErrorCode::None);
    
    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify normals point outward
    const auto* impl = tool.GetImpl();
    for (size_t i = 0; i < impl->mesh_normals.size(); i += 3) {
        glm::vec3 normal(impl->mesh_normals[i], impl->mesh_normals[i+1], impl->mesh_normals[i+2]);
        glm::vec3 vertex(impl->mesh_vertices[i], impl->mesh_vertices[i+1], impl->mesh_vertices[i+2]);
        
        // Normal should point away from axis
        glm::vec3 to_vertex = vertex - params.axis_start;
        float dot_product = glm::dot(glm::normalize(to_vertex), normal);
        EXPECT_GT(dot_product, 0.0f);
    }
}

TEST_F(RevolveToolTest, NormalGeneration) {
    auto profile = createRectProfile();
    ASSERT_EQ(tool.SetProfile(profile), RevolveTool::ErrorCode::None);

    RevolveParams params;
    params.angle = 360.0f;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.segments = 32;
    
    ASSERT_EQ(tool.Configure(params), RevolveTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), RevolveTool::ErrorCode::None);
    
    // Verify normals
    const auto* impl = tool.GetImpl();
    const auto& normals = impl->mesh_normals;
    ASSERT_FALSE(normals.empty());
    
    // Check normal lengths
    for (size_t i = 0; i < normals.size(); i += 3) {
        glm::vec3 normal(normals[i], normals[i+1], normals[i+2]);
        EXPECT_NEAR(glm::length(normal), 1.0f, 0.001f);
    }
}

} // namespace Modeling
} // namespace RebelCAD
