#include "sketching/OffsetTool.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

using namespace RebelCAD::Sketching;

class OffsetToolTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up default configuration
        OffsetTool::Config config;
        config.setGridCellSize(1.0f)
              .setTolerance(1e-6f)
              .setSpatialPartitioningThreshold(100);
        OffsetTool::setConfig(config);
    }

    static constexpr float kTolerance = 1e-6f;

    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1,
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }
};

TEST_F(OffsetToolTests, LineOffset) {
    // Create a horizontal line
    Line line(0.0f, 0.0f, 2.0f, 0.0f);

    // Offset upward
    auto offset = OffsetTool::offsetLine(line, 1.0f);
    EXPECT_TRUE(pointsEqual(offset->getStartPoint(), {0.0f, 1.0f}));
    EXPECT_TRUE(pointsEqual(offset->getEndPoint(), {2.0f, 1.0f}));

    // Offset downward
    offset = OffsetTool::offsetLine(line, -1.0f);
    EXPECT_TRUE(pointsEqual(offset->getStartPoint(), {0.0f, -1.0f}));
    EXPECT_TRUE(pointsEqual(offset->getEndPoint(), {2.0f, -1.0f}));

    // Test with zero-length line
    Line zeroLine(1.0f, 1.0f, 1.0f, 1.0f);
    EXPECT_THROW(
        OffsetTool::offsetLine(zeroLine, 1.0f),
        RebelCAD::Core::Error
    );
}

TEST_F(OffsetToolTests, ArcOffset) {
    // Create a quarter arc
    Arc arc(0.0f, 0.0f, 1.0f, 0.0f, static_cast<float>(M_PI/2));

    // Offset outward
    auto offset = OffsetTool::offsetArc(arc, 1.0f);
    EXPECT_TRUE(pointsEqual(offset->getCenter(), {0.0f, 0.0f}));
    EXPECT_NEAR(offset->getRadius(), 2.0f, kTolerance);
    EXPECT_NEAR(offset->getStartAngle(), 0.0f, kTolerance);
    EXPECT_NEAR(offset->getEndAngle(), static_cast<float>(M_PI/2), kTolerance);

    // Offset inward
    offset = OffsetTool::offsetArc(arc, -0.5f);
    EXPECT_TRUE(pointsEqual(offset->getCenter(), {0.0f, 0.0f}));
    EXPECT_NEAR(offset->getRadius(), 0.5f, kTolerance);

    // Test with invalid inward offset
    EXPECT_THROW(
        OffsetTool::offsetArc(arc, -1.0f),
        RebelCAD::Core::Error
    );
}

TEST_F(OffsetToolTests, CircleOffset) {
    // Create a circle
    Circle circle(0.0f, 0.0f, 1.0f);

    // Offset outward
    auto offset = OffsetTool::offsetCircle(circle, 1.0f);
    EXPECT_TRUE(pointsEqual(offset->getCenter(), {0.0f, 0.0f}));
    EXPECT_NEAR(offset->getRadius(), 2.0f, kTolerance);

    // Offset inward
    offset = OffsetTool::offsetCircle(circle, -0.5f);
    EXPECT_NEAR(offset->getRadius(), 0.5f, kTolerance);

    // Test with invalid inward offset
    EXPECT_THROW(
        OffsetTool::offsetCircle(circle, -1.0f),
        RebelCAD::Core::Error
    );
}

TEST_F(OffsetToolTests, ConnectedLineOffset) {
    // Create two connected lines forming a right angle
    std::vector<Line> lines = {
        Line(0.0f, 0.0f, 2.0f, 0.0f),
        Line(2.0f, 0.0f, 2.0f, 2.0f)
    };

    // Offset outward
    auto offsets = OffsetTool::offsetElements(lines, 1.0f);
    ASSERT_EQ(offsets.size(), 2);

    // Verify first line
    EXPECT_TRUE(pointsEqual(offsets[0]->getStartPoint(), {0.0f, 1.0f}));
    EXPECT_TRUE(pointsEqual(offsets[0]->getEndPoint(), {2.0f, 1.0f}));

    // Verify second line
    EXPECT_TRUE(pointsEqual(offsets[1]->getStartPoint(), {3.0f, 0.0f}));
    EXPECT_TRUE(pointsEqual(offsets[1]->getEndPoint(), {3.0f, 2.0f}));
}

TEST_F(OffsetToolTests, VariableDistanceOffset) {
    // Create a line
    std::vector<Line> lines = {
        Line(0.0f, 0.0f, 2.0f, 0.0f)
    };

    // Offset with varying distance
    auto offsets = OffsetTool::offsetElementsWithVariableDistance(
        lines, 0.5f, 1.0f
    );
    ASSERT_EQ(offsets.size(), 1);

    // Verify start and end points
    EXPECT_TRUE(pointsEqual(offsets[0]->getStartPoint(), {0.0f, 0.5f}));
    EXPECT_TRUE(pointsEqual(offsets[0]->getEndPoint(), {2.0f, 1.0f}));

    // Test with invalid position
    EXPECT_THROW(
        OffsetTool::interpolateOffset(0.0f, 1.0f, 1.5f),
        RebelCAD::Core::Error
    );
}

TEST_F(OffsetToolTests, SelfIntersectionDetection) {
    // Create a U-shaped path
    std::vector<Line> lines = {
        Line(0.0f, 0.0f, 0.0f, 2.0f),
        Line(0.0f, 2.0f, 2.0f, 2.0f),
        Line(2.0f, 2.0f, 2.0f, 0.0f)
    };

    // Offset inward - should create self-intersection
    EXPECT_THROW(
        OffsetTool::offsetElements(lines, -1.5f),
        RebelCAD::Core::Error
    );
}

TEST_F(OffsetToolTests, ConfigurationSettings) {
    OffsetTool::Config config;
    
    // Test grid cell size validation
    EXPECT_THROW(
        config.setGridCellSize(0.0f),
        RebelCAD::Core::Error
    );

    // Test tolerance validation
    EXPECT_THROW(
        config.setTolerance(0.0f),
        RebelCAD::Core::Error
    );

    // Test valid settings
    EXPECT_NO_THROW(config.setGridCellSize(2.0f));
    EXPECT_NO_THROW(config.setTolerance(1e-5f));
    EXPECT_NO_THROW(config.setSpatialPartitioningThreshold(50));
}

TEST_F(OffsetToolTests, EmptyInputHandling) {
    std::vector<Line> empty;

    // Test empty input for regular offset
    auto result1 = OffsetTool::offsetElements(empty, 1.0f);
    EXPECT_TRUE(result1.empty());

    // Test empty input for variable offset
    auto result2 = OffsetTool::offsetElementsWithVariableDistance(empty, 1.0f, 2.0f);
    EXPECT_TRUE(result2.empty());
}

TEST_F(OffsetToolTests, InvalidOffsetValidation) {
    Line line(0.0f, 0.0f, 1.0f, 0.0f);

    // Test with NaN
    EXPECT_THROW(
        OffsetTool::offsetLine(line, std::numeric_limits<float>::quiet_NaN()),
        RebelCAD::Core::Error
    );

    // Test with infinity
    EXPECT_THROW(
        OffsetTool::offsetLine(line, std::numeric_limits<float>::infinity()),
        RebelCAD::Core::Error
    );
}
