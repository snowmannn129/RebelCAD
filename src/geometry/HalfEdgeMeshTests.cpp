#include <gtest/gtest.h>
#include "modeling/HalfEdgeMesh.hpp"
#include <glm/glm.hpp>

using namespace RebelCAD::Modeling;

class HalfEdgeMeshTest : public ::testing::Test {
protected:
    HalfEdgeMesh mesh;

    // Helper to create a simple quad mesh
    void createQuad() {
        auto v1 = mesh.createVertex(glm::vec3(0.0f, 0.0f, 0.0f));
        auto v2 = mesh.createVertex(glm::vec3(1.0f, 0.0f, 0.0f));
        auto v3 = mesh.createVertex(glm::vec3(1.0f, 1.0f, 0.0f));
        auto v4 = mesh.createVertex(glm::vec3(0.0f, 1.0f, 0.0f));
        
        mesh.createFace({v1, v2, v3, v4});
    }

    // Helper to create a cube mesh
    void createCube() {
        // Front face vertices
        auto v1 = mesh.createVertex(glm::vec3(-1.0f, -1.0f,  1.0f));
        auto v2 = mesh.createVertex(glm::vec3( 1.0f, -1.0f,  1.0f));
        auto v3 = mesh.createVertex(glm::vec3( 1.0f,  1.0f,  1.0f));
        auto v4 = mesh.createVertex(glm::vec3(-1.0f,  1.0f,  1.0f));
        
        // Back face vertices
        auto v5 = mesh.createVertex(glm::vec3(-1.0f, -1.0f, -1.0f));
        auto v6 = mesh.createVertex(glm::vec3( 1.0f, -1.0f, -1.0f));
        auto v7 = mesh.createVertex(glm::vec3( 1.0f,  1.0f, -1.0f));
        auto v8 = mesh.createVertex(glm::vec3(-1.0f,  1.0f, -1.0f));

        // Create faces
        mesh.createFace({v1, v2, v3, v4}); // Front
        mesh.createFace({v5, v6, v7, v8}); // Back
        mesh.createFace({v1, v2, v6, v5}); // Bottom
        mesh.createFace({v4, v3, v7, v8}); // Top
        mesh.createFace({v1, v4, v8, v5}); // Left
        mesh.createFace({v2, v3, v7, v6}); // Right
    }
};

TEST_F(HalfEdgeMeshTest, CreateVertex) {
    auto vertex = mesh.createVertex(glm::vec3(1.0f, 2.0f, 3.0f));
    ASSERT_NE(vertex, nullptr);
    EXPECT_EQ(vertex->position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(vertex->outgoingHalfEdge, nullptr);
    EXPECT_FALSE(vertex->isSharp);
}

TEST_F(HalfEdgeMeshTest, CreateQuadFace) {
    createQuad();
    EXPECT_EQ(mesh.getVertices().size(), 4);
    EXPECT_EQ(mesh.getEdges().size(), 4);
    EXPECT_EQ(mesh.getFaces().size(), 1);
    EXPECT_TRUE(mesh.validateTopology());
}

TEST_F(HalfEdgeMeshTest, CreateCube) {
    createCube();
    EXPECT_EQ(mesh.getVertices().size(), 8);
    EXPECT_EQ(mesh.getEdges().size(), 12);
    EXPECT_EQ(mesh.getFaces().size(), 6);
    EXPECT_TRUE(mesh.validateTopology());
}

TEST_F(HalfEdgeMeshTest, InvalidFaceCreation) {
    auto v1 = mesh.createVertex(glm::vec3(0.0f, 0.0f, 0.0f));
    auto v2 = mesh.createVertex(glm::vec3(1.0f, 0.0f, 0.0f));

    EXPECT_THROW({
        mesh.createFace({v1, v2}); // Less than 3 vertices
    }, RebelCAD::Error);
}

TEST_F(HalfEdgeMeshTest, HalfEdgePairLinking) {
    createQuad();
    
    // Get the first face's half-edges
    auto face = mesh.getFaces()[0];
    auto he = face->halfEdge;
    
    // Verify half-edge pairs are properly linked
    EXPECT_NE(he->pair, nullptr);
    EXPECT_EQ(he->pair->pair, he);
}

TEST_F(HalfEdgeMeshTest, FaceNormalComputation) {
    createQuad();
    auto face = mesh.getFaces()[0];
    
    // For a quad in XY plane, normal should point along +Z
    EXPECT_NEAR(face->normal.x, 0.0f, 1e-6f);
    EXPECT_NEAR(face->normal.y, 0.0f, 1e-6f);
    EXPECT_NEAR(face->normal.z, 1.0f, 1e-6f);
}

TEST_F(HalfEdgeMeshTest, SharpFeatures) {
    createCube();
    
    // Mark an edge as sharp
    auto edge = mesh.getEdges()[0];
    mesh.markSharpEdge(edge, 1.0f);
    EXPECT_TRUE(edge->isSharp);
    EXPECT_FLOAT_EQ(edge->sharpness, 1.0f);

    // Mark a vertex as sharp
    auto vertex = mesh.getVertices()[0];
    mesh.markSharpVertex(vertex);
    EXPECT_TRUE(vertex->isSharp);
}

TEST_F(HalfEdgeMeshTest, InvalidSharpFeatures) {
    EXPECT_THROW({
        mesh.markSharpEdge(nullptr, 1.0f);
    }, RebelCAD::Error);

    EXPECT_THROW({
        mesh.markSharpVertex(nullptr);
    }, RebelCAD::Error);
}

TEST_F(HalfEdgeMeshTest, ClearMesh) {
    createCube();
    EXPECT_FALSE(mesh.getVertices().empty());
    
    mesh.clear();
    EXPECT_TRUE(mesh.getVertices().empty());
    EXPECT_TRUE(mesh.getEdges().empty());
    EXPECT_TRUE(mesh.getFaces().empty());
}

TEST_F(HalfEdgeMeshTest, TopologyValidation) {
    createCube();
    EXPECT_TRUE(mesh.validateTopology());

    // Corrupt topology by breaking a half-edge link
    auto face = mesh.getFaces()[0];
    face->halfEdge->next = nullptr;
    EXPECT_FALSE(mesh.validateTopology());
}

TEST_F(HalfEdgeMeshTest, FaceCentroidComputation) {
    createQuad();
    auto face = mesh.getFaces()[0];
    
    // For a unit quad at origin, centroid should be at (0.5, 0.5, 0)
    EXPECT_NEAR(face->centroid.x, 0.5f, 1e-6f);
    EXPECT_NEAR(face->centroid.y, 0.5f, 1e-6f);
    EXPECT_NEAR(face->centroid.z, 0.0f, 1e-6f);
}
