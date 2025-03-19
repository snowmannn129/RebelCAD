#include "modeling/LoftTool.h"
#include "sketching/Spline.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace RebelCAD;
using namespace RebelCAD::Modeling;

class LoftToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<LoftTool>();
    }

    std::shared_ptr<Sketching::Spline> CreateCircleProfile(double radius, double centerX = 0.0, double centerY = 0.0) {
        auto profile = std::make_shared<Sketching::Spline>();
        
        // Create circle control points
        const int numPoints = 12;
        for (int i = 0; i <= numPoints; ++i) {
            double angle = (2.0 * M_PI * i) / numPoints;
            double x = centerX + radius * std::cos(angle);
            double y = centerY + radius * std::sin(angle);
            profile->addControlPoint(x, y);
        }
        
        return profile;
    }

    std::shared_ptr<Sketching::Spline> CreateSquareProfile(double size, double centerX = 0.0, double centerY = 0.0) {
        auto profile = std::make_shared<Sketching::Spline>();
        
        double halfSize = size / 2.0;
        // Create square control points (5 points to close the square)
        profile->addControlPoint(centerX - halfSize, centerY - halfSize);
        profile->addControlPoint(centerX + halfSize, centerY - halfSize);
        profile->addControlPoint(centerX + halfSize, centerY + halfSize);
        profile->addControlPoint(centerX - halfSize, centerY + halfSize);
        profile->addControlPoint(centerX - halfSize, centerY - halfSize);
        
        return profile;
    }

    std::unique_ptr<LoftTool> tool;
};

TEST_F(LoftToolTests, InitialState) {
    EXPECT_EQ(tool->GetProfileCount(), 0);
    EXPECT_EQ(tool->GetGuideCurveCount(), 0);
}

TEST_F(LoftToolTests, AddValidProfile) {
    auto profile = CreateCircleProfile(1.0);
    auto result = tool->AddProfile(profile);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_EQ(tool->GetProfileCount(), 1);
}

TEST_F(LoftToolTests, AddNullProfile) {
    auto result = tool->AddProfile(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::GeometryError);
    EXPECT_EQ(tool->GetProfileCount(), 0);
}

TEST_F(LoftToolTests, AddGuideCurve) {
    auto guide = std::make_shared<Sketching::Spline>();
    guide->addControlPoint(0.0, 0.0);
    guide->addControlPoint(1.0, 1.0);
    
    auto result = tool->AddGuideCurve(guide);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_EQ(tool->GetGuideCurveCount(), 1);
}

TEST_F(LoftToolTests, Reset) {
    auto profile = CreateCircleProfile(1.0);
    auto guide = std::make_shared<Sketching::Spline>();
    
    tool->AddProfile(profile);
    tool->AddGuideCurve(guide);
    EXPECT_EQ(tool->GetProfileCount(), 1);
    EXPECT_EQ(tool->GetGuideCurveCount(), 1);
    
    tool->Reset();
    EXPECT_EQ(tool->GetProfileCount(), 0);
    EXPECT_EQ(tool->GetGuideCurveCount(), 0);
}

TEST_F(LoftToolTests, ExecuteWithInsufficientProfiles) {
    auto profile = CreateCircleProfile(1.0);
    tool->AddProfile(profile);
    
    LoftTool::LoftOptions options;
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(LoftToolTests, ExecuteWithValidProfiles) {
    // Add two circles of different sizes
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0, 0.0, 0.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Ruled;
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, ExecuteWithDifferentProfileTypes) {
    // Add a circle and a square
    auto circle = CreateCircleProfile(1.0);
    auto square = CreateSquareProfile(2.0);
    
    tool->AddProfile(circle);
    tool->AddProfile(square);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Ruled;
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

// Mock GraphicsSystem for testing
class MockGraphicsSystem : public Graphics::GraphicsSystem {
public:
    MockGraphicsSystem() 
        : Graphics::GraphicsSystem()
        , previewStarted(false)
        , previewEnded(false)
        , meshRendered(false)
        , lastVertexCount(0) {}

    void setRenderMode(Graphics::GraphicsSystem::RenderMode mode) override { lastMode = mode; }
    void setColor(const Graphics::Color& color) override { lastColor = color; }
    void beginPreview() override { previewStarted = true; }
    void endPreview() override { previewEnded = true; }
    void renderTriangleMesh(const float* vertices, size_t vertexCount) override {
        lastVertexCount = vertexCount;
        meshRendered = true;
    }

    Graphics::GraphicsSystem::RenderMode lastMode;
    Graphics::Color lastColor;
    bool previewStarted;
    bool previewEnded;
    bool meshRendered;
    size_t lastVertexCount;
};

TEST_F(LoftToolTests, PreviewWithoutProfiles) {
    auto graphics = std::make_shared<MockGraphicsSystem>();
    auto result = tool->Preview(graphics);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
    EXPECT_FALSE(graphics->previewStarted);
}

TEST_F(LoftToolTests, PreviewWithValidProfiles) {
    auto graphics = std::make_shared<MockGraphicsSystem>();
    
    // Add two profiles
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    // Configure and execute
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    tool->Execute(options);
    
    // Test preview
    auto result = tool->Preview(graphics);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_TRUE(graphics->previewStarted);
    EXPECT_TRUE(graphics->meshRendered);
    EXPECT_TRUE(graphics->previewEnded);
    EXPECT_EQ(graphics->lastMode, Graphics::GraphicsSystem::RenderMode::Preview);
    EXPECT_GT(graphics->lastVertexCount, 0);
}

TEST_F(LoftToolTests, PreviewWithNullGraphics) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    auto result = tool->Preview(nullptr);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::CADError);
}

