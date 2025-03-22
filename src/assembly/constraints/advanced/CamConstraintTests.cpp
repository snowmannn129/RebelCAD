#include "CamConstraint.h"
#include <gtest/gtest.h>
#include <memory>
#include <cmath>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {
namespace Tests {

class CamConstraintTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create two components for testing
        cam = std::make_shared<Component>("Cam");
        follower = std::make_shared<Component>("Follower");
        
        // Set initial positions and orientations
        cam->setPosition(Eigen::Vector3d(0.0, 0.0, 0.0));
        cam->setOrientation(Eigen::Quaterniond::Identity());
        
        follower->setPosition(Eigen::Vector3d(0.0, 1.0, 0.0));
        follower->setOrientation(Eigen::Quaterniond::Identity());
        
        // Create axes (cam rotates around Z, follower moves along Y)
        camAxis = Eigen::Vector3d(0.0, 0.0, 1.0);
        followerAxis = Eigen::Vector3d(0.0, 1.0, 0.0);
        
        // Create a circular cam constraint with radius 0.5
        camRadius = 0.5;
        camConstraint = std::make_shared<CamConstraint>(
            cam, follower, camAxis, followerAxis, camRadius
        );
    }
    
    std::shared_ptr<Component> cam;
    std::shared_ptr<Component> follower;
    Eigen::Vector3d camAxis;
    Eigen::Vector3d followerAxis;
    double camRadius;
    std::shared_ptr<CamConstraint> camConstraint;
};

TEST_F(CamConstraintTest, ConstructorSetsProperties) {
    EXPECT_EQ(camConstraint->getOffset(), 0.0);
    
    Eigen::Vector3d testCamAxis, testFollowerAxis;
    camConstraint->getAxes(testCamAxis, testFollowerAxis);
    
    EXPECT_NEAR(testCamAxis.x(), camAxis.x(), 1e-6);
    EXPECT_NEAR(testCamAxis.y(), camAxis.y(), 1e-6);
    EXPECT_NEAR(testCamAxis.z(), camAxis.z(), 1e-6);
    
    EXPECT_NEAR(testFollowerAxis.x(), followerAxis.x(), 1e-6);
    EXPECT_NEAR(testFollowerAxis.y(), followerAxis.y(), 1e-6);
    EXPECT_NEAR(testFollowerAxis.z(), followerAxis.z(), 1e-6);
}

TEST_F(CamConstraintTest, GetComponentsReturnsCorrectComponents) {
    auto components = camConstraint->getComponents();
    ASSERT_EQ(components.size(), 2);
    EXPECT_EQ(components[0], cam);
    EXPECT_EQ(components[1], follower);
}

TEST_F(CamConstraintTest, InitiallyConstraintIsSatisfied) {
    // The follower should be at position (0, camRadius, 0) initially
    EXPECT_NEAR(follower->getPosition().y(), camRadius, 1e-6);
    EXPECT_TRUE(camConstraint->isSatisfied());
    EXPECT_NEAR(camConstraint->getError(), 0.0, 1e-6);
}

TEST_F(CamConstraintTest, SimulateMotionMovesFollower) {
    // Set an angular velocity
    Eigen::Vector3d linearVelocity = Eigen::Vector3d::Zero();
    Eigen::Vector3d angularVelocity(0.0, 0.0, 1.0); // 1 rad/s around Z-axis
    
    camConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Simulate for 1 second
    double dt = 1.0;
    EXPECT_TRUE(camConstraint->simulateMotion(dt));
    
    // For a circular cam, the follower position should not change
    EXPECT_NEAR(follower->getPosition().y(), camRadius, 1e-6);
    
    // Create a non-circular cam constraint with a piecewise linear profile
    std::vector<std::pair<double, double>> angleRadiusPairs = {
        {0.0, 0.5},
        {M_PI / 2, 1.0},
        {M_PI, 0.5},
        {3 * M_PI / 2, 0.25},
        {2 * M_PI, 0.5}
    };
    
    auto nonCircularCam = std::make_shared<CamConstraint>(
        cam, follower, camAxis, followerAxis, angleRadiusPairs
    );
    
    // Reset follower position
    follower->setPosition(Eigen::Vector3d(0.0, 0.5, 0.0));
    
    // Set velocity
    nonCircularCam->setVelocity(linearVelocity, angularVelocity);
    
    // Simulate for 1 second (should rotate to angle M_PI/2)
    EXPECT_TRUE(nonCircularCam->simulateMotion(dt));
    
    // Follower should now be at position (0, 1.0, 0)
    EXPECT_NEAR(follower->getPosition().y(), 1.0, 1e-6);
}

TEST_F(CamConstraintTest, EnforceCorrectsMisalignment) {
    // Move the follower to create a misalignment
    follower->setPosition(Eigen::Vector3d(0.0, 0.7, 0.0));
    
    // The constraint should now be violated
    EXPECT_FALSE(camConstraint->isSatisfied());
    
    // Enforce the constraint
    EXPECT_TRUE(camConstraint->enforce());
    
    // The constraint should now be satisfied
    EXPECT_TRUE(camConstraint->isSatisfied());
    EXPECT_NEAR(follower->getPosition().y(), camRadius, 1e-6);
}

