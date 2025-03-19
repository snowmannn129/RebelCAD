#include "sketching/MirrorTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

using namespace RebelCAD::Sketching;

class MirrorToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<MirrorTool>();
        // Set up vertical mirror line at x=1
        tool->setMirrorLine(1.0, -1.0, 1.0, 1.0);
    }

    std::unique_ptr<MirrorTool> tool;
    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }

    // Helper function to compare points
    static bool pointsEqual(const std::pair<double, double>& p1,
                          const std::pair<double, double>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1.first - p2.first) <= tolerance &&
               std::abs(p1.second - p2.second) <= tolerance;
    }
};

TEST_F(MirrorToolTests, MirrorLineSetup) {
    MirrorTool localTool;

    // Test setting mirror line from points
    EXPECT_NO_THROW(localTool.setMirrorLine(0.0, 0.0, 1.0, 1.0));

    // Test with coincident points
    EXPECT_THROW(
        localTool.setMirrorLine(1.0, 1.0, 1.0, 1.0),
        RebelCAD::Core::Error
    );

    // Test setting mirror line from Line object
    auto line = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 1.0f);
    EXPECT_NO_THROW(localTool.setMirrorLine(line));

    // Test with null line
    EXPECT_THROW(
        localTool.setMirrorLine(nullptr),
        RebelCAD::Core::Error
    );
}

TEST_F(MirrorToolTests, PointMirroring) {
    // Test point mirroring about vertical line at x=1
    auto mirrored = tool->mirrorPoint(0.0, 0.0);
    EXPECT_TRUE(pointsEqual(mirrored, {2.0, 0.0}));

    // Test point on mirror line
    mirrored = tool->mirrorPoint(1.0, 0.0);
    EXPECT_TRUE(pointsEqual(mirrored, {1.0, 0.0}));

    // Test point with non-zero y coordinate
    mirrored = tool->mirrorPoint(0.0, 1.0);
    EXPECT_TRUE(pointsEqual(mirrored, {2.0, 1.0}));
}

TEST_F(MirrorToolTests, LineMirroring) {
    // Create a vertical line
    auto line = std::make_shared<Line>(0.0f, -1.0f, 0.0f, 1.0f);

    // Mirror the line
    auto mirrored = tool->mirrorLine(line);

    // Verify mirrored line endpoints
    EXPECT_TRUE(pointsEqual(mirrored->getStartPoint(), {2.0f, -1.0f}));
    EXPECT_TRUE(pointsEqual(mirrored->getEndPoint(), {2.0f, 1.0f}));

    // Test with null line
    EXPECT_THROW(
        tool->mirrorLine(nullptr),
        RebelCAD::Core::Error
    );
}

TEST_F(MirrorToolTests, ArcMirroring) {
    // Create a quarter arc
    auto arc = std::make_shared<Arc>(0.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI/2));

    // Mirror the arc
    auto mirrored = tool->mirrorArc(arc);

    // Verify mirrored arc properties
    auto center = mirrored->getCenter();
    EXPECT_TRUE(pointsEqual(std::make_pair(center[0], center[1]), std::make_pair(2.0, 0.0)));
    EXPECT_NEAR(mirrored->getRadius(), 1.0f, kTolerance);

    // Verify angles are mirrored correctly
    float startAngle = mirrored->getStartAngle();
    float endAngle = mirrored->getEndAngle();
    while (startAngle < 0) startAngle += 2.0f * static_cast<float>(M_PI);
    while (endAngle < 0) endAngle += 2.0f * static_cast<float>(M_PI);

    EXPECT_NEAR(startAngle, static_cast<float>(M_PI/2), kTolerance);
    EXPECT_NEAR(endAngle, static_cast<float>(M_PI), kTolerance);
}

TEST_F(MirrorToolTests, CircleMirroring) {
    // Create a circle
    auto circle = std::make_shared<Circle>(0.0f, 0.0f, 1.0f);

    // Mirror the circle
    auto mirrored = tool->mirrorCircle(circle);

    // Verify mirrored circle properties
    auto center = mirrored->getCenter();
    EXPECT_TRUE(pointsEqual(std::make_pair(center[0], center[1]), std::make_pair(2.0, 0.0)));
    EXPECT_NEAR(mirrored->getRadius(), 1.0f, kTolerance);
}

TEST_F(MirrorToolTests, BezierCurveMirroring) {
    // Create a bezier curve
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 1.0}
    };
    auto curve = std::make_shared<BezierCurve>(points);

    // Mirror the curve
    auto mirrored = tool->mirrorBezierCurve(curve);

    // Verify mirrored control points
    auto mirroredPoints = mirrored->getControlPoints();
    EXPECT_EQ(mirroredPoints.size(), 3);
    EXPECT_TRUE(pointsEqual(mirroredPoints[0], {1.0, 1.0}));
    EXPECT_TRUE(pointsEqual(mirroredPoints[1], {2.0, 1.0}));
    EXPECT_TRUE(pointsEqual(mirroredPoints[2], {2.0, 0.0}));
}

TEST_F(MirrorToolTests, SplineMirroring) {
    // Create a spline
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0},
        {0.0, 1.0},
        {1.0, 1.0}
    };
    auto spline = std::make_shared<Spline>(points);

    // Mirror the spline
    auto mirrored = tool->mirrorSpline(spline);

    // Verify mirrored control points
    auto mirroredPoints = mirrored->getControlPoints();
    EXPECT_EQ(mirroredPoints.size(), 3);
    EXPECT_TRUE(pointsEqual(mirroredPoints[0], {1.0, 1.0}));
    EXPECT_TRUE(pointsEqual(mirroredPoints[1], {2.0, 1.0}));
    EXPECT_TRUE(pointsEqual(mirroredPoints[2], {2.0, 0.0}));
}

TEST_F(MirrorToolTests, MirrorLineValidation) {
    MirrorTool localTool;

    // Try to mirror point without setting mirror line
    EXPECT_THROW(
        localTool.mirrorPoint(0.0, 0.0),
        RebelCAD::Core::Error
    );

    // Try to mirror line without setting mirror line
    auto line = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 1.0f);
    EXPECT_THROW(
        localTool.mirrorLine(line),
        RebelCAD::Core::Error
    );
}

TEST_F(MirrorToolTests, MirrorAboutAngledLine) {
    // Set up 45-degree mirror line
    tool->setMirrorLine(0.0, 0.0, 1.0, 1.0);

    // Mirror a point
    auto mirrored = tool->mirrorPoint(1.0, 0.0);
    EXPECT_TRUE(pointsEqual(mirrored, {0.0, 1.0}));

    // Mirror a horizontal line
    auto line = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 0.0f);
    auto mirroredLine = tool->mirrorLine(line);
    EXPECT_TRUE(pointsEqual(mirroredLine->getStartPoint(), {0.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(mirroredLine->getEndPoint(), {0.0f, 1.0f}));
}
