#include "modeling/OffsetSurfaceTool.h"
#include "core/Error.h"
#include <stdexcept>
#include <cmath>

namespace RebelCAD {
namespace Modeling {

class OffsetSurface : public Surface {
public:
    OffsetSurface(std::shared_ptr<Surface> baseSurface, double distance)
        : m_baseSurface(baseSurface), m_distance(distance) {}

    Point3D Evaluate(double u, double v) const override {
        // Get base surface point and normal
        Point3D basePoint = m_baseSurface->Evaluate(u, v);
        Vector3D normal = m_baseSurface->EvaluateNormal(u, v).Normalize();
        
        // Offset point along normal direction
        return Point3D(
            basePoint.x + normal.x * m_distance,
            basePoint.y + normal.y * m_distance,
            basePoint.z + normal.z * m_distance
        );
    }

    Vector3D EvaluateNormal(double u, double v) const override {
        // Normal direction remains same as base surface
        // (or inverted if offset is negative)
        Vector3D normal = m_baseSurface->EvaluateNormal(u, v);
        return m_distance >= 0 ? normal : Vector3D(-normal.x, -normal.y, -normal.z);
    }

    std::shared_ptr<TriangleMesh> ToMesh(size_t resolution) const override {
        // Use base surface's mesh and offset each vertex
        auto baseMesh = m_baseSurface->ToMesh(resolution);
        auto offsetMesh = TriangleMesh::Create();
        
        // Copy triangles and normals
        offsetMesh->triangles = baseMesh->triangles;
        offsetMesh->normals = baseMesh->normals;
        
        // Offset each vertex along its normal
        offsetMesh->vertices.reserve(baseMesh->vertices.size());
        for (size_t i = 0; i < baseMesh->vertices.size(); i++) {
            const auto& vertex = baseMesh->vertices[i];
            const auto& normal = baseMesh->normals[i];
            offsetMesh->vertices.push_back(Point3D(
                vertex.x + normal.x * m_distance,
                vertex.y + normal.y * m_distance,
                vertex.z + normal.z * m_distance
            ));
        }
        
        return offsetMesh;
    }

private:
    std::shared_ptr<Surface> m_baseSurface;
    double m_distance;
};

OffsetSurfaceTool::OffsetSurfaceTool()
    : m_offsetDistance(0.0)
    , m_tolerance(0.001)
    , m_preserveSharpFeatures(true)
{
}

OffsetSurfaceTool::~OffsetSurfaceTool() = default;

bool OffsetSurfaceTool::setInputSurface(std::shared_ptr<Surface> surface) {
    if (!surface) {
        return false;
    }
    m_inputSurface = surface;
    return true;
}

void OffsetSurfaceTool::setOffsetDistance(double distance) {
    m_offsetDistance = distance;
}

void OffsetSurfaceTool::setTolerance(double tolerance) {
    if (tolerance <= 0) {
        throw std::invalid_argument("Tolerance must be positive");
    }
    m_tolerance = tolerance;
}

void OffsetSurfaceTool::setPreserveSharpFeatures(bool preserve) {
    m_preserveSharpFeatures = preserve;
}

bool OffsetSurfaceTool::validate() const {
    if (!m_inputSurface) {
        return false;
    }
    
    // Check if offset distance is too large (could cause self-intersections)
    auto mesh = m_inputSurface->ToMesh();
    for (const auto& normal : mesh->normals) {
        if (std::abs(m_offsetDistance) * normal.Magnitude() > m_tolerance) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Surface> OffsetSurfaceTool::execute() {
    if (!validate()) {
        throw std::runtime_error("Invalid offset surface parameters");
    }

    // Create initial offset surface
    auto offsetSurface = createInitialOffset();
    
    // Handle self-intersections if needed
    if (checkSelfIntersections()) {
        handleSelfIntersections(offsetSurface);
    }
    
    // Preserve sharp features if enabled
    if (m_preserveSharpFeatures) {
        preserveSharpFeatures(offsetSurface);
    }
    
    return offsetSurface;
}

bool OffsetSurfaceTool::checkSelfIntersections() const {
    // Convert to mesh for intersection testing
    auto mesh = m_inputSurface->ToMesh();
    
    // Check each vertex against offset direction for potential intersections
    for (size_t i = 0; i < mesh->vertices.size(); i++) {
        const auto& vertex = mesh->vertices[i];
        const auto& normal = mesh->normals[i];
        
        Point3D offsetPoint(
            vertex.x + normal.x * m_offsetDistance,
            vertex.y + normal.y * m_offsetDistance,
            vertex.z + normal.z * m_offsetDistance
        );
        
        // Check if offset point intersects with any triangles
        for (const auto& triangle : mesh->triangles) {
            // Simple intersection test - could be optimized
            const auto& v1 = mesh->vertices[triangle.v1];
            const auto& v2 = mesh->vertices[triangle.v2];
            const auto& v3 = mesh->vertices[triangle.v3];
            
            // Basic triangle-point distance check
            Vector3D edge1(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
            Vector3D edge2(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
            Vector3D normal = edge1.Cross(edge2).Normalize();
            
            Vector3D toPoint(
                offsetPoint.x - v1.x,
                offsetPoint.y - v1.y,
                offsetPoint.z - v1.z
            );
            
            double dist = std::abs(toPoint.Dot(normal));
            if (dist < m_tolerance) {
                return true;
            }
        }
    }
    
    return false;
}

void OffsetSurfaceTool::handleSelfIntersections(std::shared_ptr<Surface>& surface) {
    // For now, just adjust the offset distance until no self-intersections occur
    // This is a simplified approach - a more robust solution would involve:
    // 1. Detecting exact intersection curves
    // 2. Trimming the surface at intersections
    // 3. Creating blend surfaces at sharp features
    
    double adjustedDistance = m_offsetDistance;
    while (checkSelfIntersections()) {
        adjustedDistance *= 0.9; // Reduce offset distance by 10%
        surface = std::make_shared<OffsetSurface>(m_inputSurface, adjustedDistance);
    }
}

std::shared_ptr<Surface> OffsetSurfaceTool::createInitialOffset() {
    return std::make_shared<OffsetSurface>(m_inputSurface, m_offsetDistance);
}

void OffsetSurfaceTool::preserveSharpFeatures(std::shared_ptr<Surface>& surface) {
    // This would involve:
    // 1. Detecting sharp edges/corners in input surface
    // 2. Creating additional surfaces to maintain sharp features
    // 3. Blending between offset surfaces
    // For now, we'll keep the basic offset surface
}

} // namespace Modeling
} // namespace RebelCAD
