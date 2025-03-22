#include "GearRackConstraint.h"
#include <gtest/gtest.h>
#include <memory>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {
namespace Tests {

class GearRackConstraintTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create components for testing
        gear = std::make_shared<Component>("Gear");
        rack = std::make_shared<Component>("Rack");
        
        // Set initial positions and orientations
        gear->setPosition(Eigen::Vector3d(0.0, 0.0, 0.0));
        gear->setOrientation(Eigen::Quaterniond::Identity());
        
        rack->setPosition(Eigen::Vector3d(2.0, 0.0, 0.0));
        rack->setOrientation(Eigen::Quaterniond::Identity());
        
        // Create a gear-rack constraint
        gearAxis = Eigen::Vector3d(0.0, 0.0, 1.0); // Z-axis
        rackAxis = Eigen::Vector3d(1.0, 0.0, 0.0); // X-axis
        gearRadius = 1.0;
        
        gearRackConstraint = std::make_shared<GearRackConstraint>(
            gear, rack, gearAxis, rackAxis, gearRadius
        );
    }
    
    std::shared_ptr<Component> gear;
    std::shared_ptr<Component> rack;
    Eigen::Vector3d gearAxis;
    Eigen::Vector3d rackAxis;
    double gearRadius;
    std::shared_ptr<GearRackConstraint> gearRackConstraint;
};

TEST_F(GearRackConstraintTest, ConstructorSetsProperties) {
    // Check that the components are set correctly
    auto components = gearRackConstraint->getComponents();
    ASSERT_EQ(components.size(), 2);
    EXPECT_EQ(components[0], gear);
    EXPECT_EQ(components[1], rack);
    
    // Check that the gear radius is set correctly
    EXPECT_NEAR(gearRackConstraint->getGearRadius(), gearRadius, 1e-6);
    
    // Check that the efficiency is set to the default value
    EXPECT_NEAR(gearRackConstraint->getEfficiency(), 0.98, 1e-6);
    
    // Check that the axes are set correctly
    Eigen::Vector3d testGearAxis, testRackAxis;
    gearRackConstraint->getAxes(testGearAxis, testRackAxis);
    
    EXPECT_NEAR(testGearAxis.x(), gearAxis.x(), 1e-6);
    EXPECT_NEAR(testGearAxis.y(), gearAxis.y(), 1e-6);
    EXPECT_NEAR(testGearAxis.z(), gearAxis.z(), 1e-6);
    
    EXPECT_NEAR(testRackAxis.x(), rackAxis.x(), 1e-6);
    EXPECT_NEAR(testRackAxis.y(), rackAxis.y(), 1e-6);
    EXPECT_NEAR(testRackAxis.z(), rackAxis.z(), 1e-6);
}

TEST_F(GearRackConstraintTest, InitiallyConstraintIsNotSatisfied) {
    // The initial positions of the gear and rack are not consistent with the constraint
    EXPECT_FALSE(gearRackConstraint->isSatisfied());
}

TEST_F(GearRackConstraintTest, EnforceCorrectsMisalignment) {
    // Enforce the constraint
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // The rack position should be consistent with the gear angle
    double gearAngle = gearRackConstraint->getGearAngle();
    double rackPosition = gearRackConstraint->getRackPosition();
    
    // For a gear-rack, rack position = gear angle * gear radius
    EXPECT_NEAR(rackPosition, gearAngle * gearRadius, 1e-6);
}

TEST_F(GearRackConstraintTest, SimulateMotionWorksCorrectly) {
    // Enforce the constraint to start from a valid state
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // Set a velocity
    Eigen::Vector3d linearVelocity(1.0, 0.0, 0.0); // 1 unit/s along X-axis
    Eigen::Vector3d angularVelocity(0.0, 0.0, 1.0); // 1 rad/s around Z-axis
    
    gearRackConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Simulate for 1 second
    double dt = 1.0;
    EXPECT_TRUE(gearRackConstraint->simulateMotion(dt));
    
    // The constraint should still be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // The gear should have rotated
    double gearAngle = gearRackConstraint->getGearAngle();
    EXPECT_NEAR(gearAngle, 1.0, 1e-6); // 1 rad/s * 1 s = 1 rad
    
    // The rack should have moved
    double rackPosition = gearRackConstraint->getRackPosition();
    EXPECT_NEAR(rackPosition, 1.0, 1e-6); // 1 unit/s * 1 s = 1 unit
}

