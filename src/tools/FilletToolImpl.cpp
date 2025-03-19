#include "modeling/FilletToolImpl.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_set>
#include <map>
#include <cmath>

namespace RebelCAD {
namespace Modeling {

void FilletTool::Impl::BuildTopology() {
    topology = TopologyData();

    // Build vertex to face mapping
    for (size_t i = 0; i < mesh_indices.size(); i += 3) {
        unsigned int face_idx = i / 3;
        for (size_t j = 0; j < 3; ++j) {
            topology.vertex_to_faces[mesh_indices[i + j]].push_back(face_idx);
        }
    }

    // Build vertex to edge mapping
    size_t edge_idx = 0;
    for (size_t i = 0; i < mesh_indices.size(); i += 3) {
        for (size_t j = 0; j < 3; ++j) {
            unsigned int v1 = mesh_indices[i + j];
            unsigned int v2 = mesh_indices[i + ((j + 1) % 3)];
            
            // Ensure consistent edge ordering
            if (v1 > v2) std::swap(v1, v2);
            
            topology.vertex_to_edges[v1].push_back(edge_idx);
            topology.vertex_to_edges[v2].push_back(edge_idx);
            edge_idx++;
        }
    }

    // Build face to edge mapping
    edge_idx = 0;
    for (size_t i = 0; i < mesh_indices.size(); i += 3) {
        unsigned int face_idx = i / 3;
        for (size_t j = 0; j < 3; ++j) {
            topology.face_to_edges[face_idx].push_back(edge_idx);
            edge_idx++;
        }
    }

    // Build edge to face mapping
    edge_idx = 0;
    for (size_t i = 0; i < mesh_indices.size(); i += 3) {
        unsigned int face_idx = i / 3;
        for (size_t j = 0; j < 3; ++j) {
            topology.edge_to_faces[edge_idx].push_back(face_idx);
            edge_idx++;
        }
    }
}

FilletTool::Impl::FilletGeometry
FilletTool::Impl::GenerateEdgeFillet(const Edge& edge) const {
    FilletGeometry geometry;

    // Get edge vertices
    glm::vec3 v1(mesh_vertices[edge.vertex_indices[0] * 3],
                 mesh_vertices[edge.vertex_indices[0] * 3 + 1],
                 mesh_vertices[edge.vertex_indices[0] * 3 + 2]);
    glm::vec3 v2(mesh_vertices[edge.vertex_indices[1] * 3],
                 mesh_vertices[edge.vertex_indices[1] * 3 + 1],
                 mesh_vertices[edge.vertex_indices[1] * 3 + 2]);

    // Calculate edge direction and length
    glm::vec3 edge_dir = glm::normalize(v2 - v1);
    float edge_length = glm::length(v2 - v1);

    // Get adjacent face normals
    std::vector<glm::vec3> face_normals;
    for (unsigned int face_idx : edge.adjacent_faces) {
        std::vector<glm::vec3> face_verts;
        for (size_t i = 0; i < 3; ++i) {
            unsigned int idx = mesh_indices[face_idx * 3 + i];
            face_verts.emplace_back(
                mesh_vertices[idx * 3],
                mesh_vertices[idx * 3 + 1],
                mesh_vertices[idx * 3 + 2]
            );
        }
        face_normals.push_back(CalculateFaceNormal(face_verts));
    }

    // Generate fillet segments
    int num_segments = params.segments;
    for (int i = 0; i <= num_segments; ++i) {
        float t = static_cast<float>(i) / num_segments;
        float radius = CalculateRadius(edge, t);
        glm::vec3 center = v1 + t * (v2 - v1);

        // Generate circle points around edge
        for (int j = 0; j <= params.segments; ++j) {
            float angle = (2.0f * glm::pi<float>() * j) / params.segments;
            glm::vec3 normal = glm::normalize(
                std::cos(angle) * face_normals[0] +
                std::sin(angle) * face_normals[1]
            );

            glm::vec3 point = center + radius * normal;
            geometry.vertices.push_back(point.x);
            geometry.vertices.push_back(point.y);
            geometry.vertices.push_back(point.z);

            // Generate UV coordinates
            glm::vec2 uv = GenerateUVCoordinates(point, normal, edge_dir);
            geometry.uvs.push_back(uv.x);
            geometry.uvs.push_back(uv.y);
        }
    }

    // Generate indices for quad strips
    for (int i = 0; i < num_segments; ++i) {
        for (int j = 0; j < params.segments; ++j) {
            unsigned int base = i * (params.segments + 1) + j;
            
            // First triangle
            geometry.indices.push_back(base);
            geometry.indices.push_back(base + params.segments + 1);
            geometry.indices.push_back(base + 1);
            
            // Second triangle
            geometry.indices.push_back(base + 1);
            geometry.indices.push_back(base + params.segments + 1);
            geometry.indices.push_back(base + params.segments + 2);
        }
    }

    return geometry;
}

FilletTool::Impl::FilletGeometry
FilletTool::Impl::GenerateFaceFillet(const Face& face) const {
    FilletGeometry geometry;

    // Get face vertices
    std::vector<glm::vec3> face_verts;
    for (unsigned int idx : face.vertex_indices) {
        face_verts.emplace_back(
            mesh_vertices[idx * 3],
            mesh_vertices[idx * 3 + 1],
            mesh_vertices[idx * 3 + 2]
        );
    }

    // Calculate face center
    glm::vec3 center(0.0f);
    for (const auto& vert : face_verts) {
        center += vert;
    }
    center /= face_verts.size();

    // Generate fillet surface
    for (size_t i = 0; i < face_verts.size(); ++i) {
        size_t next = (i + 1) % face_verts.size();
        glm::vec3 edge_dir = glm::normalize(face_verts[next] - face_verts[i]);
        glm::vec3 to_center = glm::normalize(center - face_verts[i]);

        // Generate fillet points
        for (int j = 0; j <= params.segments; ++j) {
            float t = static_cast<float>(j) / params.segments;
            float radius = params.radius * (1.0f - t);  // Taper towards center
            glm::vec3 normal = glm::normalize(glm::cross(edge_dir, face.normal));
            glm::vec3 point = face_verts[i] + t * (center - face_verts[i]) + radius * normal;

            geometry.vertices.push_back(point.x);
            geometry.vertices.push_back(point.y);
            geometry.vertices.push_back(point.z);

            // Generate UV coordinates
            glm::vec2 uv = GenerateUVCoordinates(point, normal, edge_dir);
            geometry.uvs.push_back(uv.x);
            geometry.uvs.push_back(uv.y);
        }
    }

    // Generate indices
    size_t verts_per_edge = params.segments + 1;
    for (size_t i = 0; i < face_verts.size(); ++i) {
        size_t next = (i + 1) % face_verts.size();
        for (int j = 0; j < params.segments; ++j) {
            unsigned int base = i * verts_per_edge + j;
            unsigned int next_base = next * verts_per_edge + j;

            // First triangle
            geometry.indices.push_back(base);
            geometry.indices.push_back(next_base);
            geometry.indices.push_back(base + 1);

            // Second triangle
            geometry.indices.push_back(base + 1);
            geometry.indices.push_back(next_base);
            geometry.indices.push_back(next_base + 1);
        }
    }

    return geometry;
}

float FilletTool::Impl::CalculateRadius(const Edge& edge, float t) const {
    if (!params.variable_radius || params.radius_points.empty()) {
        return params.radius;
    }

    // Find radius points that bound t
    float prev_t = 0.0f;
    float prev_r = edge.start_radius;
    float next_t = 1.0f;
    float next_r = edge.end_radius;

    for (size_t i = 0; i < params.radius_points.size(); i += 2) {
        float point_t = params.radius_points[i];
        float point_r = params.radius_points[i + 1];

        if (point_t < t && point_t > prev_t) {
            prev_t = point_t;
            prev_r = point_r;
        }
        if (point_t > t && point_t < next_t) {
            next_t = point_t;
            next_r = point_r;
        }
    }

    // Interpolate radius
    float blend = (t - prev_t) / (next_t - prev_t);
    return glm::mix(prev_r, next_r, blend);
}

glm::vec2 FilletTool::Impl::GenerateUVCoordinates(
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec3& edge_direction) const {
    
    // Project position onto edge direction for U coordinate
    float u = glm::dot(position, edge_direction);

    // Use angle around edge for V coordinate
    glm::vec3 ref_vec = glm::cross(edge_direction, glm::vec3(0, 0, 1));
    if (glm::length2(ref_vec) < params.tolerance) {
        ref_vec = glm::cross(edge_direction, glm::vec3(1, 0, 0));
    }
    ref_vec = glm::normalize(ref_vec);

    float angle = std::atan2(
        glm::dot(glm::cross(ref_vec, normal), edge_direction),
        glm::dot(ref_vec, normal)
    );
    float v = angle / (2.0f * glm::pi<float>());

    return glm::vec2(u, v);
}

void FilletTool::Impl::OptimizeMesh(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices) const {
    
    if (vertices.empty() || indices.empty()) {
        return;
    }

    // Remove duplicate vertices
    std::vector<float> unique_verts;
    std::vector<unsigned int> vert_map(vertices.size() / 3);
    std::unordered_map<std::string, unsigned int> vert_lookup;

    for (size_t i = 0; i < vertices.size(); i += 3) {
        std::string key = std::to_string(vertices[i]) + "," +
                         std::to_string(vertices[i+1]) + "," +
                         std::to_string(vertices[i+2]);

        auto it = vert_lookup.find(key);
        if (it == vert_lookup.end()) {
            // New unique vertex
            vert_map[i/3] = unique_verts.size() / 3;
            vert_lookup[key] = unique_verts.size() / 3;
            unique_verts.push_back(vertices[i]);
            unique_verts.push_back(vertices[i+1]);
            unique_verts.push_back(vertices[i+2]);
        } else {
            // Duplicate vertex
            vert_map[i/3] = it->second;
        }
    }

    // Update indices to use unique vertices
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = vert_map[indices[i]];
    }

