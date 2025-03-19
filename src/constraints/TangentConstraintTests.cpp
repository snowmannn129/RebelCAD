#include "constraints/TangentConstraint.h"
#include <gtest/gtest.h>
#include <cmath>
#include <array>
#include <limits>

using namespace RebelCAD::Constraints;

class TangentConstraintTests : public ::testing::Test {
protected:
    static constexpr double kTolerance = 1e-6;

    // Helper function to compare points
    static bool pointsEqual(const std::array<double, 2>& p1,
                          const std::array<double, 2>& p2,
                          double tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }

    // Helper function to create a line-circle tangent case
    static std::tuple<std::array<double, 2>,  // line start
                     std::array<double, 2>,  // line end
                     std::array<double, 2>,  // circle center
                     double>                 // circle radius
    createLineTangentCase() {
        return {
            std::array<double, 2>{{0.0, 1.0}},  // Line start
            std::array<double, 2>{{2.0, 1.0}},  // Line end (horizontal line)
            std::array<double, 2>{{1.0, 0.0}},  // Circle center (below line)
            1.0                                  // Circle radius (touches line)
        };
    }

    // Helper function to create a circle-circle tangent case
    static std::tuple<std::array<double, 2>,  // circle1 center
                     double,                   // circle1 radius
                     std::array<double, 2>,    // circle2 center
                     double>                   // circle2 radius
    createCircleTangentCase() {
        return {
            std::array<double, 2>{{0.0, 0.0}},  // Circle1 center
            1.0,                                 // Circle1 radius
            std::array<double, 2>{{3.0, 0.0}},  // Circle2 center
            2.0                                  // Circle2 radius
        };
    }
};

TEST_F(TangentConstraintTests, LineCircleConstruction) {
    auto [line_start, line_end, circle_center, radius] = createLineTangentCase();

    // Valid construction
    EXPECT_NO_THROW(TangentConstraint(line_start, line_end, circle_center, radius));

    // Invalid radius
    EXPECT_THROW(
        TangentConstraint(line_start, line_end, circle_center, 0.0),
        std::invalid_argument
    );
    EXPECT_THROW(
        TangentConstraint(line_start, line_end, circle_center, -1.0),
        std::invalid_argument
    );

    // Invalid tolerance
    EXPECT_THROW(
        TangentConstraint(line_start, line_end, circle_center, radius, 0.0),
        std::invalid_argument
    );

    // Degenerate line
    EXPECT_THROW(
        TangentConstraint(line_start, line_start, circle_center, radius),
        std::invalid_argument
    );
}

TEST_F(TangentConstraintTests, CircleCircleConstruction) {
    auto [center1, radius1, center2, radius2] = createCircleTangentCase();

    // Valid construction
    EXPECT_NO_THROW(TangentConstraint(center1, radius1, center2, radius2));

    // Invalid radii
    EXPECT_THROW(
        TangentConstraint(center1, 0.0, center2, radius2),
        std::invalid_argument
    );
    EXPECT_THROW(
        TangentConstraint(center1, radius1, center2, -1.0),
        std::invalid_argument
    );

    // Invalid tolerance
    EXPECT_THROW(
        TangentConstraint(center1, radius1, center2, radius2, 0.0),
        std::invalid_argument
    );

    // Coincident circles
    EXPECT_THROW(
        TangentConstraint(center1, radius1, center1, radius2),
        std::invalid_argument
    );
}

TEST_F(TangentConstraintTests, LineCircleTangency) {
    auto [line_start, line_end, circle_center, radius] = createLineTangentCase();
    TangentConstraint constraint(line_start, line_end, circle_center, radius);

    // Initially tangent
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);

    // Move line up
    line_start[1] += 0.5;
    line_end[1] += 0.5;
    TangentConstraint nonTangent(line_start, line_end, circle_center, radius);

    // Should not be tangent
    EXPECT_FALSE(nonTangent.isSatisfied());
    EXPECT_NEAR(nonTangent.getError(), 0.5, kTolerance);

    // Enforce tangency
    EXPECT_TRUE(nonTangent.enforce());
    EXPECT_TRUE(nonTangent.isSatisfied());
}

TEST_F(TangentConstraintTests, CircleCircleTangency) {
    auto [center1, radius1, center2, radius2] = createCircleTangentCase();
    TangentConstraint constraint(center1, radius1, center2, radius2);

    // Initially tangent (centers 3 units apart, radii sum to 3)
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);

    // Move second circle away
    center2[0] += 1.0;
    TangentConstraint nonTangent(center1, radius1, center2, radius2);

    // Should not be tangent
    EXPECT_FALSE(nonTangent.isSatisfied());
    EXPECT_NEAR(nonTangent.getError(), 1.0, kTolerance);

    // Enforce tangency
    EXPECT_TRUE(nonTangent.enforce());
    EXPECT_TRUE(nonTangent.isSatisfied());
}

TEST_F(TangentConstraintTests, TangencyVerification) {
    // Line-Circle case
    {
        auto [line_start, line_end, circle_center, radius] = createLineTangentCase();
        TangentConstraint constraint(line_start, line_end, circle_center, radius);

        // Should be tangent at (1.0, 1.0)
        EXPECT_TRUE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);

        // Move line slightly and verify it's no longer tangent
        line_start[1] += 0.1;
        line_end[1] += 0.1;
        TangentConstraint movedConstraint(line_start, line_end, circle_center, radius);
        EXPECT_FALSE(movedConstraint.isSatisfied());
        EXPECT_NEAR(movedConstraint.getError(), 0.1, kTolerance);
    }

    // Circle-Circle case
    {
        auto [center1, radius1, center2, radius2] = createCircleTangentCase();
        TangentConstraint constraint(center1, radius1, center2, radius2);

        // Should be tangent at (1.0, 0.0)
        EXPECT_TRUE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);

        // Move second circle slightly and verify it's no longer tangent
        center2[0] += 0.1;
        TangentConstraint movedConstraint(center1, radius1, center2, radius2);
        EXPECT_FALSE(movedConstraint.isSatisfied());
        EXPECT_NEAR(movedConstraint.getError(), 0.1, kTolerance);
    }
}

TEST_F(TangentConstraintTests, EdgeCases) {
    auto [line_start, line_end, circle_center, radius] = createLineTangentCase();

    // Very small radius
    {
        TangentConstraint constraint(line_start, line_end, circle_center, 1e-5);
        EXPECT_TRUE(constraint.enforce());
        EXPECT_TRUE(constraint.isSatisfied());
    }

    // Very large radius
    {
        TangentConstraint constraint(line_start, line_end, circle_center, 1e5);
        EXPECT_TRUE(constraint.enforce());
        EXPECT_TRUE(constraint.isSatisfied());
    }

    // Nearly parallel line to circle center
    {
        std::array<double, 2> almostParallelEnd = {2.0, 1.001};
        TangentConstraint constraint(line_start, almostParallelEnd, circle_center, radius);
        EXPECT_TRUE(constraint.enforce());
        EXPECT_TRUE(constraint.isSatisfied());
    }
}

TEST_F(TangentConstraintTests, Validation) {
    auto [line_start, line_end, circle_center, radius] = createLineTangentCase();
    TangentConstraint constraint(line_start, line_end, circle_center, radius);

    // Initially valid
    EXPECT_TRUE(constraint.isValid());

    // Test with infinite radius
    TangentConstraint invalidConstraint(
        line_start, line_end, circle_center,
        std::numeric_limits<double>::infinity()
    );
    EXPECT_FALSE(invalidConstraint.isValid());
    EXPECT_EQ(invalidConstraint.getError(), std::numeric_limits<double>::infinity());
}
