#include "constraints/AngleConstraint.h"
#include "sketching/Line.h"
#include <gtest/gtest.h>
#include <cmath>
#include <memory>

using namespace RebelCAD::Constraints;
using namespace RebelCAD::Sketching;

class AngleConstraintTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create two perpendicular lines for testing
        line1 = std::make_shared<Line>(0.0f, 0.0f, 1.0f, 0.0f); // Horizontal
        line2 = std::make_shared<Line>(0.0f, 0.0f, 0.0f, 1.0f); // Vertical
    }

    std::shared_ptr<Line> line1;
    std::shared_ptr<Line> line2;

    static constexpr double kTolerance = 1e-6;
};

TEST_F(AngleConstraintTests, Construction) {
    // Valid construction
    EXPECT_NO_THROW(AngleConstraint(line1, line2, M_PI / 2.0));
    
    // Invalid lines
    EXPECT_THROW(AngleConstraint(nullptr, line2, M_PI / 2.0), std::invalid_argument);
    EXPECT_THROW(AngleConstraint(line1, nullptr, M_PI / 2.0), std::invalid_argument);
    
    // Invalid tolerance
    EXPECT_THROW(AngleConstraint(line1, line2, M_PI / 2.0, 0.0), std::invalid_argument);
    EXPECT_THROW(AngleConstraint(line1, line2, M_PI / 2.0, -1.0), std::invalid_argument);
}

TEST_F(AngleConstraintTests, AngleCalculation) {
    AngleConstraint constraint(line1, line2, M_PI / 2.0);
    
    // Initial angle should be 90 degrees (π/2)
    EXPECT_NEAR(constraint.getCurrentAngle(), M_PI / 2.0, kTolerance);
    
    // Rotate second line by 45 degrees
    line2->rotate(M_PI / 4.0);
    EXPECT_NEAR(constraint.getCurrentAngle(), M_PI / 4.0, kTolerance);
    
    // Rotate second line to 180 degrees
    line2->rotate(3.0 * M_PI / 4.0);
    EXPECT_NEAR(constraint.getCurrentAngle(), M_PI, kTolerance);
}

TEST_F(AngleConstraintTests, AngleNormalization) {
    AngleConstraint constraint(line1, line2, M_PI / 2.0);
    
    // Test setting angles beyond 2π
    constraint.setTargetAngle(5.0 * M_PI / 2.0); // Should normalize to π/2
    EXPECT_NEAR(constraint.getTargetAngle(), M_PI / 2.0, kTolerance);
    
    // Test setting negative angles
    constraint.setTargetAngle(-3.0 * M_PI / 2.0); // Should normalize to π/2
    EXPECT_NEAR(constraint.getTargetAngle(), M_PI / 2.0, kTolerance);
}

TEST_F(AngleConstraintTests, ConstraintEnforcement) {
    // Create constraint for 45 degrees
    AngleConstraint constraint(line1, line2, M_PI / 4.0);
    
    // Initially at 90 degrees
    EXPECT_FALSE(constraint.isSatisfied());
    
    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getCurrentAngle(), M_PI / 4.0, kTolerance);
    
    // Test with degenerate lines
    auto degenerateLine = std::make_shared<Line>(0.0f, 0.0f, 0.0f, 0.0f);
    AngleConstraint degenerateConstraint(line1, degenerateLine, M_PI / 4.0);
    EXPECT_FALSE(degenerateConstraint.enforce());
}

TEST_F(AngleConstraintTests, ErrorCalculation) {
    AngleConstraint constraint(line1, line2, M_PI / 4.0); // Target 45 degrees
    
    // Initially at 90 degrees, error should be 45 degrees
    EXPECT_NEAR(constraint.getError(), M_PI / 4.0, kTolerance);
    
    // After enforcement, error should be near zero
    constraint.enforce();
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
    
    // Test wraparound case (e.g., between 350° and 10°)
    line2->rotate(23.0 * M_PI / 12.0); // Rotate to 345 degrees
    constraint.setTargetAngle(M_PI / 18.0); // Target 10 degrees
    EXPECT_NEAR(constraint.getError(), M_PI / 9.0, kTolerance); // Error should be 20 degrees
}

TEST_F(AngleConstraintTests, Validation) {
    AngleConstraint constraint(line1, line2, M_PI / 2.0);
    
    // Initially valid
    EXPECT_TRUE(constraint.isValid());
    
    // Test with zero-length line
    line2->setEndPoint(0.0f, 0.0f);
    EXPECT_FALSE(constraint.isValid());
}

TEST_F(AngleConstraintTests, CacheInvalidation) {
    AngleConstraint constraint(line1, line2, M_PI / 2.0);
    
    // Initial angle calculation
    double initialAngle = constraint.getCurrentAngle();
    EXPECT_NEAR(initialAngle, M_PI / 2.0, kTolerance);
    
    // Rotate line and verify cache is invalidated
    line2->rotate(M_PI / 4.0);
    double newAngle = constraint.getCurrentAngle();
    EXPECT_NEAR(newAngle, M_PI / 4.0, kTolerance);
    
    // Change target and verify error cache is invalidated
    constraint.setTargetAngle(M_PI / 3.0);
    EXPECT_NEAR(constraint.getError(), std::abs(M_PI / 4.0 - M_PI / 3.0), kTolerance);
}

TEST_F(AngleConstraintTests, SmallestRotation) {
    AngleConstraint constraint(line1, line2, 3.0 * M_PI / 2.0); // Target 270 degrees
    
    // Line should rotate counterclockwise by -90 degrees rather than clockwise by 270
    constraint.enforce();
    auto end = line2->getEndPoint();
    EXPECT_NEAR(end[0], 0.0f, kTolerance);
    EXPECT_NEAR(end[1], -1.0f, kTolerance);
}
