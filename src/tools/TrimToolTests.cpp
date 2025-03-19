#include "sketching/TrimTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace RebelCAD::Sketching;

class TrimToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<TrimTool>();
    }

    std::unique_ptr<TrimTool> tool;
    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }
};

TEST_F(TrimToolTests, LineIntersections) {
    // Create intersecting lines
    Line line1(0.0f, 0.0f, 2.0f, 2.0f);  // 45-degree line
    Line line2(0.0f, 2.0f, 2.0f, 0.0f);  // -45-degree line

    // Find intersections
    auto points = tool->findLineIntersections(line1, line2);
    ASSERT_EQ(points.size(), 1);

    // Check intersection point
    EXPECT_NEAR(points[0].x, 1.0f, kTolerance);
    EXPECT_NEAR(points[0].y, 1.0f, kTolerance);
    EXPECT_NEAR(points[0].parameter1, 0.5f, kTolerance);
    EXPECT_NEAR(points[0].parameter2, 0.5f, kTolerance);

    // Test parallel lines
    Line line3(0.0f, 0.0f, 2.0f, 0.0f);  // Horizontal line
    Line line4(0.0f, 1.0f, 2.0f, 1.0f);  // Parallel horizontal line
    points = tool->findLineIntersections(line3, line4);
    EXPECT_TRUE(points.empty());
}

TEST_F(TrimToolTests, LineCurveIntersections) {
    // Create a line and spline that intersect
    Line line(0.0f, 1.0f, 2.0f, 1.0f);  // Horizontal line at y=1
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0},
        {1.0, 2.0},
        {2.0, 0.0}
    };
    Spline spline(points);  // Parabolic curve crossing y=1 twice

    // Find intersections
    auto intersections = tool->findLineCurveIntersections(line, spline);
    ASSERT_EQ(intersections.size(), 2);

    // Sort intersections by x coordinate
    if (intersections[0].x > intersections[1].x) {
        std::swap(intersections[0], intersections[1]);
    }

    // Check intersection points
    EXPECT_NEAR(intersections[0].x, 0.5f, 0.1f);  // Approximate due to sampling
    EXPECT_NEAR(intersections[0].y, 1.0f, kTolerance);
    EXPECT_NEAR(intersections[1].x, 1.5f, 0.1f);
    EXPECT_NEAR(intersections[1].y, 1.0f, kTolerance);
}

TEST_F(TrimToolTests, LineTrimming) {
    // Create a line from (0,0) to (2,2)
    Line line(0.0f, 0.0f, 2.0f, 2.0f);

    // Trim from start, keeping end portion
    auto trimmed = tool->trimLine(line, 1.0f, 1.0f, false);
    EXPECT_TRUE(pointsEqual(trimmed.getStartPoint(), {1.0f, 1.0f}));
    EXPECT_TRUE(pointsEqual(trimmed.getEndPoint(), {2.0f, 2.0f}));

    // Trim from end, keeping start portion
    trimmed = tool->trimLine(line, 1.0f, 1.0f, true);
    EXPECT_TRUE(pointsEqual(trimmed.getStartPoint(), {0.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(trimmed.getEndPoint(), {1.0f, 1.0f}));

    // Test with point not on line
    EXPECT_THROW(
        tool->trimLine(line, 1.0f, 0.0f, true),
        RebelCAD::Core::Error
    );
}

TEST_F(TrimToolTests, CurveTrimming) {
    // Create a simple spline
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0},
        {1.0, 1.0},
        {2.0, 0.0}
    };
    Spline spline(points);

    // Trim at midpoint
    auto trimmed = tool->trimCurve(spline, 1.0f, 1.0f, true);
    auto trimPoints = trimmed.getControlPoints();
    EXPECT_EQ(trimPoints.size(), 2);
    EXPECT_NEAR(trimPoints.front().first, 0.0, kTolerance);
    EXPECT_NEAR(trimPoints.front().second, 0.0, kTolerance);
    EXPECT_NEAR(trimPoints.back().first, 1.0, kTolerance);
    EXPECT_NEAR(trimPoints.back().second, 1.0, kTolerance);
}

TEST_F(TrimToolTests, PolygonEdgeTrimming) {
    // Create a square
    std::vector<std::array<float, 2>> vertices = {
        {0.0f, 0.0f},
        {2.0f, 0.0f},
        {2.0f, 2.0f},
        {0.0f, 2.0f}
    };
    Polygon polygon(vertices);

    // Trim bottom edge at midpoint
    auto trimmed = tool->trimPolygonEdge(polygon, 0, 1.0f, 0.0f, true);
    auto trimVertices = trimmed.getVertices();
    EXPECT_TRUE(pointsEqual(trimVertices[0], {0.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(trimVertices[1], {1.0f, 0.0f}));

    // Test with invalid edge index
    EXPECT_THROW(
        tool->trimPolygonEdge(polygon, 4, 1.0f, 0.0f, true),
        RebelCAD::Core::Error
    );
}

TEST_F(TrimToolTests, InvalidTrimPoints) {
    Line line(0.0f, 0.0f, 2.0f, 2.0f);

    // Test with infinite coordinates
    EXPECT_THROW(
        tool->trimLine(line, std::numeric_limits<float>::infinity(), 0.0f, true),
        RebelCAD::Core::Error
    );

    // Test with NaN coordinates
    EXPECT_THROW(
        tool->trimLine(line, std::numeric_limits<float>::quiet_NaN(), 0.0f, true),
        RebelCAD::Core::Error
    );
}

TEST_F(TrimToolTests, TrimValidation) {
    Line line(0.0f, 0.0f, 2.0f, 2.0f);

    // Test point on line (should succeed)
    EXPECT_NO_THROW(tool->trimLine(line, 1.0f, 1.0f, true));

    // Test point off line (should throw)
    EXPECT_THROW(
        tool->trimLine(line, 1.0f, 0.0f, true),
        RebelCAD::Core::Error
    );

    // Test endpoint (should succeed)
    EXPECT_NO_THROW(tool->trimLine(line, 0.0f, 0.0f, true));
}

TEST_F(TrimToolTests, TrimParameterization) {
    Line line(0.0f, 0.0f, 2.0f, 2.0f);

    // Test trimming at different points along the line
    auto start = tool->trimLine(line, 0.0f, 0.0f, true);
    EXPECT_TRUE(pointsEqual(start.getEndPoint(), {0.0f, 0.0f}));

    auto mid = tool->trimLine(line, 1.0f, 1.0f, true);
    EXPECT_TRUE(pointsEqual(mid.getEndPoint(), {1.0f, 1.0f}));

    auto end = tool->trimLine(line, 2.0f, 2.0f, true);
    EXPECT_TRUE(pointsEqual(end.getEndPoint(), {2.0f, 2.0f}));
}
