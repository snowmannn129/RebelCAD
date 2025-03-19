#include "sketching/ExtendTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace RebelCAD::Sketching;

class ExtendToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<ExtendTool>();
    }

    std::unique_ptr<ExtendTool> tool;
    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }
};

TEST_F(ExtendToolTests, LineExtension) {
    // Create a horizontal line from (0,0) to (1,0)
    Line line(0.0f, 0.0f, 1.0f, 0.0f);

    // Extend from end point to (2,0)
    auto extended = tool->extendLine(line, 2.0f, 0.0f, false);
    EXPECT_TRUE(pointsEqual(extended.getStartPoint(), {0.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(extended.getEndPoint(), {2.0f, 0.0f}));

    // Extend from start point to (-1,0)
    extended = tool->extendLine(line, -1.0f, 0.0f, true);
    EXPECT_TRUE(pointsEqual(extended.getStartPoint(), {-1.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(extended.getEndPoint(), {1.0f, 0.0f}));
}

TEST_F(ExtendToolTests, CurveExtension) {
    // Create a simple spline
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0},
        {1.0, 1.0},
        {2.0, 1.0},
        {3.0, 0.0}
    };
    Spline spline(points);

    // Extend from end point
    auto extended = tool->extendCurve(spline, 4.0f, 0.0f, false);
    auto endPoint = extended.getControlPoints().back();
    EXPECT_NEAR(endPoint.first, 4.0, kTolerance);
    EXPECT_NEAR(endPoint.second, 0.0, kTolerance);

    // Extend from start point
    extended = tool->extendCurve(spline, -1.0f, 0.0f, true);
    auto startPoint = extended.getControlPoints().front();
    EXPECT_NEAR(startPoint.first, -1.0, kTolerance);
    EXPECT_NEAR(startPoint.second, 0.0, kTolerance);
}

TEST_F(ExtendToolTests, PolygonEdgeExtension) {
    // Create a square
    std::vector<std::array<float, 2>> vertices = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
    Polygon polygon(vertices);

    // Extend bottom edge from start
    auto extended = tool->extendPolygonEdge(polygon, 0, -1.0f, 0.0f, true);
    EXPECT_TRUE(pointsEqual(extended.getVertices()[0], {-1.0f, 0.0f}));

    // Extend right edge from end
    extended = tool->extendPolygonEdge(polygon, 1, 1.0f, 2.0f, false);
    EXPECT_TRUE(pointsEqual(extended.getVertices()[2], {1.0f, 2.0f}));
}

TEST_F(ExtendToolTests, FindExtensionPoints) {
    // Create a horizontal line to extend
    Line line(0.0f, 0.0f, 1.0f, 0.0f);

    // Create potential target elements
    std::vector<std::shared_ptr<Line>> targets;
    targets.push_back(std::make_shared<Line>(2.0f, -1.0f, 2.0f, 1.0f)); // Vertical line
    targets.push_back(std::make_shared<Line>(1.5f, 0.0f, 2.5f, 0.0f)); // Horizontal line

    // Find extension points
    auto points = tool->findExtensionPoints(line, targets);
    ASSERT_EQ(points.size(), 2);

    // Check first extension point (to vertical line)
    EXPECT_TRUE(pointsEqual({points[0].x, points[0].y}, {2.0f, 0.0f}));
    EXPECT_EQ(points[0].targetElementIndex, 0);
    EXPECT_FALSE(points[0].isStart);

    // Check second extension point (to horizontal line)
    EXPECT_TRUE(pointsEqual({points[1].x, points[1].y}, {1.5f, 0.0f}));
    EXPECT_EQ(points[1].targetElementIndex, 1);
    EXPECT_FALSE(points[1].isStart);
}

TEST_F(ExtendToolTests, FindNearestExtensionPoint) {
    // Create potential target elements
    std::vector<std::shared_ptr<Line>> targets;
    targets.push_back(std::make_shared<Line>(1.0f, 0.0f, 2.0f, 0.0f));
    targets.push_back(std::make_shared<Line>(3.0f, 0.0f, 4.0f, 0.0f));

    // Find nearest point to (1.1, 0.1)
    auto point = tool->findNearestExtensionPoint(1.1f, 0.1f, targets);
    ASSERT_TRUE(point != nullptr);
    EXPECT_TRUE(pointsEqual({point->x, point->y}, {1.0f, 0.0f}));
    EXPECT_EQ(point->targetElementIndex, 0);
    EXPECT_TRUE(point->isStart);
}

TEST_F(ExtendToolTests, InvalidExtensionPoints) {
    Line line(0.0f, 0.0f, 1.0f, 0.0f);

    // Test with infinite coordinates
    EXPECT_THROW(
        tool->extendLine(line, std::numeric_limits<float>::infinity(), 0.0f, false),
        RebelCAD::Core::Error
    );

    // Test with NaN coordinates
    EXPECT_THROW(
        tool->extendLine(line, std::numeric_limits<float>::quiet_NaN(), 0.0f, false),
        RebelCAD::Core::Error
    );
}

TEST_F(ExtendToolTests, PolygonEdgeValidation) {
    std::vector<std::array<float, 2>> vertices = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f}
    };
    Polygon polygon(vertices);

    // Test with invalid edge index
    EXPECT_THROW(
        tool->extendPolygonEdge(polygon, 3, 2.0f, 0.0f, false),
        RebelCAD::Core::Error
    );
}

TEST_F(ExtendToolTests, ExtensionDistanceCalculation) {
    // Create a horizontal line
    Line line(0.0f, 0.0f, 2.0f, 0.0f);

    // Find extension points to a vertical line
    std::vector<std::shared_ptr<Line>> targets;
    targets.push_back(std::make_shared<Line>(3.0f, -1.0f, 3.0f, 1.0f));

    auto points = tool->findExtensionPoints(line, targets);
    ASSERT_EQ(points.size(), 1);
    EXPECT_NEAR(points[0].extensionDistance, 1.0f, kTolerance);
}
