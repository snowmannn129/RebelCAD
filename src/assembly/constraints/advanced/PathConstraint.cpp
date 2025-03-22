#include "PathConstraint.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

PathConstraint::PathConstraint(
    std::shared_ptr<Component> component,
    PathFunction pathFunction,
    double initialParameter,
    const Eigen::Vector3d& localOrigin
)
    : m_component(component)
    , m_pathFunction(pathFunction)
    , m_parameter(clampParameter(initialParameter))
    , m_localOrigin(localOrigin)
    , m_velocity(0.0)
    , m_pathLength(calculatePathLength())
{
    if (!component) {
        throw std::invalid_argument("Component cannot be null");
    }
    if (!pathFunction) {
        throw std::invalid_argument("Path function cannot be null");
    }
}

PathConstraint::PathConstraint(
    std::shared_ptr<Component> component,
    const Eigen::Vector3d& startPoint,
    const Eigen::Vector3d& endPoint,
    double initialParameter,
    const Eigen::Vector3d& localOrigin
)
    : PathConstraint(component, createLinearPath(startPoint, endPoint), initialParameter, localOrigin)
{
}

PathConstraint::PathConstraint(
    std::shared_ptr<Component> component,
    const std::vector<Eigen::Vector3d>& points,
    double initialParameter,
    const Eigen::Vector3d& localOrigin
)
    : PathConstraint(component, createPiecewiseLinearPath(points), initialParameter, localOrigin)
{
    if (points.size() < 2) {
        throw std::invalid_argument("At least two points are required for a path");
    }
}

std::string PathConstraint::getName() const
{
    return "Path Constraint";
}

std::vector<std::shared_ptr<Component>> PathConstraint::getComponents() const
{
    return { m_component };
}

bool PathConstraint::isSatisfied() const
{
    return getError() < 1e-6;
}

bool PathConstraint::enforce()
{
    if (!isValid()) {
        return false;
    }
    
    // Calculate the expected position on the path
    Eigen::Vector3d pathPosition = getCurrentPathPosition();
    
    // Calculate the world-space origin point on the component
    Eigen::Vector3d worldOrigin = calculateWorldOrigin();
    
    // Calculate the error vector
    Eigen::Vector3d error = worldOrigin - pathPosition;
    
    if (error.norm() < 1e-6) {
        // Already satisfied
        return true;
    }
    
    // Adjust the component position to satisfy the constraint
    Eigen::Vector3d currentPosition = m_component->getPosition();
    Eigen::Vector3d newPosition = currentPosition - error;
    m_component->setPosition(newPosition);
    
    return true;
}

double PathConstraint::getError() const
{
    if (!isValid()) {
        return std::numeric_limits<double>::max();
    }
    
    // Calculate the expected position on the path
    Eigen::Vector3d pathPosition = getCurrentPathPosition();
    
    // Calculate the world-space origin point on the component
    Eigen::Vector3d worldOrigin = calculateWorldOrigin();
    
    // Calculate the error as the distance between the origin and the path
    return (worldOrigin - pathPosition).norm();
}

bool PathConstraint::isValid() const
{
    return m_component && m_pathFunction != nullptr;
}

int PathConstraint::getConstrainedDegreesOfFreedom() const
{
    return 2; // A path constraint removes two translational degrees of freedom
}

bool PathConstraint::simulateMotion(double dt)
{
    if (!isValid()) {
        return false;
    }
    
    // Update the parameter based on the current velocity
    double distanceAlongPath = m_velocity * dt;
    double parameterChange = distanceAlongPath / m_pathLength;
    m_parameter = clampParameter(m_parameter + parameterChange);
    
    // Calculate the new position on the path
    Eigen::Vector3d pathPosition = getCurrentPathPosition();
    
    // Calculate the world-space origin point on the component
    Eigen::Vector3d worldOrigin = calculateWorldOrigin();
    
    // Adjust the component position to keep the origin on the path
    Eigen::Vector3d currentPosition = m_component->getPosition();
    Eigen::Vector3d newPosition = currentPosition + (pathPosition - worldOrigin);
    m_component->setPosition(newPosition);
    
    return true;
}

void PathConstraint::getVelocity(Eigen::Vector3d& linearVelocity, Eigen::Vector3d& angularVelocity) const
{
    // Calculate the tangent vector at the current position
    Eigen::Vector3d tangent = getCurrentPathTangent();
    
    // Linear velocity is along the tangent, scaled by the velocity
    linearVelocity = tangent * m_velocity;
    
    // No angular velocity for a simple path constraint
    angularVelocity = Eigen::Vector3d::Zero();
}

bool PathConstraint::setVelocity(const Eigen::Vector3d& linearVelocity, const Eigen::Vector3d& angularVelocity)
{
    // Calculate the tangent vector at the current position
    Eigen::Vector3d tangent = getCurrentPathTangent();
    
    // Project the linear velocity onto the tangent
    m_velocity = linearVelocity.dot(tangent);
    
    // Ignore angular velocity for a simple path constraint
    
    return true;
}

