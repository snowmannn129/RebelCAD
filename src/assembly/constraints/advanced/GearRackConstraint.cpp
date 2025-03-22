#include "GearRackConstraint.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

GearRackConstraint::GearRackConstraint(
    std::shared_ptr<Component> gear,
    std::shared_ptr<Component> rack,
    const Eigen::Vector3d& gearAxis,
    const Eigen::Vector3d& rackAxis,
    double gearRadius,
    double efficiency
)
    : m_gear(gear)
    , m_rack(rack)
    , m_gearAxis(gearAxis.normalized())
    , m_rackAxis(rackAxis.normalized())
    , m_gearRadius(gearRadius)
    , m_efficiency(efficiency)
    , m_linearVelocity(0.0)
    , m_angularVelocity(0.0)
    , m_gearAngle(0.0)
    , m_rackPosition(0.0)
    , m_initialGearPosition(gear ? gear->getPosition() : Eigen::Vector3d::Zero())
    , m_initialRackPosition(rack ? rack->getPosition() : Eigen::Vector3d::Zero())
{
    if (!validateGearRadius(gearRadius)) {
        throw std::invalid_argument("Invalid gear radius (must be positive)");
    }
    
    if (!validateEfficiency(efficiency)) {
        throw std::invalid_argument("Invalid efficiency (must be between 0.0 and 1.0)");
    }
    
    if (!gear || !rack) {
        throw std::invalid_argument("Components cannot be null");
    }
}

std::string GearRackConstraint::getName() const
{
    return "Gear-Rack Constraint";
}

std::vector<std::shared_ptr<Component>> GearRackConstraint::getComponents() const
{
    return { m_gear, m_rack };
}

bool GearRackConstraint::isSatisfied() const
{
    return getError() < 1e-6;
}

bool GearRackConstraint::enforce()
{
    if (!isValid()) {
        return false;
    }
    
    // Calculate the current gear angle and rack position
    double gearAngle, rackPosition;
    calculateCurrentState(gearAngle, rackPosition);
    
    // Calculate the expected rack position based on the gear angle
    double expectedRackPosition = calculateRackPositionFromGearAngle(gearAngle);
    
    // Calculate the expected gear angle based on the rack position
    double expectedGearAngle = calculateGearAngleFromRackPosition(rackPosition);
    
    // Calculate the errors
    double rackError = std::fabs(rackPosition - expectedRackPosition);
    double gearError = std::fabs(gearAngle - expectedGearAngle);
    
    if (rackError < 1e-6 && gearError < 1e-6) {
        // Already satisfied
        return true;
    }
    
    // Decide which component to adjust based on the smaller error
    if (rackError <= gearError) {
        // Adjust the rack position to match the gear angle
        Eigen::Vector3d worldGearAxis, worldRackAxis;
        calculateWorldAxes(worldGearAxis, worldRackAxis);
        
        // Calculate the new rack position
        Eigen::Vector3d gearPosition = m_gear->getPosition();
        Eigen::Vector3d newRackPosition = gearPosition + worldRackAxis * expectedRackPosition;
        
        // Update the rack position
        m_rack->setPosition(newRackPosition);
    } else {
        // Adjust the gear angle to match the rack position
        Eigen::Vector3d worldGearAxis, worldRackAxis;
        calculateWorldAxes(worldGearAxis, worldRackAxis);
        
        // Create a rotation to correct the angle
        Eigen::AngleAxisd correction(expectedGearAngle - gearAngle, worldGearAxis);
        
        // Apply the correction to the gear's orientation
        Eigen::Quaterniond newOrientation = correction * m_gear->getOrientation();
        m_gear->setOrientation(newOrientation);
    }
    
    return true;
}

double GearRackConstraint::getError() const
{
    if (!isValid()) {
        return std::numeric_limits<double>::max();
    }
    
    // Calculate the current gear angle and rack position
    double gearAngle, rackPosition;
    calculateCurrentState(gearAngle, rackPosition);
    
    // Calculate the expected rack position based on the gear angle
    double expectedRackPosition = calculateRackPositionFromGearAngle(gearAngle);
    
    // Calculate the error
    return std::fabs(rackPosition - expectedRackPosition);
}

bool GearRackConstraint::isValid() const
{
    return m_gear && m_rack && validateGearRadius(m_gearRadius) && validateEfficiency(m_efficiency);
}

int GearRackConstraint::getConstrainedDegreesOfFreedom() const
{
    return 1; // A gear-rack constraint removes one degree of freedom
}

bool GearRackConstraint::simulateMotion(double dt)
{
    if (!isValid()) {
        return false;
    }
    
    // Update the gear angle and rack position based on the current velocities
    m_gearAngle += m_angularVelocity * dt;
    m_rackPosition += m_linearVelocity * dt;
    
    // Calculate the world-space axes
    Eigen::Vector3d worldGearAxis, worldRackAxis;
    calculateWorldAxes(worldGearAxis, worldRackAxis);
    
    // Update the gear orientation
    Eigen::AngleAxisd rotation(m_gearAngle, worldGearAxis);
    m_gear->setOrientation(rotation);
    
    // Update the rack position
    Eigen::Vector3d newRackPosition = m_initialRackPosition + worldRackAxis * m_rackPosition;
    m_rack->setPosition(newRackPosition);
    
    return true;
}

void GearRackConstraint::getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const
{
    // Calculate the world-space axes
    Eigen::Vector3d worldGearAxis, worldRackAxis;
    calculateWorldAxes(worldGearAxis, worldRackAxis);
    
    // Linear velocity is along the rack axis
    linearVelocity = worldRackAxis * m_linearVelocity;
    
    // Angular velocity is along the gear axis
    angularVelocity = worldGearAxis * m_angularVelocity;
}

