#pragma once

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Base class for all geometric constraints
 * 
 * The Constraint class serves as the foundation for RebelCAD's constraint system.
 * It defines the interface that all specific constraints must implement.
 */
class Constraint {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~Constraint() = default;

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

protected:
    /**
     * @brief Protected constructor to prevent direct instantiation
     */
    Constraint() = default;
};

} // namespace Constraints
} // namespace RebelCAD