TEST_F(GearRackConstraintTest, SetGearAngleUpdatesRackPosition) {
    // Enforce the constraint to start from a valid state
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // Set a new gear angle
    double newGearAngle = 2.0;
    EXPECT_TRUE(gearRackConstraint->setGearAngle(newGearAngle));
    
    // The constraint should still be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // The rack position should be updated
    double rackPosition = gearRackConstraint->getRackPosition();
    EXPECT_NEAR(rackPosition, newGearAngle * gearRadius, 1e-6);
}

TEST_F(GearRackConstraintTest, SetRackPositionUpdatesGearAngle) {
    // Enforce the constraint to start from a valid state
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // Set a new rack position
    double newRackPosition = 3.0;
    EXPECT_TRUE(gearRackConstraint->setRackPosition(newRackPosition));
    
    // The constraint should still be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // The gear angle should be updated
    double gearAngle = gearRackConstraint->getGearAngle();
    EXPECT_NEAR(gearAngle, newRackPosition / gearRadius, 1e-6);
}

TEST_F(GearRackConstraintTest, GetConstrainedDegreesOfFreedom) {
    // A gear-rack constraint removes one degree of freedom
    EXPECT_EQ(gearRackConstraint->getConstrainedDegreesOfFreedom(), 1);
}

TEST_F(GearRackConstraintTest, SetGearRadiusUpdatesConstraint) {
    // Enforce the constraint to start from a valid state
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // Set a new gear radius
    double newGearRadius = 2.0;
    gearRackConstraint->setGearRadius(newGearRadius);
    
    // The constraint should no longer be satisfied
    EXPECT_FALSE(gearRackConstraint->isSatisfied());
    
    // Enforce the constraint again
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // The rack position should be consistent with the gear angle and the new radius
    double gearAngle = gearRackConstraint->getGearAngle();
    double rackPosition = gearRackConstraint->getRackPosition();
    
    EXPECT_NEAR(rackPosition, gearAngle * newGearRadius, 1e-6);
}

TEST_F(GearRackConstraintTest, SetEfficiencyUpdatesConstraint) {
    // Set a new efficiency
    double newEfficiency = 0.75;
    gearRackConstraint->setEfficiency(newEfficiency);
    
    // Check that the efficiency is set correctly
    EXPECT_NEAR(gearRackConstraint->getEfficiency(), newEfficiency, 1e-6);
}

TEST_F(GearRackConstraintTest, SetAxesUpdatesConstraint) {
    // Enforce the constraint to start from a valid state
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // Set new axes
    Eigen::Vector3d newGearAxis(0.0, 1.0, 0.0); // Y-axis
    Eigen::Vector3d newRackAxis(0.0, 0.0, 1.0); // Z-axis
    
    gearRackConstraint->setAxes(newGearAxis, newRackAxis);
    
    // The constraint should no longer be satisfied
    EXPECT_FALSE(gearRackConstraint->isSatisfied());
    
    // Enforce the constraint again
    EXPECT_TRUE(gearRackConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(gearRackConstraint->isSatisfied());
    
    // Check that the axes are set correctly
    Eigen::Vector3d testGearAxis, testRackAxis;
    gearRackConstraint->getAxes(testGearAxis, testRackAxis);
    
    EXPECT_NEAR(testGearAxis.x(), newGearAxis.x(), 1e-6);
    EXPECT_NEAR(testGearAxis.y(), newGearAxis.y(), 1e-6);
    EXPECT_NEAR(testGearAxis.z(), newGearAxis.z(), 1e-6);
    
    EXPECT_NEAR(testRackAxis.x(), newRackAxis.x(), 1e-6);
    EXPECT_NEAR(testRackAxis.y(), newRackAxis.y(), 1e-6);
    EXPECT_NEAR(testRackAxis.z(), newRackAxis.z(), 1e-6);
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
