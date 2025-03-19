#include "constraints/ConcentricConstraint.h"
#include "core/Error.h"
#include <cmath>

namespace RebelCAD {
namespace Constraints {

ConcentricConstraint::ConcentricConstraint(Entity first, Entity second)
    : mFirst(first), mSecond(second) {
    // Validate entities are not null
    bool firstValid = std::visit([](const auto& ptr) { return ptr != nullptr; }, first);
    bool secondValid = std::visit([](const auto& ptr) { return ptr != nullptr; }, second);
    
    if (!firstValid || !secondValid) {
        throw Core::Error(Core::ErrorCode::GeometryError, "Entities cannot be null");
    }
}

bool ConcentricConstraint::isSatisfied() const {
    return getError() <= TOLERANCE;
}

bool ConcentricConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    // Get center points
    auto center1 = getEntityCenter(mFirst);
    auto center2 = getEntityCenter(mSecond);

    // Calculate midpoint
    float midX = (center1[0] + center2[0]) / 2.0f;
    float midY = (center1[1] + center2[1]) / 2.0f;

    // Move both entities to midpoint
    setEntityCenter(mFirst, midX, midY);
    setEntityCenter(mSecond, midX, midY);

    return true;
}

double ConcentricConstraint::getError() const {
    if (!isValid()) {
        return std::numeric_limits<double>::infinity();
    }

    auto center1 = getEntityCenter(mFirst);
    auto center2 = getEntityCenter(mSecond);

    float dx = center2[0] - center1[0];
    float dy = center2[1] - center1[1];
    return std::sqrt(dx * dx + dy * dy);
}

bool ConcentricConstraint::isValid() const {
    return std::visit([](const auto& ptr) { return ptr != nullptr; }, mFirst) &&
           std::visit([](const auto& ptr) { return ptr != nullptr; }, mSecond);
}

std::array<float, 2> ConcentricConstraint::getEntityCenter(const Entity& entity) const {
    return std::visit([](const auto& ptr) { return ptr->getCenter(); }, entity);
}

void ConcentricConstraint::setEntityCenter(Entity& entity, float x, float y) {
    std::visit([x, y](auto& ptr) {
        auto center = ptr->getCenter();
        float dx = x - center[0];
        float dy = y - center[1];
        ptr->translate(dx, dy);
    }, entity);
}

} // namespace Constraints
} // namespace RebelCAD
