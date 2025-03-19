#include "constraints/ParallelConstraint.h"
#include <stdexcept>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RebelCAD {
namespace Constraints {

ParallelConstraint::ParallelConstraint(
    const std::array<double, 2>& line1_start,
    const std::array<double, 2>& line1_end,
    const std::array<double, 2>& line2_start,
    const std::array<double, 2>& line2_end,
    double tolerance
) : m_line1_start(line1_start),
    m_line1_end(line1_end),
    m_line2_start(line2_start),
    m_line2_end(line2_end),
    m_tolerance(tolerance) {
    
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
}

bool ParallelConstraint::isSatisfied() const {
    return getError() <= m_tolerance;
}

bool ParallelConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    // Calculate angles of both lines
    double angle1 = calculateLineAngle(m_line1_start, m_line1_end);
    double angle2 = calculateLineAngle(m_line2_start, m_line2_end);

    // Calculate rotation needed
    double rotation = angle1 - angle2;

    // Normalize rotation to [-π, π]
    while (rotation > M_PI) rotation -= 2.0 * M_PI;
    while (rotation < -M_PI) rotation += 2.0 * M_PI;

    // Rotate second line around its start point
    m_line2_end = rotatePoint(m_line2_end, m_line2_start, rotation);

    return true;
}

double ParallelConstraint::getError() const {
    if (!isValid()) {
        return std::numeric_limits<double>::infinity();
    }

    // Calculate angles of both lines
    double angle1 = calculateLineAngle(m_line1_start, m_line1_end);
    double angle2 = calculateLineAngle(m_line2_start, m_line2_end);

    // Calculate absolute angular difference
    double diff = std::abs(angle1 - angle2);

    // Normalize to [0, π/2]
    while (diff > M_PI) {
        diff = std::abs(diff - 2.0 * M_PI);
    }
    if (diff > M_PI/2.0) {
        diff = M_PI - diff;
    }

    return diff;
}

bool ParallelConstraint::isValid() const {
    // Calculate lengths
    double dx1 = m_line1_end[0] - m_line1_start[0];
    double dy1 = m_line1_end[1] - m_line1_start[1];
    double dx2 = m_line2_end[0] - m_line2_start[0];
    double dy2 = m_line2_end[1] - m_line2_start[1];

    double length1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double length2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    // Check for non-zero lengths and finite coordinates
    return length1 > 1e-10 && length2 > 1e-10 &&
           std::isfinite(length1) && std::isfinite(length2);
}

double ParallelConstraint::calculateLineAngle(
    const std::array<double, 2>& start,
    const std::array<double, 2>& end) const {
    double dx = end[0] - start[0];
    double dy = end[1] - start[1];
    return std::atan2(dy, dx);
}

std::array<double, 2> ParallelConstraint::rotatePoint(
    const std::array<double, 2>& point,
    const std::array<double, 2>& center,
    double angle) const {
    // Translate to origin
    double dx = point[0] - center[0];
    double dy = point[1] - center[1];

    // Rotate
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    double rx = dx * cos_a - dy * sin_a;
    double ry = dx * sin_a + dy * cos_a;

    // Translate back
    return {center[0] + rx, center[1] + ry};
}

} // namespace Constraints
} // namespace RebelCAD
