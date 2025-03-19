#pragma once

#include <memory>
#include <vector>
#include <array>
#include "core/Error.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Represents a 3D point in space
 */
struct Point3D {
    double x, y, z;
    
    Point3D() : x(0), y(0), z(0) {}
    Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
};

/**
 * @brief Represents a 3D vector
 */
struct Vector3D {
    double x, y, z;
    
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    
    /**
     * @brief Computes vector magnitude
     */
    double Magnitude() const;
    
    /**
     * @brief Returns normalized vector
     */
    Vector3D Normalize() const;
    
    /**
     * @brief Computes dot product with another vector
     */
    double Dot(const Vector3D& other) const;
    
    /**
     * @brief Computes cross product with another vector
     */
    Vector3D Cross(const Vector3D& other) const;
};

/**
 * @brief Represents a triangulated mesh
 */
class TriangleMesh {
public:
    struct Triangle {
        size_t v1, v2, v3;  // Vertex indices
        Vector3D normal;     // Face normal
    };
    
    std::vector<Point3D> vertices;    // Vertex positions
    std::vector<Vector3D> normals;    // Vertex normals
    std::vector<Triangle> triangles;  // Triangle definitions
    
    /**
     * @brief Creates an empty mesh
     */
    static std::shared_ptr<TriangleMesh> Create();
    
    /**
     * @brief Merges this mesh with another
     * @param other Mesh to merge with
     * @return Combined mesh
     */
    std::shared_ptr<TriangleMesh> Merge(const std::shared_ptr<TriangleMesh>& other) const;
};

/**
 * @brief Represents a topological edge between two vertices
 */
class Edge {
public:
    Edge(size_t v1, size_t v2) : vertex1(v1), vertex2(v2) {}
    size_t vertex1, vertex2;
};

/**
 * @brief Represents a face defined by a set of edges
 */
class Face {
public:
    std::vector<size_t> edge_indices;  // Indices into edge array
    std::vector<bool> edge_directions;  // True if edge direction matches face loop
};

/**
 * @brief Represents the complete topological structure of a solid
 * 
 * Topology maintains the relationships between vertices, edges, and faces
 * that make up a solid body. This structure is essential for performing
 * Boolean operations as it maintains adjacency information.
 */
class Topology {
public:
    std::vector<Point3D> vertices;
    std::vector<Edge> edges;
    std::vector<Face> faces;
    
    /**
     * @brief Validates the topology is manifold and closed
     * @return true if topology is valid
     */
    bool IsValid() const;
    
    /**
     * @brief Merges this topology with another
     * @param other Topology to merge with
     * @return Combined topology
     */
    Topology Merge(const Topology& other) const;
};

/**
 * @brief Represents a solid 3D body with complete boundary representation
 * 
 * SolidBody maintains both the geometric and topological representation
 * of a 3D solid. It provides high-level operations for solid modeling
 * while maintaining a valid boundary representation.
 */
/**
 * @brief Base class for all parametric surfaces
 * 
 * Surface provides the common interface for all surface types in the system.
 * This includes evaluation of points and normals at parametric coordinates,
 * as well as conversion to discrete representations.
 */
class Surface : public std::enable_shared_from_this<Surface> {
public:
    virtual ~Surface() = default;

    /**
     * @brief Evaluates surface point at given parameters
     * @param u First parameter (0-1)
     * @param v Second parameter (0-1)
     * @return 3D point on surface
     */
    virtual Point3D Evaluate(double u, double v) const = 0;

    /**
     * @brief Evaluates surface normal at given parameters
     * @param u First parameter (0-1)
     * @param v Second parameter (0-1)
     * @return Surface normal vector
     */
    virtual Vector3D EvaluateNormal(double u, double v) const = 0;

    /**
     * @brief Converts surface to a triangulated mesh
     * @param resolution Grid resolution for triangulation
     * @return Triangle mesh representation
     */
    virtual std::shared_ptr<TriangleMesh> ToMesh(size_t resolution = 20) const = 0;
};

class SolidBody {
public:
    /**
     * @brief Creates a solid body from topology
     * @param topology The topological representation
     * @return Shared pointer to new solid body
     */
    static std::shared_ptr<SolidBody> FromTopology(const Topology& topology);
    
    /**
     * @brief Gets the body's topology representation
     * @return Reference to topology
     */
    const Topology& GetTopology() const { return m_topology; }
    
    /**
     * @brief Checks if this body intersects with another
     * @param other Body to check intersection with
     * @return true if bodies intersect
     */
    bool IntersectsWith(const std::shared_ptr<SolidBody>& other) const;
    
    /**
     * @brief Creates a compound body from multiple bodies
     * @param bodies Vector of bodies to combine
     * @return New compound solid body
     */
    static std::shared_ptr<SolidBody> CreateCompound(
        const std::vector<std::shared_ptr<SolidBody>>& bodies);

private:
    Topology m_topology;
};

} // namespace Modeling
} // namespace RebelCAD
