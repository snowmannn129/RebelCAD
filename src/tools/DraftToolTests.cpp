#include <gtest/gtest.h>
#include "modeling/DraftTool.h"
#include "modeling/Geometry.h"
#include <memory>
#include <vector>
#include <cmath>

using namespace RebelCAD::Modeling;

class DraftToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        draft_tool = std::make_unique<DraftTool>();
    }

    std::unique_ptr<DraftTool> draft_tool;

    std::shared_ptr<SolidBody> CreateTestCube() {
        Topology topology;
        
        // Create a simple cube
        topology.vertices = {
            {0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1},  // Bottom face
            {0, 1, 0}, {1, 1, 0}, {1, 1, 1}, {0, 1, 1}   // Top face
        };

        topology.edges = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom edges
            {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top edges
            {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Vertical edges
        };

        topology.faces = {
            {{0, 1, 2, 3}, {true, true, true, true}},     // Bottom
            {{4, 5, 6, 7}, {true, true, true, true}},     // Top
            {{0, 9, 4, 8}, {true, true, false, false}},   // Front
            {{1, 10, 5, 9}, {true, true, false, false}},  // Right
            {{2, 11, 6, 10}, {true, true, false, false}}, // Back
            {{3, 8, 7, 11}, {true, true, false, false}}   // Left
        };

        return SolidBody::FromTopology(topology);
    }
};

TEST_F(DraftToolTests, ValidateEmptyFacesList) {
    std::vector<std::shared_ptr<SolidBody>> faces;
    DraftTool::DraftParams params{};
    params.angle = 5.0;
    params.pull_direction[0] = 0;
    params.pull_direction[1] = 1;
    params.pull_direction[2] = 0;

    auto result = draft_tool->ApplyDraft(faces, params);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::InvalidVertex);
}

TEST_F(DraftToolTests, ValidateInvalidAngle) {
    auto cube = CreateTestCube();
    std::vector<std::shared_ptr<SolidBody>> faces = {cube};
    
    DraftTool::DraftParams params{};
    params.angle = 90.0;  // Invalid angle
    params.pull_direction[0] = 0;
    params.pull_direction[1] = 1;
    params.pull_direction[2] = 0;

    auto result = draft_tool->ApplyDraft(faces, params);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(DraftToolTests, ValidateZeroPullDirection) {
    auto cube = CreateTestCube();
    std::vector<std::shared_ptr<SolidBody>> faces = {cube};
    
    DraftTool::DraftParams params{};
    params.angle = 5.0;
    params.pull_direction[0] = 0;
    params.pull_direction[1] = 0;
    params.pull_direction[2] = 0;

    auto result = draft_tool->ApplyDraft(faces, params);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(DraftToolTests, SuccessfulDraft) {
    auto cube = CreateTestCube();
    std::vector<std::shared_ptr<SolidBody>> faces = {cube};
    
    DraftTool::DraftParams params{};
    params.angle = 5.0;
    params.pull_direction[0] = 0;
    params.pull_direction[1] = 1;
    params.pull_direction[2] = 0;
    params.neutral_plane = 0.0;
    params.is_internal = false;

    auto result = draft_tool->ApplyDraft(faces, params);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(DraftToolTests, GetMinimumDraftAngle) {
    EXPECT_DOUBLE_EQ(draft_tool->GetMinimumDraftAngle("plastic"), 0.5);
    EXPECT_DOUBLE_EQ(draft_tool->GetMinimumDraftAngle("aluminum"), 1.0);
    EXPECT_DOUBLE_EQ(draft_tool->GetMinimumDraftAngle("steel"), 1.5);
    EXPECT_DOUBLE_EQ(draft_tool->GetMinimumDraftAngle("zinc"), 0.75);
    EXPECT_DOUBLE_EQ(draft_tool->GetMinimumDraftAngle("unknown"), 1.0);  // Default value
}

TEST_F(DraftToolTests, CanApplyDraft) {
    auto cube = CreateTestCube();
    std::vector<std::shared_ptr<SolidBody>> faces = {cube};
    EXPECT_TRUE(draft_tool->CanApplyDraft(faces));
}

TEST_F(DraftToolTests, InternalDraft) {
    auto cube = CreateTestCube();
    std::vector<std::shared_ptr<SolidBody>> faces = {cube};
    
    DraftTool::DraftParams params{};
    params.angle = 3.0;
    params.pull_direction[0] = 0;
    params.pull_direction[1] = 1;
    params.pull_direction[2] = 0;
    params.neutral_plane = 0.5;
    params.is_internal = true;

    auto result = draft_tool->ApplyDraft(faces, params);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}
