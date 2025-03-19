#include "constraints/ParallelConstraint.h"
#include <gtest/gtest.h>
#include <cmath>
#include <array>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace RebelCAD::Constraints;

class ParallelConstraintTests : public ::testing::Test {
protected:
    static constexpr double kTolerance = 1e-6;

    // Helper function to compare points
    static bool pointsEqual(const std::array<double, 2>& p1,
                          const std::array<double, 2>& p2,
                          double tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }

    // Helper function to create parallel lines
    static std::pair<std::array<std::array<double, 2>, 2>,
                    std::array<std::array<double, 2>, 2>>
    createParallelLines() {
        return {
            {{{0.0, 0.0}, {1.0, 0.0}}},  // Horizontal line
            {{{0.0, 1.0}, {1.0, 1.0}}}   // Parallel line above
        };
    }

    // Helper function to create non-parallel lines
    static std::pair<std::array<std::array<double, 2>, 2>,
                    std::array<std::array<double, 2>, 2>>
    createNonParallelLines() {
        return {
            {{{0.0, 0.0}, {1.0, 0.0}}},  // Horizontal line
            {{{0.0, 0.0}, {1.0, 1.0}}}   // 45-degree line
        };
    }
};

TEST_F(ParallelConstraintTests, Construction) {
    auto [line1, line2] = createParallelLines();

    // Valid construction
    EXPECT_NO_THROW(ParallelConstraint(line1[0], line1[1], line2[0], line2[1]));

    // Invalid tolerance
    EXPECT_THROW(
        ParallelConstraint(line1[0], line1[1], line2[0], line2[1], 0.0),
        std::invalid_argument
    );
    EXPECT_THROW(
        ParallelConstraint(line1[0], line1[1], line2[0], line2[1], -1.0),
        std::invalid_argument
    );
}

TEST_F(ParallelConstraintTests, ParallelDetection) {
    // Test horizontal parallel lines
    {
        auto [line1, line2] = createParallelLines();
        ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);
        EXPECT_TRUE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
    }

    // Test vertical parallel lines
    {
        std::array<std::array<double, 2>, 2> line1 = {{{0.0, 0.0}, {0.0, 1.0}}};
        std::array<std::array<double, 2>, 2> line2 = {{{1.0, 0.0}, {1.0, 1.0}}};
        ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);
        EXPECT_TRUE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
    }

    // Test non-parallel lines
    {
        auto [line1, line2] = createNonParallelLines();
        ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);
        EXPECT_FALSE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), M_PI/4.0, kTolerance); // 45 degrees
    }
}

TEST_F(ParallelConstraintTests, ConstraintEnforcement) {
    auto [line1, line2] = createNonParallelLines();
    ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);

    // Initially not parallel
    EXPECT_FALSE(constraint.isSatisfied());

    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());

    // Verify second line is now horizontal
    auto end = constraint.getLine2End();
    EXPECT_NEAR(end[1], constraint.getLine2Start()[1], kTolerance);
}

TEST_F(ParallelConstraintTests, EdgeCases) {
    // Test perpendicular lines
    {
        std::array<std::array<double, 2>, 2> line1 = {{{0.0, 0.0}, {1.0, 0.0}}};
        std::array<std::array<double, 2>, 2> line2 = {{{0.0, 0.0}, {0.0, 1.0}}};
        ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);
        EXPECT_FALSE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), M_PI/2.0, kTolerance);
    }

    // Test coincident lines
    {
        std::array<std::array<double, 2>, 2> line = {{{0.0, 0.0}, {1.0, 0.0}}};
        ParallelConstraint constraint(line[0], line[1], line[0], line[1]);
        EXPECT_TRUE(constraint.isSatisfied());
        EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
    }

    // Test degenerate line (zero length)
    {
        std::array<std::array<double, 2>, 2> line1 = {{{0.0, 0.0}, {1.0, 0.0}}};
        std::array<std::array<double, 2>, 2> line2 = {{{0.0, 0.0}, {0.0, 0.0}}};
        ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);
        EXPECT_FALSE(constraint.isValid());
        EXPECT_FALSE(constraint.enforce());
    }
}

TEST_F(ParallelConstraintTests, AngleNormalization) {
    // Test lines with same direction but different orientations
    std::array<std::array<double, 2>, 2> line1 = {{{0.0, 0.0}, {1.0, 0.0}}};
    std::array<std::array<double, 2>, 2> line2 = {{{0.0, 1.0}, {-1.0, 1.0}}};
    ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);

    // Lines should be considered parallel despite opposite directions
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
}

TEST_F(ParallelConstraintTests, LengthPreservation) {
    auto [line1, line2] = createNonParallelLines();
    ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);

    // Calculate initial length
    double dx = line2[1][0] - line2[0][0];
    double dy = line2[1][1] - line2[0][1];
    double initialLength = std::sqrt(dx * dx + dy * dy);

    // Enforce constraint
    constraint.enforce();

    // Calculate new length
    auto end = constraint.getLine2End();
    auto start = constraint.getLine2Start();
    dx = end[0] - start[0];
    dy = end[1] - start[1];
    double newLength = std::sqrt(dx * dx + dy * dy);

    // Length should be preserved
    EXPECT_NEAR(newLength, initialLength, kTolerance);
}

TEST_F(ParallelConstraintTests, Validation) {
    auto [line1, line2] = createParallelLines();
    ParallelConstraint constraint(line1[0], line1[1], line2[0], line2[1]);

    // Initially valid
    EXPECT_TRUE(constraint.isValid());

    // Test with infinite coordinates
    std::array<std::array<double, 2>, 2> invalidLine = {
        {{std::numeric_limits<double>::infinity(), 0.0}, {1.0, 0.0}}
    };
    ParallelConstraint invalidConstraint(
        invalidLine[0], invalidLine[1], line2[0], line2[1]
    );
    EXPECT_FALSE(invalidConstraint.isValid());
}