void PathConstraint::calculateForceAndTorque(Eigen::Vector3d& force, Eigen::Vector3d& torque) const
{
    // Calculate the tangent vector at the current position
    Eigen::Vector3d tangent = getCurrentPathTangent();
    
    // Force is along the tangent, proportional to the velocity
    double forceMagnitude = m_velocity * 10.0; // Arbitrary scaling factor
    force = tangent * forceMagnitude;
    
    // No torque for a simple path constraint
    torque = Eigen::Vector3d::Zero();
}

double PathConstraint::getParameter() const
{
    return m_parameter;
}

bool PathConstraint::setParameter(double parameter)
{
    m_parameter = clampParameter(parameter);
    return enforce();
}

PathConstraint::PathFunction PathConstraint::getPathFunction() const
{
    return m_pathFunction;
}

void PathConstraint::setPathFunction(PathFunction pathFunction)
{
    if (pathFunction) {
        m_pathFunction = pathFunction;
        m_pathLength = calculatePathLength();
    } else {
        throw std::invalid_argument("Path function cannot be null");
    }
}

Eigen::Vector3d PathConstraint::getLocalOrigin() const
{
    return m_localOrigin;
}

void PathConstraint::setLocalOrigin(const Eigen::Vector3d& localOrigin)
{
    m_localOrigin = localOrigin;
}

Eigen::Vector3d PathConstraint::getCurrentPathPosition() const
{
    return m_pathFunction(m_parameter);
}

Eigen::Vector3d PathConstraint::getCurrentPathTangent() const
{
    return calculatePathTangent(m_parameter);
}

double PathConstraint::getPathLength() const
{
    return m_pathLength;
}

Eigen::Vector3d PathConstraint::calculateWorldOrigin() const
{
    // Transform the local origin to world space
    return m_component->getPosition() + m_component->getOrientation() * m_localOrigin;
}

Eigen::Vector3d PathConstraint::calculatePathTangent(double parameter) const
{
    // Calculate the tangent using a numerical approximation
    double epsilon = 1e-6;
    double p1 = clampParameter(parameter - epsilon);
    double p2 = clampParameter(parameter + epsilon);
    
    Eigen::Vector3d pos1 = m_pathFunction(p1);
    Eigen::Vector3d pos2 = m_pathFunction(p2);
    
    Eigen::Vector3d tangent = pos2 - pos1;
    double length = tangent.norm();
    
    if (length > 1e-10) {
        return tangent / length;
    } else {
        // Default to X-axis if tangent is too small
        return Eigen::Vector3d::UnitX();
    }
}

double PathConstraint::calculatePathLength(int numSegments) const
{
    double length = 0.0;
    double step = 1.0 / numSegments;
    
    Eigen::Vector3d prevPoint = m_pathFunction(0.0);
    
    for (int i = 1; i <= numSegments; ++i) {
        double t = i * step;
        Eigen::Vector3d currentPoint = m_pathFunction(t);
        length += (currentPoint - prevPoint).norm();
        prevPoint = currentPoint;
    }
    
    return length;
}

PathConstraint::PathFunction PathConstraint::createLinearPath(const Eigen::Vector3d& startPoint, const Eigen::Vector3d& endPoint)
{
    return [startPoint, endPoint](double t) {
        return startPoint + t * (endPoint - startPoint);
    };
}

PathConstraint::PathFunction PathConstraint::createPiecewiseLinearPath(const std::vector<Eigen::Vector3d>& points)
{
    if (points.size() < 2) {
        throw std::invalid_argument("At least two points are required for a path");
    }
    
    return [points](double t) {
        if (t <= 0.0) {
            return points.front();
        }
        if (t >= 1.0) {
            return points.back();
        }
        
        // Calculate the total length of the path
        double totalLength = 0.0;
        std::vector<double> segmentLengths;
        
        for (size_t i = 1; i < points.size(); ++i) {
            double length = (points[i] - points[i-1]).norm();
            segmentLengths.push_back(length);
            totalLength += length;
        }
        
        // Find the segment that contains the parameter t
        double targetDistance = t * totalLength;
        double currentDistance = 0.0;
        
        for (size_t i = 0; i < segmentLengths.size(); ++i) {
            double nextDistance = currentDistance + segmentLengths[i];
            
            if (targetDistance <= nextDistance || i == segmentLengths.size() - 1) {
                // Found the segment
                double segmentT = (targetDistance - currentDistance) / segmentLengths[i];
                return points[i] + segmentT * (points[i+1] - points[i]);
            }
            
            currentDistance = nextDistance;
        }
        
        // Should never reach here
        return points.back();
    };
}

double PathConstraint::clampParameter(double parameter)
{
    return std::max(0.0, std::min(1.0, parameter));
}

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
