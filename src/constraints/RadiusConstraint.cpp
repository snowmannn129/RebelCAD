#include "constraints/RadiusConstraint.h"
#include <stdexcept>

namespace RebelCAD {
namespace Constraints {

RadiusConstraint::RadiusConstraint(std::shared_ptr<Sketching::Circle> circle, double radius)
    : shape(circle), targetRadius(radius) {
    if (!circle) {
        throw std::invalid_argument("Circle cannot be null");
    }
    if (radius <= 0.0) {
        throw std::invalid_argument("Radius must be positive");
    }
}

RadiusConstraint::RadiusConstraint(std::shared_ptr<Sketching::Arc> arc, double radius)
    : shape(arc), targetRadius(radius) {
    if (!arc) {
        throw std::invalid_argument("Arc cannot be null");
    }
    if (radius <= 0.0) {
        throw std::invalid_argument("Radius must be positive");
    }
}

bool RadiusConstraint::isSatisfied() const {
    return getError() <= TOLERANCE;
}

bool RadiusConstraint::enforce() {
    if (!isValid()) {
        return false;
    }

    // Set radius based on shape type
    std::visit([this](auto& shape) {
        shape->setRadius(targetRadius);
    }, shape);

    return true;
}

double RadiusConstraint::getError() const {
    if (!isValid()) {
        return std::numeric_limits<double>::infinity();
    }

    return std::abs(getCurrentRadius() - targetRadius);
}

bool RadiusConstraint::isValid() const {
    // Check if shape exists and target radius is positive
    if (targetRadius <= 0.0) {
        return false;
    }

    return std::visit([](const auto& ptr) {
        return ptr != nullptr && std::isfinite(ptr->getRadius());
    }, shape);
}

double RadiusConstraint::getTargetRadius() const {
    return targetRadius;
}

double RadiusConstraint::getCurrentRadius() const {
    return std::visit([](const auto& shape) {
        return shape ? shape->getRadius() : 0.0;
    }, shape);
}

} // namespace Constraints
} // namespace RebelCAD