TEST_F(CamConstraintTest, CalculateForceAndTorque) {
    // Set an angular velocity
    Eigen::Vector3d linearVelocity = Eigen::Vector3d::Zero();
    Eigen::Vector3d angularVelocity(0.0, 0.0, 2.0); // 2 rad/s around Z-axis
    
    camConstraint->setVelocity(linearVelocity, angularVelocity);
    
    // Calculate force and torque
    Eigen::Vector3d force, torque;
    camConstraint->calculateForceAndTorque(force, torque);
    
    // For a circular cam, the force should be zero (no change in radius)
    EXPECT_NEAR(force.norm(), 0.0, 1e-6);
    
    // Create a non-circular cam constraint with a piecewise linear profile
    std::vector<std::pair<double, double>> angleRadiusPairs = {
        {0.0, 0.5},
        {M_PI / 2, 1.0},
        {M_PI, 0.5},
        {3 * M_PI / 2, 0.25},
        {2 * M_PI, 0.5}
    };
    
    auto nonCircularCam = std::make_shared<CamConstraint>(
        cam, follower, camAxis, followerAxis, angleRadiusPairs
    );
    
    // Set velocity
    nonCircularCam->setVelocity(linearVelocity, angularVelocity);
    
    // Calculate force and torque
    nonCircularCam->calculateForceAndTorque(force, torque);
    
    // For a non-circular cam, the force should be non-zero
    EXPECT_GT(force.norm(), 0.0);
    
    // Force should be along the follower axis
    EXPECT_NEAR(force.normalized().dot(followerAxis.normalized()), 1.0, 1e-6);
    
    // Torque should be along the cam axis
    EXPECT_NEAR(torque.normalized().dot(camAxis.normalized()), 1.0, 1e-6);
}

TEST_F(CamConstraintTest, GetConstrainedDegreesOfFreedom) {
    // A cam constraint removes one translational degree of freedom
    EXPECT_EQ(camConstraint->getConstrainedDegreesOfFreedom(), 1);
}

TEST_F(CamConstraintTest, SetCamAngleChangesFollowerPosition) {
    // Create a non-circular cam constraint with a piecewise linear profile
    std::vector<std::pair<double, double>> angleRadiusPairs = {
        {0.0, 0.5},
        {M_PI / 2, 1.0},
        {M_PI, 0.5},
        {3 * M_PI / 2, 0.25},
        {2 * M_PI, 0.5}
    };
    
    auto nonCircularCam = std::make_shared<CamConstraint>(
        cam, follower, camAxis, followerAxis, angleRadiusPairs
    );
    
    // Set cam angle to M_PI/2
    EXPECT_TRUE(nonCircularCam->setCamAngle(M_PI / 2));
    
    // Follower should now be at position (0, 1.0, 0)
    EXPECT_NEAR(follower->getPosition().y(), 1.0, 1e-6);
    
    // Set cam angle to 3*M_PI/2
    EXPECT_TRUE(nonCircularCam->setCamAngle(3 * M_PI / 2));
    
    // Follower should now be at position (0, 0.25, 0)
    EXPECT_NEAR(follower->getPosition().y(), 0.25, 1e-6);
}

TEST_F(CamConstraintTest, SetOffsetChangesFollowerPosition) {
    // Set offset to 0.5
    camConstraint->setOffset(0.5);
    
    // Enforce the constraint
    EXPECT_TRUE(camConstraint->enforce());
    
    // Follower should now be at position (0, camRadius + 0.5, 0)
    EXPECT_NEAR(follower->getPosition().y(), camRadius + 0.5, 1e-6);
}

TEST_F(CamConstraintTest, CustomProfileFunctionWorks) {
    // Create a custom profile function (elliptical cam)
    auto ellipticalProfile = [](double angle) {
        double a = 0.5; // Semi-major axis
        double b = 0.25; // Semi-minor axis
        return a * b / std::sqrt(b * b * std::cos(angle) * std::cos(angle) + a * a * std::sin(angle) * std::sin(angle));
    };
    
    auto ellipticalCam = std::make_shared<CamConstraint>(
        cam, follower, camAxis, followerAxis, ellipticalProfile
    );
    
    // Set cam angle to 0
    EXPECT_TRUE(ellipticalCam->setCamAngle(0.0));
    
    // Follower should be at position (0, 0.5, 0)
    EXPECT_NEAR(follower->getPosition().y(), 0.5, 1e-6);
    
    // Set cam angle to M_PI/2
    EXPECT_TRUE(ellipticalCam->setCamAngle(M_PI / 2));
    
    // Follower should be at position (0, 0.25, 0)
    EXPECT_NEAR(follower->getPosition().y(), 0.25, 1e-6);
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
