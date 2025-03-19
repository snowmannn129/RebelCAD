#include "constraints/RadiusConstraint.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>
#include <limits>

using namespace RebelCAD::Constraints;
using namespace RebelCAD::Sketching;

class RadiusConstraintTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test shapes
        circle = std::make_shared<Circle>(0.0f, 0.0f, 1.0f);
        arc = std::make_shared<Arc>(0.0f, 0.0f, 1.0f, 0.0f, M_PI);
    }

    std::shared_ptr<Circle> circle;
    std::shared_ptr<Arc> arc;
    static constexpr double kTolerance = 1e-6;
};

TEST_F(RadiusConstraintTests, CircleConstruction) {
    // Valid construction
    EXPECT_NO_THROW(RadiusConstraint(circle, 2.0));

    // Null circle
    std::shared_ptr<Circle> nullCircle;
    EXPECT_THROW(RadiusConstraint(nullCircle, 1.0), std::invalid_argument);

    // Invalid radius
    EXPECT_THROW(RadiusConstraint(circle, 0.0), std::invalid_argument);
    EXPECT_THROW(RadiusConstraint(circle, -1.0), std::invalid_argument);
}

TEST_F(RadiusConstraintTests, ArcConstruction) {
    // Valid construction
    EXPECT_NO_THROW(RadiusConstraint(arc, 2.0));

    // Null arc
    std::shared_ptr<Arc> nullArc;
    EXPECT_THROW(RadiusConstraint(nullArc, 1.0), std::invalid_argument);

    // Invalid radius
    EXPECT_THROW(RadiusConstraint(arc, 0.0), std::invalid_argument);
    EXPECT_THROW(RadiusConstraint(arc, -1.0), std::invalid_argument);
}

TEST_F(RadiusConstraintTests, CircleConstraintEnforcement) {
    RadiusConstraint constraint(circle, 2.0);

    // Initially not satisfied
    EXPECT_FALSE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getError(), 1.0, kTolerance);

    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(circle->getRadius(), 2.0, kTolerance);

    // Center should remain unchanged
    auto center = circle->getCenter();
    EXPECT_NEAR(center[0], 0.0f, kTolerance);
    EXPECT_NEAR(center[1], 0.0f, kTolerance);
}

TEST_F(RadiusConstraintTests, ArcConstraintEnforcement) {
    RadiusConstraint constraint(arc, 2.0);

    // Initially not satisfied
    EXPECT_FALSE(constraint.isSatisfied());
    EXPECT_NEAR(constraint.getError(), 1.0, kTolerance);

    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    EXPECT_NEAR(arc->getRadius(), 2.0, kTolerance);

    // Center and angles should remain unchanged
    auto center = arc->getCenter();
    EXPECT_NEAR(center[0], 0.0f, kTolerance);
    EXPECT_NEAR(center[1], 0.0f, kTolerance);
    EXPECT_NEAR(arc->getStartAngle(), 0.0f, kTolerance);
    EXPECT_NEAR(arc->getEndAngle(), M_PI, kTolerance);
}

TEST_F(RadiusConstraintTests, EdgeCases) {
    // Test with very small radius
    {
        RadiusConstraint constraint(circle, 1e-5);
        EXPECT_TRUE(constraint.enforce());
        EXPECT_NEAR(circle->getRadius(), 1e-5, kTolerance);
    }

    // Test with very large radius
    {
        RadiusConstraint constraint(circle, 1e5);
        EXPECT_TRUE(constraint.enforce());
        EXPECT_NEAR(circle->getRadius(), 1e5, kTolerance);
    }

    // Test with infinite radius in shape
    circle->setRadius(std::numeric_limits<float>::infinity());
    RadiusConstraint constraint(circle, 1.0);
    EXPECT_FALSE(constraint.isValid());
    EXPECT_FALSE(constraint.enforce());
}

TEST_F(RadiusConstraintTests, Validation) {
    RadiusConstraint constraint(circle, 1.0);

    // Initially valid
    EXPECT_TRUE(constraint.isValid());

    // Test with null shape
    circle.reset();
    EXPECT_FALSE(constraint.isValid());
    EXPECT_EQ(constraint.getError(), std::numeric_limits<double>::infinity());
}

TEST_F(RadiusConstraintTests, RadiusAccessors) {
    const double targetRadius = 2.5;
    RadiusConstraint constraint(circle, targetRadius);

    // Test target radius getter
    EXPECT_NEAR(constraint.getTargetRadius(), targetRadius, kTolerance);

    // Test error calculation to verify current radius indirectly
    EXPECT_NEAR(constraint.getError(), std::abs(targetRadius - 1.0), kTolerance);
    constraint.enforce();
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);
}

TEST_F(RadiusConstraintTests, ErrorCalculation) {
    RadiusConstraint constraint(circle, 3.0);

    // Initial error should be |3.0 - 1.0| = 2.0
    EXPECT_NEAR(constraint.getError(), 2.0, kTolerance);

    // After enforcement, error should be near zero
    constraint.enforce();
    EXPECT_NEAR(constraint.getError(), 0.0, kTolerance);

    // Set radius to something else and check error
    circle->setRadius(2.0);
    EXPECT_NEAR(constraint.getError(), 1.0, kTolerance);
}
