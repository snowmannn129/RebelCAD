#include "modeling/BooleanTool.h"
#include "modeling/BooleanToolImpl.h"
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

// Test helper class that exposes protected members
class TestableBooleanTool : public BooleanTool {
public:
    const Impl* GetImpl() const { return impl.get(); }
    Impl* GetImpl() { return impl.get(); }
};

class BooleanToolTest : public ::testing::Test {
protected:
    TestableBooleanTool tool;
    
    // Helper to create a cube mesh centered at origin
    void createCubeMesh(float size, 
                       std::vector<float>& vertices,
                       std::vector<unsigned int>& indices) {
        float half = size * 0.5f;
        
        // Vertices (8 corners of cube)
        vertices = {
            -half, -half, -half,  // 0
            half, -half, -half,   // 1
            half, half, -half,    // 2
            -half, half, -half,   // 3
            -half, -half, half,   // 4
            half, -half, half,    // 5
            half, half, half,     // 6
            -half, half, half     // 7
        };

        // Indices for 12 triangles (6 faces)
        indices = {
            0, 1, 2,  0, 2, 3,  // Front
            1, 5, 6,  1, 6, 2,  // Right
            5, 4, 7,  5, 7, 6,  // Back
            4, 0, 3,  4, 3, 7,  // Left
            3, 2, 6,  3, 6, 7,  // Top
            4, 5, 1,  4, 1, 0   // Bottom
        };
    }

