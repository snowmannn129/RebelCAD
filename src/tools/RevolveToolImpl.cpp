#include "modeling/RevolveToolImpl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace RebelCAD {
namespace Modeling {

RevolveToolImpl::RevolveToolImpl()
    : is_configured(false)
    , has_profile(false) {
    // Initialize default params
    params.angle = 360.0f;
    params.isSubtractive = false;
    params.axis_start = glm::vec3(0.0f);
    params.axis_end = glm::vec3(0.0f, 1.0f, 0.0f);
    params.full_revolution = true;
    params.segments = 32;
    params.interpolateSections = true;

    // Initialize preview cache
    preview_cache.valid = false;
}

void RevolveToolImpl::Clear() {
    mesh_vertices.clear();
    mesh_normals.clear();
    mesh_uvs.clear();
    mesh_indices.clear();
    preview_cache.valid = false;
}

bool RevolveToolImpl::ValidateSection(const RevolveSection& section) const {
    // Check angle range
    if (section.angle < 0.0f || section.angle > params.angle) {
        return false;
    }

    // Check scale is positive
    if (section.scale <= 0.0f) {
        return false;
    }

    // Check profile vertices
    if (section.profile_vertices.empty() || 
        section.profile_vertices.size() % 3 != 0 ||
        section.profile_vertices.size() != profile_vertices.size()) {
        return false;
    }

    return true;
}

std::vector<float> RevolveToolImpl::InterpolateProfiles(
    const std::vector<float>& profile1,
    const std::vector<float>& profile2,
    float t) const {
    
    if (profile1.size() != profile2.size()) {
        return profile1; // Return first profile if sizes don't match
    }

    std::vector<float> result;
    result.reserve(profile1.size());

    for (size_t i = 0; i < profile1.size(); i += 3) {
        glm::vec3 p1(profile1[i], profile1[i+1], profile1[i+2]);
        glm::vec3 p2(profile2[i], profile2[i+1], profile2[i+2]);
        
        glm::vec3 interpolated = glm::mix(p1, p2, t);
        
        result.push_back(interpolated.x);
        result.push_back(interpolated.y);
        result.push_back(interpolated.z);
    }

    return result;
}

std::vector<float> RevolveToolImpl::TransformProfile(
    const std::vector<float>& profile,
    float scale,
    float twist) const {
    
    std::vector<float> result;
    result.reserve(profile.size());

    // Create transformation matrices
    glm::mat4 scaleMatrix = glm::scale(
        glm::mat4(1.0f), 
        glm::vec3(scale, scale, scale)
    );
    
    glm::mat4 twistMatrix = glm::rotate(
        glm::mat4(1.0f),
        twist,
        glm::normalize(params.axis_end - params.axis_start)
    );

    glm::mat4 transform = twistMatrix * scaleMatrix;

    // Transform each vertex
    for (size_t i = 0; i < profile.size(); i += 3) {
        glm::vec4 point(
            profile[i],
            profile[i+1],
            profile[i+2],
            1.0f
        );

        glm::vec4 transformed = transform * point;

        result.push_back(transformed.x);
        result.push_back(transformed.y);
        result.push_back(transformed.z);
    }

    return result;
}

} // namespace Modeling
} // namespace RebelCAD
