#include "sketching/ChamferTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

using namespace RebelCAD::Sketching;

class ChamferToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        tool = std::make_unique<ChamferTool>();
    }

    std::unique_ptr<ChamferTool> tool;
    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }

    // Helper function to verify chamfer length
    static bool verifyChamferLength(const Line& chamfer,
                                  float expectedLength,
                                  float tolerance = kTolerance) {
        auto start = chamfer.getStartPoint();
        auto end = chamfer.getEndPoint();
        float dx = end[0] - start[0];
        float dy = end[1] - start[1];
        float length = std::sqrt(dx * dx + dy * dy);
        return std::abs(length - expectedLength) < tolerance;
    }
};

TEST_F(ChamferToolTests, LineLineChamferCreation) {
    // Create two perpendicular lines
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 2.0f, 0.0f);  // Horizontal
    auto line2 = std::make_shared<Line>(1.0f, -1.0f, 1.0f, 1.0f); // Vertical

    // Create chamfer with equal distances
    float distance = 0.5f;
    auto chamfer = tool->createLineLine(line1, line2, distance, distance);

    // Verify chamfer endpoints
    auto start = chamfer->getStartPoint();
    auto end = chamfer->getEndPoint();
    EXPECT_TRUE(pointsEqual(start, {1.0f + distance, 0.0f}));
    EXPECT_TRUE(pointsEqual(end, {1.0f, distance}));

    // Verify chamfer length
    float expectedLength = distance * std::sqrt(2.0f);
    EXPECT_TRUE(verifyChamferLength(*chamfer, expectedLength));

    // Test with parallel lines
    auto line3 = std::make_shared<Line>(0.0f, 1.0f, 2.0f, 1.0f);
    EXPECT_THROW(
        tool->createLineLine(line1, line3, distance, distance),
        RebelCAD::Core::Error
    );
}

TEST_F(ChamferToolTests, LineArcChamferCreation) {
    // Create a horizontal line and a circle
    auto line = std::make_shared<Line>(0.0f, 1.0f, 2.0f, 1.0f);
    auto arc = std::make_shared<Arc>(1.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));

    // Create chamfer
    float lineDistance = 0.5f;
    float arcDistance = 0.5f;
    auto chamfer = tool->createLineArc(line, arc, lineDistance, arcDistance);

    // Verify chamfer endpoints are at correct distances
    auto start = chamfer->getStartPoint();
    auto end = chamfer->getEndPoint();
    
    // Point on line should be at lineDistance from intersection
    EXPECT_NEAR(start[1], 1.0f, kTolerance); // Should be on the line
    
    // Point on arc should be at arcDistance along arc from intersection
    auto arcCenter = arc->getCenter();
    float dx = end[0] - arcCenter[0];
    float dy = end[1] - arcCenter[1];
    float radius = std::sqrt(dx * dx + dy * dy);
    EXPECT_NEAR(radius, arc->getRadius(), kTolerance);

    // Test with line too far from arc
    auto farLine = std::make_shared<Line>(0.0f, 3.0f, 2.0f, 3.0f);
    EXPECT_THROW(
        tool->createLineArc(farLine, arc, lineDistance, arcDistance),
        RebelCAD::Core::Error
    );
}

TEST_F(ChamferToolTests, ArcArcChamferCreation) {
    // Create two intersecting circles
    auto arc1 = std::make_shared<Arc>(0.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));
    auto arc2 = std::make_shared<Arc>(1.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));

    // Create chamfer
    float distance = 0.5f;
    auto chamfer = tool->createArcArc(arc1, arc2, distance, distance);

    // Verify chamfer endpoints are at correct distances from arc centers
    auto start = chamfer->getStartPoint();
    auto end = chamfer->getEndPoint();
    
    auto center1 = arc1->getCenter();
    auto center2 = arc2->getCenter();
    
    // Calculate distances from endpoints to respective arc centers
    float dist1 = std::hypot(start[0] - center1[0], start[1] - center1[1]);
    float dist2 = std::hypot(end[0] - center2[0], end[1] - center2[1]);
    
    EXPECT_NEAR(dist1, arc1->getRadius(), kTolerance);
    EXPECT_NEAR(dist2, arc2->getRadius(), kTolerance);

    // Test with non-intersecting arcs
    auto farArc = std::make_shared<Arc>(3.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI));
    EXPECT_THROW(
        tool->createArcArc(arc1, farArc, distance, distance),
        RebelCAD::Core::Error
    );
}

TEST_F(ChamferToolTests, InvalidDistances) {
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 0.0f);
    auto line2 = std::make_shared<Line>(1.0f, 0.0f, 1.0f, 1.0f);

    // Test with zero distance
    EXPECT_THROW(
        tool->createLineLine(line1, line2, 0.0, 1.0),
        RebelCAD::Core::Error
    );

    // Test with negative distance
    EXPECT_THROW(
        tool->createLineLine(line1, line2, -1.0, 1.0),
        RebelCAD::Core::Error
    );
}

TEST_F(ChamferToolTests, ChamferAnglePreservation) {
    // Create two lines at 45 degrees
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 1.0f);
    auto line2 = std::make_shared<Line>(0.0f, 1.0f, 1.0f, 0.0f);

    float distance = 0.5f;
    auto chamfer = tool->createLineLine(line1, line2, distance, distance);

    // Verify chamfer angle is 45 degrees
    auto start = chamfer->getStartPoint();
    auto end = chamfer->getEndPoint();
    float dx = end[0] - start[0];
    float dy = end[1] - start[1];
    float angle = std::abs(std::atan2(dy, dx));
    EXPECT_NEAR(angle, M_PI / 4.0f, kTolerance);
}

TEST_F(ChamferToolTests, ChamferDistanceLimits) {
    // Create two perpendicular lines
    auto line1 = std::make_shared<Line>(0.0f, 0.0f, 2.0f, 0.0f);
    auto line2 = std::make_shared<Line>(1.0f, -1.0f, 1.0f, 1.0f);

    // Test with very small distance
    EXPECT_NO_THROW(tool->createLineLine(line1, line2, 1e-5, 1e-5));

    // Test with large distance
    float largeDistance = 2.0f;
    auto chamfer = tool->createLineLine(line1, line2, largeDistance, largeDistance);
    EXPECT_TRUE(verifyChamferLength(*chamfer, largeDistance * std::sqrt(2.0f)));
}
