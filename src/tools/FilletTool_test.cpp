#include "modeling/FilletTool.h"
#include "modeling/FilletToolImpl.h"
#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <chrono>

namespace RebelCAD {
namespace Modeling {

// Test helper class that exposes protected members
class TestableFilletTool : public FilletTool {
public:
    const Impl* GetImpl() const { return impl.get(); }
    Impl* GetImpl() { return impl.get(); }
};

class FilletToolTest : public ::testing::Test {
protected:
    TestableFilletTool tool;
    
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

    // Helper to verify UV coordinates
    void verifyUVCoordinates(const std::vector<float>& uvs) {
        ASSERT_FALSE(uvs.empty());
        ASSERT_EQ(uvs.size() % 2, 0);

        // Verify UV values are in valid range [0,1]
        for (size_t i = 0; i < uvs.size(); i += 2) {
            EXPECT_GE(uvs[i], 0.0f);     // U coordinate
            EXPECT_LE(uvs[i], 1.0f);
            EXPECT_GE(uvs[i+1], 0.0f);   // V coordinate
            EXPECT_LE(uvs[i+1], 1.0f);
        }
    }
};

TEST_F(FilletToolTest, BasicEdgeFillet) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);

    // Add edge (front-right edge of cube)
    std::vector<unsigned int> edge_indices = {1, 2};  // Vertices 1 and 2
    ASSERT_EQ(tool.AddEdge(edge_indices), FilletTool::ErrorCode::None);

    // Configure fillet
    FilletParams params;
    params.radius = 0.2f;
    params.segments = 8;
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);

    // Execute fillet operation
    ASSERT_EQ(tool.Execute(), FilletTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should have more vertices than input due to fillet geometry
    EXPECT_GT(result_verts.size(), vertices.size());
    EXPECT_GT(result_indices.size(), indices.size());

    // Verify UV coordinates
    const auto* impl = tool.GetImpl();
    verifyUVCoordinates(impl->result_uvs);
}

TEST_F(FilletToolTest, BasicFaceFillet) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);

    // Add face (top face of cube)
    std::vector<unsigned int> face_indices = {3, 2, 6, 7};  // Top face vertices
    ASSERT_EQ(tool.AddFace(face_indices), FilletTool::ErrorCode::None);

    // Configure fillet
    FilletParams params;
    params.radius = 0.2f;
    params.segments = 8;
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);

    // Execute fillet operation
    ASSERT_EQ(tool.Execute(), FilletTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Result should have more vertices than input due to fillet geometry
    EXPECT_GT(result_verts.size(), vertices.size());
    EXPECT_GT(result_indices.size(), indices.size());

    // Verify UV coordinates
    const auto* impl = tool.GetImpl();
    verifyUVCoordinates(impl->result_uvs);
}

TEST_F(FilletToolTest, VariableRadiusFillet) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);

    // Add edge (front-right edge of cube)
    std::vector<unsigned int> edge_indices = {1, 2};
    ASSERT_EQ(tool.AddEdge(edge_indices), FilletTool::ErrorCode::None);

    // Configure variable radius fillet
    FilletParams params;
    params.radius = 0.2f;
    params.segments = 8;
    params.variable_radius = true;
    params.radius_points = {
        0.0f, 0.1f,   // Start with small radius
        0.5f, 0.3f,   // Larger in middle
        1.0f, 0.1f    // Small again at end
    };
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);

    // Execute fillet operation
    ASSERT_EQ(tool.Execute(), FilletTool::ErrorCode::None);

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);

    // Verify UV coordinates
    const auto* impl = tool.GetImpl();
    verifyUVCoordinates(impl->result_uvs);
}

TEST_F(FilletToolTest, PreviewCaching) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);

    // Add edge
    std::vector<unsigned int> edge_indices = {1, 2};
    ASSERT_EQ(tool.AddEdge(edge_indices), FilletTool::ErrorCode::None);

    // Configure fillet
    FilletParams params;
    params.radius = 0.2f;
    params.segments = 8;
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);

    // Create mock graphics system for preview
    auto graphics = std::make_shared<RebelCAD::Graphics::GraphicsSystem>();

    // First preview should generate new mesh
    ASSERT_EQ(tool.Preview(graphics), FilletTool::ErrorCode::None);
    const auto* impl = tool.GetImpl();
    auto initial_vertices = impl->preview_cache.mesh_vertices;

    // Second preview with same parameters should use cache
    ASSERT_EQ(tool.Preview(graphics), FilletTool::ErrorCode::None);
    EXPECT_EQ(impl->preview_cache.mesh_vertices, initial_vertices);

    // Modified parameters should generate new preview
    params.radius = 0.3f;
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);
    ASSERT_EQ(tool.Preview(graphics), FilletTool::ErrorCode::None);
    EXPECT_NE(impl->preview_cache.mesh_vertices, initial_vertices);
}

TEST_F(FilletToolTest, ErrorHandling) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    // Test invalid mesh
    std::vector<float> invalid_verts = {0.0f, 0.0f};  // Invalid vertex count
    EXPECT_EQ(tool.SetMesh(invalid_verts, indices), FilletTool::ErrorCode::InvalidMesh);

    // Test invalid edge indices
    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);
    std::vector<unsigned int> invalid_edge = {0};  // Not enough vertices
    EXPECT_EQ(tool.AddEdge(invalid_edge), FilletTool::ErrorCode::InvalidMesh);

    // Test invalid face indices
    std::vector<unsigned int> invalid_face = {0, 1};  // Not enough vertices
    EXPECT_EQ(tool.AddFace(invalid_face), FilletTool::ErrorCode::InvalidMesh);

    // Test invalid parameters
    FilletParams params;
    params.radius = -1.0f;  // Invalid radius
    EXPECT_EQ(tool.Configure(params), FilletTool::ErrorCode::InvalidMesh);

    params.radius = 0.2f;
    params.segments = 2;  // Invalid segment count
    EXPECT_EQ(tool.Configure(params), FilletTool::ErrorCode::InvalidMesh);
}

TEST_F(FilletToolTest, LargeMeshPerformance) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    createCubeMesh(2.0f, vertices, indices);

    ASSERT_EQ(tool.SetMesh(vertices, indices), FilletTool::ErrorCode::None);

    // Add multiple edges
    for (size_t i = 0; i < 12; i += 2) {
        std::vector<unsigned int> edge_indices = {
            indices[i], indices[i + 1]
        };
        ASSERT_EQ(tool.AddEdge(edge_indices), FilletTool::ErrorCode::None);
    }

    // Configure high-resolution fillet
    FilletParams params;
    params.radius = 0.2f;
    params.segments = 32;
    params.optimize_result = true;
    ASSERT_EQ(tool.Configure(params), FilletTool::ErrorCode::None);

    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    ASSERT_EQ(tool.Execute(), FilletTool::ErrorCode::None);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Large mesh fillet operation took " << duration.count() << "ms" << std::endl;

    // Verify result
    const auto& result_verts = tool.GetResultVertices();
    const auto& result_indices = tool.GetResultIndices();
    verifyMeshProperties(result_verts, result_indices);
}

} // namespace Modeling
} // namespace RebelCAD
