#include "modeling/BooleanTool.h"
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/IPreviewRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stdexcept>

namespace RebelCAD {
namespace Modeling {

// Forward declarations of helper structs
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct Triangle {
    unsigned int v1, v2, v3;
    glm::vec3 normal;
};

struct Edge {
    unsigned int v1, v2;
    bool isIntersection;
};

// Implementation class
class BooleanTool::Impl {
public:
    // Input data
    std::vector<Vertex> firstVertices;
    std::vector<Triangle> firstTriangles;
    std::vector<Vertex> secondVertices;
    std::vector<Triangle> secondTriangles;

    // Result data
    std::vector<float> resultVertices;
    std::vector<unsigned int> resultIndices;

    // Operation parameters
    BooleanParams params;

    // Intersection data
    std::vector<Edge> intersectionEdges;
    std::unordered_set<unsigned int> intersectionVertices;

    // Helper methods
    bool ValidateInputMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
        if (vertices.empty() || indices.empty()) {
            return false;
        }
        if (vertices.size() % 3 != 0) {
            return false;
        }
        if (indices.size() % 3 != 0) {
            return false;
        }
        for (unsigned int index : indices) {
            if (index >= vertices.size() / 3) {
                return false;
            }
        }
        return true;
    }

    void ConvertToInternalFormat(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices,
        std::vector<Vertex>& outVertices,
        std::vector<Triangle>& outTriangles) {
        
        // Convert vertices
        outVertices.clear();
        outVertices.reserve(vertices.size() / 3);
        for (size_t i = 0; i < vertices.size(); i += 3) {
            Vertex v;
            v.position = glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
            v.normal = glm::vec3(0.0f); // Will be computed later
            v.texCoord = glm::vec2(0.0f); // Default UV coordinates
            outVertices.push_back(v);
        }

        // Convert triangles and compute normals
        outTriangles.clear();
        outTriangles.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size(); i += 3) {
            Triangle tri;
            tri.v1 = indices[i];
            tri.v2 = indices[i + 1];
            tri.v3 = indices[i + 2];

            const auto& v1 = outVertices[tri.v1].position;
            const auto& v2 = outVertices[tri.v2].position;
            const auto& v3 = outVertices[tri.v3].position;
            tri.normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

            outTriangles.push_back(tri);
        }

        // Compute vertex normals
        std::vector<glm::vec3> vertexNormals(outVertices.size(), glm::vec3(0.0f));
        for (const auto& tri : outTriangles) {
            vertexNormals[tri.v1] += tri.normal;
            vertexNormals[tri.v2] += tri.normal;
            vertexNormals[tri.v3] += tri.normal;
        }
        for (size_t i = 0; i < outVertices.size(); ++i) {
            outVertices[i].normal = glm::normalize(vertexNormals[i]);
        }
    }