    // Helper to create a sphere mesh centered at origin
    void createSphereMesh(float radius, int segments,
                         std::vector<float>& vertices,
                         std::vector<unsigned int>& indices) {
        vertices.clear();
        indices.clear();

        // Generate vertices
        for (int lat = 0; lat <= segments; lat++) {
            float theta = lat * glm::pi<float>() / segments;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            for (int lon = 0; lon <= segments; lon++) {
                float phi = lon * 2.0f * glm::pi<float>() / segments;
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                float x = cosPhi * sinTheta;
                float y = cosTheta;
                float z = sinPhi * sinTheta;

                vertices.push_back(x * radius);
                vertices.push_back(y * radius);
                vertices.push_back(z * radius);
            }
        }

        // Generate indices
        for (int lat = 0; lat < segments; lat++) {
            for (int lon = 0; lon < segments; lon++) {
                unsigned int first = lat * (segments + 1) + lon;
                unsigned int second = first + segments + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
    }

    // Helper to verify mesh properties
    void verifyMeshProperties(const std::vector<float>& vertices,
                            const std::vector<unsigned int>& indices) {
        ASSERT_FALSE(vertices.empty());
        ASSERT_FALSE(indices.empty());
        ASSERT_EQ(vertices.size() % 3, 0);
        ASSERT_EQ(indices.size() % 3, 0);

        // Verify indices are valid
        for (unsigned int idx : indices) {
            EXPECT_LT(idx * 3 + 2, vertices.size());
        }
    }
};

TEST_F(BooleanToolTest, BasicUnion) {
    std::vector<float> cube1_verts, cube2_verts;
    std::vector<unsigned int> cube1_indices, cube2_indices;
    
    // Create two overlapping cubes
    createCubeMesh(2.0f, cube1_verts, cube1_indices);
    createCubeMesh(2.0f, cube2_verts, cube2_indices);
    
    // Offset second cube
    for (size_t i = 0; i < cube2_verts.size(); i += 3) {
        cube2_verts[i] += 1.0f;  // Offset in X
    }

    ASSERT_EQ(tool.SetFirstBody(cube1_verts, cube1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube2_verts, cube2_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should have more vertices than either input due to new intersection geometry
    EXPECT_GT(result_verts.size(), cube1_verts.size());
    EXPECT_GT(result_verts.size(), cube2_verts.size());
}

TEST_F(BooleanToolTest, BasicSubtract) {
    std::vector<float> cube_verts, sphere_verts;
    std::vector<unsigned int> cube_indices, sphere_indices;
    
    // Create cube with sphere to subtract from it
    createCubeMesh(2.0f, cube_verts, cube_indices);
    createSphereMesh(1.0f, 16, sphere_verts, sphere_indices);

    ASSERT_EQ(tool.SetFirstBody(cube_verts, cube_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(sphere_verts, sphere_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Subtract;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should have more vertices than cube due to new intersection geometry
    EXPECT_GT(result_verts.size(), cube_verts.size());
}

TEST_F(BooleanToolTest, BasicIntersect) {
    std::vector<float> cube1_verts, cube2_verts;
    std::vector<unsigned int> cube1_indices, cube2_indices;
    
    // Create two overlapping cubes
    createCubeMesh(2.0f, cube1_verts, cube1_indices);
    createCubeMesh(2.0f, cube2_verts, cube2_indices);
    
    // Offset second cube
    for (size_t i = 0; i < cube2_verts.size(); i += 3) {
        cube2_verts[i] += 1.0f;  // Offset in X
    }

    ASSERT_EQ(tool.SetFirstBody(cube1_verts, cube1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube2_verts, cube2_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Intersect;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should be smaller than either input
    EXPECT_LT(result_verts.size(), cube1_verts.size());
    EXPECT_LT(result_verts.size(), cube2_verts.size());
}

TEST_F(BooleanToolTest, PreviewCaching) {
    std::vector<float> cube1_verts, cube2_verts;
    std::vector<unsigned int> cube1_indices, cube2_indices;
    createCubeMesh(2.0f, cube1_verts, cube1_indices);
    createCubeMesh(2.0f, cube2_verts, cube2_indices);

    ASSERT_EQ(tool.SetFirstBody(cube1_verts, cube1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube2_verts, cube2_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);

    // Create mock graphics system for preview
    auto graphics = std::make_shared<RebelCAD::Graphics::GraphicsSystem>();

    // First preview should generate new mesh
    ASSERT_EQ(tool.Preview(graphics), BooleanTool::ErrorCode::None);
    const auto* impl = tool.GetImpl();
    auto initial_vertices = impl->preview_cache.mesh_vertices;

    // Second preview with same parameters should use cache
    ASSERT_EQ(tool.Preview(graphics), BooleanTool::ErrorCode::None);
    EXPECT_EQ(impl->preview_cache.mesh_vertices, initial_vertices);

    // Modified parameters should generate new preview
    params.operation_type = BooleanOperationType::Subtract;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Preview(graphics), BooleanTool::ErrorCode::None);
    EXPECT_NE(impl->preview_cache.mesh_vertices, initial_vertices);
}

TEST_F(BooleanToolTest, ErrorHandling) {
    std::vector<float> cube_verts;
    std::vector<unsigned int> cube_indices;
    createCubeMesh(2.0f, cube_verts, cube_indices);

    // Test invalid first body
    std::vector<float> invalid_verts = {0.0f, 0.0f};  // Invalid vertex count
    EXPECT_EQ(tool.SetFirstBody(invalid_verts, cube_indices), BooleanTool::ErrorCode::InvalidMesh);

    // Test invalid second body
    std::vector<unsigned int> invalid_indices = {0, 1};  // Invalid index count
    EXPECT_EQ(tool.SetSecondBody(cube_verts, invalid_indices), BooleanTool::ErrorCode::InvalidMesh);

    // Test invalid parameters
    BooleanParams params;
    params.intersection_tolerance = -1.0f;  // Invalid tolerance
    EXPECT_EQ(tool.Configure(params), BooleanTool::ErrorCode::InvalidMesh);

    params.intersection_tolerance = 1e-6f;
    params.max_refinement_steps = 0;  // Invalid step count
    EXPECT_EQ(tool.Configure(params), BooleanTool::ErrorCode::InvalidMesh);
}

TEST_F(BooleanToolTest, CoplanarFaces) {
    std::vector<float> cube1_verts, cube2_verts;
    std::vector<unsigned int> cube1_indices, cube2_indices;
    
    // Create two cubes sharing a face
    createCubeMesh(2.0f, cube1_verts, cube1_indices);
    createCubeMesh(2.0f, cube2_verts, cube2_indices);
    
    // Move second cube to share a face with first cube
    for (size_t i = 0; i < cube2_verts.size(); i += 3) {
        cube2_verts[i] += 2.0f;  // Move along X to share YZ face
    }

    ASSERT_EQ(tool.SetFirstBody(cube1_verts, cube1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube2_verts, cube2_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should have fewer vertices than sum of inputs due to shared face
    EXPECT_LT(result_verts.size(), cube1_verts.size() + cube2_verts.size());
}

TEST_F(BooleanToolTest, ComplexIntersection) {
    std::vector<float> sphere1_verts, sphere2_verts;
    std::vector<unsigned int> sphere1_indices, sphere2_indices;
    
    // Create two intersecting spheres
    createSphereMesh(1.0f, 32, sphere1_verts, sphere1_indices);
    createSphereMesh(1.0f, 32, sphere2_verts, sphere2_indices);
    
    // Offset second sphere for partial intersection
    for (size_t i = 0; i < sphere2_verts.size(); i += 3) {
        sphere2_verts[i] += 1.0f;  // Offset in X
    }

    ASSERT_EQ(tool.SetFirstBody(sphere1_verts, sphere1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(sphere2_verts, sphere2_indices), BooleanTool::ErrorCode::None);

    // Test all operations with complex intersection
    BooleanParams params;
    
    // Union
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    verifyMeshProperties(tool.GetResultVertices(), tool.GetResultIndices());

    // Subtract
    params.operation_type = BooleanOperationType::Subtract;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    verifyMeshProperties(tool.GetResultVertices(), tool.GetResultIndices());

    // Intersect
    params.operation_type = BooleanOperationType::Intersect;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    verifyMeshProperties(tool.GetResultVertices(), tool.GetResultIndices());
}

TEST_F(BooleanToolTest, LargeMeshPerformance) {
    std::vector<float> sphere_verts, cube_verts;
    std::vector<unsigned int> sphere_indices, cube_indices;
    
    // Create high-resolution sphere and cube
    createSphereMesh(1.0f, 64, sphere_verts, sphere_indices);
    createCubeMesh(2.0f, cube_verts, cube_indices);

    ASSERT_EQ(tool.SetFirstBody(sphere_verts, sphere_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube_verts, cube_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    params.operation_type = BooleanOperationType::Subtract;
    params.optimize_result = true;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);

    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Large mesh operation took " << duration.count() << "ms" << std::endl;

    // Verify result
    verifyMeshProperties(tool.GetResultVertices(), tool.GetResultIndices());
}

TEST_F(BooleanToolTest, NonIntersectingBodies) {
    std::vector<float> cube1_verts, cube2_verts;
    std::vector<unsigned int> cube1_indices, cube2_indices;
    
    // Create two non-intersecting cubes
    createCubeMesh(1.0f, cube1_verts, cube1_indices);
    createCubeMesh(1.0f, cube2_verts, cube2_indices);
    
    // Move second cube far away
    for (size_t i = 0; i < cube2_verts.size(); i += 3) {
        cube2_verts[i] += 10.0f;
    }

    ASSERT_EQ(tool.SetFirstBody(cube1_verts, cube1_indices), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.SetSecondBody(cube2_verts, cube2_indices), BooleanTool::ErrorCode::None);

    BooleanParams params;
    
    // Union should succeed and include both meshes
    params.operation_type = BooleanOperationType::Union;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    EXPECT_EQ(tool.GetResultVertices().size(), cube1_verts.size() + cube2_verts.size());

    // Intersection should succeed but produce empty result
    params.operation_type = BooleanOperationType::Intersect;
    ASSERT_EQ(tool.Configure(params), BooleanTool::ErrorCode::None);
    ASSERT_EQ(tool.Execute(), BooleanTool::ErrorCode::None);
    EXPECT_TRUE(tool.GetResultVertices().empty());
}

} // namespace Modeling
} // namespace RebelCAD
