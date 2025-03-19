#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace rebel_cad {
namespace modeling {

/**
 * @brief Represents a 3D mesh with vertices, faces, and attributes
 * 
 * The Mesh class provides a data structure for storing and manipulating
 * 3D geometry. It maintains topology information and supports various
 * mesh operations needed for CAD modeling.
 */
class Mesh {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct Face {
        std::vector<size_t> indices;  // Vertex indices
        glm::vec3 normal;
    };

    struct Edge {
        size_t v1, v2;  // Vertex indices
        std::vector<size_t> adjacentFaces;
    };

    Mesh();
    ~Mesh();

    // Vertex operations
    size_t addVertex(const glm::vec3& position);
    void setVertexNormal(size_t index, const glm::vec3& normal);
    void setVertexUV(size_t index, const glm::vec2& uv);
    const Vertex& getVertex(size_t index) const;
    size_t getVertexCount() const;

    // Face operations
    size_t addFace(const std::vector<size_t>& indices);
    void setFaceNormal(size_t index, const glm::vec3& normal);
    const Face& getFace(size_t index) const;
    size_t getFaceCount() const;

    // Edge operations
    const std::vector<Edge>& getEdges() const;
    void updateEdgeTopology();

    // Mesh operations
    void clear();
    void computeNormals();
    bool validate() const;
    void optimize();

    // Transform operations
    void transform(const glm::mat4& matrix);
    void translate(const glm::vec3& offset);
    void rotate(float angle, const glm::vec3& axis);
    void scale(const glm::vec3& factors);

private:
    std::vector<Vertex> vertices_;
    std::vector<Face> faces_;
    std::vector<Edge> edges_;

    void rebuildEdges();
    bool validateTopology() const;
};

} // namespace modeling
} // namespace rebel_cad
