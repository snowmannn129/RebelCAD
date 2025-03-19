#include "constraints/ConcentricConstraint.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include "core/Error.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>
#include <limits>

using namespace RebelCAD::Constraints;
using namespace RebelCAD::Sketching;

class ConcentricConstraintTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test entities
        circle1 = std::make_shared<Circle>(0.0f, 0.0f, 1.0f);
        circle2 = std::make_shared<Circle>(1.0f, 1.0f, 2.0f);
        arc1 = std::make_shared<Arc>(0.0f, 0.0f, 1.0f, 0.0f, M_PI);
        arc2 = std::make_shared<Arc>(1.0f, 1.0f, 2.0f, M_PI/2.0f, 3.0f*M_PI/2.0f);
    }

    std::shared_ptr<Circle> circle1;
    std::shared_ptr<Circle> circle2;
    std::shared_ptr<Arc> arc1;
    std::shared_ptr<Arc> arc2;

    static constexpr float kTolerance = 1e-5f;

    // Helper function to check if points are equal within tolerance
    static bool pointsEqual(const std::array<float, 2>& p1, 
                          const std::array<float, 2>& p2,
                          float tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance && 
               std::abs(p1[1] - p2[1]) <= tolerance;
    }
};

TEST_F(ConcentricConstraintTests, Construction) {
    // Test valid constructions
    EXPECT_NO_THROW(ConcentricConstraint(circle1, circle2));
    EXPECT_NO_THROW(ConcentricConstraint(circle1, arc1));
    EXPECT_NO_THROW(ConcentricConstraint(arc1, arc2));
    
    // Test null entities
    std::shared_ptr<Circle> nullCircle;
    std::shared_ptr<Arc> nullArc;
    EXPECT_THROW(ConcentricConstraint(nullCircle, circle2), RebelCAD::Core::Error);
    EXPECT_THROW(ConcentricConstraint(circle1, nullCircle), RebelCAD::Core::Error);
    EXPECT_THROW(ConcentricConstraint(nullArc, arc2), RebelCAD::Core::Error);
    EXPECT_THROW(ConcentricConstraint(arc1, nullArc), RebelCAD::Core::Error);
}

TEST_F(ConcentricConstraintTests, CircleCircleConstraint) {
    ConcentricConstraint constraint(circle1, circle2);
    
    // Initially not satisfied
    EXPECT_FALSE(constraint.isSatisfied());
    
    // Check initial error
    float expectedError = std::sqrt(2.0f); // Distance between (0,0) and (1,1)
    EXPECT_NEAR(constraint.getError(), expectedError, kTolerance);
    
    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    
    // Centers should now be at midpoint (0.5, 0.5)
    auto center1 = circle1->getCenter();
    auto center2 = circle2->getCenter();
    EXPECT_TRUE(pointsEqual(center1, center2));
    EXPECT_NEAR(center1[0], 0.5f, kTolerance);
    EXPECT_NEAR(center1[1], 0.5f, kTolerance);
    
    // Radii should remain unchanged
    EXPECT_NEAR(circle1->getRadius(), 1.0f, kTolerance);
    EXPECT_NEAR(circle2->getRadius(), 2.0f, kTolerance);
}

TEST_F(ConcentricConstraintTests, CircleArcConstraint) {
    ConcentricConstraint constraint(circle1, arc2);
    
    // Initially not satisfied
    EXPECT_FALSE(constraint.isSatisfied());
    
    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    
    // Centers should be coincident
    auto circleCenter = circle1->getCenter();
    auto arcCenter = arc2->getCenter();
    EXPECT_TRUE(pointsEqual(circleCenter, arcCenter));
    
    // Arc angles should remain unchanged
    EXPECT_NEAR(arc2->getStartAngle(), M_PI/2.0f, kTolerance);
    EXPECT_NEAR(arc2->getEndAngle(), 3.0f*M_PI/2.0f, kTolerance);
}

TEST_F(ConcentricConstraintTests, ArcArcConstraint) {
    ConcentricConstraint constraint(arc1, arc2);
    
    // Initially not satisfied
    EXPECT_FALSE(constraint.isSatisfied());
    
    // Enforce constraint
    EXPECT_TRUE(constraint.enforce());
    EXPECT_TRUE(constraint.isSatisfied());
    
    // Centers should be coincident
    auto center1 = arc1->getCenter();
    auto center2 = arc2->getCenter();
    EXPECT_TRUE(pointsEqual(center1, center2));
    
    // Arc angles should remain unchanged
    EXPECT_NEAR(arc1->getStartAngle(), 0.0f, kTolerance);
    EXPECT_NEAR(arc1->getEndAngle(), M_PI, kTolerance);
    EXPECT_NEAR(arc2->getStartAngle(), M_PI/2.0f, kTolerance);
    EXPECT_NEAR(arc2->getEndAngle(), 3.0f*M_PI/2.0f, kTolerance);
}

TEST_F(ConcentricConstraintTests, Validation) {
    ConcentricConstraint constraint(circle1, circle2);
    
    // Initially valid
    EXPECT_TRUE(constraint.isValid());
    
    // Test with invalid entities
    circle1.reset();
    EXPECT_FALSE(constraint.isValid());
    EXPECT_FALSE(constraint.enforce());
    EXPECT_EQ(constraint.getError(), std::numeric_limits<double>::infinity());
}

TEST_F(ConcentricConstraintTests, RepeatedEnforcement) {
    ConcentricConstraint constraint(circle1, circle2);
    
    // First enforcement
    EXPECT_TRUE(constraint.enforce());
    auto firstCenter = circle1->getCenter();
    
    // Move second circle
    circle2->translate(1.0f, 1.0f);
    
    // Second enforcement
    EXPECT_TRUE(constraint.enforce());
    auto secondCenter = circle1->getCenter();
    
    // Centers should be at new midpoint
    EXPECT_TRUE(pointsEqual(secondCenter, {1.0f, 1.0f}));
}

TEST_F(ConcentricConstraintTests, EdgeCases) {
    // Test with coincident centers
    auto circle3 = std::make_shared<Circle>(0.0f, 0.0f, 2.0f);
    ConcentricConstraint coincident(circle1, circle3);
    EXPECT_TRUE(coincident.isSatisfied());
    EXPECT_NEAR(coincident.getError(), 0.0f, kTolerance);
    
    // Test with very distant centers
    auto circle4 = std::make_shared<Circle>(1e6f, 1e6f, 1.0f);
    ConcentricConstraint distant(circle1, circle4);
    EXPECT_FALSE(distant.isSatisfied());
    EXPECT_TRUE(distant.enforce());
    EXPECT_TRUE(distant.isSatisfied());
}