    // Remove degenerate triangles
    std::vector<unsigned int> valid_indices;
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i1 = indices[i];
        unsigned int i2 = indices[i+1];
        unsigned int i3 = indices[i+2];

        // Skip triangles with duplicate vertices
        if (i1 != i2 && i2 != i3 && i3 != i1) {
            glm::vec3 v1(unique_verts[i1*3], unique_verts[i1*3+1], unique_verts[i1*3+2]);
            glm::vec3 v2(unique_verts[i2*3], unique_verts[i2*3+1], unique_verts[i2*3+2]);
            glm::vec3 v3(unique_verts[i3*3], unique_verts[i3*3+1], unique_verts[i3*3+2]);

            // Check triangle area
            glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
            if (glm::length2(normal) > params.tolerance) {
                valid_indices.push_back(i1);
                valid_indices.push_back(i2);
                valid_indices.push_back(i3);
            }
        }
    }

    // Update mesh data
    vertices = unique_verts;
    indices = valid_indices;
}

std::vector<unsigned int> FilletTool::Impl::FindSharedVertices(
    const std::vector<unsigned int>& face1_indices,
    const std::vector<unsigned int>& face2_indices) const {
    
    std::vector<unsigned int> shared_vertices;
    std::unordered_set<unsigned int> face1_verts(
        face1_indices.begin(),
        face1_indices.end()
    );

    for (unsigned int idx : face2_indices) {
        if (face1_verts.find(idx) != face1_verts.end()) {
            shared_vertices.push_back(idx);
        }
    }

    return shared_vertices;
}

