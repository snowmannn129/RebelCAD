#pragma once

#include "AdvancedAssemblyConstraint.h"
#include "../../../external/eigen/Eigen/Geometry"
#include "../../Component.h"
#include <cmath>
#include <limits>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

/**
 * @brief Represents a gear-rack constraint between two components
 * 
 * The GearRackConstraint ensures that a gear component rotates as it moves along a rack component,
 * with the rotation angle proportional to the linear displacement.
 * It can be used to model gear-rack mechanisms, linear actuators with rotary output, etc.
 */
class GearRackConstraint : public AdvancedAssemblyConstraint {
public:
    /**
     * @brief Constructs a gear-rack constraint between two components
     * @param gear The gear component
     * @param rack The rack component
     * @param gearAxis Rotation axis of the gear in local coordinates
     * @param rackAxis Direction of the rack in local coordinates
     * @param gearRadius Radius of the gear (pitch radius)
     * @param efficiency Mechanical efficiency of the gear-rack pair (0.0-1.0)
     */
    GearRackConstraint(
        std::shared_ptr<Component> gear,
        std::shared_ptr<Component> rack,
        const Eigen::Vector3d& gearAxis,
        const Eigen::Vector3d& rackAxis,
        double gearRadius,
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
     * @return Number of degrees of freedom removed (1 for a gear-rack constraint)
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
     * @brief Gets the gear radius
     * @return The gear radius
     */
    double getGearRadius() const;

    /**
     * @brief Sets the gear radius
     * @param radius The new gear radius
     */
    void setGearRadius(double radius);

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
     * @brief Gets the axes
     * @param gearAxis Output parameter for the gear rotation axis
     * @param rackAxis Output parameter for the rack direction
     */
    void getAxes(Eigen::Vector3d& gearAxis, Eigen::Vector3d& rackAxis) const;

    /**
     * @brief Sets the axes
     * @param gearAxis The new gear rotation axis
     * @param rackAxis The new rack direction
     */
    void setAxes(const Eigen::Vector3d& gearAxis, const Eigen::Vector3d& rackAxis);

    /**
     * @brief Gets the current gear angle
     * @return The current gear angle in radians
     */
    double getGearAngle() const;

    /**
     * @brief Sets the gear angle
     * @param angle The new gear angle in radians
     * @return true if the angle was successfully set, false otherwise
     */
    bool setGearAngle(double angle);

    /**
     * @brief Gets the current rack position
     * @return The current rack position along the rack axis
     */
    double getRackPosition() const;

    /**
     * @brief Sets the rack position
     * @param position The new rack position along the rack axis
     * @return true if the position was successfully set, false otherwise
     */
    bool setRackPosition(double position);

private:
    std::shared_ptr<Component> m_gear; // Gear component
    std::shared_ptr<Component> m_rack; // Rack component
    Eigen::Vector3d m_gearAxis; // Rotation axis of the gear in local coordinates
    Eigen::Vector3d m_rackAxis; // Direction of the rack in local coordinates
    double m_gearRadius; // Radius of the gear (pitch radius)
    double m_efficiency; // Mechanical efficiency of the gear-rack pair (0.0-1.0)
    double m_linearVelocity; // Current linear velocity of the rack
    double m_angularVelocity; // Current angular velocity of the gear
    double m_gearAngle; // Current rotation angle of the gear
    double m_rackPosition; // Current position of the rack along the rack axis
    Eigen::Vector3d m_initialGearPosition; // Initial position of the gear
    Eigen::Vector3d m_initialRackPosition; // Initial position of the rack

    /**
     * @brief Calculates the world-space axes
     * @param worldGearAxis Output parameter for the gear rotation axis in world coordinates
     * @param worldRackAxis Output parameter for the rack direction in world coordinates
     */
    void calculateWorldAxes(Eigen::Vector3d& worldGearAxis, Eigen::Vector3d& worldRackAxis) const;

    /**
     * @brief Calculates the current gear angle and rack position
     * @param gearAngle Output parameter for the gear angle
     * @param rackPosition Output parameter for the rack position
     */
    void calculateCurrentState(double& gearAngle, double& rackPosition) const;

    /**
     * @brief Validates the gear radius
     * @param radius The gear radius to validate
     * @return true if the radius is valid, false otherwise
     */
    bool validateGearRadius(double radius) const;

    /**
     * @brief Validates the efficiency
     * @param efficiency The efficiency to validate
     * @return true if the efficiency is valid, false otherwise
     */
    bool validateEfficiency(double efficiency) const;

    /**
     * @brief Calculates the rack position from the gear angle
     * @param gearAngle The gear angle in radians
     * @return The corresponding rack position
     */
    double calculateRackPositionFromGearAngle(double gearAngle) const;

    /**
     * @brief Calculates the gear angle from the rack position
     * @param rackPosition The rack position
     * @return The corresponding gear angle in radians
     */
    double calculateGearAngleFromRackPosition(double rackPosition) const;
};

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
