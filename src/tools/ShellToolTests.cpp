#include <gtest/gtest.h>
#include "modeling/ShellTool.h"
#include <memory>
#include <vector>

using namespace RebelCAD::Modeling;

class ShellToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<ShellTool>();
        // Create a simple cube as test geometry
        cube = CreateTestCube();
    }

    std::unique_ptr<ShellTool> tool;
    std::shared_ptr<SolidBody> cube;

    // Helper to create a simple cube for testing
    std::shared_ptr<SolidBody> CreateTestCube() {
        Topology topology;
        
        // Vertices
        topology.vertices = {
            {0.0, 0.0, 0.0}, // v0
            {1.0, 0.0, 0.0}, // v1
            {1.0, 1.0, 0.0}, // v2
            {0.0, 1.0, 0.0}, // v3
            {0.0, 0.0, 1.0}, // v4
            {1.0, 0.0, 1.0}, // v5
            {1.0, 1.0, 1.0}, // v6
            {0.0, 1.0, 1.0}  // v7
        };

        // Edges
        topology.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0}, // bottom face
            {4, 5}, {5, 6}, {6, 7}, {7, 4}, // top face
            {0, 4}, {1, 5}, {2, 6}, {3, 7}  // vertical edges
        };

        // Faces (simplified - just indices to edges that form each face)
        Face bottom_face;
        bottom_face.edge_indices = {0, 1, 2, 3};
        bottom_face.edge_directions = {true, true, true, true};

        Face top_face;
        top_face.edge_indices = {4, 5, 6, 7};
        top_face.edge_directions = {true, true, true, true};

        Face front_face;
        front_face.edge_indices = {0, 9, 4, 8};
        front_face.edge_directions = {true, true, false, false};

        topology.faces = {bottom_face, top_face, front_face};
        
        return SolidBody::FromTopology(topology);
    }
};

TEST_F(ShellToolTests, InitialState) {
    EXPECT_NE(tool, nullptr);
}

TEST_F(ShellToolTests, ConfigurationValidation) {
    // Null body should fail
    auto result = tool->Configure(nullptr, 0.1);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Zero thickness should fail
    result = tool->Configure(cube, 0.0);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Negative thickness should fail
    result = tool->Configure(cube, -0.1);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Too large thickness should fail (> half smallest dimension)
    result = tool->Configure(cube, 0.6); // Cube is 1x1x1
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Valid configuration should succeed
    result = tool->Configure(cube, 0.1);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(ShellToolTests, ExecutionValidation) {
    // Execution without configuration should fail
    auto result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Configure with valid parameters
    result = tool->Configure(cube, 0.1);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);

    // Execution with valid configuration should succeed
    result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(ShellToolTests, FaceRemovalValidation) {
    // Invalid face index should fail
    auto result = tool->Configure(cube, 0.1, {-1});
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    result = tool->Configure(cube, 0.1, {999});
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);

    // Valid face indices should succeed
    result = tool->Configure(cube, 0.1, {0, 1});
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);

    // Execute with face removal should succeed
    result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(ShellToolTests, MultipleOperations) {
    // First operation
    auto result = tool->Configure(cube, 0.1);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);

    // Second operation with different parameters
    result = tool->Configure(cube, 0.2, {1});
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(ShellToolTests, EdgeCases) {
    // Test with minimum valid thickness
    auto result = tool->Configure(cube, 0.001);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);

    // Test with maximum valid thickness
    result = tool->Configure(cube, 0.499); // Just under half the cube size
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);

    // Test removing all faces (should fail as it would create invalid geometry)
    result = tool->Configure(cube, 0.1, {0, 1, 2});
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    result = tool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}