    BooleanTool::ErrorCode DetectIntersections() {
        intersectionEdges.clear();
        intersectionVertices.clear();

        // Build spatial acceleration structure (simple grid-based)
        const float gridSize = params.intersection_tolerance * 10.0f;
        std::unordered_map<glm::ivec3, std::vector<size_t>> grid;

        // Helper to get grid cell for a position
        auto getGridCell = [gridSize](const glm::vec3& pos) {
            return glm::ivec3(
                static_cast<int>(pos.x / gridSize),
                static_cast<int>(pos.y / gridSize),
                static_cast<int>(pos.z / gridSize));
        };

        // Insert first body triangles into grid
        for (size_t i = 0; i < firstTriangles.size(); ++i) {
            const auto& tri = firstTriangles[i];
            const auto& v1 = firstVertices[tri.v1].position;
            const auto& v2 = firstVertices[tri.v2].position;
            const auto& v3 = firstVertices[tri.v3].position;

            auto cell1 = getGridCell(v1);
            auto cell2 = getGridCell(v2);
            auto cell3 = getGridCell(v3);

            // Add triangle to all cells it touches
            for (int x = std::min({cell1.x, cell2.x, cell3.x}); x <= std::max({cell1.x, cell2.x, cell3.x}); ++x) {
                for (int y = std::min({cell1.y, cell2.y, cell3.y}); y <= std::max({cell1.y, cell2.y, cell3.y}); ++y) {
                    for (int z = std::min({cell1.z, cell2.z, cell3.z}); z <= std::max({cell1.z, cell2.z, cell3.z}); ++z) {
                        grid[glm::ivec3(x, y, z)].push_back(i);
                    }
                }
            }
        }

        // Check second body triangles against grid
        for (size_t i = 0; i < secondTriangles.size(); ++i) {
            const auto& tri = secondTriangles[i];
            const auto& v1 = secondVertices[tri.v1].position;
            const auto& v2 = secondVertices[tri.v2].position;
            const auto& v3 = secondVertices[tri.v3].position;

            auto cell1 = getGridCell(v1);
            auto cell2 = getGridCell(v2);
            auto cell3 = getGridCell(v3);

            std::unordered_set<size_t> potentialIntersections;

            // Collect potential intersections from all cells
            for (int x = std::min({cell1.x, cell2.x, cell3.x}); x <= std::max({cell1.x, cell2.x, cell3.x}); ++x) {
                for (int y = std::min({cell1.y, cell2.y, cell3.y}); y <= std::max({cell1.y, cell2.y, cell3.y}); ++y) {
                    for (int z = std::min({cell1.z, cell2.z, cell3.z}); z <= std::max({cell1.z, cell2.z, cell3.z}); ++z) {
                        const auto& cellTriangles = grid[glm::ivec3(x, y, z)];
                        potentialIntersections.insert(cellTriangles.begin(), cellTriangles.end());
                    }
                }
            }

            // Check actual intersections
            for (size_t j : potentialIntersections) {
                const auto& tri2 = firstTriangles[j];
                const auto& w1 = firstVertices[tri2.v1].position;
                const auto& w2 = firstVertices[tri2.v2].position;
                const auto& w3 = firstVertices[tri2.v3].position;

                // Perform triangle-triangle intersection test
                if (TriangleIntersectsTriangle(v1, v2, v3, w1, w2, w3)) {
                    // Store intersection information
                    Edge edge;
                    edge.v1 = tri.v1;
                    edge.v2 = tri2.v1;
                    edge.isIntersection = true;
                    intersectionEdges.push_back(edge);
                    intersectionVertices.insert(edge.v1);
                    intersectionVertices.insert(edge.v2);
                }
            }
        }

        return ErrorCode::None;
    }

    bool TriangleIntersectsTriangle(
        const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3,
        const glm::vec3& w1, const glm::vec3& w2, const glm::vec3& w3) {
        
        // Compute triangle normals
        glm::vec3 n1 = glm::normalize(glm::cross(v2 - v1, v3 - v1));
        glm::vec3 n2 = glm::normalize(glm::cross(w2 - w1, w3 - w1));

        // Check if triangles are coplanar
        if (glm::abs(glm::dot(n1, n2)) > 1.0f - params.intersection_tolerance) {
            return false; // Coplanar triangles handled separately
        }

        // Compute intersection line
        glm::vec3 d = glm::normalize(glm::cross(n1, n2));

        // Project triangles onto line
        float p1min = glm::dot(d, v1);
        float p1max = p1min;
        p1min = std::min(p1min, glm::dot(d, v2));
        p1max = std::max(p1max, glm::dot(d, v2));
        p1min = std::min(p1min, glm::dot(d, v3));
        p1max = std::max(p1max, glm::dot(d, v3));

        float p2min = glm::dot(d, w1);
        float p2max = p2min;
        p2min = std::min(p2min, glm::dot(d, w2));
        p2max = std::max(p2max, glm::dot(d, w2));
        p2min = std::min(p2min, glm::dot(d, w3));
        p2max = std::max(p2max, glm::dot(d, w3));

        // Check for overlap
        if (p1max < p2min - params.intersection_tolerance || 
            p2max < p1min - params.intersection_tolerance) {
            return false;
        }

        return true;
    }

