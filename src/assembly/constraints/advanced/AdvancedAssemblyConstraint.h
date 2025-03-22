#pragma once

#include "../AssemblyConstraint.h"
#include "../../../external/eigen/Eigen/Geometry"

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

/**
 * @brief Base class for advanced assembly constraints
 * 
 * The AdvancedAssemblyConstraint class serves as the foundation for RebelCAD's advanced assembly constraint system.
 * It extends the basic AssemblyConstraint with additional functionality for advanced constraints like gears, cams, etc.
 */
class AdvancedAssemblyConstraint : public AssemblyConstraint {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~AdvancedAssemblyConstraint() = default;

    /**
     * @brief Checks if the constraint allows motion
     * @return true if the constraint allows motion, false if it's a rigid constraint
     */
    bool allowsMotion() const override { return true; }

    /**
     * @brief Simulates the constraint motion for a given time step
     * @param dt Time step in seconds
     * @return true if the simulation was successful, false otherwise
     */
    virtual bool simulateMotion(double dt) = 0;

    /**
     * @brief Gets the constraint velocity (linear and angular)
     * @param linearVelocity Output parameter for linear velocity
     * @param angularVelocity Output parameter for angular velocity
     */
    virtual void getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const = 0;

    /**
     * @brief Sets the constraint velocity (linear and angular)
     * @param linearVelocity Linear velocity to set
     * @param angularVelocity Angular velocity to set
     * @return true if the velocity was successfully set, false otherwise
     */
    virtual bool setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity) = 0;

    /**
     * @brief Calculates the constraint forces and torques
     * @param force Output parameter for force
     * @param torque Output parameter for torque
     */
    virtual void calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const = 0;

protected:
    /**
     * @brief Protected constructor to prevent direct instantiation
     */
    AdvancedAssemblyConstraint() = default;
};

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
