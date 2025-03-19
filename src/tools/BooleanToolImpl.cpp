#include "modeling/BooleanToolImpl.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/intersect.hpp>
#include <unordered_set>
#include <map>

namespace RebelCAD {
namespace Modeling {

std::vector<glm::vec3> BooleanTool::Impl::FindTriangleIntersection(
    const std::vector<glm::vec3>& tri1_verts,
    const std::vector<glm::vec3>& tri2_verts) const {
    
    std::vector<glm::vec3> intersection_points;
    
    // Get triangle normals
    glm::vec3 n1 = glm::normalize(glm::cross(
        tri1_verts[1] - tri1_verts[0],
        tri1_verts[2] - tri1_verts[0]
    ));
    glm::vec3 n2 = glm::normalize(glm::cross(
        tri2_verts[1] - tri2_verts[0],
        tri2_verts[2] - tri2_verts[0]
    ));

    // Check if triangles are coplanar
    if (glm::abs(glm::dot(n1, n2)) > 1.0f - params.intersection_tolerance) {
        return intersection_points;  // Coplanar triangles handled separately
    }

    // Find intersection line between triangle planes
    glm::vec3 line_dir = glm::normalize(glm::cross(n1, n2));
    
    // Find a point on the intersection line
    float d1 = glm::dot(n1, tri1_verts[0]);
    float d2 = glm::dot(n2, tri2_verts[0]);
    glm::vec3 line_point = glm::vec3(0.0f);
    
    // Solve for intersection line point using plane equations
    glm::mat3 A(n1, n2, line_dir);
    glm::vec3 b(d1, d2, 0.0f);
    line_point = glm::inverse(A) * b;

    // Find intersection points with triangle edges
    for (int i = 0; i < 3; ++i) {
        glm::vec3 edge_start = tri1_verts[i];
        glm::vec3 edge_end = tri1_verts[(i + 1) % 3];
        glm::vec3 edge_dir = edge_end - edge_start;
        
        float t;
        if (glm::intersectRayPlane(
            edge_start, glm::normalize(edge_dir),
            tri2_verts[0], n2,
            t)) {
            
            if (t >= 0.0f && t <= glm::length(edge_dir)) {
                glm::vec3 point = edge_start + t * glm::normalize(edge_dir);
                if (IsPointInTriangle(point, tri2_verts)) {
                    intersection_points.push_back(point);
                }
            }
        }
    }

    // Check second triangle's edges
    for (int i = 0; i < 3; ++i) {
        glm::vec3 edge_start = tri2_verts[i];
        glm::vec3 edge_end = tri2_verts[(i + 1) % 3];
        glm::vec3 edge_dir = edge_end - edge_start;
        
        float t;
        if (glm::intersectRayPlane(
            edge_start, glm::normalize(edge_dir),
            tri1_verts[0], n1,
            t)) {
            
            if (t >= 0.0f && t <= glm::length(edge_dir)) {
                glm::vec3 point = edge_start + t * glm::normalize(edge_dir);
                if (IsPointInTriangle(point, tri1_verts)) {
                    intersection_points.push_back(point);
                }
            }
        }
    }

    return intersection_points;
}

std::vector<std::vector<glm::vec3>> BooleanTool::Impl::SplitTriangle(
    const std::vector<glm::vec3>& triangle_verts,
    const std::pair<glm::vec3, glm::vec3>& intersection_line) const {
    
    std::vector<std::vector<glm::vec3>> result_triangles;
    
    // Find intersection points with triangle edges
    std::vector<glm::vec3> intersection_points;
    glm::vec3 line_dir = glm::normalize(
        intersection_line.second - intersection_line.first
    );

    for (int i = 0; i < 3; ++i) {
        glm::vec3 edge_start = triangle_verts[i];
        glm::vec3 edge_end = triangle_verts[(i + 1) % 3];
        glm::vec3 edge_dir = edge_end - edge_start;
        
        float t;
        if (glm::intersectRayPlane(
            intersection_line.first, line_dir,
            edge_start, glm::normalize(glm::cross(edge_dir, glm::vec3(0, 0, 1))),
            t)) {
            
            glm::vec3 point = intersection_line.first + t * line_dir;
            if (IsPointOnLineSegment(point, edge_start, edge_end)) {
                intersection_points.push_back(point);
            }
        }
    }

    if (intersection_points.size() != 2) {
        // Invalid intersection, return original triangle
        result_triangles.push_back(triangle_verts);
        return result_triangles;
    }

    // Sort intersection points by distance from first vertex
    std::sort(intersection_points.begin(), intersection_points.end(),
        [&](const glm::vec3& a, const glm::vec3& b) {
            return glm::length2(a - triangle_verts[0]) < 
                   glm::length2(b - triangle_verts[0]);
        });

    // Create new triangles based on intersection
    std::vector<glm::vec3> tri1, tri2, tri3;
    tri1 = {
        triangle_verts[0],
        intersection_points[0],
        intersection_points[1]
    };
    tri2 = {
        intersection_points[0],
        triangle_verts[1],
        intersection_points[1]
    };
    tri3 = {
        intersection_points[1],
        triangle_verts[1],
        triangle_verts[2]
    };

    // Add valid triangles to result
    if (IsValidTriangle(tri1)) result_triangles.push_back(tri1);
    if (IsValidTriangle(tri2)) result_triangles.push_back(tri2);
    if (IsValidTriangle(tri3)) result_triangles.push_back(tri3);

    return result_triangles;
}

bool BooleanTool::Impl::IsPointInMesh(
    const glm::vec3& point,
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices) const {
    
    int intersections = 0;
    glm::vec3 ray_dir(1.0f, 0.0f, 0.0f);  // Ray along X axis

    // Check intersection with each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0(vertices[indices[i] * 3],
                     vertices[indices[i] * 3 + 1],
                     vertices[indices[i] * 3 + 2]);
        glm::vec3 v1(vertices[indices[i+1] * 3],
                     vertices[indices[i+1] * 3 + 1],
                     vertices[indices[i+1] * 3 + 2]);
        glm::vec3 v2(vertices[indices[i+2] * 3],
                     vertices[indices[i+2] * 3 + 1],
                     vertices[indices[i+2] * 3 + 2]);

