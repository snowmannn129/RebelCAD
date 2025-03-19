#include "constraints/LengthConstraint.h"
#include <gtest/gtest.h>
#include <cmath>

using namespace RebelCAD::Constraints;

TEST(LengthConstraintTests, Constructor) {
    // Valid construction
    EXPECT_NO_THROW(LengthConstraint({0, 0}, {3, 4}, 5.0));
    
    // Invalid target length
    EXPECT_THROW(LengthConstraint({0, 0}, {1, 1}, 0.0), std::invalid_argument);
    EXPECT_THROW(LengthConstraint({0, 0}, {1, 1}, -1.0), std::invalid_argument);
}

TEST(LengthConstraintTests, IsSatisfied) {
    // Create a 3-4-5 triangle line
    LengthConstraint constraint({0, 0}, {3, 4}, 5.0);
    EXPECT_TRUE(constraint.isSatisfied());
    
    // Test with different tolerance
    LengthConstraint precise_constraint({0, 0}, {3, 4}, 5.0, 1e-10);
    EXPECT_TRUE(precise_constraint.isSatisfied());
    
    // Test unsatisfied case
    LengthConstraint wrong_length({0, 0}, {3, 4}, 6.0, 1e-6);
    EXPECT_FALSE(wrong_length.isSatisfied());
}

TEST(LengthConstraintTests, Enforce) {
    // Test enforcing length on horizontal line
    LengthConstraint horizontal({0, 0}, {2, 0}, 4.0);
    EXPECT_TRUE(horizontal.enforce());
    EXPECT_NEAR(horizontal.getCurrentLength(), 4.0, 1e-6);
    
    // Test enforcing length on vertical line
    LengthConstraint vertical({0, 0}, {0, 3}, 6.0);
    EXPECT_TRUE(vertical.enforce());
    EXPECT_NEAR(vertical.getCurrentLength(), 6.0, 1e-6);
    
    // Test enforcing length on diagonal line
    LengthConstraint diagonal({0, 0}, {1, 1}, 5.0);
    EXPECT_TRUE(diagonal.enforce());
    EXPECT_NEAR(diagonal.getCurrentLength(), 5.0, 1e-6);
    
    // Test enforcing length on degenerate line
    LengthConstraint degenerate({1, 1}, {1, 1}, 2.0);
    EXPECT_TRUE(degenerate.enforce());
    EXPECT_NEAR(degenerate.getCurrentLength(), 2.0, 1e-6);
}

TEST(LengthConstraintTests, GetError) {
    // Test error calculation
    LengthConstraint constraint({0, 0}, {3, 0}, 4.0);
    EXPECT_NEAR(constraint.getError(), 1.0, 1e-6);
    
    // Test zero error case
    LengthConstraint perfect({0, 0}, {3, 4}, 5.0);
    EXPECT_NEAR(perfect.getError(), 0.0, 1e-6);
}

TEST(LengthConstraintTests, SetMethods) {
    LengthConstraint constraint({0, 0}, {3, 4}, 5.0);
    
    // Test setTargetLength
    EXPECT_NO_THROW(constraint.setTargetLength(6.0));
    EXPECT_THROW(constraint.setTargetLength(0.0), std::invalid_argument);
    EXPECT_THROW(constraint.setTargetLength(-1.0), std::invalid_argument);
    
    // Test setStartPoint and setEndPoint
    constraint.setStartPoint({1, 1});
    constraint.setEndPoint({4, 5});
    EXPECT_NEAR(constraint.getCurrentLength(), 5.0, 1e-6);
}

TEST(LengthConstraintTests, CacheInvalidation) {
    LengthConstraint constraint({0, 0}, {3, 4}, 5.0);
    
    // Initial length should be cached
    double initial_length = constraint.getCurrentLength();
    EXPECT_NEAR(initial_length, 5.0, 1e-6);
    
    // Modify end point and verify cache is invalidated
    constraint.setEndPoint({6, 8});
    double new_length = constraint.getCurrentLength();
    EXPECT_NEAR(new_length, 10.0, 1e-6);
}
