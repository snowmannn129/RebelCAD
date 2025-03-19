#pragma once

#include "constraints/Constraint.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include <memory>
#include <variant>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Constraint that enforces a specific radius on a circle or arc
 * 
 * The RadiusConstraint ensures that a circle or arc maintains a specified radius.
 * It can be applied to either geometric type and will adjust the shape to meet
 * the constraint while maintaining other properties (like center point).
 */
class RadiusConstraint : public Constraint {
public:
    /**
     * @brief Constructs a radius constraint for a circle
     * @param circle The circle to constrain
     * @param radius The target radius value
     */
    RadiusConstraint(std::shared_ptr<Sketching::Circle> circle, double radius);

    /**
     * @brief Constructs a radius constraint for an arc
     * @param arc The arc to constrain
     * @param radius The target radius value
     */
    RadiusConstraint(std::shared_ptr<Sketching::Arc> arc, double radius);

    /**
     * @brief Checks if the constrained shape's radius matches the target
     * @return true if the radius matches within tolerance
     */
    bool isSatisfied() const override;

    /**
     * @brief Adjusts the shape's radius to match the target value
     * @return true if the radius was successfully adjusted
     */
    bool enforce() override;

    /**
     * @brief Gets the difference between current and target radius
     * @return Absolute difference between current and target radius
     */
    double getError() const override;

    /**
     * @brief Verifies the constraint can be enforced
     * @return true if target radius is positive and shape exists
     */
    bool isValid() const override;

    /**
     * @brief Gets the target radius value
     * @return The constraint's target radius
     */
    double getTargetRadius() const;

private:
    // Using variant to store either circle or arc
    std::variant<std::shared_ptr<Sketching::Circle>, std::shared_ptr<Sketching::Arc>> shape;
    double targetRadius;
    static constexpr double TOLERANCE = 1e-6;

    // Helper to get current radius regardless of shape type
    double getCurrentRadius() const;
};

} // namespace Constraints
} // namespace RebelCAD