        float t;
        glm::vec2 bary;
        if (glm::intersectRayTriangle(point, ray_dir, v0, v1, v2, bary, t)) {
            if (t > 0.0f) {  // Intersection is in front of point
                intersections++;
            }
        }
    }

    // Odd number of intersections means point is inside
    return (intersections % 2) == 1;
}

void BooleanTool::Impl::OptimizeMesh(
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
            if (glm::length2(normal) > params.intersection_tolerance) {
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

// Helper functions
bool BooleanTool::Impl::IsPointInTriangle(
    const glm::vec3& point,
    const std::vector<glm::vec3>& triangle_verts) const {
    
    glm::vec3 v0 = triangle_verts[1] - triangle_verts[0];
    glm::vec3 v1 = triangle_verts[2] - triangle_verts[0];
    glm::vec3 v2 = point - triangle_verts[0];

    float d00 = glm::dot(v0, v0);
    float d01 = glm::dot(v0, v1);
    float d11 = glm::dot(v1, v1);
    float d20 = glm::dot(v2, v0);
    float d21 = glm::dot(v2, v1);

    float denom = d00 * d11 - d01 * d01;
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
}

bool BooleanTool::Impl::IsPointOnLineSegment(
    const glm::vec3& point,
    const glm::vec3& line_start,
    const glm::vec3& line_end) const {
    
    float line_length2 = glm::length2(line_end - line_start);
    if (line_length2 < params.intersection_tolerance) {
        return glm::length2(point - line_start) < params.intersection_tolerance;
    }

    float t = glm::dot(point - line_start, line_end - line_start) / line_length2;
    if (t < 0.0f || t > 1.0f) {
        return false;
    }

    glm::vec3 projection = line_start + t * (line_end - line_start);
    return glm::length2(point - projection) < params.intersection_tolerance;
}

bool BooleanTool::Impl::IsValidTriangle(
    const std::vector<glm::vec3>& triangle_verts) const {
    
    if (triangle_verts.size() != 3) {
        return false;
    }

    glm::vec3 v1 = triangle_verts[1] - triangle_verts[0];
    glm::vec3 v2 = triangle_verts[2] - triangle_verts[0];
    glm::vec3 normal = glm::cross(v1, v2);

    return glm::length2(normal) > params.intersection_tolerance;
}

} // namespace Modeling
} // namespace RebelCAD
