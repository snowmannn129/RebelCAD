#pragma once

#include "Constraint.h"
#include <array>
#include <cmath>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Represents a perpendicular constraint between two line segments
 * 
 * The PerpendicularConstraint ensures that two line segments remain perpendicular 
 * (90 degrees) to each other. A line segment is defined by two points (x1,y1) and 
 * (x2,y2). The constraint uses vector mathematics to determine perpendicularity 
 * and enforce the constraint.
 */
class PerpendicularConstraint : public Constraint {
public:
    /**
     * @brief Constructs a perpendicular constraint between two line segments
     * @param line1_start First point of first line {x,y}
     * @param line1_end Second point of first line {x,y}
     * @param line2_start First point of second line {x,y}
     * @param line2_end Second point of second line {x,y}
     * @param tolerance Maximum allowed angular deviation from 90 degrees in radians
     */
    PerpendicularConstraint(
        const std::array<double, 2>& line1_start,
        const std::array<double, 2>& line1_end,
        const std::array<double, 2>& line2_start,
        const std::array<double, 2>& line2_end,
        double tolerance = 1e-6
    );

    /**
     * @brief Checks if the lines are perpendicular within tolerance
     * @return true if lines are perpendicular (90 degrees), false otherwise
     */
    bool isSatisfied() const override;

    /**
     * @brief Attempts to make the lines perpendicular by rotating the second line
     * @return true if the constraint was successfully enforced, false otherwise
     */
    bool enforce() override;

    /**
     * @brief Gets the angular deviation from 90 degrees in radians
     * @return The absolute difference between the actual angle and 90 degrees
     */
    double getError() const override;

    /**
     * @brief Checks if the constraint is valid (non-zero length lines)
     * @return true if both lines have non-zero length, false otherwise
     */
    bool isValid() const override;

    /**
     * @brief Gets the first line's start point
     * @return Array containing {x,y} coordinates
     */
    std::array<double, 2> getLine1Start() const { return m_line1_start; }

    /**
     * @brief Gets the first line's end point
     * @return Array containing {x,y} coordinates
     */
    std::array<double, 2> getLine1End() const { return m_line1_end; }

    /**
     * @brief Gets the second line's start point
     * @return Array containing {x,y} coordinates
     */
    std::array<double, 2> getLine2Start() const { return m_line2_start; }

    /**
     * @brief Gets the second line's end point
     * @return Array containing {x,y} coordinates
     */
    std::array<double, 2> getLine2End() const { return m_line2_end; }

private:
    std::array<double, 2> m_line1_start;
    std::array<double, 2> m_line1_end;
    std::array<double, 2> m_line2_start;
    std::array<double, 2> m_line2_end;
    double m_tolerance;
    const double M_PI_2 = M_PI / 2.0; // 90 degrees in radians

    /**
     * @brief Calculates the angle of a line relative to the x-axis
     * @param start Start point of the line
     * @param end End point of the line
     * @return Angle in radians
     */
    double calculateLineAngle(const std::array<double, 2>& start, 
                            const std::array<double, 2>& end) const;

    /**
     * @brief Rotates a point around a center point by a given angle
     * @param point Point to rotate
     * @param center Center of rotation
     * @param angle Angle in radians
     * @return Rotated point coordinates
     */
    std::array<double, 2> rotatePoint(const std::array<double, 2>& point,
                                    const std::array<double, 2>& center,
                                    double angle) const;
};

} // namespace Constraints
} // namespace RebelCAD
