#include "GearConstraint.h"
#include <algorithm>
#include <stdexcept>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

GearConstraint::GearConstraint(
    std::shared_ptr<Component> component1,
    std::shared_ptr<Component> component2,
    const Eigen::Vector3d& axis1,
    const Eigen::Vector3d& axis2,
    double ratio,
    double efficiency
)
    : m_component1(component1)
    , m_component2(component2)
    , m_axis1(axis1.normalized())
    , m_axis2(axis2.normalized())
    , m_ratio(ratio)
    , m_efficiency(efficiency)
    , m_angularVelocity(0.0)
    , m_angle1(0.0)
    , m_angle2(0.0)
{
    if (!validateRatio(ratio)) {
        throw std::invalid_argument("Invalid gear ratio");
    }
    
    if (!validateEfficiency(efficiency)) {
        throw std::invalid_argument("Invalid efficiency (must be between 0.0 and 1.0)");
    }
    
    if (!component1 || !component2) {
        throw std::invalid_argument("Components cannot be null");
    }
}

std::string GearConstraint::getName() const
{
    return "Gear Constraint";
}

std::vector<std::shared_ptr<Component>> GearConstraint::getComponents() const
{
    return { m_component1, m_component2 };
}

bool GearConstraint::isSatisfied() const
{
    return getError() < 1e-6;
}

bool GearConstraint::enforce()
{
    if (!isValid()) {
        return false;
    }
    
    double angle1, angle2;
    calculateAngles(angle1, angle2);
    
    // Calculate the expected angle of the second component based on the first component
    double expectedAngle2 = angle1 * m_ratio;
    
    // Calculate the error
    double error = std::fabs(angle2 - expectedAngle2);
    
    if (error < 1e-6) {
        // Already satisfied
        return true;
    }
    
    // Adjust the orientation of the second component to satisfy the constraint
    Eigen::Vector3d worldAxis1, worldAxis2;
    calculateWorldAxes(worldAxis1, worldAxis2);
    
    // Create a rotation to correct the angle
    Eigen::AngleAxisd correction(expectedAngle2 - angle2, worldAxis2);
    
    // Apply the correction to the second component's orientation
    Eigen::Quaterniond newOrientation = correction * m_component2->getOrientation();
    m_component2->setOrientation(newOrientation);
    
    return true;
}

double GearConstraint::getError() const
{
    if (!isValid()) {
        return std::numeric_limits<double>::max();
    }
    
    double angle1, angle2;
    calculateAngles(angle1, angle2);
    
    // Calculate the expected angle of the second component based on the first component
    double expectedAngle2 = angle1 * m_ratio;
    
    // Calculate the error
    return std::fabs(angle2 - expectedAngle2);
}

bool GearConstraint::isValid() const
{
    return m_component1 && m_component2 && validateRatio(m_ratio) && validateEfficiency(m_efficiency);
}

int GearConstraint::getConstrainedDegreesOfFreedom() const
{
    return 1; // A gear constraint removes one rotational degree of freedom
}

bool GearConstraint::simulateMotion(double dt)
{
    if (!isValid()) {
        return false;
    }
    
    // Update the angles based on the current angular velocity
    m_angle1 += m_angularVelocity * dt;
    m_angle2 += m_angularVelocity * m_ratio * dt;
    
    // Create rotations for both components
    Eigen::Vector3d worldAxis1, worldAxis2;
    calculateWorldAxes(worldAxis1, worldAxis2);
    
    Eigen::AngleAxisd rotation1(m_angle1, worldAxis1);
    Eigen::AngleAxisd rotation2(m_angle2, worldAxis2);
    
    // Apply the rotations to the components
    Eigen::Quaterniond orientation1 = rotation1 * m_component1->getOrientation();
    Eigen::Quaterniond orientation2 = rotation2 * m_component2->getOrientation();
    
    m_component1->setOrientation(orientation1);
    m_component2->setOrientation(orientation2);
    
    return true;
}

void GearConstraint::getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const
{
    // Gear constraints don't have linear velocity
    linearVelocity = Eigen::Vector3d::Zero();
    
    // Angular velocity is along the rotation axis
    Eigen::Vector3d worldAxis1, worldAxis2;
    calculateWorldAxes(worldAxis1, worldAxis2);
    
    angularVelocity = worldAxis1 * m_angularVelocity;
}

bool GearConstraint::setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity)
{
    // Gear constraints don't have linear velocity, so we ignore it
    
    // Project the angular velocity onto the rotation axis
    Eigen::Vector3d worldAxis1, worldAxis2;
    calculateWorldAxes(worldAxis1, worldAxis2);
    
    m_angularVelocity = angularVelocity.dot(worldAxis1);
    
    return true;
}

void GearConstraint::calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const
{
    // Gear constraints don't exert linear forces
    force = Eigen::Vector3d::Zero();
    
    // Calculate the torque based on the angular velocity and efficiency
    Eigen::Vector3d worldAxis1, worldAxis2;
    calculateWorldAxes(worldAxis1, worldAxis2);
    
    // For simplicity, we assume a constant torque proportional to the angular velocity
    double torqueMagnitude = m_angularVelocity * 10.0 * m_efficiency; // Arbitrary scaling factor
    
    torque = worldAxis1 * torqueMagnitude;
}

double GearConstraint::getRatio() const
{
    return m_ratio;
}

void GearConstraint::setRatio(double ratio)
{
    if (validateRatio(ratio)) {
        m_ratio = ratio;
    } else {
        throw std::invalid_argument("Invalid gear ratio");
    }
}

double GearConstraint::getEfficiency() const
{
    return m_efficiency;
}

void GearConstraint::setEfficiency(double efficiency)
{
    if (validateEfficiency(efficiency)) {
        m_efficiency = efficiency;
    } else {
        throw std::invalid_argument("Invalid efficiency (must be between 0.0 and 1.0)");
    }
}

void GearConstraint::getAxes(Eigen::Vector3d& axis1, Eigen::Vector3d& axis2) const
{
    axis1 = m_axis1;
    axis2 = m_axis2;
}

void GearConstraint::setAxes(const Eigen::Vector3d& axis1, const Eigen::Vector3d& axis2)
{
    m_axis1 = axis1.normalized();
    m_axis2 = axis2.normalized();
}

void GearConstraint::calculateWorldAxes(Eigen::Vector3d& worldAxis1, Eigen::Vector3d& worldAxis2) const
{
    // Transform the local axes to world space using the component orientations
    worldAxis1 = m_component1->getOrientation() * m_axis1;
    worldAxis2 = m_component2->getOrientation() * m_axis2;
}

void GearConstraint::calculateAngles(double& angle1, double& angle2) const
{
    // For simplicity, we use the current angles stored in the constraint
    // In a real implementation, these would be calculated from the component orientations
    angle1 = m_angle1;
    angle2 = m_angle2;
}

bool GearConstraint::validateRatio(double ratio) const
{
    // Gear ratio should be non-zero
    return ratio != 0.0;
}

bool GearConstraint::validateEfficiency(double efficiency) const
{
    // Efficiency should be between 0.0 and 1.0
    return efficiency >= 0.0 && efficiency <= 1.0;
}

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
