#pragma once

#include <memory>
#include <vector>
#include "constraints/Constraint.h"

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Automatically detects geometric constraints between sketch elements
 */
class AutoConstraintDetector {
public:
    /**
     * @brief Constructs a new Auto Constraint Detector
     */
    AutoConstraintDetector();

    /**
     * @brief Enables or disables the constraint detector
     * @param enabled Whether the detector should be enabled
     */
    void setEnabled(bool enabled);

    /**
     * @brief Gets whether the detector is enabled
     * @return true if enabled
     */
    bool isEnabled() const;

    /**
     * @brief Sets the sensitivity threshold for constraint detection
     * @param sensitivity Value between 0.1 and 1.0
     */
    void setSensitivity(float sensitivity);

    /**
     * @brief Gets the current sensitivity threshold
     * @return Current sensitivity value
     */
    float getSensitivity() const;

    /**
     * @brief Detects constraints between geometric elements
     * @return Vector of detected constraints
     */
    std::vector<std::shared_ptr<Constraint>> detectConstraints();

private:
    bool m_enabled;
    float m_sensitivity;
};

} // namespace Constraints
} // namespace RebelCAD
