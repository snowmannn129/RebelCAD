#pragma once

#include <memory>
#include <string>
#include <vector>

namespace RebelCAD {
namespace Assembly {

// Forward declarations
class Component;

namespace Constraints {

/**
 * @brief Base class for all assembly constraints
 * 
 * The AssemblyConstraint class serves as the foundation for RebelCAD's assembly constraint system.
 * It defines the interface that all specific assembly constraints must implement.
 */
class AssemblyConstraint {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~AssemblyConstraint() = default;

    /**
     * @brief Gets the name of the constraint
     * @return The constraint name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Gets the components involved in this constraint
     * @return Vector of components involved in the constraint
     */
    virtual std::vector<std::shared_ptr<Component>> getComponents() const = 0;

    /**
     * @brief Evaluates if the constraint is currently satisfied
     * @return true if constraint conditions are met, false otherwise
     */
    virtual bool isSatisfied() const = 0;

    /**
     * @brief Attempts to enforce the constraint
     * @return true if constraint was successfully enforced, false if it failed
     */
    virtual bool enforce() = 0;

    /**
     * @brief Gets the error magnitude of the constraint violation
     * @return A value indicating how far from satisfaction the constraint is
     */
    virtual double getError() const = 0;

    /**
     * @brief Checks if the constraint is valid and can be enforced
     * @return true if the constraint is valid, false otherwise
     */
    virtual bool isValid() const = 0;

    /**
     * @brief Checks if the constraint allows motion
     * @return true if the constraint allows motion, false if it's a rigid constraint
     */
    virtual bool allowsMotion() const = 0;

    /**
     * @brief Gets the degrees of freedom removed by this constraint
     * @return Number of degrees of freedom removed (0-6)
     */
    virtual int getConstrainedDegreesOfFreedom() const = 0;

    /**
     * @brief Gets the priority of the constraint
     * 
     * Priority is used by the constraint solver to determine the order in which
     * constraints are enforced. Higher priority constraints are enforced first.
     * 
     * @return The constraint priority (default is 0)
     */
    virtual int getPriority() const { return 0; }

protected:
    /**
     * @brief Protected constructor to prevent direct instantiation
     */
    AssemblyConstraint() = default;
};

} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
