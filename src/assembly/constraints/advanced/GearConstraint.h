#pragma once

#include "AdvancedAssemblyConstraint.h"
#include <cmath>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

/**
 * @brief Represents a gear constraint between two components
 * 
 * The GearConstraint ensures that two components rotate together with a specified gear ratio.
 * It can be used to model gear pairs, belt drives, chain drives, etc.
 */
class GearConstraint : public AdvancedAssemblyConstraint {
public:
    /**
     * @brief Constructs a gear constraint between two components
     * @param component1 First component (driving gear)
     * @param component2 Second component (driven gear)
     * @param axis1 Rotation axis of the first component in local coordinates
     * @param axis2 Rotation axis of the second component in local coordinates
     * @param ratio Gear ratio (ratio of angular velocities: omega2 = ratio * omega1)
     * @param efficiency Mechanical efficiency of the gear pair (0.0-1.0)
     */
    GearConstraint(
        std::shared_ptr<Component> component1,
        std::shared_ptr<Component> component2,
        const Eigen::Vector3d& axis1,
        const Eigen::Vector3d& axis2,
        double ratio,
        double efficiency = 0.98
    );

    /**
     * @brief Gets the name of the constraint
     * @return The constraint name
     */
    std::string getName() const override;

    /**
     * @brief Gets the components involved in this constraint
     * @return Vector of components involved in the constraint
     */
    std::vector<std::shared_ptr<Component>> getComponents() const override;

    /**
     * @brief Evaluates if the constraint is currently satisfied
     * @return true if constraint conditions are met, false otherwise
     */
    bool isSatisfied() const override;

    /**
     * @brief Attempts to enforce the constraint
     * @return true if constraint was successfully enforced, false if it failed
     */
    bool enforce() override;

    /**
     * @brief Gets the error magnitude of the constraint violation
     * @return A value indicating how far from satisfaction the constraint is
     */
    double getError() const override;

    /**
     * @brief Checks if the constraint is valid and can be enforced
     * @return true if the constraint is valid, false otherwise
     */
    bool isValid() const override;

    /**
     * @brief Gets the degrees of freedom removed by this constraint
     * @return Number of degrees of freedom removed (1 for a gear constraint)
     */
    int getConstrainedDegreesOfFreedom() const override;

    /**
     * @brief Simulates the constraint motion for a given time step
     * @param dt Time step in seconds
     * @return true if the simulation was successful, false otherwise
     */
    bool simulateMotion(double dt) override;

    /**
     * @brief Gets the constraint velocity (linear and angular)
     * @param linearVelocity Output parameter for linear velocity
     * @param angularVelocity Output parameter for angular velocity
     */
    void getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const override;

    /**
     * @brief Sets the constraint velocity (linear and angular)
     * @param linearVelocity Linear velocity to set
     * @param angularVelocity Angular velocity to set
     * @return true if the velocity was successfully set, false otherwise
     */
    bool setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity) override;

    /**
     * @brief Calculates the constraint forces and torques
     * @param force Output parameter for force
     * @param torque Output parameter for torque
     */
    void calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const override;

    /**
     * @brief Gets the gear ratio
     * @return The gear ratio
     */
    double getRatio() const;

    /**
     * @brief Sets the gear ratio
     * @param ratio The new gear ratio
     */
    void setRatio(double ratio);

    /**
     * @brief Gets the mechanical efficiency
     * @return The mechanical efficiency (0.0-1.0)
     */
    double getEfficiency() const;

    /**
     * @brief Sets the mechanical efficiency
     * @param efficiency The new mechanical efficiency (0.0-1.0)
     */
    void setEfficiency(double efficiency);

    /**
     * @brief Gets the rotation axes
     * @param axis1 Output parameter for the first rotation axis
     * @param axis2 Output parameter for the second rotation axis
     */
    void getAxes(Eigen::Vector3d& axis1, Eigen::Vector3d& axis2) const;

    /**
     * @brief Sets the rotation axes
     * @param axis1 The new first rotation axis
     * @param axis2 The new second rotation axis
     */
    void setAxes(const Eigen::Vector3d& axis1, const Eigen::Vector3d& axis2);

private:
    std::shared_ptr<Component> m_component1; // Driving gear
    std::shared_ptr<Component> m_component2; // Driven gear
    Eigen::Vector3d m_axis1; // Rotation axis of the first component in local coordinates
    Eigen::Vector3d m_axis2; // Rotation axis of the second component in local coordinates
    double m_ratio; // Gear ratio (ratio of angular velocities: omega2 = ratio * omega1)
    double m_efficiency; // Mechanical efficiency of the gear pair (0.0-1.0)
    double m_angularVelocity; // Current angular velocity of the driving gear
    double m_angle1; // Current rotation angle of the first component
    double m_angle2; // Current rotation angle of the second component

    /**
     * @brief Calculates the world-space rotation axes
     * @param worldAxis1 Output parameter for the first rotation axis in world coordinates
     * @param worldAxis2 Output parameter for the second rotation axis in world coordinates
     */
    void calculateWorldAxes(Eigen::Vector3d& worldAxis1, Eigen::Vector3d& worldAxis2) const;

    /**
     * @brief Calculates the current rotation angles
     * @param angle1 Output parameter for the first rotation angle
     * @param angle2 Output parameter for the second rotation angle
     */
    void calculateAngles(double& angle1, double& angle2) const;

    /**
     * @brief Validates the gear ratio
     * @param ratio The gear ratio to validate
     * @return true if the ratio is valid, false otherwise
     */
    bool validateRatio(double ratio) const;

    /**
     * @brief Validates the efficiency
     * @param efficiency The efficiency to validate
     * @return true if the efficiency is valid, false otherwise
     */
    bool validateEfficiency(double efficiency) const;
};

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
