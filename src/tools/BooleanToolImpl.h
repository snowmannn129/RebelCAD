#pragma once

#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "modeling/BooleanTool.h"

namespace RebelCAD {
namespace Modeling {

class BooleanTool::Impl {
public:
    // Input data
    std::vector<float> first_body_vertices;
    std::vector<unsigned int> first_body_indices;
    std::vector<float> second_body_vertices;
    std::vector<unsigned int> second_body_indices;
    BooleanParams params;
    bool has_first_body = false;
    bool has_second_body = false;
    bool is_configured = false;

    // Result data
    std::vector<float> result_vertices;
    std::vector<float> result_normals;
    std::vector<float> result_uvs;
    std::vector<unsigned int> result_indices;

    // Preview cache
    struct PreviewCache {
        bool valid = false;
        std::vector<float> cached_first_vertices;
        std::vector<unsigned int> cached_first_indices;
        std::vector<float> cached_second_vertices;
        std::vector<unsigned int> cached_second_indices;
        BooleanParams cached_params;
        std::vector<float> mesh_vertices;
        std::vector<float> mesh_normals;
        std::vector<float> mesh_uvs;
        std::vector<unsigned int> mesh_indices;
    } preview_cache;

    // Intersection data
    struct IntersectionData {
        std::vector<glm::vec3> intersection_points;
        std::vector<glm::vec3> intersection_normals;
        std::unordered_map<size_t, size_t> vertex_map;  // Maps original vertices to split vertices
    };
    IntersectionData intersection_data;

    Impl() = default;

    /**
     * @brief Clear all cached and temporary data
     */
    void Clear() {
        result_vertices.clear();
        result_normals.clear();
        result_uvs.clear();
        result_indices.clear();
        preview_cache.valid = false;
        intersection_data = IntersectionData();
    }

    /**
     * @brief Find intersections between two triangles
     * @param tri1_verts First triangle vertices
     * @param tri2_verts Second triangle vertices
     * @return Vector of intersection points
     */
    std::vector<glm::vec3> FindTriangleIntersection(
        const std::vector<glm::vec3>& tri1_verts,
        const std::vector<glm::vec3>& tri2_verts) const;

    /**
     * @brief Split a triangle along an intersection line
     * @param triangle_verts Triangle vertices to split
     * @param intersection_line Line to split along
     * @return Vector of new triangles
     */
    std::vector<std::vector<glm::vec3>> SplitTriangle(
        const std::vector<glm::vec3>& triangle_verts,
        const std::pair<glm::vec3, glm::vec3>& intersection_line) const;

    /**
     * @brief Determine if a point is inside a mesh
     * @param point Point to test
     * @param vertices Mesh vertices
     * @param indices Mesh indices
     * @return true if point is inside mesh
     */
    bool IsPointInMesh(
        const glm::vec3& point,
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices) const;

    /**
     * @brief Generate UV coordinates for new geometry
     * @param position Vertex position
     * @param normal Vertex normal
     * @return UV coordinates
     */
    glm::vec2 GenerateUVCoordinates(
        const glm::vec3& position,
        const glm::vec3& normal) const;

    /**
     * @brief Optimize mesh by removing duplicate vertices and degenerate triangles
     * @param vertices Vertices to optimize (modified in place)
     * @param indices Indices to optimize (modified in place)
     */
    void OptimizeMesh(
        std::vector<float>& vertices,
        std::vector<unsigned int>& indices) const;

private:
    /**
     * @brief Check if a point lies within a triangle
     * @param point Point to test
     * @param triangle_verts Triangle vertices
     * @return true if point is inside triangle
     */
    bool IsPointInTriangle(
        const glm::vec3& point,
        const std::vector<glm::vec3>& triangle_verts) const;

    /**
     * @brief Check if a point lies on a line segment
     * @param point Point to test
     * @param line_start Start of line segment
     * @param line_end End of line segment
     * @return true if point is on line segment
     */
    bool IsPointOnLineSegment(
        const glm::vec3& point,
        const glm::vec3& line_start,
        const glm::vec3& line_end) const;

    /**
     * @brief Check if a triangle is valid (non-degenerate)
     * @param triangle_verts Triangle vertices
     * @return true if triangle is valid
     */
    bool IsValidTriangle(
        const std::vector<glm::vec3>& triangle_verts) const;
};

} // namespace Modeling
} // namespace RebelCAD