    BooleanTool::ErrorCode PerformUnion() {
        // Start with first body
        resultVertices.clear();
        resultIndices.clear();

        // Add vertices from first body
        for (const auto& vertex : firstVertices) {
            resultVertices.push_back(vertex.position.x);
            resultVertices.push_back(vertex.position.y);
            resultVertices.push_back(vertex.position.z);
        }

        // Add non-intersecting triangles from first body
        for (const auto& tri : firstTriangles) {
            if (!intersectionVertices.count(tri.v1) &&
                !intersectionVertices.count(tri.v2) &&
                !intersectionVertices.count(tri.v3)) {
                resultIndices.push_back(tri.v1);
                resultIndices.push_back(tri.v2);
                resultIndices.push_back(tri.v3);
            }
        }

        // Add vertices from second body
        size_t offset = firstVertices.size();
        for (const auto& vertex : secondVertices) {
            resultVertices.push_back(vertex.position.x);
            resultVertices.push_back(vertex.position.y);
            resultVertices.push_back(vertex.position.z);
        }

        // Add non-intersecting triangles from second body
        for (const auto& tri : secondTriangles) {
            if (!intersectionVertices.count(tri.v1) &&
                !intersectionVertices.count(tri.v2) &&
                !intersectionVertices.count(tri.v3)) {
                resultIndices.push_back(tri.v1 + offset);
                resultIndices.push_back(tri.v2 + offset);
                resultIndices.push_back(tri.v3 + offset);
            }
        }

        // Add intersection edges
        for (const auto& edge : intersectionEdges) {
            resultIndices.push_back(edge.v1);
            resultIndices.push_back(edge.v2);
            resultIndices.push_back(edge.v1 + offset);
        }

        return ErrorCode::None;
    }

    BooleanTool::ErrorCode PerformSubtract() {
        // Similar to union but invert second body normals
        // and only keep first body triangles outside second body
        resultVertices.clear();
        resultIndices.clear();

        // Add vertices from first body
        for (const auto& vertex : firstVertices) {
            resultVertices.push_back(vertex.position.x);
            resultVertices.push_back(vertex.position.y);
            resultVertices.push_back(vertex.position.z);
        }

        // Add triangles from first body that are outside second body
        for (const auto& tri : firstTriangles) {
            const auto& v1 = firstVertices[tri.v1].position;
            bool isOutside = true;
            for (const auto& vertex : secondVertices) {
                if (glm::distance2(v1, vertex.position) < params.intersection_tolerance * params.intersection_tolerance) {
                    isOutside = false;
                    break;
                }
            }
            if (isOutside) {
                resultIndices.push_back(tri.v1);
                resultIndices.push_back(tri.v2);
                resultIndices.push_back(tri.v3);
            }
        }

        return ErrorCode::None;
    }

    BooleanTool::ErrorCode PerformIntersect() {
        // Keep only triangles inside both bodies
        resultVertices.clear();
        resultIndices.clear();

        // Add vertices from intersection regions
        for (unsigned int idx : intersectionVertices) {
            const auto& vertex = firstVertices[idx];
            resultVertices.push_back(vertex.position.x);
            resultVertices.push_back(vertex.position.y);
            resultVertices.push_back(vertex.position.z);
        }

        // Add triangles that form the intersection volume
        for (const auto& edge : intersectionEdges) {
            resultIndices.push_back(edge.v1);
            resultIndices.push_back(edge.v2);
            // Find or create a third vertex to form triangle
            auto it = intersectionVertices.begin();
            while (*it == edge.v1 || *it == edge.v2) ++it;
            resultIndices.push_back(*it);
        }

        return ErrorCode::None;
    }
};

// Public interface implementation
BooleanTool::BooleanTool() : impl(std::make_unique<Impl>()) {}
BooleanTool::~BooleanTool() = default;

BooleanTool::ErrorCode BooleanTool::SetFirstBody(
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices) {
    
    if (!impl->ValidateInputMesh(vertices, indices)) {
        return ErrorCode::InvalidMesh;
    }

    impl->ConvertToInternalFormat(vertices, indices, impl->firstVertices, impl->firstTriangles);
    return ErrorCode::None;
}

BooleanTool::ErrorCode BooleanTool::SetSecondBody(
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices) {
    
    if (!impl->ValidateInputMesh(vertices, indices)) {
        return ErrorCode::InvalidMesh;
    }

    impl->ConvertToInternalFormat(vertices, indices, impl->secondVertices, impl->secondTriangles);
    return ErrorCode::None;
}

