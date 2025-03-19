#pragma once

#include "Constraint.h"
#include "sketching/Line.h"
#include <memory>
#include <optional>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Represents an angle constraint between two lines in a sketch
 * 
 * The AngleConstraint ensures that the angle between two lines remains at a specified value.
 * The angle is measured counterclockwise from the first line to the second line.
 */
class AngleConstraint : public Constraint {
public:
    /**
     * @brief Constructs an angle constraint between two lines
     * @param line1 First line for angle measurement
     * @param line2 Second line for angle measurement
     * @param target_angle Desired angle in radians (0-2π)
     * @param tolerance Maximum allowed deviation from target angle
     * @throws std::invalid_argument if angle is not in valid range
     */
    AngleConstraint(
        std::shared_ptr<Sketching::Line> line1,
        std::shared_ptr<Sketching::Line> line2,
        double target_angle,
        double tolerance = 1e-6
    );
    
    /**
     * @brief Gets the current angle between the lines
     * @return Current angle in radians
     */
    double getCurrentAngle() const;
    
    /**
     * @brief Gets the target angle
     * @return Target angle in radians
     */
    double getTargetAngle() const;
    
    /**
     * @brief Sets the desired angle between the lines
     * @param angle New angle in radians (0-2π)
     * @throws std::invalid_argument if angle is not in valid range
     */
    void setTargetAngle(double angle);

    /**
     * @brief Checks if the constraint is currently satisfied
     * @return true if angle condition is satisfied, false otherwise
     */
    bool isSatisfied() const override;

    /**
     * @brief Attempts to enforce the angle constraint
     * @return true if constraint was successfully enforced, false otherwise
     */
    bool enforce() override;

    /**
     * @brief Gets the deviation from target angle
     * @return Absolute difference between current and target angle
     */
    double getError() const override;

    /**
     * @brief Checks if the constraint is valid (non-null lines and valid angle)
     * @return true if constraint can be enforced, false otherwise
     */
    bool isValid() const override;

private:
    std::shared_ptr<Sketching::Line> m_line1;
    std::shared_ptr<Sketching::Line> m_line2;
    double m_target_angle;
    double m_tolerance;

    // Cache for optimization
    mutable std::optional<double> m_cached_current_angle;
    mutable std::optional<double> m_cached_error;
    
    /**
     * @brief Calculates the current angle between the two lines
     * @return Angle in radians (0-2π)
     */
    double calculateCurrentAngle() const;
    
    /**
     * @brief Validates that an angle value is in the valid range
     * @param angle Angle to validate in radians
     * @throws std::invalid_argument if angle is not in valid range
     */
    void validateAngle(double angle) const;

    /**
     * @brief Invalidates cached values when geometry changes
     */
    void invalidateCache();

    /**
     * @brief Normalizes an angle to be between 0 and 2π
     * @param angle Angle to normalize in radians
     * @return Normalized angle in radians
     */
    double normalizeAngle(double angle) const;
};

} // namespace Constraints
} // namespace RebelCAD
