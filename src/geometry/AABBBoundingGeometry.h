#pragma once

#include "graphics/BoundingGeometry.h"
#include "graphics/SceneNode.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Axis-aligned bounding box implementation of BoundingGeometry
 */
class AABBBoundingGeometry : public BoundingGeometry {
public:
    AABBBoundingGeometry(const AABB& aabb = AABB())
        : m_aabb(aabb)
        , m_worldTransform(1.0f) {}

    virtual ~AABBBoundingGeometry() = default;

    /**
     * @brief Tests intersection with a sphere
     * @param origin Ray origin for sphere cast
     * @param direction Ray direction
     * @param radius Sphere radius
     * @return Intersection result
     */
    SphereIntersection intersectSphere(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float radius) const override {
        
        SphereIntersection result;

        // Transform ray to local space
        glm::mat4 invTransform = glm::inverse(m_worldTransform);
        glm::vec3 localOrigin = glm::vec3(invTransform * glm::vec4(origin, 1.0f));
        glm::vec3 localDir = glm::normalize(glm::vec3(invTransform * glm::vec4(direction, 0.0f)));

        // Ray-AABB intersection
        glm::vec3 invDir = 1.0f / localDir;
        glm::vec3 t1 = (m_aabb.min - localOrigin) * invDir;
        glm::vec3 t2 = (m_aabb.max - localOrigin) * invDir;
        glm::vec3 tmin = glm::min(t1, t2);
        glm::vec3 tmax = glm::max(t1, t2);

        float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
        float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

        if (tNear > tFar || tFar < 0) {
            return result;
        }

        // Account for sphere radius
        tNear -= radius;
        tFar += radius;

        if (tNear < 0) {
            // Inside sphere
            result.hasCollision = true;
            result.distance = 0;
            result.penetration = -tNear;
            result.point = origin;
            // Approximate normal as direction to closest point on AABB
            glm::vec3 center = (m_aabb.min + m_aabb.max) * 0.5f;
            result.normal = glm::normalize(localOrigin - center);
            result.normal = glm::normalize(glm::vec3(m_worldTransform * glm::vec4(result.normal, 0.0f)));
        } else {
            // Outside sphere
            result.hasCollision = true;
            result.distance = tNear;
            result.penetration = 0;
            result.point = origin + direction * tNear;
            glm::vec3 localPoint = localOrigin + localDir * tNear;
            // Calculate normal based on which face was hit
            glm::vec3 localNormal(0.0f);
            float epsilon = 0.0001f;
            if (std::abs(localPoint.x - m_aabb.min.x) < epsilon) localNormal.x = -1.0f;
            else if (std::abs(localPoint.x - m_aabb.max.x) < epsilon) localNormal.x = 1.0f;
            else if (std::abs(localPoint.y - m_aabb.min.y) < epsilon) localNormal.y = -1.0f;
            else if (std::abs(localPoint.y - m_aabb.max.y) < epsilon) localNormal.y = 1.0f;
            else if (std::abs(localPoint.z - m_aabb.min.z) < epsilon) localNormal.z = -1.0f;
            else if (std::abs(localPoint.z - m_aabb.max.z) < epsilon) localNormal.z = 1.0f;
            result.normal = glm::normalize(glm::vec3(m_worldTransform * glm::vec4(localNormal, 0.0f)));
        }

        return result;
    }

    /**
     * @brief Finds closest point on geometry to given point
     * @param point Point to test against
     * @return Pair of closest point and surface normal
     */
    std::pair<glm::vec3, glm::vec3> findClosestPoint(
        const glm::vec3& point) const override {
        
        // Transform point to local space
        glm::mat4 invTransform = glm::inverse(m_worldTransform);
        glm::vec3 localPoint = glm::vec3(invTransform * glm::vec4(point, 1.0f));

        // Clamp point to AABB bounds
        glm::vec3 closest = glm::clamp(localPoint, m_aabb.min, m_aabb.max);

        // Calculate normal
        glm::vec3 normal(0.0f);
        if (localPoint.x < m_aabb.min.x) normal.x = -1.0f;
        else if (localPoint.x > m_aabb.max.x) normal.x = 1.0f;
        if (localPoint.y < m_aabb.min.y) normal.y = -1.0f;
        else if (localPoint.y > m_aabb.max.y) normal.y = 1.0f;
        if (localPoint.z < m_aabb.min.z) normal.z = -1.0f;
        else if (localPoint.z > m_aabb.max.z) normal.z = 1.0f;
        normal = glm::normalize(normal);

        // Transform back to world space
        closest = glm::vec3(m_worldTransform * glm::vec4(closest, 1.0f));
        normal = glm::normalize(glm::vec3(m_worldTransform * glm::vec4(normal, 0.0f)));

        return {closest, normal};
    }

    /**
     * @brief Gets the geometry's axis-aligned bounding box
     * @return Pair of min and max points
     */
    std::pair<glm::vec3, glm::vec3> getAABB() const override {
        // Transform AABB corners to world space
        std::vector<glm::vec3> corners = {
            glm::vec3(m_aabb.min.x, m_aabb.min.y, m_aabb.min.z),
            glm::vec3(m_aabb.max.x, m_aabb.min.y, m_aabb.min.z),
            glm::vec3(m_aabb.min.x, m_aabb.max.y, m_aabb.min.z),
            glm::vec3(m_aabb.max.x, m_aabb.max.y, m_aabb.min.z),
            glm::vec3(m_aabb.min.x, m_aabb.min.y, m_aabb.max.z),
            glm::vec3(m_aabb.max.x, m_aabb.min.y, m_aabb.max.z),
            glm::vec3(m_aabb.min.x, m_aabb.max.y, m_aabb.max.z),
            glm::vec3(m_aabb.max.x, m_aabb.max.y, m_aabb.max.z)
        };

        glm::vec3 worldMin(std::numeric_limits<float>::max());
        glm::vec3 worldMax(std::numeric_limits<float>::lowest());

        for (const auto& corner : corners) {
            glm::vec3 worldCorner = glm::vec3(m_worldTransform * glm::vec4(corner, 1.0f));
            worldMin = glm::min(worldMin, worldCorner);
            worldMax = glm::max(worldMax, worldCorner);
        }

        return {worldMin, worldMax};
    }

    /**
     * @brief Updates geometry transform
     * @param transform New world transform matrix
     */
    void updateTransform(const glm::mat4& transform) override {
        m_worldTransform = transform;
    }

private:
    AABB m_aabb;
    glm::mat4 m_worldTransform;
};

} // namespace Graphics
} // namespace RebelCAD