TEST_F(LoftToolTests, CancelOperation) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    tool->Execute(options);
    tool->Cancel();
    
    // After canceling, executing again should still work
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, ClosedLoftOperation) {
    // Add three profiles for a closed loft
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0, 0.0, 0.0);
    auto profile3 = CreateCircleProfile(1.5, 0.0, 0.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    tool->AddProfile(profile3);
    
    LoftTool::LoftOptions options;
    options.closed = true;
    options.transitionType = LoftTool::TransitionType::Ruled;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, SmoothTransitionWithoutGuides) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    options.startCondition = LoftTool::TangencyCondition::Natural;
    options.endCondition = LoftTool::TangencyCondition::Natural;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, SmoothTransitionWithGuide) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    // Add a guide curve
    auto guide = std::make_shared<Sketching::Spline>();
    guide->addControlPoint(0.0, 0.0);
    guide->addControlPoint(0.5, 0.5);
    guide->addControlPoint(1.0, 1.0);
    tool->AddGuideCurve(guide);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_EQ(tool->GetGuideCurveCount(), 1);
}

TEST_F(LoftToolTests, NormalTangencyCondition) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    options.startCondition = LoftTool::TangencyCondition::Normal;
    options.endCondition = LoftTool::TangencyCondition::Normal;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, CustomTangencyCondition) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    
    // Create custom tangent vectors
    std::vector<LoftTool::TangentVector> startTangents;
    std::vector<LoftTool::TangentVector> endTangents;
    
    // Add tangent vectors for each profile point (12 points per circle)
    for (int i = 0; i <= 12; ++i) {
        double angle = (2.0 * M_PI * i) / 12;
        // Create tangent vectors perpendicular to circle at each point
        startTangents.emplace_back(-std::sin(angle), std::cos(angle), 1.0f);
        endTangents.emplace_back(-std::sin(angle), std::cos(angle), 1.0f);
    }
    
    // Set custom tangents
    options.SetStartTangents(startTangents);
    options.SetEndTangents(endTangents);
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_EQ(options.startCondition, LoftTool::TangencyCondition::Custom);
    EXPECT_EQ(options.endCondition, LoftTool::TangencyCondition::Custom);
    EXPECT_EQ(options.startTangents.size(), startTangents.size());
    EXPECT_EQ(options.endTangents.size(), endTangents.size());
}

TEST_F(LoftToolTests, CustomTangencyValidation) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    
    // Set only start tangents (should still work)
    std::vector<LoftTool::TangentVector> startTangents;
    for (int i = 0; i <= 12; ++i) {
        double angle = (2.0 * M_PI * i) / 12;
        startTangents.emplace_back(-std::sin(angle), std::cos(angle), 1.0f);
    }
    options.SetStartTangents(startTangents);
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
    EXPECT_EQ(options.startCondition, LoftTool::TangencyCondition::Custom);
    EXPECT_EQ(options.endCondition, LoftTool::TangencyCondition::Natural);
}

TEST_F(LoftToolTests, MultipleProfilesSmooth) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    auto profile3 = CreateCircleProfile(1.5);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    tool->AddProfile(profile3);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    options.closed = true;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}

TEST_F(LoftToolTests, EdgeCaseEmptyGuide) {
    auto profile1 = CreateCircleProfile(1.0);
    auto profile2 = CreateCircleProfile(2.0);
    
    tool->AddProfile(profile1);
    tool->AddProfile(profile2);
    
    // Add an empty guide curve
    auto guide = std::make_shared<Sketching::Spline>();
    tool->AddGuideCurve(guide);
    
    LoftTool::LoftOptions options;
    options.transitionType = LoftTool::TransitionType::Smooth;
    
    auto result = tool->Execute(options);
    EXPECT_EQ(result.code(), rebel::core::ErrorCode::None);
}