BooleanTool::ErrorCode BooleanTool::Configure(const BooleanParams& params) {
    impl->params = params;
    return ErrorCode::None;
}

BooleanTool::ErrorCode BooleanTool::Execute() {
    // Validate input
    if (impl->firstVertices.empty() || impl->secondVertices.empty()) {
        return ErrorCode::InvalidMesh;
    }

    // Detect intersections
    auto err = impl->DetectIntersections();
    if (err != ErrorCode::None) {
        return err;
    }

    // Perform operation
    switch (impl->params.operation_type) {
        case BooleanOperationType::Union:
            return impl->PerformUnion();
        case BooleanOperationType::Subtract:
            return impl->PerformSubtract();
        case BooleanOperationType::Intersect:
            return impl->PerformIntersect();
        default:
            return ErrorCode::BooleanOperationFailed;
    }
}

BooleanTool::ErrorCode BooleanTool::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) {
    if (!graphics) {
        return ErrorCode::GraphicsError;
    }

    // Execute operation
    auto err = Execute();
    if (err != ErrorCode::None) {
        return err;
    }

    // Begin preview rendering
    graphics->beginPreview();

    // Send result to graphics system for preview
    graphics->renderTriangleMeshWithAttributes(
        impl->resultVertices.data(),
        nullptr, // No normals for preview
        nullptr, // No UVs for preview
        impl->resultIndices.data(),
        impl->resultVertices.size() / 3,
        impl->resultIndices.size());

    // End preview rendering
    graphics->endPreview();
    return ErrorCode::None;
}

void BooleanTool::Cancel() {
    impl->resultVertices.clear();
    impl->resultIndices.clear();
    impl->intersectionEdges.clear();
    impl->intersectionVertices.clear();
}

const std::vector<float>& BooleanTool::GetResultVertices() const {
    return impl->resultVertices;
}

const std::vector<unsigned int>& BooleanTool::GetResultIndices() const {
    return impl->resultIndices;
}

bool BooleanTool::ValidateMeshes() const {
    return !impl->firstVertices.empty() && !impl->secondVertices.empty();
}

BooleanTool::ErrorCode BooleanTool::GenerateIntersectionGeometry() {
    return impl->DetectIntersections();
}

BooleanTool::ErrorCode BooleanTool::PerformOperation() {
    return Execute();
}

BooleanTool::ErrorCode BooleanTool::OptimizeResult() {
    // Perform mesh optimization if requested
    if (!impl->params.optimize_result) {
        return ErrorCode::None;
    }

    // Remove duplicate vertices
    std::vector<float> optimizedVertices;
    std::vector<unsigned int> optimizedIndices;
    std::unordered_map<size_t, size_t> vertexMap;

    for (size_t i = 0; i < impl->resultVertices.size(); i += 3) {
        glm::vec3 vertex(
            impl->resultVertices[i],
            impl->resultVertices[i + 1],
            impl->resultVertices[i + 2]);

        // Look for duplicate within tolerance
        bool found = false;
        for (size_t j = 0; j < optimizedVertices.size(); j += 3) {
            glm::vec3 existing(
                optimizedVertices[j],
                optimizedVertices[j + 1],
                optimizedVertices[j + 2]);

            if (glm::distance2(vertex, existing) < impl->params.intersection_tolerance * impl->params.intersection_tolerance) {
                vertexMap[i / 3] = j / 3;
                found = true;
                break;
            }
        }

        if (!found) {
            vertexMap[i / 3] = optimizedVertices.size() / 3;
            optimizedVertices.push_back(vertex.x);
            optimizedVertices.push_back(vertex.y);
            optimizedVertices.push_back(vertex.z);
        }
    }

    // Remap indices
    optimizedIndices.reserve(impl->resultIndices.size());
    for (unsigned int idx : impl->resultIndices) {
        optimizedIndices.push_back(vertexMap[idx]);
    }

    // Update result
    impl->resultVertices = std::move(optimizedVertices);
    impl->resultIndices = std::move(optimizedIndices);

    return ErrorCode::None;
}

} // namespace Modeling
} // namespace RebelCAD
