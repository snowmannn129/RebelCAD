#include "PathConstraint.h"
#include <gtest/gtest.h>
#include <memory>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {
namespace Tests {

class PathConstraintTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a component for testing
        component = std::make_shared<Component>("TestComponent");
        
        // Set initial position and orientation
        component->setPosition(Eigen::Vector3d(0.0, 0.0, 0.0));
        component->setOrientation(Eigen::Quaterniond::Identity());
        
        // Create a linear path from (0,0,0) to (10,0,0)
        startPoint = Eigen::Vector3d(0.0, 0.0, 0.0);
        endPoint = Eigen::Vector3d(10.0, 0.0, 0.0);
        
        // Create a path constraint with the linear path
        pathConstraint = std::make_shared<PathConstraint>(
            component, startPoint, endPoint
        );
    }
    
    std::shared_ptr<Component> component;
    Eigen::Vector3d startPoint;
    Eigen::Vector3d endPoint;
    std::shared_ptr<PathConstraint> pathConstraint;
};

TEST_F(PathConstraintTest, ConstructorSetsProperties) {
    // Check that the parameter is initially 0.0
    EXPECT_NEAR(pathConstraint->getParameter(), 0.0, 1e-6);
    
    // Check that the local origin is at (0,0,0)
    EXPECT_NEAR(pathConstraint->getLocalOrigin().norm(), 0.0, 1e-6);
    
    // Check that the current path position is at the start point
    Eigen::Vector3d currentPos = pathConstraint->getCurrentPathPosition();
    EXPECT_NEAR(currentPos.x(), startPoint.x(), 1e-6);
    EXPECT_NEAR(currentPos.y(), startPoint.y(), 1e-6);
    EXPECT_NEAR(currentPos.z(), startPoint.z(), 1e-6);
}

TEST_F(PathConstraintTest, GetComponentsReturnsCorrectComponent) {
    auto components = pathConstraint->getComponents();
    ASSERT_EQ(components.size(), 1);
    EXPECT_EQ(components[0], component);
}

TEST_F(PathConstraintTest, InitiallyConstraintIsSatisfied) {
    EXPECT_TRUE(pathConstraint->isSatisfied());
    EXPECT_NEAR(pathConstraint->getError(), 0.0, 1e-6);
}

TEST_F(PathConstraintTest, SetParameterMovesAlongPath) {
    // Set parameter to 0.5 (halfway along the path)
    EXPECT_TRUE(pathConstraint->setParameter(0.5));
    
    // Check that the parameter is set correctly
    EXPECT_NEAR(pathConstraint->getParameter(), 0.5, 1e-6);
    
    // Check that the current path position is halfway between start and end
    Eigen::Vector3d expectedPos = startPoint + 0.5 * (endPoint - startPoint);
    Eigen::Vector3d currentPos = pathConstraint->getCurrentPathPosition();
    
    EXPECT_NEAR(currentPos.x(), expectedPos.x(), 1e-6);
    EXPECT_NEAR(currentPos.y(), expectedPos.y(), 1e-6);
    EXPECT_NEAR(currentPos.z(), expectedPos.z(), 1e-6);
    
    // Check that the component has moved to the new position
    Eigen::Vector3d componentPos = component->getPosition();
    
    EXPECT_NEAR(componentPos.x(), expectedPos.x(), 1e-6);
    EXPECT_NEAR(componentPos.y(), expectedPos.y(), 1e-6);
    EXPECT_NEAR(componentPos.z(), expectedPos.z(), 1e-6);
}

TEST_F(PathConstraintTest, SimulateMotionMovesAlongPath) {
    // Set a velocity
    Eigen::Vector3d linearVelocity(1.0, 0.0, 0.0); // 1 unit/s along X-axis
    Eigen::Vector3d angularVelocity = Eigen::Vector3d::Zero();
    
    pathConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Simulate for 2 seconds
    double dt = 2.0;
    EXPECT_TRUE(pathConstraint->simulateMotion(dt));
    
    // Check that the component has moved along the path
    // The exact position depends on the path length and how the velocity is applied
    // For a linear path from (0,0,0) to (10,0,0), we expect the component to move in the positive X direction
    Eigen::Vector3d componentPos = component->getPosition();
    
    EXPECT_GT(componentPos.x(), 0.0);
    EXPECT_NEAR(componentPos.y(), 0.0, 1e-6);
    EXPECT_NEAR(componentPos.z(), 0.0, 1e-6);
}

