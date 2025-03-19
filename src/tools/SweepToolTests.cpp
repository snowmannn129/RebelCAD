#include <gtest/gtest.h>
#include "modeling/SweepTool.h"
#include "sketching/Spline.h"
#include "graphics/GraphicsSystem.h"

using namespace RebelCAD::Modeling;
using namespace RebelCAD::Sketching;
using namespace RebelCAD::Graphics;

class SweepToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        sweepTool = std::make_unique<SweepTool>();
        profile = std::make_shared<Spline>();  // Create a test profile
        path = std::make_shared<Spline>();     // Create a test path
        guideRail = std::make_shared<Spline>(); // Create a test guide rail
        graphics = std::make_shared<GraphicsSystem>();
    }

    std::unique_ptr<SweepTool> sweepTool;
    std::shared_ptr<Spline> profile;
    std::shared_ptr<Spline> path;
    std::shared_ptr<Spline> guideRail;
    std::shared_ptr<GraphicsSystem> graphics;
};

TEST_F(SweepToolTests, InitialState) {
    EXPECT_FALSE(sweepTool->IsReady());
}

TEST_F(SweepToolTests, SetProfile) {
    auto result = sweepTool->SetProfile(profile);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_FALSE(sweepTool->IsReady());  // Not ready until path is set
}

TEST_F(SweepToolTests, SetPath) {
    auto result = sweepTool->SetPath(path);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_FALSE(sweepTool->IsReady());  // Not ready until profile is set
}

TEST_F(SweepToolTests, SetGuideRail) {
    auto result = sweepTool->SetGuideRail(guideRail);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(SweepToolTests, ReadyState) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    EXPECT_TRUE(sweepTool->IsReady());
}

TEST_F(SweepToolTests, NullProfile) {
    auto result = sweepTool->SetProfile(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::InvalidVertex);
}

TEST_F(SweepToolTests, NullPath) {
    auto result = sweepTool->SetPath(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::InvalidVertex);
}

TEST_F(SweepToolTests, NullGuideRail) {
    auto result = sweepTool->SetGuideRail(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::InvalidVertex);
}

TEST_F(SweepToolTests, Reset) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    EXPECT_TRUE(sweepTool->IsReady());
    
    sweepTool->Reset();
    EXPECT_FALSE(sweepTool->IsReady());
}

TEST_F(SweepToolTests, ExecuteWithoutProfile) {
    sweepTool->SetPath(path);
    auto result = sweepTool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(SweepToolTests, ExecuteWithoutPath) {
    sweepTool->SetProfile(profile);
    auto result = sweepTool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(SweepToolTests, PreviewWithoutGraphics) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    auto result = sweepTool->Preview(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(SweepToolTests, ValidPreview) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    auto result = sweepTool->Preview(graphics);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(SweepToolTests, GuideRailModeWithoutRail) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    
    SweepTool::SweepOptions options;
    options.orientationMode = SweepTool::OrientationMode::GuideRail;
    
    auto result = sweepTool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(SweepToolTests, ValidExecution) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    
    SweepTool::SweepOptions options;
    options.orientationMode = SweepTool::OrientationMode::PathNormal;
    options.SetTwistAngle(45.0f);
    options.SetScaleFactors(1.0f, 2.0f);
    
    auto result = sweepTool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(SweepToolTests, CancelPreview) {
    sweepTool->SetProfile(profile);
    sweepTool->SetPath(path);
    sweepTool->Preview(graphics);
    sweepTool->Cancel();
    
    // After canceling, should still be able to execute
    auto result = sweepTool->Execute();
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}