glm::vec3 FilletTool::Impl::CalculateFaceNormal(
    const std::vector<glm::vec3>& face_vertices) const {
    
    if (face_vertices.size() < 3) {
        return glm::vec3(0.0f);
    }

    glm::vec3 normal(0.0f);
    for (size_t i = 0; i < face_vertices.size(); ++i) {
        const glm::vec3& v1 = face_vertices[i];
        const glm::vec3& v2 = face_vertices[(i + 1) % face_vertices.size()];
        const glm::vec3& v3 = face_vertices[(i + 2) % face_vertices.size()];
        normal += glm::cross(v2 - v1, v3 - v1);
    }

    return glm::normalize(normal);
}

bool FilletTool::Impl::IsValidEdge(const Edge& edge) const {
    if (edge.vertex_indices.size() != 2) {
        return false;
    }

    if (edge.adjacent_faces.size() < 2) {
        return false;
    }

    // Check vertex indices are valid
    for (unsigned int idx : edge.vertex_indices) {
        if (idx * 3 >= mesh_vertices.size()) {
            return false;
        }
    }

    // Check face indices are valid
    for (unsigned int idx : edge.adjacent_faces) {
        if (idx * 3 >= mesh_indices.size()) {
            return false;
        }
    }

    return true;
}

bool FilletTool::Impl::IsValidFace(const Face& face) const {
    if (face.vertex_indices.size() < 3) {
        return false;
    }

    // Check vertex indices are valid
    for (unsigned int idx : face.vertex_indices) {
        if (idx * 3 >= mesh_vertices.size()) {
            return false;
        }
    }

    // Check edge indices are valid
    for (unsigned int idx : face.adjacent_edges) {
        auto it = topology.edge_to_faces.find(idx);
        if (it == topology.edge_to_faces.end()) {
            return false;
        }
    }

    return true;
}

} // namespace Modeling
} // namespace RebelCAD
