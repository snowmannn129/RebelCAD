#include <gtest/gtest.h>
#include "modeling/SubdivisionBooleanOperations.hpp"
#include "modeling/SubdivisionSurface.h"
#include "modeling/BooleanTool.h"
#include "modeling/Geometry.h"
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include <memory>
#include <vector>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace RebelCAD::Modeling;
using namespace RebelCAD::Core;
using namespace RebelCAD::Graphics;

class SubdivisionBooleanOperationsTest : public ::testing::Test {
protected:
    Face CreateFace(const std::vector<size_t>& vertices) {
        Face face;
        // Create edges between consecutive vertices
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t next = (i + 1) % vertices.size();
            face.edge_indices.push_back(vertices[i]);
            face.edge_directions.push_back(true);
        }
        return face;
    }

    void SetUp() override {
        // Create two simple subdivision surfaces for testing
        // First surface: cube
        std::vector<Point3D> vertices1 = {
            {0.0f, 0.0f, 0.0f}, // 0
            {1.0f, 0.0f, 0.0f}, // 1
            {1.0f, 1.0f, 0.0f}, // 2
            {0.0f, 1.0f, 0.0f}, // 3
            {0.0f, 0.0f, 1.0f}, // 4
            {1.0f, 0.0f, 1.0f}, // 5
            {1.0f, 1.0f, 1.0f}, // 6
            {0.0f, 1.0f, 1.0f}  // 7
        };

        std::vector<Edge> edges1 = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},  // front face
            {4, 5}, {5, 6}, {6, 7}, {7, 4},  // back face
            {0, 4}, {1, 5}, {2, 6}, {3, 7}   // connecting edges
        };

        std::vector<Face> faces1;
        faces1.push_back(CreateFace({0, 1, 2, 3}));  // front
        faces1.push_back(CreateFace({4, 5, 6, 7}));  // back
        faces1.push_back(CreateFace({0, 1, 5, 4}));  // bottom
        faces1.push_back(CreateFace({2, 3, 7, 6}));  // top
        faces1.push_back(CreateFace({0, 3, 7, 4}));  // left
        faces1.push_back(CreateFace({1, 2, 6, 5}));  // right

        std::vector<size_t> sharpEdges1 = {0, 1, 2, 3, 4, 5, 6, 7};

        // Second surface: sphere
        std::vector<Point3D> vertices2;
        std::vector<Edge> edges2;
        std::vector<Face> faces2;
        std::vector<size_t> sharpEdges2;

        CreateSphere(vertices2, edges2, faces2, sharpEdges2, 0.5f, 16);

        // Create subdivision surfaces
        firstSurface = SubdivisionSurface::Create(vertices1, edges1, faces1, sharpEdges1);
        secondSurface = SubdivisionSurface::Create(vertices2, edges2, faces2, sharpEdges2);

        // Create a cube with very small features for degenerate tests
        std::vector<Point3D> smallVertices = vertices1;
        for (auto& v : smallVertices) {
            v.x *= 0.001f;
            v.y *= 0.001f;
            v.z *= 0.001f;
        }
        smallFeatureSurface = SubdivisionSurface::Create(smallVertices, edges1, faces1, sharpEdges1);

        // Create overlapping cubes
        std::vector<Point3D> overlappingVertices = vertices1;
        for (auto& v : overlappingVertices) {
            v.x += 0.5f;
            v.y += 0.5f;
            v.z += 0.5f;
        }
        overlappingSurface = SubdivisionSurface::Create(overlappingVertices, edges1, faces1, sharpEdges1);
    }

    void CreateSphere(
        std::vector<Point3D>& vertices,
        std::vector<Edge>& edges,
        std::vector<Face>& faces,
        std::vector<size_t>& sharpEdges,
        float radius,
        int segments) {
        
        // Create vertices
        for (int i = 0; i <= segments; ++i) {
            float phi = static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(segments);
            for (int j = 0; j <= segments; ++j) {
                float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / static_cast<float>(segments);
                float x = radius * std::sin(phi) * std::cos(theta);
                float y = radius * std::sin(phi) * std::sin(theta);
                float z = radius * std::cos(phi);
                vertices.push_back({x, y, z});
            }
        }

        // Create edges and faces
        for (int i = 0; i < segments; ++i) {
            for (int j = 0; j < segments; ++j) {
                int v1 = i * (segments + 1) + j;
                int v2 = v1 + 1;
                int v3 = (i + 1) * (segments + 1) + j;
                int v4 = v3 + 1;

                edges.push_back({static_cast<size_t>(v1), static_cast<size_t>(v2)});
                edges.push_back({static_cast<size_t>(v2), static_cast<size_t>(v4)});
                edges.push_back({static_cast<size_t>(v4), static_cast<size_t>(v3)});
                edges.push_back({static_cast<size_t>(v3), static_cast<size_t>(v1)});

                faces.push_back(CreateFace({
                    static_cast<size_t>(v1),
                    static_cast<size_t>(v2),
                    static_cast<size_t>(v4),
                    static_cast<size_t>(v3)
                }));
            }
        }
    }

    // Helper function to calculate face normal
    glm::vec3 CalculateFaceNormal(const Face& face, const std::vector<Point3D>& vertices) {
        // Need at least 3 vertices to calculate normal
        if (face.edge_indices.size() < 3) {
            return glm::vec3(0.0f);
        }

        // Get three non-collinear points
        const auto& v1 = vertices[face.edge_indices[0]];
        const auto& v2 = vertices[face.edge_indices[1]];
        const auto& v3 = vertices[face.edge_indices[2]];

        // Convert Point3D to glm::vec3
        glm::vec3 p1(v1.x, v1.y, v1.z);
        glm::vec3 p2(v2.x, v2.y, v2.z);
        glm::vec3 p3(v3.x, v3.y, v3.z);

        // Calculate normal using cross product
        glm::vec3 edge1 = p2 - p1;
        glm::vec3 edge2 = p3 - p1;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        return normal;
    }

    std::shared_ptr<SubdivisionSurface> firstSurface;
    std::shared_ptr<SubdivisionSurface> secondSurface;
    std::shared_ptr<SubdivisionSurface> smallFeatureSurface;
    std::shared_ptr<SubdivisionSurface> overlappingSurface;
};

