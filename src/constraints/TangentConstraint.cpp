#include "constraints/TangentConstraint.h"
#include <stdexcept>

namespace RebelCAD {
namespace Constraints {

TangentConstraint::TangentConstraint(
    const std::array<double, 2>& line_start,
    const std::array<double, 2>& line_end,
    const std::array<double, 2>& circle_center,
    double circle_radius,
    double tolerance
) : m_type(Type::LINE_CIRCLE),
    m_point1(line_start),
    m_point2(line_end),
    m_center1(circle_center),
    m_radius1(circle_radius),
    m_tolerance(tolerance) {
    
    if (circle_radius <= 0.0) {
        throw std::invalid_argument("Circle radius must be positive");
    }
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }

    // Check for degenerate line
    double dx = line_end[0] - line_start[0];
    double dy = line_end[1] - line_start[1];
    if (std::sqrt(dx * dx + dy * dy) < 1e-10) {
        throw std::invalid_argument("Line segment is degenerate");
    }
}

TangentConstraint::TangentConstraint(
    const std::array<double, 2>& circle1_center,
    double circle1_radius,
    const std::array<double, 2>& circle2_center,
    double circle2_radius,
    double tolerance
) : m_type(Type::CIRCLE_CIRCLE),
    m_center1(circle1_center),
    m_radius1(circle1_radius),
    m_center2(circle2_center),
    m_radius2(circle2_radius),
    m_tolerance(tolerance) {
    
    if (circle1_radius <= 0.0 || circle2_radius <= 0.0) {
        throw std::invalid_argument("Circle radii must be positive");
    }
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }

    // Check for coincident circles
    double dx = circle2_center[0] - circle1_center[0];
    double dy = circle2_center[1] - circle1_center[1];
    if (std::sqrt(dx * dx + dy * dy) < 1e-10) {
        throw std::invalid_argument("Circles are coincident");
    }
}

bool TangentConstraint::isSatisfied() const {
    return getError() <= m_tolerance;
}

bool TangentConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    invalidateCache();

    if (m_type == Type::LINE_CIRCLE) {
        // Calculate perpendicular distance from circle center to line
        double d = pointToLineDistance(m_center1, m_point1, m_point2);
        
        // If distance is not equal to radius, move line parallel to itself
        if (std::abs(d - m_radius1) > m_tolerance) {
            // Calculate unit normal vector to line
            double dx = m_point2[0] - m_point1[0];
            double dy = m_point2[1] - m_point1[1];
            double len = std::sqrt(dx * dx + dy * dy);
            double nx = -dy / len;
            double ny = dx / len;

            // Calculate required shift
            double shift = m_radius1 - d;
            
            // Move line points
            m_point1[0] += nx * shift;
            m_point1[1] += ny * shift;
            m_point2[0] += nx * shift;
            m_point2[1] += ny * shift;
        }
    } else { // CIRCLE_CIRCLE
        // Calculate distance between centers
        double dx = m_center2[0] - m_center1[0];
        double dy = m_center2[1] - m_center1[1];
        double d = std::sqrt(dx * dx + dy * dy);
        
        // Target distance for external tangency is sum of radii
        double target = m_radius1 + m_radius2;
        
        if (std::abs(d - target) > m_tolerance) {
            // Calculate unit vector between centers
            double ux = dx / d;
            double uy = dy / d;
            
            // Move second circle along this vector
            double shift = target - d;
            m_center2[0] += ux * shift;
            m_center2[1] += uy * shift;
        }
    }

    return true;
}

double TangentConstraint::getError() const {
    if (!isValid()) {
        return std::numeric_limits<double>::infinity();
    }

    if (!m_cached_error) {
        if (m_type == Type::LINE_CIRCLE) {
            // Error is difference between perpendicular distance and radius
            double d = pointToLineDistance(m_center1, m_point1, m_point2);
            m_cached_error = std::abs(d - m_radius1);
        } else { // CIRCLE_CIRCLE
            // Error is difference between center distance and sum of radii
            double dx = m_center2[0] - m_center1[0];
            double dy = m_center2[1] - m_center1[1];
            double d = std::sqrt(dx * dx + dy * dy);
            m_cached_error = std::abs(d - (m_radius1 + m_radius2));
        }
    }

    return *m_cached_error;
}

bool TangentConstraint::isValid() const {
    if (m_type == Type::LINE_CIRCLE) {
        // Check line length
        double dx = m_point2[0] - m_point1[0];
        double dy = m_point2[1] - m_point1[1];
        return std::sqrt(dx * dx + dy * dy) > 1e-10 &&
               m_radius1 > 0.0 &&
               std::isfinite(m_radius1);
    } else { // CIRCLE_CIRCLE
        // Check circle separation and radii
        double dx = m_center2[0] - m_center1[0];
        double dy = m_center2[1] - m_center1[1];
        return std::sqrt(dx * dx + dy * dy) > 1e-10 &&
               m_radius1 > 0.0 && m_radius2 > 0.0 &&
               std::isfinite(m_radius1) && std::isfinite(m_radius2);
    }
}

std::optional<std::array<double, 2>> TangentConstraint::calculateLineTangentPoint() const {
    if (!isValid() || m_type != Type::LINE_CIRCLE) {
        return std::nullopt;
    }

    if (!m_cached_tangent_point) {
        // Project circle center onto line
        double dx = m_point2[0] - m_point1[0];
        double dy = m_point2[1] - m_point1[1];
        double len = std::sqrt(dx * dx + dy * dy);
        double ux = dx / len;
        double uy = dy / len;

        // Calculate projection parameter
        double t = (m_center1[0] - m_point1[0]) * ux +
                  (m_center1[1] - m_point1[1]) * uy;

        // Calculate projection point
        std::array<double, 2> proj = {
            m_point1[0] + t * ux,
            m_point1[1] + t * uy
        };

        // Move from projection point towards circle center by radius
        double px = m_center1[0] - proj[0];
        double py = m_center1[1] - proj[1];
        double d = std::sqrt(px * px + py * py);
        
        m_cached_tangent_point = std::array<double, 2>{
            proj[0] + px * m_radius1 / d,
            proj[1] + py * m_radius1 / d
        };
    }

    return m_cached_tangent_point;
}

std::optional<std::array<double, 2>> TangentConstraint::calculateCircleTangentPoint() const {
    if (!isValid() || m_type != Type::CIRCLE_CIRCLE) {
        return std::nullopt;
    }

    if (!m_cached_tangent_point) {
        // Calculate vector between centers
        double dx = m_center2[0] - m_center1[0];
        double dy = m_center2[1] - m_center1[1];
        double d = std::sqrt(dx * dx + dy * dy);

        // Calculate tangent point on first circle
        double t = m_radius1 / (m_radius1 + m_radius2);
        m_cached_tangent_point = std::array<double, 2>{
            m_center1[0] + dx * t,
            m_center1[1] + dy * t
        };
    }

    return m_cached_tangent_point;
}

double TangentConstraint::pointToLineDistance(
    const std::array<double, 2>& point,
    const std::array<double, 2>& line_start,
    const std::array<double, 2>& line_end) const {
    double dx = line_end[0] - line_start[0];
    double dy = line_end[1] - line_start[1];
    double len = std::sqrt(dx * dx + dy * dy);
    
    if (len < 1e-10) {
        return std::numeric_limits<double>::infinity();
    }

    return std::abs((point[1] - line_start[1]) * dx -
                   (point[0] - line_start[0]) * dy) / len;
}

void TangentConstraint::invalidateCache() const {
    m_cached_error.reset();
    m_cached_tangent_point.reset();
}

} // namespace Constraints
} // namespace RebelCAD