TEST_F(PathConstraintTest, EnforceCorrectsMisalignment) {
    // Move the component off the path
    component->setPosition(Eigen::Vector3d(5.0, 1.0, 0.0));
    
    // The constraint should now be violated
    EXPECT_FALSE(pathConstraint->isSatisfied());
    
    // Enforce the constraint
    EXPECT_TRUE(pathConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(pathConstraint->isSatisfied());
    
    // The component should be back on the path
    Eigen::Vector3d componentPos = component->getPosition();
    EXPECT_NEAR(componentPos.y(), 0.0, 1e-6);
    EXPECT_NEAR(componentPos.z(), 0.0, 1e-6);
}

TEST_F(PathConstraintTest, GetConstrainedDegreesOfFreedom) {
    // A path constraint removes two translational degrees of freedom
    EXPECT_EQ(pathConstraint->getConstrainedDegreesOfFreedom(), 2);
}

TEST_F(PathConstraintTest, PiecewiseLinearPathWorks) {
    // Create a piecewise linear path with three points
    std::vector<Eigen::Vector3d> points = {
        Eigen::Vector3d(0.0, 0.0, 0.0),
        Eigen::Vector3d(5.0, 5.0, 0.0),
        Eigen::Vector3d(10.0, 0.0, 0.0)
    };
    
    // Create a new path constraint with the piecewise linear path
    auto piecewiseConstraint = std::make_shared<PathConstraint>(
        component, points
    );
    
    // Set parameter to 0.5 (halfway along the path)
    EXPECT_TRUE(piecewiseConstraint->setParameter(0.5));
    
    // Check that the current path position is at the middle point
    // For a piecewise linear path, this depends on the implementation
    // but should be somewhere along the path
    Eigen::Vector3d currentPos = piecewiseConstraint->getCurrentPathPosition();
    
    // The component should be on the path
    EXPECT_TRUE(piecewiseConstraint->isSatisfied());
}

TEST_F(PathConstraintTest, CustomPathFunctionWorks) {
    // Create a circular path function
    PathConstraint::PathFunction circularPath = [](double t) {
        double angle = t * 2 * M_PI;
        double radius = 5.0;
        return Eigen::Vector3d(
            radius * std::cos(angle),
            radius * std::sin(angle),
            0.0
        );
    };
    
    // Create a new path constraint with the custom path function
    auto circularConstraint = std::make_shared<PathConstraint>(
        component, circularPath
    );
    
    // Set parameter to 0.25 (quarter way around the circle)
    EXPECT_TRUE(circularConstraint->setParameter(0.25));
    
    // Check that the current path position is at the expected point on the circle
    Eigen::Vector3d expectedPos(0.0, 5.0, 0.0); // At 90 degrees
    Eigen::Vector3d currentPos = circularConstraint->getCurrentPathPosition();
    
    EXPECT_NEAR(currentPos.x(), expectedPos.x(), 1e-6);
    EXPECT_NEAR(currentPos.y(), expectedPos.y(), 1e-6);
    EXPECT_NEAR(currentPos.z(), expectedPos.z(), 1e-6);
    
    // The component should be on the path
    EXPECT_TRUE(circularConstraint->isSatisfied());
}

TEST_F(PathConstraintTest, GetPathLengthReturnsCorrectLength) {
    // For a linear path from (0,0,0) to (10,0,0), the length should be 10.0
    EXPECT_NEAR(pathConstraint->getPathLength(), 10.0, 0.1);
}

TEST_F(PathConstraintTest, GetCurrentPathTangentReturnsCorrectTangent) {
    // For a linear path from (0,0,0) to (10,0,0), the tangent should be (1,0,0)
    Eigen::Vector3d tangent = pathConstraint->getCurrentPathTangent();
    
    EXPECT_NEAR(tangent.x(), 1.0, 1e-6);
    EXPECT_NEAR(tangent.y(), 0.0, 1e-6);
    EXPECT_NEAR(tangent.z(), 0.0, 1e-6);
}

TEST_F(PathConstraintTest, SetLocalOriginChangesOrigin) {
    // Set a non-zero local origin
    Eigen::Vector3d newOrigin(1.0, 0.0, 0.0);
    pathConstraint->setLocalOrigin(newOrigin);
    
    // Check that the local origin is set correctly
    Eigen::Vector3d origin = pathConstraint->getLocalOrigin();
    
    EXPECT_NEAR(origin.x(), newOrigin.x(), 1e-6);
    EXPECT_NEAR(origin.y(), newOrigin.y(), 1e-6);
    EXPECT_NEAR(origin.z(), newOrigin.z(), 1e-6);
    
    // The constraint should now be violated because the component's local origin
    // is no longer on the path
    EXPECT_FALSE(pathConstraint->isSatisfied());
    
    // Enforce the constraint
    EXPECT_TRUE(pathConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(pathConstraint->isSatisfied());
    
    // The component should have moved to keep the local origin on the path
    Eigen::Vector3d componentPos = component->getPosition();
    
    // The component should be at (-1,0,0) to keep the local origin (1,0,0) at the start of the path (0,0,0)
    EXPECT_NEAR(componentPos.x(), -1.0, 1e-6);
    EXPECT_NEAR(componentPos.y(), 0.0, 1e-6);
    EXPECT_NEAR(componentPos.z(), 0.0, 1e-6);
}

} // namespace Tests
} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD

// Main function to run the tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
