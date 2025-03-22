#include "CamConstraint.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

CamConstraint::CamConstraint(
    std::shared_ptr<Component> cam,
    std::shared_ptr<Component> follower,
    const Eigen::Vector3d& camAxis,
    const Eigen::Vector3d& followerAxis,
    ProfileFunction profileFunction,
    double offset
)
    : m_cam(cam)
    , m_follower(follower)
    , m_camAxis(camAxis.normalized())
    , m_followerAxis(followerAxis.normalized())
    , m_profileFunction(profileFunction)
    , m_offset(offset)
    , m_camAngle(0.0)
    , m_angularVelocity(0.0)
{
    if (!cam || !follower) {
        throw std::invalid_argument("Components cannot be null");
    }
}

CamConstraint::CamConstraint(
    std::shared_ptr<Component> cam,
    std::shared_ptr<Component> follower,
    const Eigen::Vector3d& camAxis,
    const Eigen::Vector3d& followerAxis,
    double radius,
    double offset
)
    : CamConstraint(cam, follower, camAxis, followerAxis, createCircularProfile(radius), offset)
{
}

CamConstraint::CamConstraint(
    std::shared_ptr<Component> cam,
    std::shared_ptr<Component> follower,
    const Eigen::Vector3d& camAxis,
    const Eigen::Vector3d& followerAxis,
    const std::vector<std::pair<double, double>>& angleRadiusPairs,
    double offset
)
    : CamConstraint(cam, follower, camAxis, followerAxis, createPiecewiseLinearProfile(angleRadiusPairs), offset)
{
}

std::string CamConstraint::getName() const
{
    return "Cam Constraint";
}

std::vector<std::shared_ptr<Component>> CamConstraint::getComponents() const
{
    return { m_cam, m_follower };
}

bool CamConstraint::isSatisfied() const
{
    return getError() < 1e-6;
}

bool CamConstraint::enforce()
{
    if (!isValid()) {
        return false;
    }
    
    // Calculate the expected follower position based on the current cam angle
    double expectedPosition = calculateFollowerPosition(m_camAngle);
    
    // Calculate the world-space follower axis
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Calculate the current follower position along the follower axis
    Eigen::Vector3d camPosition = m_cam->getPosition();
    Eigen::Vector3d followerPosition = m_follower->getPosition();
    Eigen::Vector3d relativePosition = followerPosition - camPosition;
    double currentPosition = relativePosition.dot(worldFollowerAxis);
    
    // Calculate the error
    double error = std::fabs(currentPosition - expectedPosition);
    
    if (error < 1e-6) {
        // Already satisfied
        return true;
    }
    
    // Adjust the follower position to satisfy the constraint
    Eigen::Vector3d newPosition = camPosition + worldFollowerAxis * expectedPosition;
    m_follower->setPosition(newPosition);
    
    return true;
}

double CamConstraint::getError() const
{
    if (!isValid()) {
        return std::numeric_limits<double>::max();
    }
    
    // Calculate the expected follower position based on the current cam angle
    double expectedPosition = calculateFollowerPosition(m_camAngle);
    
    // Calculate the world-space follower axis
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Calculate the current follower position along the follower axis
    Eigen::Vector3d camPosition = m_cam->getPosition();
    Eigen::Vector3d followerPosition = m_follower->getPosition();
    Eigen::Vector3d relativePosition = followerPosition - camPosition;
    double currentPosition = relativePosition.dot(worldFollowerAxis);
    
    // Calculate the error
    return std::fabs(currentPosition - expectedPosition);
}

bool CamConstraint::isValid() const
{
    return m_cam && m_follower && m_profileFunction != nullptr;
}

int CamConstraint::getConstrainedDegreesOfFreedom() const
{
    return 1; // A cam constraint removes one translational degree of freedom
}

bool CamConstraint::simulateMotion(double dt)
{
    if (!isValid()) {
        return false;
    }
    
    // Update the cam angle based on the current angular velocity
    m_camAngle = normalizeAngle(m_camAngle + m_angularVelocity * dt);
    
    // Calculate the new follower position
    double newPosition = calculateFollowerPosition(m_camAngle);
    
    // Calculate the world-space follower axis
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Update the follower position
    Eigen::Vector3d camPosition = m_cam->getPosition();
    Eigen::Vector3d newFollowerPosition = camPosition + worldFollowerAxis * newPosition;
    m_follower->setPosition(newFollowerPosition);
    
    return true;
}

void CamConstraint::getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const
{
    // Calculate the world-space axes
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Calculate the derivative of the profile function at the current angle
    // For simplicity, we use a numerical approximation
    double epsilon = 1e-6;
    double r1 = m_profileFunction(normalizeAngle(m_camAngle - epsilon));
    double r2 = m_profileFunction(normalizeAngle(m_camAngle + epsilon));
    double derivative = (r2 - r1) / (2 * epsilon);
    
    // Linear velocity is along the follower axis, proportional to the angular velocity and the derivative
    linearVelocity = worldFollowerAxis * (m_angularVelocity * derivative);
    
    // Angular velocity is along the cam axis
    angularVelocity = worldCamAxis * m_angularVelocity;
}

bool CamConstraint::setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity)
{
    // Calculate the world-space cam axis
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Project the angular velocity onto the cam axis
    m_angularVelocity = angularVelocity.dot(worldCamAxis);
    
    return true;
}

