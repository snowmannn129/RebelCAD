#include "constraints/AngleConstraint.h"
#include <cmath>
#include <stdexcept>

namespace RebelCAD {
namespace Constraints {

AngleConstraint::AngleConstraint(
    std::shared_ptr<Sketching::Line> line1,
    std::shared_ptr<Sketching::Line> line2,
    double target_angle,
    double tolerance
) : m_line1(line1),
    m_line2(line2),
    m_target_angle(normalizeAngle(target_angle)),
    m_tolerance(tolerance) {
    
    if (!line1 || !line2) {
        throw std::invalid_argument("Lines cannot be null");
    }
    validateAngle(target_angle);
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
}

double AngleConstraint::getCurrentAngle() const {
    if (!m_cached_current_angle) {
        m_cached_current_angle = calculateCurrentAngle();
    }
    return *m_cached_current_angle;
}

double AngleConstraint::getTargetAngle() const {
    return m_target_angle;
}

void AngleConstraint::setTargetAngle(double angle) {
    validateAngle(angle);
    m_target_angle = normalizeAngle(angle);
    invalidateCache();
}

bool AngleConstraint::isSatisfied() const {
    return std::abs(getError()) <= m_tolerance;
}

bool AngleConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    // Get current angle and calculate needed rotation
    double current = getCurrentAngle();
    double rotation = m_target_angle - current;

    // Normalize rotation to smallest angle
    if (rotation > M_PI) {
        rotation -= 2.0 * M_PI;
    } else if (rotation < -M_PI) {
        rotation += 2.0 * M_PI;
    }

    // Rotate second line around its start point
    m_line2->rotate(rotation);
    
    invalidateCache();
    return true;
}

double AngleConstraint::getError() const {
    if (!m_cached_error) {
        double error = std::abs(getCurrentAngle() - m_target_angle);
        // Handle wraparound case
        if (error > M_PI) {
            error = 2.0 * M_PI - error;
        }
        m_cached_error = error;
    }
    return *m_cached_error;
}

bool AngleConstraint::isValid() const {
    return m_line1 && m_line2 && 
           m_line1->getLength() > 1e-6 && 
           m_line2->getLength() > 1e-6;
}

double AngleConstraint::calculateCurrentAngle() const {
    // Get vectors representing the lines
    auto start1 = m_line1->getStartPoint();
    auto end1 = m_line1->getEndPoint();
    auto start2 = m_line2->getStartPoint();
    auto end2 = m_line2->getEndPoint();

    // Calculate direction vectors
    float dx1 = end1[0] - start1[0];
    float dy1 = end1[1] - start1[1];
    float dx2 = end2[0] - start2[0];
    float dy2 = end2[1] - start2[1];

    // Calculate angle using atan2
    double angle1 = std::atan2(dy1, dx1);
    double angle2 = std::atan2(dy2, dx2);

    // Return normalized angle difference
    return normalizeAngle(angle2 - angle1);
}

void AngleConstraint::validateAngle(double angle) const {
    if (!std::isfinite(angle)) {
        throw std::invalid_argument("Angle must be a finite number");
    }
}

void AngleConstraint::invalidateCache() {
    m_cached_current_angle.reset();
    m_cached_error.reset();
}

double AngleConstraint::normalizeAngle(double angle) const {
    // Normalize angle to [0, 2π)
    angle = std::fmod(angle, 2.0 * M_PI);
    if (angle < 0.0) {
        angle += 2.0 * M_PI;
    }
    return angle;
}

} // namespace Constraints
} // namespace RebelCAD
