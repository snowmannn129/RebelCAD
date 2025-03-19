#pragma once

#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Result of a sphere intersection test
 */
struct SphereIntersection {
    bool hasCollision = false;        ///< Whether intersection occurred
    float distance = 0.0f;            ///< Distance to intersection point
    glm::vec3 point;                  ///< Point of intersection
    glm::vec3 normal;                 ///< Surface normal at intersection
    float penetration = 0.0f;         ///< Penetration depth
};

/**
 * @brief Abstract base class for bounding geometry used in collision detection
 */
class BoundingGeometry {
public:
    BoundingGeometry() = default;
    virtual ~BoundingGeometry() = default;

    /**
     * @brief Tests intersection with a sphere
     * @param origin Ray origin for sphere cast
     * @param direction Ray direction
     * @param radius Sphere radius
     * @return Intersection result
     */
    virtual SphereIntersection intersectSphere(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float radius) const = 0;

    /**
     * @brief Finds closest point on geometry to given point
     * @param point Point to test against
     * @return Pair of closest point and surface normal
     */
    virtual std::pair<glm::vec3, glm::vec3> findClosestPoint(
        const glm::vec3& point) const = 0;

    /**
     * @brief Gets the geometry's axis-aligned bounding box
     * @return Pair of min and max points
     */
    virtual std::pair<glm::vec3, glm::vec3> getAABB() const = 0;

    /**
     * @brief Updates geometry transform
     * @param transform New world transform matrix
     */
    virtual void updateTransform(const glm::mat4& transform) = 0;
};

} // namespace Graphics
} // namespace RebelCAD
