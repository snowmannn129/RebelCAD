#include <gtest/gtest.h>
#include "../../../src/assembly/constraints/ConstraintSolver.h"
#include "../../../src/assembly/Component.h"
#include <memory>
#include <string>
#include <vector>

using namespace RebelCAD::Assembly;
using namespace RebelCAD::Assembly::Constraints;

// Mock constraint for testing
class MockConstraint : public AssemblyConstraint {
public:
    using EnforceCallback = std::function<bool()>;

    MockConstraint(const std::string& name, double error, bool satisfied, int dofs)
        : m_name(name), m_error(error), m_satisfied(satisfied), m_dofs(dofs), m_enforceCallback(nullptr) {
    }

    std::string getName() const override {
        return m_name;
    }

    std::vector<std::shared_ptr<Component>> getComponents() const override {
        return m_components;
    }

    bool isSatisfied() const override {
        return m_satisfied;
    }

    bool enforce() override {
        if (m_enforceCallback) {
            return m_enforceCallback();
        }
        m_satisfied = true;
        m_error = 0.0;
        return true;
    }

    double getError() const override {
        return m_error;
    }

    bool isValid() const override {
        return true;
    }

    bool allowsMotion() const override {
        return true;
    }

    int getConstrainedDegreesOfFreedom() const override {
        return m_dofs;
    }

    void addComponent(std::shared_ptr<Component> component) {
        m_components.push_back(component);
    }

    void setError(double error) {
        m_error = error;
    }

    void setSatisfied(bool satisfied) {
        m_satisfied = satisfied;
    }

    void setEnforceCallback(EnforceCallback callback) {
        m_enforceCallback = callback;
    }

private:
    std::string m_name;
    double m_error;
    bool m_satisfied;
    int m_dofs;
    std::vector<std::shared_ptr<Component>> m_components;
    EnforceCallback m_enforceCallback;
};

// Test fixture for ConstraintSolver tests
class ConstraintSolverTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a constraint solver
        solver = std::make_unique<ConstraintSolver>();

        // Create some components
        component1 = std::make_shared<Component>("Component 1");
        component2 = std::make_shared<Component>("Component 2");

        // Create some constraints
        constraint1 = std::make_shared<MockConstraint>("Constraint 1", 1.0, false, 1);
        constraint1->addComponent(component1);
        constraint1->addComponent(component2);

        constraint2 = std::make_shared<MockConstraint>("Constraint 2", 2.0, false, 2);
        constraint2->addComponent(component1);
    }

    void TearDown() override {
        solver.reset();
        component1.reset();
        component2.reset();
        constraint1.reset();
        constraint2.reset();
    }

    std::unique_ptr<ConstraintSolver> solver;
    std::shared_ptr<Component> component1;
    std::shared_ptr<Component> component2;
    std::shared_ptr<MockConstraint> constraint1;
    std::shared_ptr<MockConstraint> constraint2;
};

// Test adding and removing constraints
TEST_F(ConstraintSolverTest, AddRemoveConstraints) {
    // Initially, the solver should have no constraints
    EXPECT_EQ(0, solver->getConstraintCount());

    // Add a constraint
    EXPECT_TRUE(solver->addConstraint(constraint1));
    EXPECT_EQ(1, solver->getConstraintCount());

    // Add another constraint
    EXPECT_TRUE(solver->addConstraint(constraint2));
    EXPECT_EQ(2, solver->getConstraintCount());

    // Try to add the same constraint again
    EXPECT_FALSE(solver->addConstraint(constraint1));
    EXPECT_EQ(2, solver->getConstraintCount());

    // Remove a constraint
    EXPECT_TRUE(solver->removeConstraint(constraint1));
    EXPECT_EQ(1, solver->getConstraintCount());

    // Try to remove a constraint that's not in the solver
    EXPECT_FALSE(solver->removeConstraint(constraint1));
    EXPECT_EQ(1, solver->getConstraintCount());

    // Clear all constraints
    solver->clearConstraints();
    EXPECT_EQ(0, solver->getConstraintCount());
}

