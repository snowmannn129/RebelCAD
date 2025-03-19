#pragma once

#include "constraints/Constraint.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include <memory>
#include <variant>

namespace RebelCAD {
namespace Constraints {

/**
 * @brief Constraint that enforces concentricity between circles and/or arcs
 * 
 * The ConcentricConstraint ensures that two geometric entities (circles or arcs)
 * share the same center point. This is a fundamental constraint in CAD systems
 * for creating concentric geometry.
 */
class ConcentricConstraint : public Constraint {
public:
    using Entity = std::variant<std::shared_ptr<Sketching::Circle>, std::shared_ptr<Sketching::Arc>>;

    /**
     * @brief Constructs a concentric constraint between two entities
     * @param first First circle or arc
     * @param second Second circle or arc
     * @throws Error if either entity is null
     */
    ConcentricConstraint(Entity first, Entity second);

    /**
     * @brief Checks if the constraint is satisfied
     * @return true if entities are concentric within tolerance
     */
    bool isSatisfied() const override;

    /**
     * @brief Enforces the concentric constraint
     * @return true if constraint was successfully enforced
     */
    bool enforce() override;

    /**
     * @brief Gets the distance between center points
     * @return Distance between centers (0 if perfectly concentric)
     */
    double getError() const override;

    /**
     * @brief Validates that both entities exist and are valid
     * @return true if constraint can be enforced
     */
    bool isValid() const override;

private:
    Entity mFirst;
    Entity mSecond;
    static constexpr float TOLERANCE = 1e-5f;

    // Helper methods
    std::array<float, 2> getEntityCenter(const Entity& entity) const;
    void setEntityCenter(Entity& entity, float x, float y);
};

} // namespace Constraints
} // namespace RebelCAD
