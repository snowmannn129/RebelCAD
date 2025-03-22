#include "GearConstraint.h"
#include <gtest/gtest.h>
#include <memory>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {
namespace Tests {

class GearConstraintTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create two components for testing
        component1 = std::make_shared<Component>("Gear1");
        component2 = std::make_shared<Component>("Gear2");
        
        // Set initial positions and orientations
        component1->setPosition(Eigen::Vector3d(0.0, 0.0, 0.0));
        component1->setOrientation(Eigen::Quaterniond::Identity());
        
        component2->setPosition(Eigen::Vector3d(1.0, 0.0, 0.0));
        component2->setOrientation(Eigen::Quaterniond::Identity());
        
        // Create rotation axes (both along Z-axis for simplicity)
        axis1 = Eigen::Vector3d(0.0, 0.0, 1.0);
        axis2 = Eigen::Vector3d(0.0, 0.0, 1.0);
        
        // Create a gear constraint with 2:1 ratio
        gearRatio = 2.0;
        gearConstraint = std::make_shared<GearConstraint>(
            component1, component2, axis1, axis2, gearRatio
        );
    }
    
    std::shared_ptr<Component> component1;
    std::shared_ptr<Component> component2;
    Eigen::Vector3d axis1;
    Eigen::Vector3d axis2;
    double gearRatio;
    std::shared_ptr<GearConstraint> gearConstraint;
};

TEST_F(GearConstraintTest, ConstructorSetsProperties) {
    EXPECT_EQ(gearConstraint->getRatio(), gearRatio);
    EXPECT_NEAR(gearConstraint->getEfficiency(), 0.98, 1e-6);
    
    Eigen::Vector3d testAxis1, testAxis2;
    gearConstraint->getAxes(testAxis1, testAxis2);
    
    EXPECT_NEAR(testAxis1.x(), axis1.x(), 1e-6);
    EXPECT_NEAR(testAxis1.y(), axis1.y(), 1e-6);
    EXPECT_NEAR(testAxis1.z(), axis1.z(), 1e-6);
    
    EXPECT_NEAR(testAxis2.x(), axis2.x(), 1e-6);
    EXPECT_NEAR(testAxis2.y(), axis2.y(), 1e-6);
    EXPECT_NEAR(testAxis2.z(), axis2.z(), 1e-6);
}

TEST_F(GearConstraintTest, GetComponentsReturnsCorrectComponents) {
    auto components = gearConstraint->getComponents();
    ASSERT_EQ(components.size(), 2);
    EXPECT_EQ(components[0], component1);
    EXPECT_EQ(components[1], component2);
}

TEST_F(GearConstraintTest, InitiallyConstraintIsSatisfied) {
    EXPECT_TRUE(gearConstraint->isSatisfied());
    EXPECT_NEAR(gearConstraint->getError(), 0.0, 1e-6);
}

TEST_F(GearConstraintTest, SimulateMotionRotatesBothComponents) {
    // Set an angular velocity
    Eigen::Vector3d linearVelocity = Eigen::Vector3d::Zero();
    Eigen::Vector3d angularVelocity(0.0, 0.0, 1.0); // 1 rad/s around Z-axis
    
    gearConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Simulate for 1 second
    double dt = 1.0;
    EXPECT_TRUE(gearConstraint->simulateMotion(dt));
    
    // Check that the components have rotated correctly
    // Component 1 should rotate by 1 radian
    // Component 2 should rotate by 2 radians (due to 2:1 gear ratio)
    
    // Extract the rotation angles from the quaternions
    Eigen::AngleAxisd rotation1(component1->getOrientation());
    Eigen::AngleAxisd rotation2(component2->getOrientation());
    
    EXPECT_NEAR(rotation1.angle(), 1.0, 1e-6);
    EXPECT_NEAR(rotation2.angle(), 2.0, 1e-6);
}

TEST_F(GearConstraintTest, EnforceCorrectsMisalignment) {
    // Rotate component 2 to create a misalignment
    Eigen::AngleAxisd rotation(0.5, Eigen::Vector3d(0.0, 0.0, 1.0));
    component2->setOrientation(Eigen::Quaterniond(rotation));
    
    // The constraint should now be violated
    EXPECT_FALSE(gearConstraint->isSatisfied());
    
    // Enforce the constraint
    EXPECT_TRUE(gearConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(gearConstraint->isSatisfied());
}

TEST_F(GearConstraintTest, CalculateForceAndTorque) {
    // Set an angular velocity
    Eigen::Vector3d linearVelocity = Eigen::Vector3d::Zero();
    Eigen::Vector3d angularVelocity(0.0, 0.0, 2.0); // 2 rad/s around Z-axis
    
    gearConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Calculate force and torque
    Eigen::Vector3d force, torque;
    gearConstraint->calculateForceAndTorque(force, torque);
    
    // Force should be zero (gear constraints don't exert linear forces)
    EXPECT_NEAR(force.norm(), 0.0, 1e-6);
    
    // Torque should be along the Z-axis
    EXPECT_NEAR(torque.x(), 0.0, 1e-6);
    EXPECT_NEAR(torque.y(), 0.0, 1e-6);
    EXPECT_GT(torque.z(), 0.0); // Positive torque for positive angular velocity
}

TEST_F(GearConstraintTest, GetConstrainedDegreesOfFreedom) {
    // A gear constraint removes one rotational degree of freedom
    EXPECT_EQ(gearConstraint->getConstrainedDegreesOfFreedom(), 1);
}

TEST_F(GearConstraintTest, SetRatioChangesRatio) {
    double newRatio = 3.0;
    gearConstraint->setRatio(newRatio);
    EXPECT_EQ(gearConstraint->getRatio(), newRatio);
}

TEST_F(GearConstraintTest, SetEfficiencyChangesEfficiency) {
    double newEfficiency = 0.75;
    gearConstraint->setEfficiency(newEfficiency);
    EXPECT_EQ(gearConstraint->getEfficiency(), newEfficiency);
}

TEST_F(GearConstraintTest, InvalidRatioThrowsException) {
    EXPECT_THROW(gearConstraint->setRatio(0.0), std::invalid_argument);
}

TEST_F(GearConstraintTest, InvalidEfficiencyThrowsException) {
    EXPECT_THROW(gearConstraint->setEfficiency(-0.1), std::invalid_argument);
    EXPECT_THROW(gearConstraint->setEfficiency(1.1), std::invalid_argument);
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
