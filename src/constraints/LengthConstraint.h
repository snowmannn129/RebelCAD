#pragma once

#include "Constraint.h"
#include <array>
#include <optional>
#include <cmath>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Represents a length constraint on geometric entities
 * 
 * The LengthConstraint ensures that a geometric entity maintains a specific length.
 * For lines, this is the direct distance between endpoints. For circles/arcs, this
 * would be the diameter/arc length (to be implemented).
 */
class LengthConstraint : public Constraint {
public:
    /**
     * @brief Constructs a length constraint for a line
     * @param start_point Start point of line {x,y}
     * @param end_point End point of line {x,y}
     * @param target_length Desired length of the line
     * @param tolerance Maximum allowed deviation from target length
     * @throws std::invalid_argument if target_length is non-positive
     */
    LengthConstraint(
        const std::array<double, 2>& start_point,
        const std::array<double, 2>& end_point,
        double target_length,
        double tolerance = 1e-6
    );

    /**
     * @brief Checks if the entity meets the length constraint within tolerance
     * @return true if length condition is satisfied, false otherwise
     */
    bool isSatisfied() const override;

    /**
     * @brief Attempts to enforce the length constraint by adjusting entity
     * @return true if constraint was successfully enforced, false otherwise
     */
    bool enforce() override;

    /**
     * @brief Gets the deviation from target length
     * @return Absolute difference between current and target length
     */
    double getError() const override;

    /**
     * @brief Checks if the constraint is valid (non-degenerate entity and positive length)
     * @return true if constraint can be enforced, false otherwise
     */
    bool isValid() const override;

    /**
     * @brief Gets the current length of the constrained entity
     * @return Current length
     */
    double getCurrentLength() const;

    /**
     * @brief Gets the target length
     * @return Target length value
     */
    double getTargetLength() const;

    /**
     * @brief Updates the target length
     * @param new_length New target length value
     * @throws std::invalid_argument if new_length is non-positive
     */
    void setTargetLength(double new_length);

    /**
     * @brief Updates the start point of the line
     * @param new_start New start point coordinates
     */
    void setStartPoint(const std::array<double, 2>& new_start);

    /**
     * @brief Updates the end point of the line
     * @param new_end New end point coordinates
     */
    void setEndPoint(const std::array<double, 2>& new_end);

private:
    std::array<double, 2> m_start_point;
    std::array<double, 2> m_end_point;
    double m_target_length;
    double m_tolerance;

    // Cache for optimization
    mutable std::optional<double> m_cached_current_length;
    mutable std::optional<double> m_cached_error;

    /**
     * @brief Calculates the current length between points
     * @return Current length value
     */
    double calculateCurrentLength() const;

    /**
     * @brief Invalidates cached values when geometry changes
     */
    void invalidateCache();
};

} // namespace Constraints
} // namespace RebelCAD