bool GearRackConstraint::setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity)
{
    // Calculate the world-space axes
    Eigen::Vector3d worldGearAxis, worldRackAxis;
    calculateWorldAxes(worldGearAxis, worldRackAxis);
    
    // Project the linear velocity onto the rack axis
    m_linearVelocity = linearVelocity.dot(worldRackAxis);
    
    // Project the angular velocity onto the gear axis
    m_angularVelocity = angularVelocity.dot(worldGearAxis);
    
    // Ensure the linear and angular velocities are consistent with the gear-rack relationship
    // Linear velocity = angular velocity * gear radius
    if (std::fabs(m_linearVelocity - m_angularVelocity * m_gearRadius) > 1e-6) {
        // If they're inconsistent, prioritize the angular velocity
        m_linearVelocity = m_angularVelocity * m_gearRadius;
    }
    
    return true;
}

void GearRackConstraint::calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const
{
    // Calculate the world-space axes
    Eigen::Vector3d worldGearAxis, worldRackAxis;
    calculateWorldAxes(worldGearAxis, worldRackAxis);
    
    // For simplicity, we assume a constant force proportional to the linear velocity
    double forceMagnitude = m_linearVelocity * 10.0 * m_efficiency; // Arbitrary scaling factor
    
    // Force is along the rack axis
    force = worldRackAxis * forceMagnitude;
    
    // Torque is perpendicular to both the gear axis and the force
    // For a gear-rack, the torque is around the gear axis, proportional to the force and the gear radius
    double torqueMagnitude = forceMagnitude * m_gearRadius;
    torque = worldGearAxis * torqueMagnitude;
}

double GearRackConstraint::getGearRadius() const
{
    return m_gearRadius;
}

void GearRackConstraint::setGearRadius(double radius)
{
    if (validateGearRadius(radius)) {
        m_gearRadius = radius;
    } else {
        throw std::invalid_argument("Invalid gear radius (must be positive)");
    }
}

double GearRackConstraint::getEfficiency() const
{
    return m_efficiency;
}

void GearRackConstraint::setEfficiency(double efficiency)
{
    if (validateEfficiency(efficiency)) {
        m_efficiency = efficiency;
    } else {
        throw std::invalid_argument("Invalid efficiency (must be between 0.0 and 1.0)");
    }
}

void GearRackConstraint::getAxes(Eigen::Vector3d& gearAxis, Eigen::Vector3d& rackAxis) const
{
    gearAxis = m_gearAxis;
    rackAxis = m_rackAxis;
}

void GearRackConstraint::setAxes(const Eigen::Vector3d& gearAxis, const Eigen::Vector3d& rackAxis)
{
    m_gearAxis = gearAxis.normalized();
    m_rackAxis = rackAxis.normalized();
}

double GearRackConstraint::getGearAngle() const
{
    return m_gearAngle;
}

bool GearRackConstraint::setGearAngle(double angle)
{
    m_gearAngle = angle;
    
    // Update the rack position to match the gear angle
    m_rackPosition = calculateRackPositionFromGearAngle(angle);
    
    return enforce();
}

double GearRackConstraint::getRackPosition() const
{
    return m_rackPosition;
}

bool GearRackConstraint::setRackPosition(double position)
{
    m_rackPosition = position;
    
    // Update the gear angle to match the rack position
    m_gearAngle = calculateGearAngleFromRackPosition(position);
    
    return enforce();
}

void GearRackConstraint::calculateWorldAxes(Eigen::Vector3d& worldGearAxis, Eigen::Vector3d& worldRackAxis) const
{
    // Transform the local axes to world space using the component orientations
    worldGearAxis = m_gear->getOrientation() * m_gearAxis;
    worldRackAxis = m_rack->getOrientation() * m_rackAxis;
}

void GearRackConstraint::calculateCurrentState(double& gearAngle, double& rackPosition) const
{
    // For simplicity, we use the current angles and positions stored in the constraint
    // In a real implementation, these would be calculated from the component orientations and positions
    gearAngle = m_gearAngle;
    
    // Calculate the rack position as the projection of the relative position onto the rack axis
    Eigen::Vector3d worldGearAxis, worldRackAxis;
    calculateWorldAxes(worldGearAxis, worldRackAxis);
    
    Eigen::Vector3d gearPosition = m_gear->getPosition();
    Eigen::Vector3d rackPosition = m_rack->getPosition();
    Eigen::Vector3d relativePosition = rackPosition - gearPosition;
    
    rackPosition = relativePosition.dot(worldRackAxis);
}

bool GearRackConstraint::validateGearRadius(double radius) const
{
    // Gear radius should be positive
    return radius > 0.0;
}

bool GearRackConstraint::validateEfficiency(double efficiency) const
{
    // Efficiency should be between 0.0 and 1.0
    return efficiency >= 0.0 && efficiency <= 1.0;
}

double GearRackConstraint::calculateRackPositionFromGearAngle(double gearAngle) const
{
    // For a gear-rack, the rack position is proportional to the gear angle
    // Rack position = gear angle * gear radius
    return gearAngle * m_gearRadius;
}

double GearRackConstraint::calculateGearAngleFromRackPosition(double rackPosition) const
{
    // For a gear-rack, the gear angle is proportional to the rack position
    // Gear angle = rack position / gear radius
    return rackPosition / m_gearRadius;
}

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