TEST_F(SubdivisionBooleanOperationsTest, UnionOperation) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = true;

    auto result = SubdivisionBooleanOperations::PerformOperation(
        firstSurface,
        secondSurface,
        BooleanOperationType::Union,
        params);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);
}

TEST_F(SubdivisionBooleanOperationsTest, SubtractOperation) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = true;

    auto result = SubdivisionBooleanOperations::PerformOperation(
        firstSurface,
        secondSurface,
        BooleanOperationType::Subtract,
        params);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);
}

TEST_F(SubdivisionBooleanOperationsTest, IntersectOperation) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = true;

    auto result = SubdivisionBooleanOperations::PerformOperation(
        firstSurface,
        secondSurface,
        BooleanOperationType::Intersect,
        params);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);
}

TEST_F(SubdivisionBooleanOperationsTest, PreviewOperation) {
    BooleanParams params;
    auto graphics = std::shared_ptr<GraphicsSystem>(&GraphicsSystem::getInstance(), [](GraphicsSystem*){});

    auto result = SubdivisionBooleanOperations::PreviewOperation(
        firstSurface,
        secondSurface,
        BooleanOperationType::Union,
        graphics,
        params);

    EXPECT_EQ(result, ErrorCode::None);
}

TEST_F(SubdivisionBooleanOperationsTest, InvalidInputs) {
    BooleanParams params;

    // Test with null first surface
    EXPECT_THROW(
        SubdivisionBooleanOperations::PerformOperation(
            nullptr,
            secondSurface,
            BooleanOperationType::Union,
            params),
        std::runtime_error);

    // Test with null second surface
    EXPECT_THROW(
        SubdivisionBooleanOperations::PerformOperation(
            firstSurface,
            nullptr,
            BooleanOperationType::Union,
            params),
        std::runtime_error);

    // Test preview with null graphics system
    auto result = SubdivisionBooleanOperations::PreviewOperation(
        firstSurface,
        secondSurface,
        BooleanOperationType::Union,
        nullptr,
        params);
    EXPECT_EQ(result, ErrorCode::GraphicsError);
}