void CamConstraint::calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const
{
    // Calculate the world-space axes
    Eigen::Vector3d worldCamAxis, worldFollowerAxis;
    calculateWorldAxes(worldCamAxis, worldFollowerAxis);
    
    // Calculate the derivative of the profile function at the current angle
    // For simplicity, we use a numerical approximation
    double epsilon = 1e-6;
    double r1 = m_profileFunction(normalizeAngle(m_camAngle - epsilon));
    double r2 = m_profileFunction(normalizeAngle(m_camAngle + epsilon));
    double derivative = (r2 - r1) / (2 * epsilon);
    
    // Force is along the follower axis, proportional to the angular velocity and the derivative
    double forceMagnitude = m_angularVelocity * derivative * 10.0; // Arbitrary scaling factor
    force = worldFollowerAxis * forceMagnitude;
    
    // Torque is along the cam axis, proportional to the force and the radius
    double radius = m_profileFunction(m_camAngle);
    double torqueMagnitude = forceMagnitude * radius;
    torque = worldCamAxis * torqueMagnitude;
}

double CamConstraint::getCamAngle() const
{
    return m_camAngle;
}

bool CamConstraint::setCamAngle(double angle)
{
    m_camAngle = normalizeAngle(angle);
    return enforce();
}

CamConstraint::ProfileFunction CamConstraint::getProfileFunction() const
{
    return m_profileFunction;
}

void CamConstraint::setProfileFunction(ProfileFunction profileFunction)
{
    if (profileFunction) {
        m_profileFunction = profileFunction;
    } else {
        throw std::invalid_argument("Profile function cannot be null");
    }
}

double CamConstraint::getOffset() const
{
    return m_offset;
}

void CamConstraint::setOffset(double offset)
{
    m_offset = offset;
}

void CamConstraint::getAxes(Eigen::Vector3d& camAxis, Eigen::Vector3d& followerAxis) const
{
    camAxis = m_camAxis;
    followerAxis = m_followerAxis;
}

void CamConstraint::setAxes(const Eigen::Vector3d& camAxis, const Eigen::Vector3d& followerAxis)
{
    m_camAxis = camAxis.normalized();
    m_followerAxis = followerAxis.normalized();
}

void CamConstraint::calculateWorldAxes(Eigen::Vector3d& worldCamAxis, Eigen::Vector3d& worldFollowerAxis) const
{
    // Transform the local axes to world space using the component orientations
    worldCamAxis = m_cam->getOrientation() * m_camAxis;
    worldFollowerAxis = m_follower->getOrientation() * m_followerAxis;
}

double CamConstraint::calculateFollowerPosition(double camAngle) const
{
    // Get the radius at the current angle
    double radius = m_profileFunction(normalizeAngle(camAngle));
    
    // Add the offset
    return radius + m_offset;
}

CamConstraint::ProfileFunction CamConstraint::createCircularProfile(double radius)
{
    if (radius <= 0.0) {
        throw std::invalid_argument("Radius must be positive");
    }
    
    // A circular cam has a constant radius regardless of angle
    return [radius](double angle) {
        return radius;
    };
}

CamConstraint::ProfileFunction CamConstraint::createPiecewiseLinearProfile(const std::vector<std::pair<double, double>>& angleRadiusPairs)
{
    if (angleRadiusPairs.size() < 2) {
        throw std::invalid_argument("At least two angle-radius pairs are required");
    }
    
    // Create a copy of the pairs and sort by angle
    auto pairs = angleRadiusPairs;
    std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    
    // Ensure all angles are normalized
    for (auto& pair : pairs) {
        pair.first = normalizeAngle(pair.first);
    }
    
    // Create a piecewise linear interpolation function
    return [pairs](double angle) {
        angle = normalizeAngle(angle);
        
        // Find the two points to interpolate between
        auto it = std::lower_bound(pairs.begin(), pairs.end(), angle,
            [](const auto& pair, double value) {
                return pair.first < value;
            });
        
        if (it == pairs.begin()) {
            // Angle is before the first point, interpolate between last and first
            auto last = pairs.back();
            auto first = pairs.front();
            
            // Handle wrap-around
            if (angle > last.first) {
                // Interpolate between last and first (with 2π added)
                double t = (angle - last.first) / (2 * M_PI + first.first - last.first);
                return last.second + t * (first.second - last.second);
            } else {
                // Angle is exactly at or slightly after the first point
                return first.second;
            }
        } else if (it == pairs.end()) {
            // Angle is after the last point, interpolate between last and first
            auto last = pairs.back();
            auto first = pairs.front();
            
            // Handle wrap-around
            double t = (angle - last.first) / (2 * M_PI + first.first - last.first);
            return last.second + t * (first.second - last.second);
        } else {
            // Interpolate between two points
            auto next = *it;
            auto prev = *(it - 1);
            
            double t = (angle - prev.first) / (next.first - prev.first);
            return prev.second + t * (next.second - prev.second);
        }
    };
}

double CamConstraint::normalizeAngle(double angle)
{
    // Normalize angle to [0, 2π)
    const double twoPi = 2 * M_PI;
    return angle - twoPi * std::floor(angle / twoPi);
}

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
