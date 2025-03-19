#include "modeling/HalfEdgeMesh.hpp"
#include "core/Error.hpp"
#include <unordered_set>
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

HalfEdgeMesh::VertexPtr HalfEdgeMesh::createVertex(const glm::vec3& position, const glm::vec2& texCoord) {
    auto vertex = std::make_shared<Vertex>();
    vertex->position = position;
    vertex->texCoord = texCoord;
    vertex->outgoingHalfEdge = nullptr;
    vertex->setName("vertex_" + std::to_string(vertices_.size()));
    vertices_.push_back(vertex);
    return vertex;
}

HalfEdgeMesh::EdgePtr HalfEdgeMesh::createEdge(const VertexPtr& v1, const VertexPtr& v2) {
    // Check if edge already exists
    if (auto existingEdge = findEdge(v1, v2)) {
        return existingEdge;
    }

    auto edge = std::make_shared<Edge>();
    edge->setName("edge_" + std::to_string(edges_.size()));
    edges_.push_back(edge);
    return edge;
}

HalfEdgeMesh::HalfEdgePtr HalfEdgeMesh::createHalfEdge(const VertexPtr& vertex, const FacePtr& face) {
    auto halfEdge = std::make_shared<HalfEdge>();
    halfEdge->vertex = vertex;
    halfEdge->face = face;
    halfEdges_.push_back(halfEdge);
    return halfEdge;
}

void HalfEdgeMesh::linkHalfEdgePair(const HalfEdgeMesh::HalfEdgePtr& he1, const HalfEdgeMesh::HalfEdgePtr& he2) {
    he1->pair = he2;
    he2->pair = he1;
}

HalfEdgeMesh::EdgePtr HalfEdgeMesh::findEdge(const VertexPtr& v1, const VertexPtr& v2) {
    // Start from v1's outgoing half-edge and traverse
    if (!v1->outgoingHalfEdge) {
        return nullptr;
    }

    auto current = v1->outgoingHalfEdge;
    auto start = current;
    do {
        if (current->vertex == v2) {
            return current->edge;
        }
        if (!current->pair) {
            break;
        }
        current = current->pair->next;
    } while (current != start && current != nullptr);

    return nullptr;
}

HalfEdgeMesh::FacePtr HalfEdgeMesh::createFace(const std::vector<VertexPtr>& vertices) {
    if (vertices.size() < 3) {
        throw RebelCAD::Error::InvalidOperation("Face requires at least 3 vertices");
    }

    auto face = std::make_shared<Face>();
    face->setName("face_" + std::to_string(faces_.size()));
    faces_.push_back(face);

    std::vector<HalfEdgePtr> faceHalfEdges;
    const size_t vertexCount = vertices.size();

    // Create half-edges for the face
    for (size_t i = 0; i < vertexCount; ++i) {
        auto halfEdge = createHalfEdge(vertices[(i + 1) % vertexCount], face);
        faceHalfEdges.push_back(halfEdge);
    }

    // Link half-edges in the face
    for (size_t i = 0; i < vertexCount; ++i) {
        faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % vertexCount];
        faceHalfEdges[i]->prev = faceHalfEdges[(i - 1 + vertexCount) % vertexCount];
    }

    // Create or find edges and link half-edge pairs
    for (size_t i = 0; i < vertexCount; ++i) {
        auto v1 = vertices[i];
        auto v2 = vertices[(i + 1) % vertexCount];
        
        auto edge = createEdge(v1, v2);
        faceHalfEdges[i]->edge = edge;

        // If this is the first half-edge for this edge
        if (!edge->halfEdge) {
            edge->halfEdge = faceHalfEdges[i];
        } else {
            // Link with existing half-edge
            linkHalfEdgePair(faceHalfEdges[i], edge->halfEdge);
        }

        // Update vertex outgoing half-edge if needed
        if (!v1->outgoingHalfEdge) {
            v1->outgoingHalfEdge = faceHalfEdges[i];
        }
    }

    face->halfEdge = faceHalfEdges[0];
    computeFaceNormals();
    return face;
}

void HalfEdgeMesh::markSharpEdge(const HalfEdgeMesh::EdgePtr& edge, float sharpness) {
    if (!edge) {
        throw RebelCAD::Error::InvalidArgument("Cannot mark null edge as sharp");
    }
    edge->isSharp = sharpness > 0.0f;
    edge->sharpness = std::clamp(sharpness, 0.0f, 1.0f);
}

void HalfEdgeMesh::markSharpVertex(const HalfEdgeMesh::VertexPtr& vertex) {
    if (!vertex) {
        throw RebelCAD::Error::InvalidArgument("Cannot mark null vertex as sharp");
    }
    vertex->isSharp = true;
}

bool HalfEdgeMesh::validateTopology() const {
    for (const auto& face : faces_) {
        if (!face->halfEdge) {
            return false;
        }

        // Verify face half-edge loop
        auto he = face->halfEdge;
        auto start = he;
        std::unordered_set<HalfEdgePtr> visited;
        do {
            if (!he->vertex || !he->next || !he->prev || !he->face) {
                return false;
            }
            if (visited.find(he) != visited.end()) {
                return false; // Loop detected before completing face
            }
            visited.insert(he);
            he = he->next;
        } while (he != start);
    }

    // Verify half-edge pairs
    for (const auto& halfEdge : halfEdges_) {
        if (halfEdge->pair) {
            if (halfEdge->pair->pair != halfEdge) {
                return false;
            }
        }
    }

    return true;
}

void HalfEdgeMesh::computeFaceNormals() {
    for (const auto& face : faces_) {
        if (!face->halfEdge) continue;

        // Get three vertices to compute normal
        auto he = face->halfEdge;
        auto v0 = he->prev->vertex->position;
        auto v1 = he->vertex->position;
        auto v2 = he->next->vertex->position;

        // Compute normal using cross product
        auto edge1 = v1 - v0;
        auto edge2 = v2 - v0;
        face->normal = glm::normalize(glm::cross(edge1, edge2));

        // Compute centroid
        glm::vec3 centroid(0.0f);
        int vertexCount = 0;
        auto current = he;
        do {
            centroid += current->vertex->position;
            vertexCount++;
            current = current->next;
        } while (current != he);

        face->centroid = centroid / static_cast<float>(vertexCount);

        // Update face transform
        face->updateWorldTransform(glm::mat4(1.0f));
    }

    // Update vertex and edge transforms
    for (const auto& vertex : vertices_) {
        vertex->updateWorldTransform(glm::mat4(1.0f));
    }
    for (const auto& edge : edges_) {
        edge->updateWorldTransform(glm::mat4(1.0f));
    }
}

void HalfEdgeMesh::clear() {
    vertices_.clear();
    edges_.clear();
    faces_.clear();
    halfEdges_.clear();
}

} // namespace Modeling
} // namespace RebelCAD
