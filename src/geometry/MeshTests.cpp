#include <gtest/gtest.h>
#include "modeling/Mesh.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace rebel_cad::modeling;

class MeshTest : public ::testing::Test {
protected:
    void SetUp() override {
        mesh = std::make_shared<Mesh>();
        createCubeMesh();
    }

    void createCubeMesh() {
        // Vertices
        vertices = {
            {-0.5f, -0.5f, -0.5f},  // 0
            { 0.5f, -0.5f, -0.5f},  // 1
            { 0.5f,  0.5f, -0.5f},  // 2
            {-0.5f,  0.5f, -0.5f},  // 3
            {-0.5f, -0.5f,  0.5f},  // 4
            { 0.5f, -0.5f,  0.5f},  // 5
            { 0.5f,  0.5f,  0.5f},  // 6
            {-0.5f,  0.5f,  0.5f}   // 7
        };

        // Add vertices to mesh
        for (const auto& pos : vertices) {
            vertex_indices.push_back(mesh->addVertex(pos));
        }

        // Add faces (6 faces, each as 2 triangles)
        // Front face
        mesh->addFace({0, 1, 2});
        mesh->addFace({0, 2, 3});
        // Back face
        mesh->addFace({5, 4, 7});
        mesh->addFace({5, 7, 6});
        // Right face
        mesh->addFace({1, 5, 6});
        mesh->addFace({1, 6, 2});
        // Left face
        mesh->addFace({4, 0, 3});
        mesh->addFace({4, 3, 7});
        // Top face
        mesh->addFace({3, 2, 6});
        mesh->addFace({3, 6, 7});
        // Bottom face
        mesh->addFace({1, 0, 4});
        mesh->addFace({1, 4, 5});

        mesh->updateEdgeTopology();
    }

    std::shared_ptr<Mesh> mesh;
    std::vector<glm::vec3> vertices;
    std::vector<size_t> vertex_indices;
};

TEST_F(MeshTest, VertexOperations) {
    // Test vertex addition
    size_t idx = mesh->addVertex({1.0f, 1.0f, 1.0f});
    EXPECT_EQ(idx, 8);  // Should be the 9th vertex
    
    // Test vertex normal setting
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    mesh->setVertexNormal(idx, normal);
    EXPECT_EQ(mesh->getVertex(idx).normal, normal);
    
    // Test vertex UV setting
    glm::vec2 uv(0.5f, 0.5f);
    mesh->setVertexUV(idx, uv);
    EXPECT_EQ(mesh->getVertex(idx).uv, uv);
    
    // Test vertex count
    EXPECT_EQ(mesh->getVertexCount(), 9);
}

TEST_F(MeshTest, FaceOperations) {
    // Test face addition
    size_t face_idx = mesh->addFace({0, 1, 2});
    EXPECT_GE(face_idx, 0);
    
    // Test face normal setting
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    mesh->setFaceNormal(face_idx, normal);
    EXPECT_EQ(mesh->getFace(face_idx).normal, normal);
    
    // Test face count (12 cube faces + 1 new face)
    EXPECT_EQ(mesh->getFaceCount(), 13);
}

TEST_F(MeshTest, EdgeTopology) {
    // Each edge in a cube should have exactly 2 adjacent faces
    const auto& edges = mesh->getEdges();
    for (const auto& edge : edges) {
        EXPECT_EQ(edge.adjacentFaces.size(), 2);
    }
    
    // A cube should have 12 edges
    EXPECT_EQ(edges.size(), 12);
}

TEST_F(MeshTest, MeshValidation) {
    EXPECT_TRUE(mesh->validate());

    // Create invalid mesh (face with out-of-bounds vertex index)
    auto invalid_mesh = std::make_shared<Mesh>();
    invalid_mesh->addVertex({0.0f, 0.0f, 0.0f});
    invalid_mesh->addVertex({1.0f, 0.0f, 0.0f});
    invalid_mesh->addVertex({0.0f, 1.0f, 0.0f});
    invalid_mesh->addFace({0, 1, 99});  // Invalid vertex index
    EXPECT_FALSE(invalid_mesh->validate());
}

