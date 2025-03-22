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
 * @brief Represents a cam constraint between two components
 * 
 * The CamConstraint ensures that one component (the follower) follows the profile of another component (the cam).
 * It can be used to model cam-follower mechanisms with various cam profiles.
 */
class CamConstraint : public AdvancedAssemblyConstraint {
public:
    /**
     * @brief Type definition for a cam profile function
     * 
     * A cam profile function takes an angle in radians and returns a radius.
     * The angle is the rotation angle of the cam, and the radius is the distance from the cam center to the cam surface.
     */
    using ProfileFunction = std::function<double(double)>;

    /**
     * @brief Constructs a cam constraint with a custom profile function
     * @param cam The cam component
     * @param follower The follower component
     * @param camAxis Rotation axis of the cam in local coordinates
     * @param followerAxis Direction of follower motion in local coordinates
     * @param profileFunction Function that defines the cam profile (angle -> radius)
     * @param offset Initial offset between cam and follower
     */
    CamConstraint(
        std::shared_ptr<Component> cam,
        std::shared_ptr<Component> follower,
        const Eigen::Vector3d& camAxis,
        const Eigen::Vector3d& followerAxis,
        ProfileFunction profileFunction,
        double offset = 0.0
    );

    /**
     * @brief Constructs a cam constraint with a circular profile
     * @param cam The cam component
     * @param follower The follower component
     * @param camAxis Rotation axis of the cam in local coordinates
     * @param followerAxis Direction of follower motion in local coordinates
     * @param radius Radius of the circular cam
     * @param offset Initial offset between cam and follower
     */
    CamConstraint(
        std::shared_ptr<Component> cam,
        std::shared_ptr<Component> follower,
        const Eigen::Vector3d& camAxis,
        const Eigen::Vector3d& followerAxis,
        double radius,
        double offset = 0.0
    );

    /**
     * @brief Constructs a cam constraint with a piecewise linear profile
     * @param cam The cam component
     * @param follower The follower component
     * @param camAxis Rotation axis of the cam in local coordinates
     * @param followerAxis Direction of follower motion in local coordinates
     * @param angleRadiusPairs Vector of angle-radius pairs defining the cam profile
     * @param offset Initial offset between cam and follower
     */
    CamConstraint(
        std::shared_ptr<Component> cam,
        std::shared_ptr<Component> follower,
        const Eigen::Vector3d& camAxis,
        const Eigen::Vector3d& followerAxis,
        const std::vector<std::pair<double, double>>& angleRadiusPairs,
        double offset = 0.0
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
     * @return Number of degrees of freedom removed (1 for a cam constraint)
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
     * @brief Gets the current cam angle
     * @return The current cam angle in radians
     */
    double getCamAngle() const;

    /**
     * @brief Sets the cam angle
     * @param angle The new cam angle in radians
     * @return true if the angle was successfully set, false otherwise
     */
    bool setCamAngle(double angle);

    /**
     * @brief Gets the cam profile function
     * @return The cam profile function
     */
    ProfileFunction getProfileFunction() const;

    /**
     * @brief Sets the cam profile function
     * @param profileFunction The new cam profile function
     */
    void setProfileFunction(ProfileFunction profileFunction);

    /**
     * @brief Gets the offset between cam and follower
     * @return The offset value
     */
    double getOffset() const;

    /**
     * @brief Sets the offset between cam and follower
     * @param offset The new offset value
     */
    void setOffset(double offset);

    /**
     * @brief Gets the axes
     * @param camAxis Output parameter for the cam rotation axis
     * @param followerAxis Output parameter for the follower motion direction
     */
    void getAxes(Eigen::Vector3d& camAxis, Eigen::Vector3d& followerAxis) const;

    /**
     * @brief Sets the axes
     * @param camAxis The new cam rotation axis
     * @param followerAxis The new follower motion direction
     */
    void setAxes(const Eigen::Vector3d& camAxis, const Eigen::Vector3d& followerAxis);

private:
    std::shared_ptr<Component> m_cam; // Cam component
    std::shared_ptr<Component> m_follower; // Follower component
    Eigen::Vector3d m_camAxis; // Rotation axis of the cam in local coordinates
    Eigen::Vector3d m_followerAxis; // Direction of follower motion in local coordinates
    ProfileFunction m_profileFunction; // Function that defines the cam profile
    double m_offset; // Initial offset between cam and follower
    double m_camAngle; // Current rotation angle of the cam
    double m_angularVelocity; // Current angular velocity of the cam

    /**
     * @brief Calculates the world-space axes
     * @param worldCamAxis Output parameter for the cam rotation axis in world coordinates
     * @param worldFollowerAxis Output parameter for the follower motion direction in world coordinates
     */
    void calculateWorldAxes(Eigen::Vector3d& worldCamAxis, Eigen::Vector3d& worldFollowerAxis) const;

    /**
     * @brief Calculates the follower position based on the cam angle
     * @param camAngle The cam angle in radians
     * @return The follower position along the follower axis
     */
    double calculateFollowerPosition(double camAngle) const;

    /**
     * @brief Creates a circular cam profile function
     * @param radius The radius of the circular cam
     * @return A profile function for a circular cam
     */
    static ProfileFunction createCircularProfile(double radius);

    /**
     * @brief Creates a piecewise linear cam profile function
     * @param angleRadiusPairs Vector of angle-radius pairs defining the cam profile
     * @return A profile function for a piecewise linear cam
     */
    static ProfileFunction createPiecewiseLinearProfile(const std::vector<std::pair<double, double>>& angleRadiusPairs);

    /**
     * @brief Normalizes an angle to be between 0 and 2π
     * @param angle Angle to normalize in radians
     * @return Normalized angle in radians
     */
    static double normalizeAngle(double angle);
};

} // namespace Advanced
} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
