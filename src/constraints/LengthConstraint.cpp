#include "constraints/LengthConstraint.h"
#include <stdexcept>

namespace RebelCAD {
namespace Constraints {

LengthConstraint::LengthConstraint(
    const std::array<double, 2>& start_point,
    const std::array<double, 2>& end_point,
    double target_length,
    double tolerance
) : m_start_point(start_point),
    m_end_point(end_point),
    m_target_length(target_length),
    m_tolerance(tolerance) {
    
    if (target_length <= 0.0) {
        throw std::invalid_argument("Target length must be positive");
    }
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
}

bool LengthConstraint::isSatisfied() const {
    return std::abs(getError()) <= m_tolerance;
}

bool LengthConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    // Calculate current vector between points
    double dx = m_end_point[0] - m_start_point[0];
    double dy = m_end_point[1] - m_start_point[1];
    
    // Calculate current length
    double current_length = calculateCurrentLength();
    
    if (current_length < 1e-10) {
        // Handle degenerate case - extend in x direction
        m_end_point[0] = m_start_point[0] + m_target_length;
        m_end_point[1] = m_start_point[1];
    } else {
        // Scale vector to target length
        double scale = m_target_length / current_length;
        m_end_point[0] = m_start_point[0] + dx * scale;
        m_end_point[1] = m_start_point[1] + dy * scale;
    }
    
    invalidateCache();
    return true;
}

double LengthConstraint::getError() const {
    if (!m_cached_error) {
        m_cached_error = std::abs(calculateCurrentLength() - m_target_length);
    }
    return *m_cached_error;
}

bool LengthConstraint::isValid() const {
    // Check if points are valid (not NaN/Inf)
    for (int i = 0; i < 2; ++i) {
        if (!std::isfinite(m_start_point[i]) || !std::isfinite(m_end_point[i])) {
            return false;
        }
    }
    return true;
}

double LengthConstraint::getCurrentLength() const {
    return calculateCurrentLength();
}

double LengthConstraint::getTargetLength() const {
    return m_target_length;
}

void LengthConstraint::setTargetLength(double new_length) {
    if (new_length <= 0.0) {
        throw std::invalid_argument("Target length must be positive");
    }
    m_target_length = new_length;
    invalidateCache();
}

void LengthConstraint::setStartPoint(const std::array<double, 2>& new_start) {
    m_start_point = new_start;
    invalidateCache();
}

void LengthConstraint::setEndPoint(const std::array<double, 2>& new_end) {
    m_end_point = new_end;
    invalidateCache();
}

double LengthConstraint::calculateCurrentLength() const {
    if (!m_cached_current_length) {
        double dx = m_end_point[0] - m_start_point[0];
        double dy = m_end_point[1] - m_start_point[1];
        m_cached_current_length = std::sqrt(dx * dx + dy * dy);
    }
    return *m_cached_current_length;
}

void LengthConstraint::invalidateCache() {
    m_cached_current_length.reset();
    m_cached_error.reset();
}

} // namespace Constraints
} // namespace RebelCAD
