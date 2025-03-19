#include "modeling/RevolveTool.h"
#include <cmath>
#include <algorithm>
#include <numbers>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Error.h"

namespace RebelCAD {
namespace Modeling {

RevolveTool::RevolveTool() : impl(std::make_unique<RevolveToolImpl>()) {}
RevolveTool::~RevolveTool() = default;

RevolveTool::ErrorCode RevolveTool::SetProfile(const std::vector<float>& profile_vertices) {
    if (profile_vertices.empty() || profile_vertices.size() % 3 != 0) {
        return ErrorCode::InvalidMesh;
    }

    impl->profile_vertices = profile_vertices;
    impl->has_profile = true;
    impl->Clear();
    
    return ValidateProfile() ? ErrorCode::None : ErrorCode::InvalidMesh;
}

RevolveTool::ErrorCode RevolveTool::Configure(const RevolveParams& params) {
    // Validate parameters
    if (params.angle <= 0.0f || params.angle > 360.0f) {
        return ErrorCode::InvalidMesh;
    }
    
    if (params.segments < 4) {
        return ErrorCode::InvalidMesh;
    }

    // Validate axis points aren't identical
    glm::vec3 axis = params.axis_end - params.axis_start;
    if (glm::length2(axis) < 1e-6f) {
        return ErrorCode::InvalidMesh;
    }

    // Validate sections if provided
    for (const auto& section : params.sections) {
        if (!impl->ValidateSection(section)) {
            return ErrorCode::InvalidMesh;
        }
    }

    impl->params = params;
    impl->is_configured = true;
    impl->Clear();

    return ErrorCode::None;
}

RevolveTool::ErrorCode RevolveTool::Execute() {
    if (!impl->has_profile || !impl->is_configured) {
        return ErrorCode::CADError;
    }

    return GenerateMesh();
}

void RevolveTool::Cancel() {
    impl->Clear();
}

bool RevolveTool::ValidateProfile() const {
    if (!impl->has_profile) {
        return false;
    }

    // Check profile doesn't intersect rotation axis
    for (size_t i = 0; i < impl->profile_vertices.size(); i += 3) {
        glm::vec3 point(impl->profile_vertices[i],
                       impl->profile_vertices[i + 1],
                       impl->profile_vertices[i + 2]);

        // Calculate distance from point to axis line
        glm::vec3 axis = impl->params.axis_end - impl->params.axis_start;
        glm::vec3 pointToStart = point - impl->params.axis_start;
        
        // Cross product length divided by axis length gives distance to axis
        float dist = glm::length(glm::cross(pointToStart, axis)) / glm::length(axis);

        if (dist < 1e-4f) {
            return false; // Too close to axis
        }
    }

    // Check for self-intersection
    if (impl->params.angle >= 360.0f) {
        // For full revolutions, check if any line segment in profile
        // would intersect with itself when revolved
        for (size_t i = 0; i < impl->profile_vertices.size(); i += 3) {
            glm::vec3 p1(impl->profile_vertices[i],
                        impl->profile_vertices[i + 1],
                        impl->profile_vertices[i + 2]);
            
            // Project point onto plane perpendicular to axis
            glm::vec3 axis_norm = glm::normalize(impl->params.axis_end - impl->params.axis_start);
            float dist_along_axis = glm::dot(p1 - impl->params.axis_start, axis_norm);
            glm::vec3 projected = p1 - dist_along_axis * axis_norm;
            
            // Check distance to axis
            float radius = glm::length(projected - impl->params.axis_start);
            
            // For any other point
            for (size_t j = i + 3; j < impl->profile_vertices.size(); j += 3) {
                glm::vec3 p2(impl->profile_vertices[j],
                            impl->profile_vertices[j + 1],
                            impl->profile_vertices[j + 2]);
                
                // Project second point
                dist_along_axis = glm::dot(p2 - impl->params.axis_start, axis_norm);
                glm::vec3 projected2 = p2 - dist_along_axis * axis_norm;
                float radius2 = glm::length(projected2 - impl->params.axis_start);
                
                // If radii overlap, potential self-intersection
                if (std::abs(radius - radius2) < 1e-4f) {
                    return false;
                }
            }
        }
    }

    return true;
}

int RevolveTool::CalculateOptimalSegments() const {
    // Base number of segments on revolution angle
    float angle = impl->params.angle;
    if (angle >= 360.0f) {
        return impl->params.segments;
    }
    
    // For partial revolutions, scale segments proportionally
    return std::max(4, static_cast<int>(impl->params.segments * angle / 360.0f));
}

void RevolveTool::GenerateProfileGeometry(
    const std::vector<float>& profile,
    float angle,
    float u,
    size_t& vertex_offset) {
    
    glm::vec3 axis = glm::normalize(impl->params.axis_end - impl->params.axis_start);
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

    size_t num_profile_points = profile.size() / 3;
    
    for (size_t i = 0; i < profile.size(); i += 3) {
        // Calculate vertex position
        glm::vec4 point(
            profile[i] - impl->params.axis_start.x,
            profile[i+1] - impl->params.axis_start.y,
            profile[i+2] - impl->params.axis_start.z,
            1.0f
        );

        glm::vec4 rotated = rotation * point;
        glm::vec3 final = glm::vec3(rotated) + impl->params.axis_start;

        // Add vertex
        impl->mesh_vertices.push_back(final.x);
        impl->mesh_vertices.push_back(final.y);
        impl->mesh_vertices.push_back(final.z);

        // Calculate normal
        glm::vec3 normal = glm::normalize(final - impl->params.axis_start);
        impl->mesh_normals.push_back(normal.x);
        impl->mesh_normals.push_back(normal.y);
        impl->mesh_normals.push_back(normal.z);

        // Calculate UV coordinates
        float v = static_cast<float>(i/3) / (num_profile_points - 1);
        impl->mesh_uvs.push_back(u);
        impl->mesh_uvs.push_back(v);
    }

    vertex_offset += num_profile_points;
}

RevolveTool::ErrorCode RevolveTool::GenerateMultiSectionMesh() {
    // Sort sections by angle
    auto sections = impl->params.sections;
    std::sort(sections.begin(), sections.end(),
              [](const RevolveSection& a, const RevolveSection& b) {
                  return a.angle < b.angle;
              });

    // Calculate segments between each section
    int total_segments = impl->params.segments;
    std::vector<int> section_segments;
    float total_angle = impl->params.angle;
    
    for (size_t i = 0; i < sections.size() - 1; ++i) {
        float angle_diff = sections[i+1].angle - sections[i].angle;
        int seg_count = std::max(4, static_cast<int>(total_segments * angle_diff / total_angle));
        section_segments.push_back(seg_count);
    }

    // Generate vertices for each section
    size_t num_profile_points = impl->profile_vertices.size() / 3;
    size_t total_points = 0;
    
    for (const auto& seg_count : section_segments) {
        total_points += num_profile_points * (seg_count + 1);
    }

    impl->mesh_vertices.reserve(total_points * 3);
    impl->mesh_normals.reserve(total_points * 3);
    impl->mesh_uvs.reserve(total_points * 2);

    // Generate geometry between each pair of sections
    size_t vertex_offset = 0;
    float u_offset = 0.0f;

    for (size_t sect = 0; sect < sections.size() - 1; ++sect) {
        const auto& section1 = sections[sect];
        const auto& section2 = sections[sect + 1];
        int seg_count = section_segments[sect];

        for (int i = 0; i <= seg_count; ++i) {
            float t = static_cast<float>(i) / seg_count;
            float angle = glm::mix(section1.angle, section2.angle, t);
            float scale = glm::mix(section1.scale, section2.scale, t);
            float twist = glm::mix(section1.twist, section2.twist, t);
            
            // Interpolate profiles
            auto profile = impl->InterpolateProfiles(
                section1.profile_vertices,
                section2.profile_vertices,
                t);
            
            // Transform profile
            profile = impl->TransformProfile(profile, scale, twist);

            // Generate vertices for this profile
            float u = u_offset + t * (1.0f / sections.size());
            GenerateProfileGeometry(profile, angle, u, vertex_offset);
        }

        u_offset += 1.0f / sections.size();
    }

    // Generate indices
    impl->mesh_indices.reserve(num_profile_points * total_segments * 6);
    
    vertex_offset = 0;
    for (const auto& seg_count : section_segments) {
        for (int i = 0; i < seg_count; ++i) {
            for (size_t j = 0; j < num_profile_points - 1; ++j) {
                size_t base = vertex_offset + i * num_profile_points + j;
                
                // First triangle
                impl->mesh_indices.push_back(base);
                impl->mesh_indices.push_back(base + num_profile_points);
                impl->mesh_indices.push_back(base + 1);
                
                // Second triangle
                impl->mesh_indices.push_back(base + 1);
                impl->mesh_indices.push_back(base + num_profile_points);
                impl->mesh_indices.push_back(base + num_profile_points + 1);
            }
        }
        vertex_offset += (seg_count + 1) * num_profile_points;
    }

    return ErrorCode::None;
}

RevolveTool::ErrorCode RevolveTool::GenerateMesh() {
    if (!impl->has_profile || !impl->is_configured) {
        return ErrorCode::CADError;
    }

    impl->Clear();

    // Handle multi-section revolve if sections are defined
    if (!impl->params.sections.empty()) {
        return GenerateMultiSectionMesh();
    }

    int num_segments = CalculateOptimalSegments();
    float angle_step = impl->params.angle * std::numbers::pi_v<float> / (180.0f * num_segments);

    // Generate vertices and UVs
    size_t num_profile_points = impl->profile_vertices.size() / 3;
    impl->mesh_vertices.reserve(num_profile_points * (num_segments + 1) * 3);
    impl->mesh_uvs.reserve(num_profile_points * (num_segments + 1) * 2);
    impl->mesh_normals.reserve(num_profile_points * (num_segments + 1) * 3);
    
    size_t vertex_offset = 0;
    for (int i = 0; i <= num_segments; ++i) {
        float angle = angle_step * i;
        float u = static_cast<float>(i) / num_segments;
        GenerateProfileGeometry(impl->profile_vertices, angle, u, vertex_offset);
    }

    // Generate indices for triangle strips
    impl->mesh_indices.reserve(num_profile_points * num_segments * 6);
    
    for (int i = 0; i < num_segments; ++i) {
        for (size_t j = 0; j < num_profile_points - 1; ++j) {
            size_t base = i * num_profile_points + j;
            
            // First triangle
            impl->mesh_indices.push_back(base);
            impl->mesh_indices.push_back(base + num_profile_points);
            impl->mesh_indices.push_back(base + 1);
            
            // Second triangle
            impl->mesh_indices.push_back(base + 1);
            impl->mesh_indices.push_back(base + num_profile_points);
            impl->mesh_indices.push_back(base + num_profile_points + 1);
        }
    }

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
