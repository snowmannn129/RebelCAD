#pragma once

#include "AdvancedAssemblyConstraint.h"
#include <functional>
#include <vector>
#include <cmath>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {
namespace Advanced {

/**
 * @brief Represents a path constraint for a component
 * 
 * The PathConstraint ensures that a component follows a specific path.
 * It can be used to model components that move along predefined trajectories.
 */
class PathConstraint : public AdvancedAssemblyConstraint {
public:
    /**
     * @brief Type definition for a path function
     * 
     * A path function takes a parameter t (0.0-1.0) and returns a 3D position.
     * The parameter t represents the position along the path, with 0.0 being the start and 1.0 being the end.
     */
    using PathFunction = std::function<Eigen::Vector3d(double)>;

    /**
     * @brief Constructs a path constraint with a custom path function
     * @param component The component to constrain
     * @param pathFunction Function that defines the path (t -> position)
     * @param initialParameter Initial parameter value (0.0-1.0)
     * @param localOrigin Local origin point on the component to constrain to the path
     */
    PathConstraint(
        std::shared_ptr<Component> component,
        PathFunction pathFunction,
        double initialParameter = 0.0,
        const Eigen::Vector3d& localOrigin = Eigen::Vector3d::Zero()
    );

    /**
     * @brief Constructs a path constraint with a linear path
     * @param component The component to constrain
     * @param startPoint Start point of the linear path
     * @param endPoint End point of the linear path
     * @param initialParameter Initial parameter value (0.0-1.0)
     * @param localOrigin Local origin point on the component to constrain to the path
     */
    PathConstraint(
        std::shared_ptr<Component> component,
        const Eigen::Vector3d& startPoint,
        const Eigen::Vector3d& endPoint,
        double initialParameter = 0.0,
        const Eigen::Vector3d& localOrigin = Eigen::Vector3d::Zero()
    );

    /**
     * @brief Constructs a path constraint with a piecewise linear path
     * @param component The component to constrain
     * @param points Vector of points defining the piecewise linear path
     * @param initialParameter Initial parameter value (0.0-1.0)
     * @param localOrigin Local origin point on the component to constrain to the path
     */
    PathConstraint(
        std::shared_ptr<Component> component,
        const std::vector<Eigen::Vector3d>& points,
        double initialParameter = 0.0,
        const Eigen::Vector3d& localOrigin = Eigen::Vector3d::Zero()
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
     * @return Number of degrees of freedom removed (2 for a path constraint)
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
     * @brief Gets the current parameter value
     * @return The current parameter value (0.0-1.0)
     */
    double getParameter() const;

    /**
     * @brief Sets the parameter value
     * @param parameter The new parameter value (0.0-1.0)
     * @return true if the parameter was successfully set, false otherwise
     */
    bool setParameter(double parameter);

    /**
     * @brief Gets the path function
     * @return The path function
     */
    PathFunction getPathFunction() const;

    /**
     * @brief Sets the path function
     * @param pathFunction The new path function
     */
    void setPathFunction(PathFunction pathFunction);

    /**
     * @brief Gets the local origin point
     * @return The local origin point on the component
     */
    Eigen::Vector3d getLocalOrigin() const;

    /**
     * @brief Sets the local origin point
     * @param localOrigin The new local origin point on the component
     */
    void setLocalOrigin(const Eigen::Vector3d& localOrigin);

    /**
     * @brief Gets the current position on the path
     * @return The current position on the path
     */
    Eigen::Vector3d getCurrentPathPosition() const;

    /**
     * @brief Gets the tangent vector at the current position on the path
     * @return The tangent vector
     */
    Eigen::Vector3d getCurrentPathTangent() const;

    /**
     * @brief Gets the path length
     * @return The approximate length of the path
     */
    double getPathLength() const;

private:
    std::shared_ptr<Component> m_component; // Component to constrain
    PathFunction m_pathFunction; // Function that defines the path
    double m_parameter; // Current parameter value (0.0-1.0)
    Eigen::Vector3d m_localOrigin; // Local origin point on the component
    double m_velocity; // Current velocity along the path
    double m_pathLength; // Approximate length of the path

    /**
     * @brief Calculates the world-space origin point on the component
     * @return The origin point in world coordinates
     */
    Eigen::Vector3d calculateWorldOrigin() const;

    /**
     * @brief Calculates the tangent vector at a given parameter value
     * @param parameter The parameter value (0.0-1.0)
     * @return The tangent vector
     */
    Eigen::Vector3d calculatePathTangent(double parameter) const;

    /**
     * @brief Calculates the approximate length of the path
     * @param numSegments Number of segments to use for approximation
     * @return The approximate path length
     */
    double calculatePathLength(int numSegments = 100) const;

    /**
     * @brief Creates a linear path function
     * @param startPoint Start point of the linear path
     * @param endPoint End point of the linear path
     * @return A path function for a linear path
     */
    static PathFunction createLinearPath(const Eigen::Vector3d& startPoint, const Eigen::Vector3d& endPoint);

    /**
     * @brief Creates a piecewise linear path function
     * @param points Vector of points defining the piecewise linear path
     * @return A path function for a piecewise linear path
     */
    static PathFunction createPiecewiseLinearPath(const std::vector<Eigen::Vector3d>& points);

    /**
     * @brief Clamps a parameter value to the range [0.0, 1.0]
     * @param parameter Parameter value to clamp
     * @return Clamped parameter value
     */
    static double clampParameter(double parameter);
};

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
