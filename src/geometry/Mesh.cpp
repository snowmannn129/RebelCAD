#include "modeling/Mesh.hpp"
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RebelCAD {
namespace Modeling {

Mesh::Mesh()
    : m_boundsDirty(true)
{
    m_minBounds = glm::vec3(std::numeric_limits<float>::max());
    m_maxBounds = glm::vec3(-std::numeric_limits<float>::max());
    m_center = glm::vec3(0.0f);
}

Mesh::Mesh(const Mesh& other) {
    copyFrom(other);
}

Mesh& Mesh::operator=(const Mesh& other) {
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

Mesh::~Mesh() {
    clear();
}

void Mesh::addVertex(const Vertex& vertex) {
    m_vertices.push_back(vertex);
    m_boundsDirty = true;
}

void Mesh::addFace(const Face& face) {
    validateFaceIndices(face);
    m_faces.push_back(face);
}

void Mesh::clear() {
    m_vertices.clear();
    m_faces.clear();
    m_boundsDirty = true;
}

void Mesh::transform(const glm::mat4& matrix) {
    transformOptimized(matrix);
}

void Mesh::transformOptimized(const glm::mat4& matrix) {
    const float* matPtr = glm::value_ptr(matrix);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
    const float* normalMatPtr = glm::value_ptr(normalMatrix);

    // Process vertices in chunks for better cache utilization
    const size_t CHUNK_SIZE = 64;
    const size_t numVertices = m_vertices.size();

    #pragma omp parallel for
    for (int64_t i = 0; i < static_cast<int64_t>(numVertices); i += CHUNK_SIZE) {
        const size_t end = std::min<size_t>(i + CHUNK_SIZE, static_cast<size_t>(numVertices));
        
        for (size_t j = i; j < end; ++j) {
            auto& vertex = m_vertices[j];
            
            // Manual matrix multiplication for position
            float x = vertex.position.x;
            float y = vertex.position.y;
            float z = vertex.position.z;
            
            float w = matPtr[3] * x + matPtr[7] * y + matPtr[11] * z + matPtr[15];
            vertex.position = glm::vec3(
                (matPtr[0] * x + matPtr[4] * y + matPtr[8] * z + matPtr[12]) / w,
                (matPtr[1] * x + matPtr[5] * y + matPtr[9] * z + matPtr[13]) / w,
                (matPtr[2] * x + matPtr[6] * y + matPtr[10] * z + matPtr[14]) / w
            );

            // Transform normal
            x = vertex.normal.x;
            y = vertex.normal.y;
            z = vertex.normal.z;
            vertex.normal = glm::normalize(glm::vec3(
                normalMatPtr[0] * x + normalMatPtr[3] * y + normalMatPtr[6] * z,
                normalMatPtr[1] * x + normalMatPtr[4] * y + normalMatPtr[7] * z,
                normalMatPtr[2] * x + normalMatPtr[5] * y + normalMatPtr[8] * z
            ));
        }
    }

    // Transform face normals
    const size_t numFaces = m_faces.size();
    #pragma omp parallel for
    for (int64_t i = 0; i < static_cast<int64_t>(numFaces); ++i) {
        auto& face = m_faces[i];
        float x = face.normal.x;
        float y = face.normal.y;
        float z = face.normal.z;
        face.normal = glm::normalize(glm::vec3(
            normalMatPtr[0] * x + normalMatPtr[3] * y + normalMatPtr[6] * z,
            normalMatPtr[1] * x + normalMatPtr[4] * y + normalMatPtr[7] * z,
            normalMatPtr[2] * x + normalMatPtr[5] * y + normalMatPtr[8] * z
        ));
    }

    m_boundsDirty = true;
}

void Mesh::merge(const Mesh& other) {
    const size_t baseVertexCount = m_vertices.size();
    const size_t newVertexCount = other.m_vertices.size();
    const size_t newFaceCount = other.m_faces.size();
    
    // Pre-allocate all required memory at once
    m_vertices.reserve(baseVertexCount + newVertexCount);
    m_faces.reserve(m_faces.size() + newFaceCount);

    // Batch copy vertices using memcpy for better performance
    const size_t vertexDataSize = newVertexCount * sizeof(Vertex);
    m_vertices.resize(baseVertexCount + newVertexCount);
    std::memcpy(m_vertices.data() + baseVertexCount, other.m_vertices.data(), vertexDataSize);

    // Pre-allocate face vector to avoid reallocation
    const size_t oldFaceCount = m_faces.size();
    m_faces.resize(oldFaceCount + newFaceCount);

    // Use parallel processing for large face sets
    const size_t PARALLEL_THRESHOLD = 1000;
    if (newFaceCount >= PARALLEL_THRESHOLD) {
        #pragma omp parallel for
        for (int64_t i = 0; i < static_cast<int64_t>(newFaceCount); ++i) {
            Face newFace = other.m_faces[i];
            for (int j = 0; j < 3; ++j) {
                newFace.indices[j] += static_cast<uint32_t>(baseVertexCount);
            }
            m_faces[oldFaceCount + i] = newFace;
        }
    } else {
        // Sequential processing for smaller face sets
        for (size_t i = 0; i < newFaceCount; ++i) {
            Face newFace = other.m_faces[i];
            for (int j = 0; j < 3; ++j) {
                newFace.indices[j] += static_cast<uint32_t>(baseVertexCount);
            }
            m_faces[oldFaceCount + i] = newFace;
        }
    }

    m_boundsDirty = true;
}

void Mesh::recalculateNormals() {
    // Reset vertex normals
    for (auto& vertex : m_vertices) {
        vertex.normal = glm::vec3(0.0f);
    }

    // Calculate face normals and contribute to vertex normals
    for (auto& face : m_faces) {
        const glm::vec3& v0 = m_vertices[face.indices[0]].position;
        const glm::vec3& v1 = m_vertices[face.indices[1]].position;
        const glm::vec3& v2 = m_vertices[face.indices[2]].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        face.normal = glm::normalize(glm::cross(edge1, edge2));

        // Add face normal to vertex normals
        for (int i = 0; i < 3; ++i) {
            m_vertices[face.indices[i]].normal += face.normal;
        }
    }

    // Normalize vertex normals
    for (auto& vertex : m_vertices) {
        if (glm::length(vertex.normal) > 1e-6f) {
            vertex.normal = glm::normalize(vertex.normal);
        }
    }
}

bool Mesh::isValid() const {
    return !m_vertices.empty() && !m_faces.empty() && hasValidTopology();
}

bool Mesh::hasValidTopology() const {
    if (m_vertices.empty() || m_faces.empty()) {
        return false;
    }

    // Check face indices
    for (const auto& face : m_faces) {
        for (int i = 0; i < 3; ++i) {
            if (face.indices[i] >= m_vertices.size()) {
                return false;
            }
        }
    }

    return true;
}

void Mesh::reserveVertices(size_t count) {
    m_vertices.reserve(count);
}

void Mesh::reserveFaces(size_t count) {
    m_faces.reserve(count);
}

void Mesh::reserveFromPool(size_t vertexCount, size_t faceCount) {
    auto& pool = MemoryPool::getInstance();
    
    // Allocate vertex and face vectors from memory pool
    m_vertices.clear();
    m_faces.clear();
    
    if (vertexCount > 0) {
        void* vertexMem = pool.allocate(vertexCount * sizeof(Vertex));
        m_vertices = std::vector<Vertex>(static_cast<Vertex*>(vertexMem), 
                                       static_cast<Vertex*>(vertexMem) + vertexCount);
    }
    
    if (faceCount > 0) {
        void* faceMem = pool.allocate(faceCount * sizeof(Face));
        m_faces = std::vector<Face>(static_cast<Face*>(faceMem),
                                  static_cast<Face*>(faceMem) + faceCount);
    }
    
    m_boundsDirty = true;
}

glm::vec3 Mesh::getCenter() const {
    if (m_boundsDirty) {
        updateBounds();
    }
    return m_center;
}

glm::vec3 Mesh::getMinBounds() const {
    if (m_boundsDirty) {
        updateBounds();
    }
    return m_minBounds;
}

glm::vec3 Mesh::getMaxBounds() const {
    if (m_boundsDirty) {
        updateBounds();
    }
    return m_maxBounds;
}

void Mesh::updateBounds() const {
    if (m_vertices.empty()) {
        m_minBounds = m_maxBounds = m_center = glm::vec3(0.0f);
        m_boundsDirty = false;
        return;
    }

    m_minBounds = glm::vec3(std::numeric_limits<float>::max());
    m_maxBounds = glm::vec3(-std::numeric_limits<float>::max());

    for (const auto& vertex : m_vertices) {
        m_minBounds = glm::min(m_minBounds, vertex.position);
        m_maxBounds = glm::max(m_maxBounds, vertex.position);
    }

    m_center = (m_minBounds + m_maxBounds) * 0.5f;
    m_boundsDirty = false;
}

void Mesh::validateFaceIndices(const Face& face) const {
    for (int i = 0; i < 3; ++i) {
        if (face.indices[i] >= m_vertices.size()) {
            throw std::out_of_range("Face index out of range");
        }
    }
}

void Mesh::copyFrom(const Mesh& other) {
    m_vertices = other.m_vertices;
    m_faces = other.m_faces;
    m_boundsDirty = other.m_boundsDirty;
    m_minBounds = other.m_minBounds;
    m_maxBounds = other.m_maxBounds;
    m_center = other.m_center;
}

} // namespace Modeling
} // namespace RebelCAD
