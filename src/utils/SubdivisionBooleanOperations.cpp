#include "modeling/SubdivisionBooleanOperations.hpp"
#include "modeling/SubdivisionSurface.h"
#include "modeling/Geometry.h"
#include "modeling/SolidBody.hpp"
#include "modeling/BooleanTool.h"
#include "core/Error.h"
#include "graphics/GraphicsSystemFwd.h"
#include "modeling/HalfEdgeMesh.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <cmath>

// Hash function for std::pair
namespace std {
    template<>
    struct hash<std::pair<size_t, size_t>> {
        size_t operator()(const std::pair<size_t, size_t>& p) const {
            return hash<size_t>()(p.first) ^ (hash<size_t>()(p.second) << 1);
        }
    };
}

namespace RebelCAD {
namespace Modeling {

void SubdivisionBooleanOperations::ConvertSolidBodyToRawData(
    std::shared_ptr<SolidBody> body,
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices) {
    
    const auto& bodyVertices = body->GetVertices();
    const auto& triangles = body->GetTriangles();

    // Convert vertices to raw float array
    vertices.clear();
    vertices.reserve(bodyVertices.size() * 3);
    for (const auto& vertex : bodyVertices) {
        vertices.push_back(static_cast<float>(vertex.position.x));
        vertices.push_back(static_cast<float>(vertex.position.y));
        vertices.push_back(static_cast<float>(vertex.position.z));
    }

    // Convert triangles to index array
    indices.clear();
    indices.reserve(triangles.size() * 3);
    for (const auto& triangle : triangles) {
        indices.push_back(static_cast<unsigned int>(triangle.v1));
        indices.push_back(static_cast<unsigned int>(triangle.v2));
        indices.push_back(static_cast<unsigned int>(triangle.v3));
    }
}

void SubdivisionBooleanOperations::ConvertRawDataToSolidBody(
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices,
    std::vector<SolidBody::Vertex>& outVertices,
    std::vector<SolidBody::Triangle>& outTriangles) {
    
    // Convert raw vertices to SolidBody vertices
    outVertices.clear();
    outVertices.reserve(vertices.size() / 3);
    for (size_t i = 0; i < vertices.size(); i += 3) {
        SolidBody::Vertex vertex;
        vertex.position = glm::dvec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        vertex.normal = glm::dvec3(0.0); // Will be computed later
        vertex.texCoord = glm::dvec2(0.0); // Default UV coordinates
        outVertices.push_back(vertex);
    }

    // Convert raw indices to triangles
    outTriangles.clear();
    outTriangles.reserve(indices.size() / 3);
    for (size_t i = 0; i < indices.size(); i += 3) {
        outTriangles.emplace_back(
            indices[i],
            indices[i + 1],
            indices[i + 2]);
    }

    // Compute vertex normals
    std::vector<glm::dvec3> vertexNormals(outVertices.size(), glm::dvec3(0.0));
    for (const auto& triangle : outTriangles) {
        const auto& v1 = outVertices[triangle.v1].position;
        const auto& v2 = outVertices[triangle.v2].position;
        const auto& v3 = outVertices[triangle.v3].position;
        
        auto normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        vertexNormals[triangle.v1] += normal;
        vertexNormals[triangle.v2] += normal;
        vertexNormals[triangle.v3] += normal;
    }

    // Normalize and assign vertex normals
    for (size_t i = 0; i < outVertices.size(); ++i) {
        outVertices[i].normal = glm::normalize(vertexNormals[i]);
    }
}

std::shared_ptr<SubdivisionSurface> SubdivisionBooleanOperations::PerformOperation(
    std::shared_ptr<SubdivisionSurface> first,
    std::shared_ptr<SubdivisionSurface> second,
    BooleanOperationType operation,
    const BooleanParams& params) {
    
    // Convert subdivision surfaces to solid bodies
    auto firstBody = ConvertToSolidBody(first);
    auto secondBody = ConvertToSolidBody(second);

    // Create and configure boolean tool
    BooleanTool boolTool;
    BooleanParams localParams = params;
    localParams.operation_type = operation;
    boolTool.Configure(localParams);

    // Convert vertices and triangles to BooleanTool format
    std::vector<float> firstVertices;
    std::vector<unsigned int> firstIndices;
    ConvertSolidBodyToRawData(firstBody, firstVertices, firstIndices);

    std::vector<float> secondVertices;
    std::vector<unsigned int> secondIndices;
    ConvertSolidBodyToRawData(secondBody, secondVertices, secondIndices);

    // Set input bodies
    boolTool.SetFirstBody(firstVertices, firstIndices);
    boolTool.SetSecondBody(secondVertices, secondIndices);

    // Execute boolean operation
    if (boolTool.Execute() != Core::ErrorCode::None) {
        throw std::runtime_error("Boolean operation failed");
    }

    // Convert result back to SolidBody format
    std::vector<SolidBody::Vertex> resultVertices;
    std::vector<SolidBody::Triangle> resultTriangles;
    ConvertRawDataToSolidBody(
        boolTool.GetResultVertices(),
        boolTool.GetResultIndices(),
        resultVertices,
        resultTriangles);

    // Create result body
    auto resultBody = SolidBody::Create(resultVertices, resultTriangles);

    // Convert result back to subdivision surface
    auto resultSurface = ConvertToSubdivisionSurface(resultBody, params.maintain_features);

    // Optimize topology if requested
    if (params.optimize_result) {
        resultSurface = OptimizeTopology(resultSurface);
    }

    return resultSurface;
}

Core::ErrorCode SubdivisionBooleanOperations::PreviewOperation(
    std::shared_ptr<SubdivisionSurface> first,
    std::shared_ptr<SubdivisionSurface> second,
    BooleanOperationType operation,
    std::shared_ptr<Graphics::GraphicsSystem> graphics,
    const BooleanParams& params) {
    
    try {
        // Convert surfaces to solid bodies
        auto firstBody = ConvertToSolidBody(first);
        auto secondBody = ConvertToSolidBody(second);

        // Create and configure boolean tool
        BooleanTool boolTool;
        BooleanParams localParams = params;
        localParams.operation_type = operation;
        boolTool.Configure(localParams);

        // Convert vertices and triangles to BooleanTool format
        std::vector<float> firstVertices;
        std::vector<unsigned int> firstIndices;
        ConvertSolidBodyToRawData(firstBody, firstVertices, firstIndices);

        std::vector<float> secondVertices;
        std::vector<unsigned int> secondIndices;
        ConvertSolidBodyToRawData(secondBody, secondVertices, secondIndices);

        // Set input bodies
        boolTool.SetFirstBody(firstVertices, firstIndices);
        boolTool.SetSecondBody(secondVertices, secondIndices);

        // Generate preview
        return boolTool.Preview(graphics);
    }
    catch (const std::exception&) {
        return Core::ErrorCode::None;
    }
}

std::shared_ptr<SolidBody> SubdivisionBooleanOperations::ConvertToSolidBody(
    std::shared_ptr<SubdivisionSurface> surface,
    size_t subdivisionLevel) {
    
    // First subdivide the surface to desired level
    auto subdivided = surface;
    for (size_t i = 0; i < subdivisionLevel; ++i) {
        subdivided = subdivided->Subdivide();
    }

    // Convert to solid body
    return subdivided->ToSolidBody(subdivisionLevel);
}

std::shared_ptr<SubdivisionSurface> SubdivisionBooleanOperations::ConvertToSubdivisionSurface(
    std::shared_ptr<SolidBody> body,
    bool detectFeatures) {
    
    // Get mesh data
    const auto& vertices = body->GetVertices();
    const auto& triangles = body->GetTriangles();

    // Create half-edge mesh
    auto mesh = std::make_shared<HalfEdgeMesh>();

    // Create vertices
    std::vector<HalfEdgeMesh::VertexPtr> meshVertices;
    meshVertices.reserve(vertices.size());
    for (const auto& vertex : vertices) {
        auto v = mesh->createVertex(
            glm::vec3(vertex.position),
            glm::vec2(vertex.texCoord));
        meshVertices.push_back(v);
    }

    // Create faces and edges
    for (const auto& triangle : triangles) {
        std::vector<HalfEdgeMesh::VertexPtr> faceVertices = {
            meshVertices[triangle.v1],
            meshVertices[triangle.v2],
            meshVertices[triangle.v3]
        };
        mesh->createFace(faceVertices);
    }

    // Detect sharp features if requested
    if (detectFeatures) {
        // Find edges with sharp angles between adjacent faces
        for (const auto& edge : mesh->getEdges()) {
            if (edge->halfEdge && edge->halfEdge->pair) {
                auto face1 = edge->halfEdge->face;
                auto face2 = edge->halfEdge->pair->face;
                if (face1 && face2) {
                    float angle = glm::degrees(
                        glm::acos(glm::dot(face1->normal, face2->normal)));
                    if (angle > 30.0f) { // Threshold for sharp edges
                        edge->isSharp = true;
                        edge->sharpness = 1.0f;
                    }
                }
            }
        }
    }

    // Create subdivision surface from half-edge mesh
    return std::make_shared<SubdivisionSurface>(mesh);
}

std::shared_ptr<SubdivisionSurface> SubdivisionBooleanOperations::OptimizeTopology(
    std::shared_ptr<SubdivisionSurface> surface) {
    
    // Get current topology
    const auto& vertices = surface->GetVertices();
    const auto& edges = surface->GetEdges();
    const auto& faces = surface->GetFaces();
    const auto& sharpEdges = surface->GetSharpEdges();
    
    // Create optimized vertex positions
    std::vector<Point3D> optimizedVertices = vertices;
    
    // Collapse short edges
    const float minEdgeLength = 1e-4f;
    bool changed;
    do {
        changed = false;
        for (size_t i = 0; i < edges.size(); ++i) {
            const auto& edge = edges[i];
            const auto& v1 = optimizedVertices[edge.vertex1];
            const auto& v2 = optimizedVertices[edge.vertex2];
            
            float length = glm::distance(
                glm::vec3(v1.x, v1.y, v1.z),
                glm::vec3(v2.x, v2.y, v2.z));
                
            if (length < minEdgeLength) {
                // Collapse edge to midpoint
                Point3D midpoint{
                    (v1.x + v2.x) * 0.5f,
                    (v1.y + v2.y) * 0.5f,
                    (v1.z + v2.z) * 0.5f
                };
                optimizedVertices[edge.vertex1] = midpoint;
                optimizedVertices[edge.vertex2] = midpoint;
                changed = true;
                break;
            }
        }
    } while (changed);

    // Create new subdivision surface with optimized vertices
    return SubdivisionSurface::Create(optimizedVertices, edges, faces, sharpEdges);
}

std::vector<size_t> SubdivisionBooleanOperations::DetectSharpFeatures(
    const std::vector<SolidBody::Vertex>& vertices,
    const std::vector<SolidBody::Triangle>& triangles,
    float angleTolerance) {
    
    std::vector<size_t> sharpEdges;

    // Build edge map to find adjacent triangles
    std::unordered_map<std::pair<size_t, size_t>, std::vector<size_t>> edgeToTriangles;
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto& tri = triangles[i];
        auto addEdge = [&](size_t v1, size_t v2) {
            if (v1 > v2) std::swap(v1, v2);
            edgeToTriangles[{v1, v2}].push_back(i);
        };
        addEdge(tri.v1, tri.v2);
        addEdge(tri.v2, tri.v3);
        addEdge(tri.v3, tri.v1);
    }

    // Check each edge for sharp angle between adjacent triangles
    for (const auto& [edge, tris] : edgeToTriangles) {
        if (tris.size() != 2) continue; // Skip boundary edges

        const auto& tri1 = triangles[tris[0]];
        const auto& tri2 = triangles[tris[1]];

        // Compute triangle normals
        auto getNormal = [&](const SolidBody::Triangle& tri) {
            const auto& v1 = vertices[tri.v1].position;
            const auto& v2 = vertices[tri.v2].position;
            const auto& v3 = vertices[tri.v3].position;
            return glm::normalize(glm::cross(
                glm::dvec3(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z),
                glm::dvec3(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z)));
        };

        auto normal1 = getNormal(tri1);
        auto normal2 = getNormal(tri2);

        // Check angle between normals
        float angle = glm::degrees(glm::acos(glm::dot(normal1, normal2)));
        if (angle > angleTolerance) {
            sharpEdges.push_back(edge.first);
            sharpEdges.push_back(edge.second);
        }
    }

    return sharpEdges;
}

} // namespace Modeling
} // namespace RebelCAD
