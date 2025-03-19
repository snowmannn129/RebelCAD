#pragma once

#include "Constraint.h"
#include <array>
#include <optional>
#include <cmath>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Represents a tangency constraint between geometric entities
 * 
 * The TangentConstraint ensures that two geometric entities (lines, circles, or arcs)
 * maintain a tangent relationship. For lines and circles/arcs, this means the line
 * touches the curve at exactly one point. For circles/arcs, this means they touch
 * at exactly one point with a common tangent line at that point.
 */
class TangentConstraint : public Constraint {
public:
    /**
     * @brief Constructs a tangent constraint between a line and a circle
     * @param line_start Start point of line {x,y}
     * @param line_end End point of line {x,y}
     * @param circle_center Center point of circle {x,y}
     * @param circle_radius Radius of circle
     * @param tolerance Maximum allowed deviation from perfect tangency
     * @throws std::invalid_argument if line is degenerate or radius is non-positive
     */
    TangentConstraint(
        const std::array<double, 2>& line_start,
        const std::array<double, 2>& line_end,
        const std::array<double, 2>& circle_center,
        double circle_radius,
        double tolerance = 1e-6
    );

    /**
     * @brief Constructs a tangent constraint between two circles
     * @param circle1_center Center of first circle {x,y}
     * @param circle1_radius Radius of first circle
     * @param circle2_center Center of second circle {x,y}
     * @param circle2_radius Radius of second circle
     * @param tolerance Maximum allowed deviation from perfect tangency
     * @throws std::invalid_argument if radii are non-positive or circles are coincident
     */
    TangentConstraint(
        const std::array<double, 2>& circle1_center,
        double circle1_radius,
        const std::array<double, 2>& circle2_center,
        double circle2_radius,
        double tolerance = 1e-6
    );

    /**
     * @brief Checks if the entities are tangent within tolerance
     * @return true if tangency condition is satisfied, false otherwise
     */
    bool isSatisfied() const override;

    /**
     * @brief Attempts to enforce tangency by adjusting entity positions
     * @return true if constraint was successfully enforced, false otherwise
     */
    bool enforce() override;

    /**
     * @brief Gets the deviation from perfect tangency
     * @return Distance between actual and ideal tangent points
     */
    double getError() const override;

    /**
     * @brief Checks if the constraint is valid (non-degenerate entities)
     * @return true if constraint can be enforced, false otherwise
     */
    bool isValid() const override;

private:
    enum class Type {
        LINE_CIRCLE,
        CIRCLE_CIRCLE
    };

    Type m_type;
    std::array<double, 2> m_point1;
    std::array<double, 2> m_point2;
    std::array<double, 2> m_center1;
    std::array<double, 2> m_center2;
    double m_radius1;
    double m_radius2;
    double m_tolerance;
    
    // Cache for optimization
    mutable std::optional<double> m_cached_error;
    mutable std::optional<std::array<double, 2>> m_cached_tangent_point;

    /**
     * @brief Calculates the tangent point between a line and circle
     * @return Coordinates of tangent point if it exists
     */
    std::optional<std::array<double, 2>> calculateLineTangentPoint() const;

    /**
     * @brief Calculates the tangent point between two circles
     * @return Coordinates of tangent point if it exists
     */
    std::optional<std::array<double, 2>> calculateCircleTangentPoint() const;

    /**
     * @brief Calculates distance between a point and a line
     * @param point Point coordinates
     * @param line_start Line start point
     * @param line_end Line end point
     * @return Shortest distance from point to line
     */
    double pointToLineDistance(
        const std::array<double, 2>& point,
        const std::array<double, 2>& line_start,
        const std::array<double, 2>& line_end) const;

    /**
     * @brief Invalidates cached values when geometry changes
     */
    void invalidateCache() const;
};

} // namespace Constraints
} // namespace RebelCAD
