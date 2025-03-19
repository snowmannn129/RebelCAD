#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/MemoryPool.hpp"

namespace RebelCAD {
namespace Modeling {

class Mesh {
public:
    // Vertex structure
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // Face structure (triangle)
    struct Face {
        uint32_t indices[3];  // Vertex indices
        glm::vec3 normal;     // Face normal
    };

    // Constructors
    Mesh();
    Mesh(const Mesh& other);
    Mesh& operator=(const Mesh& other);
    ~Mesh();

    // Mesh data access
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<Face>& getFaces() const { return m_faces; }
    
    // Mesh modification
    void addVertex(const Vertex& vertex);
    void addFace(const Face& face);
    void clear();

    // Mesh operations
    void transform(const glm::mat4& matrix);
    void transformOptimized(const glm::mat4& matrix); // Optimized transform using GLM
    void merge(const Mesh& other);
    void recalculateNormals();

    // Validation
    bool isValid() const;
    bool hasValidTopology() const;

    // Memory management
    void reserveVertices(size_t count);
    void reserveFaces(size_t count);
    void reserveFromPool(size_t vertexCount, size_t faceCount); // Memory pool allocation
    size_t getVertexCount() const { return m_vertices.size(); }
    size_t getFaceCount() const { return m_faces.size(); }

    // Bounding information
    glm::vec3 getCenter() const;
    glm::vec3 getMinBounds() const;
    glm::vec3 getMaxBounds() const;

private:
    std::vector<Vertex> m_vertices;
    std::vector<Face> m_faces;
    
    // Cached data
    mutable bool m_boundsDirty;
    mutable glm::vec3 m_minBounds;
    mutable glm::vec3 m_maxBounds;
    mutable glm::vec3 m_center;

    // Helper functions
    void updateBounds() const;
    void validateFaceIndices(const Face& face) const;
    void copyFrom(const Mesh& other);
};

} // namespace Modeling
} // namespace RebelCAD
