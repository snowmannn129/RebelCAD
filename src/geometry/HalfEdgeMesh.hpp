#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Error.hpp"
#include "graphics/SceneNode.h"

using SceneNodePtr = RebelCAD::Graphics::SceneNode::Ptr;

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Half-edge mesh data structure for efficient topology operations
 * 
 * This class implements a half-edge data structure optimized for subdivision surface
 * operations. It provides efficient access to adjacency information and supports
 * topology modifications required for Catmull-Clark subdivision.
 */
class HalfEdgeMesh {
public:
    // Forward declarations
    class Vertex;
    class Edge;
    class Face;
    class HalfEdge;

    using VertexPtr = std::shared_ptr<Vertex>;
    using EdgePtr = std::shared_ptr<Edge>;
    using FacePtr = std::shared_ptr<Face>;
    using HalfEdgePtr = std::shared_ptr<HalfEdge>;
    using SceneNode = RebelCAD::Graphics::SceneNode;

    /**
     * @brief Vertex class storing position and connectivity information
     */
    class Vertex : public Graphics::SceneNode {
    public:
        glm::vec3 position;           // 3D position
        glm::vec2 texCoord;          // Texture coordinates (UV)
        HalfEdgePtr outgoingHalfEdge; // Any half-edge originating from this vertex
        bool isSharp = false;         // Flag for sharp features
        
        // Cached data for subdivision
        glm::vec3 newPosition;        // Computed new position during subdivision
        bool needsUpdate = true;      // Flag for update status

        // Constructor with default UV coordinates
        Vertex() : SceneNode("vertex"), position(0.0f), texCoord(0.0f), outgoingHalfEdge(nullptr),
                  isSharp(false), newPosition(0.0f), needsUpdate(true) {
            setSelectable(true);
        }

        // Override updateWorldTransform to update position
        void updateWorldTransform(const glm::mat4& parentTransform) override {
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
            SceneNode::updateWorldTransform(parentTransform * transform);
        }
    };

    /**
     * @brief Edge class representing a complete edge between two vertices
     */
    class Edge : public Graphics::SceneNode {
    public:
        HalfEdgePtr halfEdge;        // One of the half-edges
        bool isSharp = false;        // Flag for sharp features
        float sharpness = 0.0f;      // Sharpness value for semi-sharp features

        Edge() : SceneNode("edge") {
            setSelectable(true);
        }

        // Override updateWorldTransform to update edge position
        void updateWorldTransform(const glm::mat4& parentTransform) override {
            if (halfEdge) {
                auto start = halfEdge->vertex->position;
                auto end = halfEdge->prev->vertex->position;
                auto center = (start + end) * 0.5f;
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), center);
                SceneNode::updateWorldTransform(parentTransform * transform);
            }
        }
    };

    /**
     * @brief Face class representing a polygon face
     */
    class Face : public Graphics::SceneNode {
    public:
        HalfEdgePtr halfEdge;        // Any half-edge of this face
        glm::vec3 normal;            // Face normal
        
        // Cached data for subdivision
        glm::vec3 centroid;          // Computed face point during subdivision
        bool needsUpdate = true;     // Flag for update status

        Face() : SceneNode("face") {
            setSelectable(true);
        }

        // Override updateWorldTransform to update face position
        void updateWorldTransform(const glm::mat4& parentTransform) override {
            if (halfEdge) {
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), centroid);
                SceneNode::updateWorldTransform(parentTransform * transform);
            }
        }
    };

    /**
     * @brief HalfEdge class representing one direction of an edge
     */
    class HalfEdge {
    public:
        VertexPtr vertex;            // Vertex at the end of this half-edge
        HalfEdgePtr next;           // Next half-edge around the face
        HalfEdgePtr prev;           // Previous half-edge around the face
        HalfEdgePtr pair;           // Opposite half-edge
        FacePtr face;               // Face this half-edge borders
        EdgePtr edge;               // Parent edge this half-edge is part of
    };

    // Constructors
    HalfEdgeMesh() = default;
    ~HalfEdgeMesh() = default;

    /**
     * @brief Creates a new vertex at the specified position
     * @param position The 3D position of the vertex
     * @param texCoord Optional texture coordinates (defaults to (0,0))
     * @return Shared pointer to the created vertex
     */
    VertexPtr createVertex(const glm::vec3& position, const glm::vec2& texCoord = glm::vec2(0.0f));

    /**
     * @brief Creates a new face from a list of vertices
     * @param vertices List of vertices defining the face
     * @return Shared pointer to the created face
     * @throws Error if face creation fails (e.g., invalid vertex count)
     */
    FacePtr createFace(const std::vector<VertexPtr>& vertices);

    /**
     * @brief Marks an edge as sharp
     * @param edge The edge to mark
     * @param sharpness Sharpness value (0.0 = smooth, 1.0 = completely sharp)
     */
    void markSharpEdge(const EdgePtr& edge, float sharpness = 1.0f);

    /**
     * @brief Marks a vertex as sharp (corner)
     * @param vertex The vertex to mark
     */
    void markSharpVertex(const VertexPtr& vertex);

    /**
     * @brief Gets all vertices in the mesh
     * @return Vector of vertex pointers
     */
    const std::vector<VertexPtr>& getVertices() const { return vertices_; }

    /**
     * @brief Gets all faces in the mesh
     * @return Vector of face pointers
     */
    const std::vector<FacePtr>& getFaces() const { return faces_; }

    /**
     * @brief Gets all edges in the mesh
     * @return Vector of edge pointers
     */
    const std::vector<EdgePtr>& getEdges() const { return edges_; }

    /**
     * @brief Validates the mesh topology
     * @return true if topology is valid, false otherwise
     */
    bool validateTopology() const;

    /**
     * @brief Computes face normals for all faces
     */
    void computeFaceNormals();

    /**
     * @brief Clears all mesh data
     */
    void clear();

private:
    std::vector<VertexPtr> vertices_;
    std::vector<EdgePtr> edges_;
    std::vector<FacePtr> faces_;
    std::vector<HalfEdgePtr> halfEdges_;

    /**
     * @brief Creates a new edge between two vertices
     * @param v1 First vertex
     * @param v2 Second vertex
     * @return Shared pointer to the created edge
     */
    EdgePtr createEdge(const VertexPtr& v1, const VertexPtr& v2);

    /**
     * @brief Creates a new half-edge
     * @param vertex End vertex of the half-edge
     * @param face Face the half-edge belongs to
     * @return Shared pointer to the created half-edge
     */
    HalfEdgePtr createHalfEdge(const VertexPtr& vertex, const FacePtr& face);

    /**
     * @brief Links two half-edges as pairs
     * @param he1 First half-edge
     * @param he2 Second half-edge
     */
    void linkHalfEdgePair(const HalfEdgePtr& he1, const HalfEdgePtr& he2);

    // Helper to find existing edge between two vertices
    EdgePtr findEdge(const VertexPtr& v1, const VertexPtr& v2);
};

} // namespace Modeling
} // namespace RebelCAD
