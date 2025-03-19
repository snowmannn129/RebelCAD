#pragma once

#include "Graphics/BoundingGeometry.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Simple box geometry for testing collision detection
 */
class BoxGeometry : public BoundingGeometry {
public:
    BoxGeometry(const glm::vec3& center, const glm::vec3& size)
        : center_(center), size_(size) {}

    SphereIntersection intersectSphere(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float radius) const override {
        
        SphereIntersection result;
        
        // Calculate box bounds
        glm::vec3 min = center_ - size_ * 0.5f;
        glm::vec3 max = center_ + size_ * 0.5f;
        
        // Ray-box intersection with sphere radius offset
        glm::vec3 invDir = 1.0f / direction;
        glm::vec3 t1 = (min - origin) * invDir;
        glm::vec3 t2 = (max - origin) * invDir;
        
        glm::vec3 tmin = glm::min(t1, t2);
        glm::vec3 tmax = glm::max(t1, t2);
        
        float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
        float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
        
        // Account for sphere radius
        tNear -= radius;
        tFar += radius;
        
        if (tNear > tFar || tFar < 0) {
            return result; // No intersection
        }
        
        // Collision detected
        result.hasCollision = true;
        result.distance = tNear;
        
        // Calculate intersection point
        result.point = origin + direction * tNear;
        
        // Calculate normal based on entry face
        glm::vec3 centerToPoint = result.point - center_;
        glm::vec3 absCenter = glm::abs(centerToPoint);
        
        if (absCenter.x >= absCenter.y && absCenter.x >= absCenter.z) {
            result.normal = glm::vec3(glm::sign(centerToPoint.x), 0, 0);
        } else if (absCenter.y >= absCenter.z) {
            result.normal = glm::vec3(0, glm::sign(centerToPoint.y), 0);
        } else {
            result.normal = glm::vec3(0, 0, glm::sign(centerToPoint.z));
        }
        
        // Calculate penetration depth
        result.penetration = radius - tNear;
        
        return result;
    }

    std::pair<glm::vec3, glm::vec3> findClosestPoint(
        const glm::vec3& point) const override {
        
        // Clamp point to box bounds
        glm::vec3 min = center_ - size_ * 0.5f;
        glm::vec3 max = center_ + size_ * 0.5f;
        glm::vec3 closest = glm::clamp(point, min, max);
        
        // Calculate normal based on closest face
        glm::vec3 centerToPoint = closest - center_;
        glm::vec3 absCenter = glm::abs(centerToPoint);
        glm::vec3 normal;
        
        if (absCenter.x >= absCenter.y && absCenter.x >= absCenter.z) {
            normal = glm::vec3(glm::sign(centerToPoint.x), 0, 0);
        } else if (absCenter.y >= absCenter.z) {
            normal = glm::vec3(0, glm::sign(centerToPoint.y), 0);
        } else {
            normal = glm::vec3(0, 0, glm::sign(centerToPoint.z));
        }
        
        return {closest, normal};
    }

    std::pair<glm::vec3, glm::vec3> getAABB() const override {
        return {
            center_ - size_ * 0.5f,  // min
            center_ + size_ * 0.5f   // max
        };
    }

    void updateTransform(const glm::mat4& transform) override {
        center_ = glm::vec3(transform * glm::vec4(center_, 1.0f));
        // Note: This simple implementation doesn't handle non-uniform scaling
    }

private:
    glm::vec3 center_;
    glm::vec3 size_;
};

/**
 * @brief Simple box test object for unit testing
 */
class TestBox : public SceneObject {
public:
    TestBox(const glm::vec3& center, const glm::vec3& size) 
        : center_(center), size_(size), geometry_(center, size) {}

    const BoundingGeometry& getBoundingGeometry() const override {
        return geometry_;
    }

    glm::mat4 getWorldTransform() const override {
        return glm::translate(glm::mat4(1.0f), center_);
    }

    glm::vec3 getWorldPosition() const override {
        return center_;
    }

    void update() override {}

private:
    glm::vec3 center_;
    glm::vec3 size_;
    BoxGeometry geometry_;
};

} // namespace Graphics
} // namespace RebelCAD
