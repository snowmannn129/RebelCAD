#include <gtest/gtest.h>
#include "constraints/ConstraintManagerUI.h"
#include "constraints/ParallelConstraint.h"
#include "constraints/PerpendicularConstraint.h"
#include "constraints/AutoConstraintDetector.h"
#include <memory>

using namespace RebelCAD::Constraints;

class ConstraintManagerUITests : public ::testing::Test {
protected:
    void SetUp() override {
        autoDetector = std::make_shared<AutoConstraintDetector>();
        manager = std::make_unique<ConstraintManagerUI>(autoDetector);
    }

    std::shared_ptr<AutoConstraintDetector> autoDetector;
    std::unique_ptr<ConstraintManagerUI> manager;
};

TEST_F(ConstraintManagerUITests, InitialState) {
    EXPECT_TRUE(manager->isAutoConstraintEnabled());
}

TEST_F(ConstraintManagerUITests, AddConstraint) {
    auto constraint = std::make_shared<ParallelConstraint>();
    EXPECT_TRUE(manager->addConstraint(constraint));
    
    // Adding same constraint again should fail
    EXPECT_FALSE(manager->addConstraint(constraint));
}

TEST_F(ConstraintManagerUITests, RemoveConstraint) {
    auto constraint = std::make_shared<ParallelConstraint>();
    manager->addConstraint(constraint);
    
    EXPECT_TRUE(manager->removeConstraint(constraint));
    
    // Removing non-existent constraint should fail
    EXPECT_FALSE(manager->removeConstraint(constraint));
}

TEST_F(ConstraintManagerUITests, AutoConstraintToggle) {
    EXPECT_TRUE(manager->isAutoConstraintEnabled());
    
    manager->setAutoConstraintEnabled(false);
    EXPECT_FALSE(manager->isAutoConstraintEnabled());
    
    manager->setAutoConstraintEnabled(true);
    EXPECT_TRUE(manager->isAutoConstraintEnabled());
}

TEST_F(ConstraintManagerUITests, RefreshRemovesInvalidConstraints) {
    auto validConstraint = std::make_shared<ParallelConstraint>();
    auto invalidConstraint = std::make_shared<PerpendicularConstraint>();
    
    manager->addConstraint(validConstraint);
    manager->addConstraint(invalidConstraint);
    
    // Make the perpendicular constraint invalid
    // This would typically happen when the geometric elements it references are deleted
    invalidConstraint.reset();
    
    manager->refresh();
    
    // Try to remove the invalid constraint - should fail because refresh removed it
    EXPECT_FALSE(manager->removeConstraint(invalidConstraint));
    
    // Valid constraint should still be there
    EXPECT_TRUE(manager->removeConstraint(validConstraint));
}

TEST_F(ConstraintManagerUITests, NullConstraintHandling) {
    EXPECT_FALSE(manager->addConstraint(nullptr));
    EXPECT_FALSE(manager->removeConstraint(nullptr));
}

// Note: UI rendering tests would typically be done with integration tests
// since they require a graphics context and user interaction simulation