TEST_F(MeshTest, NormalComputation) {
    mesh->computeNormals();

    // Check that all vertices have non-zero normals
    for (size_t i = 0; i < mesh->getVertexCount(); ++i) {
        const auto& normal = mesh->getVertex(i).normal;
        EXPECT_GT(glm::length(normal), 0.0f);
        
        // Normal should be normalized
        EXPECT_NEAR(glm::length(normal), 1.0f, 1e-6f);
    }

    // Check face normals
    for (size_t i = 0; i < mesh->getFaceCount(); ++i) {
        const auto& normal = mesh->getFace(i).normal;
        EXPECT_GT(glm::length(normal), 0.0f);
        EXPECT_NEAR(glm::length(normal), 1.0f, 1e-6f);
    }
}

TEST_F(MeshTest, Translation) {
    glm::vec3 offset(1.0f, 2.0f, 3.0f);
    mesh->translate(offset);

    // Check first vertex position
    const auto& translated_pos = mesh->getVertex(0).position;
    const auto& original_pos = vertices[0];
    EXPECT_EQ(translated_pos, original_pos + offset);
}

TEST_F(MeshTest, Rotation) {
    // Rotate 90 degrees around Y axis
    mesh->rotate(glm::pi<float>() / 2.0f, {0.0f, 1.0f, 0.0f});

    // Check transformation of a vertex
    const auto& rotated_pos = mesh->getVertex(1).position;
    EXPECT_NEAR(rotated_pos.x, 0.5f, 1e-6f);
    EXPECT_NEAR(rotated_pos.z, 0.5f, 1e-6f);
}

TEST_F(MeshTest, Scale) {
    glm::vec3 scale_factors(2.0f, 2.0f, 2.0f);
    mesh->scale(scale_factors);

    // Check scaled vertex position
    const auto& scaled_pos = mesh->getVertex(1).position;
    const auto& original_pos = vertices[1];
    EXPECT_EQ(scaled_pos, original_pos * scale_factors);
}

TEST_F(MeshTest, ClearOperation) {
    mesh->clear();
    EXPECT_EQ(mesh->getVertexCount(), 0);
    EXPECT_EQ(mesh->getFaceCount(), 0);
    EXPECT_EQ(mesh->getEdges().size(), 0);
}

TEST_F(MeshTest, NonManifoldCheck) {
    // Create a non-manifold edge (3 faces sharing an edge)
    auto non_manifold = std::make_shared<Mesh>();
    
    // Add vertices
    size_t v0 = non_manifold->addVertex({0.0f, 0.0f, 0.0f});
    size_t v1 = non_manifold->addVertex({1.0f, 0.0f, 0.0f});
    size_t v2 = non_manifold->addVertex({0.0f, 1.0f, 0.0f});
    size_t v3 = non_manifold->addVertex({0.0f, 0.0f, 1.0f});
    
    // Add three faces sharing an edge (v0-v1)
    non_manifold->addFace({v0, v1, v2});
    non_manifold->addFace({v0, v1, v3});
    non_manifold->addFace({v0, v3, v1});
    
    non_manifold->updateEdgeTopology();
    EXPECT_FALSE(non_manifold->validate());
}

TEST_F(MeshTest, UVCoordinates) {
    // Set UV coordinates for a face
    for (size_t i = 0; i < 3; ++i) {
        mesh->setVertexUV(i, {static_cast<float>(i) / 2.0f, 0.0f});
    }

    // Verify UV coordinates
    for (size_t i = 0; i < 3; ++i) {
        const auto& uv = mesh->getVertex(i).uv;
        EXPECT_NEAR(uv.x, static_cast<float>(i) / 2.0f, 1e-6f);
        EXPECT_NEAR(uv.y, 0.0f, 1e-6f);
    }
}
