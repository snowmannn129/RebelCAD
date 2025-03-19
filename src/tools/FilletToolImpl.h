#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>
#include "modeling/FilletTool.h"

namespace RebelCAD {
namespace Modeling {

class FilletTool::Impl {
public:
    // Input data
    std::vector<float> mesh_vertices;
    std::vector<unsigned int> mesh_indices;
    FilletParams params;
    bool has_mesh = false;
    bool is_configured = false;

    // Edge and face data
    struct Edge {
        std::vector<unsigned int> vertex_indices;
        std::vector<unsigned int> adjacent_faces;
        float start_radius;
        float end_radius;
    };

    struct Face {
        std::vector<unsigned int> vertex_indices;
        std::vector<unsigned int> adjacent_edges;
        glm::vec3 normal;
    };

    // Geometry data
    struct FilletGeometry {
        std::vector<float> vertices;
        std::vector<float> uvs;
        std::vector<unsigned int> indices;
    };

    std::vector<Edge> edges_to_fillet;
    std::vector<Face> faces_to_fillet;

    // Result data
    std::vector<float> result_vertices;
    std::vector<float> result_normals;
    std::vector<float> result_uvs;
    std::vector<unsigned int> result_indices;

    // Preview cache
    struct PreviewCache {
        bool valid = false;
        std::vector<float> cached_vertices;
        std::vector<unsigned int> cached_indices;
        std::vector<Edge> cached_edges;
        std::vector<Face> cached_faces;
        FilletParams cached_params;
        std::vector<float> mesh_vertices;
        std::vector<float> mesh_normals;
        std::vector<float> mesh_uvs;
        std::vector<unsigned int> mesh_indices;
    } preview_cache;

    // Topology data
    struct TopologyData {
        std::unordered_map<unsigned int, std::vector<unsigned int>> vertex_to_faces;
        std::unordered_map<unsigned int, std::vector<unsigned int>> vertex_to_edges;
        std::unordered_map<unsigned int, std::vector<unsigned int>> face_to_edges;
        std::unordered_map<unsigned int, std::vector<unsigned int>> edge_to_faces;
    } topology;

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
    }

    /**
     * @brief Build topology data for the mesh
     */
    void BuildTopology();

    /**
     * @brief Generate fillet geometry for an edge
     * @param edge Edge to fillet
     * @return FilletGeometry containing vertices, UVs, and indices
     */
    FilletGeometry GenerateEdgeFillet(const Edge& edge) const;

    /**
     * @brief Generate fillet geometry for a face
     * @param face Face to fillet
     * @return FilletGeometry containing vertices, UVs, and indices
     */
    FilletGeometry GenerateFaceFillet(const Face& face) const;

    /**
     * @brief Calculate radius at a point along an edge
     * @param edge Edge to calculate radius for
     * @param t Parameter along edge (0-1)
     * @return Radius at point
     */
    float CalculateRadius(const Edge& edge, float t) const;

    /**
     * @brief Generate UV coordinates for fillet surface
     * @param position Vertex position
     * @param normal Vertex normal
     * @param edge_direction Direction along edge
     * @return UV coordinates
     */
    glm::vec2 GenerateUVCoordinates(
        const glm::vec3& position,
        const glm::vec3& normal,
        const glm::vec3& edge_direction) const;

    /**
     * @brief Optimize mesh by removing duplicate vertices and degenerate triangles
     * @param vertices Vertices to optimize (modified in place)
     * @param indices Indices to optimize (modified in place)
     */
    void OptimizeMesh(
        std::vector<float>& vertices,
        std::vector<unsigned int>& indices) const;

    /**
     * @brief Find shared vertices between faces
     * @param face1_indices First face vertex indices
     * @param face2_indices Second face vertex indices
     * @return Vector of shared vertex indices
     */
    std::vector<unsigned int> FindSharedVertices(
        const std::vector<unsigned int>& face1_indices,
        const std::vector<unsigned int>& face2_indices) const;

    /**
     * @brief Calculate face normal from vertices
     * @param face_vertices Face vertex positions
     * @return Normalized face normal
     */
    glm::vec3 CalculateFaceNormal(
        const std::vector<glm::vec3>& face_vertices) const;

    /**
     * @brief Check if an edge is valid for filleting
     * @param edge Edge to check
     * @return true if edge is valid
     */
    bool IsValidEdge(const Edge& edge) const;

    /**
     * @brief Check if a face is valid for filleting
     * @param face Face to check
     * @return true if face is valid
     */
    bool IsValidFace(const Face& face) const;

private:
};

} // namespace Modeling
} // namespace RebelCAD