// Test solving constraints
TEST_F(ConstraintSolverTest, SolveConstraints) {
    // Add constraints
    solver->addConstraint(constraint1);
    solver->addConstraint(constraint2);

    // Initially, the constraints are not satisfied
    EXPECT_FALSE(constraint1->isSatisfied());
    EXPECT_FALSE(constraint2->isSatisfied());

    // Solve the constraints
    auto result = solver->solve();

    // Check that the solve was successful
    EXPECT_TRUE(result.success);

    // Check that the constraints are now satisfied
    EXPECT_TRUE(constraint1->isSatisfied());
    EXPECT_TRUE(constraint2->isSatisfied());

    // Check that the error is zero
    EXPECT_DOUBLE_EQ(0.0, result.error);
}

// Test degrees of freedom
TEST_F(ConstraintSolverTest, DegreesOfFreedom) {
    // Add constraints
    solver->addConstraint(constraint1);
    solver->addConstraint(constraint2);

    // Each component has 6 degrees of freedom (3 for position, 3 for orientation)
    // We have 2 components, so 12 total degrees of freedom
    EXPECT_EQ(12, solver->getTotalDegreesOfFreedom());

    // Constraint 1 removes 1 degree of freedom, Constraint 2 removes 2 degrees of freedom
    EXPECT_EQ(3, solver->getConstrainedDegreesOfFreedom());

    // Remaining degrees of freedom = Total - Constrained
    EXPECT_EQ(9, solver->getRemainingDegreesOfFreedom());

    // The system is under-constrained
    EXPECT_TRUE(solver->isUnderConstrained());
    EXPECT_FALSE(solver->isOverConstrained());
}

// Test prioritized constraint solving
TEST_F(ConstraintSolverTest, PrioritizedConstraintSolving) {
    // Create constraints with different priorities
    auto highPriorityConstraint = std::make_shared<MockConstraint>("High Priority", 1.0, false, 1);
    highPriorityConstraint->addComponent(component1);
    highPriorityConstraint->addComponent(component2);
    
    auto mediumPriorityConstraint = std::make_shared<MockConstraint>("Medium Priority", 2.0, false, 2);
    mediumPriorityConstraint->addComponent(component1);
    
    auto lowPriorityConstraint = std::make_shared<MockConstraint>("Low Priority", 3.0, false, 3);
    lowPriorityConstraint->addComponent(component2);
    
    // Set priorities using a custom method for the mock constraint
    class PrioritizedMockConstraint : public MockConstraint {
    public:
        PrioritizedMockConstraint(const std::string& name, double error, bool satisfied, int dofs, int priority)
            : MockConstraint(name, error, satisfied, dofs), m_priority(priority) {
        }
        
        int getPriority() const override {
            return m_priority;
        }
        
    private:
        int m_priority;
    };
    
    auto highPriority = std::make_shared<PrioritizedMockConstraint>("High Priority", 1.0, false, 1, 10);
    highPriority->addComponent(component1);
    
    auto mediumPriority = std::make_shared<PrioritizedMockConstraint>("Medium Priority", 2.0, false, 2, 5);
    mediumPriority->addComponent(component1);
    
    auto lowPriority = std::make_shared<PrioritizedMockConstraint>("Low Priority", 3.0, false, 3, 1);
    lowPriority->addComponent(component2);
    
    // Add constraints in reverse priority order
    solver->addConstraint(lowPriority);
    solver->addConstraint(mediumPriority);
    solver->addConstraint(highPriority);
    
    // Create a vector to track the order in which constraints are enforced
    std::vector<std::string> enforcementOrder;
    
    // Override the enforce method to track the order
    auto trackEnforcement = [&enforcementOrder](const std::string& name) {
        enforcementOrder.push_back(name);
        return true;
    };
    
    // Set up the mock constraints to track enforcement order
    highPriority->setEnforceCallback([&trackEnforcement, highPriority]() {
        return trackEnforcement(highPriority->getName());
    });
    
    mediumPriority->setEnforceCallback([&trackEnforcement, mediumPriority]() {
        return trackEnforcement(mediumPriority->getName());
    });
    
    lowPriority->setEnforceCallback([&trackEnforcement, lowPriority]() {
        return trackEnforcement(lowPriority->getName());
    });
    
    // Solve the constraints
    solver->solve();
    
    // Check that the constraints were enforced in priority order (highest first)
    ASSERT_EQ(3, enforcementOrder.size());
    EXPECT_EQ("High Priority", enforcementOrder[0]);
    EXPECT_EQ("Medium Priority", enforcementOrder[1]);
    EXPECT_EQ("Low Priority", enforcementOrder[2]);
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