// New test cases for sharp feature preservation
TEST_F(SubdivisionBooleanOperationsTest, SharpFeaturePreservation) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = false;  // Disable optimization to test pure feature preservation

    // Test with different angle tolerances
    std::vector<float> angles = {15.0f, 30.0f, 45.0f, 60.0f};
    for (float angle : angles) {
        params.intersection_tolerance = angle / 180.0f; // Convert angle to tolerance
        auto result = SubdivisionBooleanOperations::PerformOperation(
            firstSurface,
            secondSurface,
            BooleanOperationType::Union,
            params);

        ASSERT_NE(result, nullptr);
        
        // Verify sharp edges are preserved
        const auto& sharpEdges = result->GetSharpEdges();
        EXPECT_GT(sharpEdges.size(), 0);

        // Verify edge angles
        const auto& vertices = result->GetVertices();
        const auto& edges = result->GetEdges();
        for (size_t edgeIndex : sharpEdges) {
            const auto& edge = edges[edgeIndex];
            const auto& v1 = vertices[edge.vertex1];
            const auto& v2 = vertices[edge.vertex2];
            
            // For boolean operations, sharp features are determined by the dihedral angle
            // between adjacent faces. Calculate this angle to verify sharpness.
            const auto& faces = result->GetFaces();
            float maxAngle = 0.0f;
            
            // Find faces sharing this edge
            for (size_t i = 0; i < faces.size(); ++i) {
                for (size_t j = i + 1; j < faces.size(); ++j) {
                    const auto& face1 = faces[i];
                    const auto& face2 = faces[j];
                    
                    // Check if both faces share this edge
                    bool sharesEdge = false;
                    for (size_t k = 0; k < face1.edge_indices.size(); ++k) {
                        for (size_t l = 0; l < face2.edge_indices.size(); ++l) {
                            if (face1.edge_indices[k] == edgeIndex && face2.edge_indices[l] == edgeIndex) {
                                sharesEdge = true;
                                break;
                            }
                        }
                        if (sharesEdge) break;
                    }
                    
                    if (sharesEdge) {
                        // Calculate angle between face normals
                        glm::vec3 normal1 = CalculateFaceNormal(face1, vertices);
                        glm::vec3 normal2 = CalculateFaceNormal(face2, vertices);
                        float angle = glm::degrees(std::acos(glm::dot(normal1, normal2)));
                        maxAngle = std::max(maxAngle, angle);
                    }
                }
            }
            
            // Verify the angle is sharp enough
            EXPECT_GE(maxAngle, 30.0f);
        }
    }
}

// New test cases for topology optimization
TEST_F(SubdivisionBooleanOperationsTest, TopologyOptimization) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = true;

    // Test with small features that should be collapsed
    auto result = SubdivisionBooleanOperations::PerformOperation(
        smallFeatureSurface,
        secondSurface,
        BooleanOperationType::Union,
        params);

    ASSERT_NE(result, nullptr);

    // Verify small edges are collapsed
    const auto& vertices = result->GetVertices();
    const auto& edges = result->GetEdges();
    
    // Check that no edges are shorter than the minimum length
    const float minEdgeLength = 1e-4f;
    for (const auto& edge : edges) {
        const auto& v1 = vertices[edge.vertex1];
        const auto& v2 = vertices[edge.vertex2];
        float length = std::sqrt(
            std::pow(v2.x - v1.x, 2) +
            std::pow(v2.y - v1.y, 2) +
            std::pow(v2.z - v1.z, 2));
        EXPECT_GE(length, minEdgeLength);
    }

    // Verify topology is still valid
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);
}

// New test cases for edge cases
TEST_F(SubdivisionBooleanOperationsTest, EdgeCases) {
    BooleanParams params;
    params.maintain_features = true;
    params.optimize_result = true;

    // Test with overlapping surfaces
    auto result = SubdivisionBooleanOperations::PerformOperation(
        firstSurface,
        overlappingSurface,
        BooleanOperationType::Union,
        params);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);

    // Test with very small features
    result = SubdivisionBooleanOperations::PerformOperation(
        smallFeatureSurface,
        overlappingSurface,
        BooleanOperationType::Subtract,
        params);

    ASSERT_NE(result, nullptr);
    EXPECT_GT(result->GetVertices().size(), 0);
    EXPECT_GT(result->GetEdges().size(), 0);
    EXPECT_GT(result->GetFaces().size(), 0);

    // Test with coincident faces
    auto coincidentResult = SubdivisionBooleanOperations::PerformOperation(
        firstSurface,
        firstSurface,  // Same surface
        BooleanOperationType::Union,
        params);

    ASSERT_NE(coincidentResult, nullptr);
    EXPECT_GT(coincidentResult->GetVertices().size(), 0);
    EXPECT_GT(coincidentResult->GetEdges().size(), 0);
    EXPECT_GT(coincidentResult->GetFaces().size(), 0);
}
