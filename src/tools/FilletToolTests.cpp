#include "sketching/FilletTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

using namespace RebelCAD::Sketching;

class FilletToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<FilletTool>();
    }

    std::unique_ptr<FilletTool> tool;
    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }

    // Helper function to verify arc tangency to line
    static bool isArcTangentToLine(const Arc& arc,
                                 const Line& line,
                                 float tolerance = kTolerance) {
        // Get arc center and radius
        auto center = arc.getCenter();
        float radius = arc.getRadius();

        // Calculate perpendicular distance from arc center to line
        auto start = line.getStartPoint();
        auto end = line.getEndPoint();
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float len = std::sqrt(dx * dx + dy * dy);
        dx /= len;
        dy /= len;

        float dist = std::abs((center[1] - start[1]) * dx -
                            (center[0] - start[0]) * dy);

        return std::abs(dist - radius) < tolerance;
    }

    // Helper function to verify arc tangency to another arc
    static bool isArcTangentToArc(const Arc& arc1,
                                const Arc& arc2,
                                float tolerance = kTolerance) {
        // Get arc centers and radii
        auto center1 = arc1.getCenter();
        auto center2 = arc2.getCenter();
        float r1 = arc1.getRadius();
        float r2 = arc2.getRadius();

        // Calculate distance between centers
        float dx = center2[0] - center1[0];
        float dy = center2[1] - center1[1];
        float dist = std::sqrt(dx * dx + dy * dy);

        // Distance should equal sum or difference of radii
        return std::abs(dist - (r1 + r2)) < tolerance ||
               std::abs(dist - std::abs(r1 - r2)) < tolerance;
    }
};

TEST_F(FilletToolTests, LineLineFilletCreation) {
    // Create two perpendicular lines
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 2.0f, 0.0f);  // Horizontal
    auto line2 = std::make_shared<Line>(1.0f, -1.0f, 1.0f, 1.0f); // Vertical

    // Create fillet
    float radius = 0.5f;
    auto fillet = tool->createLineLine(line1, line2, radius);

    // Verify fillet properties
    EXPECT_NEAR(fillet->getRadius(), radius, kTolerance);
    EXPECT_TRUE(isArcTangentToLine(*fillet, *line1));
    EXPECT_TRUE(isArcTangentToLine(*fillet, *line2));

    // Test with parallel lines
    auto line3 = std::make_shared<Line>(0.0f, 1.0f, 2.0f, 1.0f);
    EXPECT_THROW(
        tool->createLineLine(line1, line3, radius),
        RebelCAD::Core::Error
    );
}

TEST_F(FilletToolTests, LineArcFilletCreation) {
    // Create a horizontal line and a circle
    auto line = std::make_shared<Line>(0.0f, 1.0f, 2.0f, 1.0f);
    auto arc = std::make_shared<Arc>(1.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));

    // Create fillet
    float radius = 0.25f;
    auto fillet = tool->createLineArc(line, arc, radius);

    // Verify fillet properties
    EXPECT_NEAR(fillet->getRadius(), radius, kTolerance);
    EXPECT_TRUE(isArcTangentToLine(*fillet, *line));
    EXPECT_TRUE(isArcTangentToArc(*fillet, *arc));

    // Test with line too far from arc
    auto farLine = std::make_shared<Line>(0.0f, 3.0f, 2.0f, 3.0f);
    EXPECT_THROW(
        tool->createLineArc(farLine, arc, radius),
        RebelCAD::Core::Error
    );
}

TEST_F(FilletToolTests, ArcArcFilletCreation) {
    // Create two circles
    auto arc1 = std::make_shared<Arc>(0.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));
    auto arc2 = std::make_shared<Arc>(2.0f, 0.0f, 0.5f, 0.0f, static_cast<float>(M_PI));

    // Create fillet
    float radius = 0.25f;
    auto fillet = tool->createArcArc(arc1, arc2, radius);

    // Verify fillet properties
    EXPECT_NEAR(fillet->getRadius(), radius, kTolerance);
    EXPECT_TRUE(isArcTangentToArc(*fillet, *arc1));
    EXPECT_TRUE(isArcTangentToArc(*fillet, *arc2));

    // Test with arcs too far apart
    auto farArc = std::make_shared<Arc>(5.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));
    EXPECT_THROW(
        tool->createArcArc(arc1, farArc, radius),
        RebelCAD::Core::Error
    );
}

TEST_F(FilletToolTests, InvalidRadius) {
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 0.0f);
    auto line2 = std::make_shared<Line>(1.0f, 0.0f, 1.0f, 1.0f);

    // Test with zero radius
    EXPECT_THROW(
        tool->createLineLine(line1, line2, 0.0),
        RebelCAD::Core::Error
    );

    // Test with negative radius
    EXPECT_THROW(
        tool->createLineLine(line1, line2, -1.0),
        RebelCAD::Core::Error
    );
}

TEST_F(FilletToolTests, FilletAnglePreservation) {
    // Create two lines at 45 degrees
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 1.0f);
    auto line2 = std::make_shared<Line>(0.0f, 1.0f, 1.0f, 0.0f);

    float radius = 0.5f;
    auto fillet = tool->createLineLine(line1, line2, radius);

    // Verify fillet sweep angle is 90 degrees
    float sweepAngle = fillet->getEndAngle() - fillet->getStartAngle();
    if (sweepAngle < 0) sweepAngle += 2.0f * static_cast<float>(M_PI);
    EXPECT_NEAR(sweepAngle, static_cast<float>(M_PI) / 2.0f, kTolerance);
}

TEST_F(FilletToolTests, FilletRadiusLimits) {
    // Create two perpendicular lines
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 2.0f, 0.0f);
    auto line2 = std::make_shared<Line>(1.0f, -1.0f, 1.0f, 1.0f);

    // Test with very small radius
    EXPECT_NO_THROW(tool->createLineLine(line1, line2, 1e-5));

    // Test with radius larger than distance to intersection
    float largeRadius = 2.0f;
    auto fillet = tool->createLineLine(line1, line2, largeRadius);
    EXPECT_NEAR(fillet->getRadius(), largeRadius, kTolerance);
}
