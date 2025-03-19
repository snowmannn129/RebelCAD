#include "modeling/SubtractTool.h"
#include "core/Log.h"
#include "core/Error.h"
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

using namespace rebel::core;

SubtractTool::SubtractTool() : m_isValid(false) {}

SubtractTool::~SubtractTool() = default;

void SubtractTool::SetMainBody(const std::shared_ptr<SolidBody>& body) {
    if (!body) {
        throw Error(ErrorCode::CADError, "Main body cannot be null");
    }
    m_mainBody = body;
    m_isValid = false;
}

void SubtractTool::SetToolBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies) {
    if (bodies.empty()) {
        throw Error(ErrorCode::CADError, "At least one tool body must be provided");
    }
    
    for (const auto& body : bodies) {
        if (!body) {
            throw Error(ErrorCode::CADError, "Tool body cannot be null");
        }
    }
    
    m_toolBodies = bodies;
    m_isValid = false;
}

std::shared_ptr<SolidBody> SubtractTool::Execute() {
    ValidateBodies();
    
    if (!CheckIntersections()) {
        // If no intersections, return copy of main body unchanged
        return SolidBody::FromTopology(m_mainBody->GetTopology());
    }
    
    // Process each tool body sequentially
    Topology resultTopology = m_mainBody->GetTopology();
    
    for (const auto& toolBody : m_toolBodies) {
        // Get the faces that will form the new boundary where subtraction occurs
        std::vector<Face> boundaryFaces = CreateBoundaryFaces(toolBody);
        
        // Update topology to remove intersecting volume and add new boundary faces
        Topology currentTopology = ProcessSubtraction();
        if (!currentTopology.IsValid()) {
            throw Error(ErrorCode::GeometryError, "Failed to create valid topology after subtraction");
        }
        
        resultTopology = currentTopology;
    }
    
    return SolidBody::FromTopology(resultTopology);
}

void SubtractTool::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) {
    if (!m_mainBody || m_toolBodies.empty()) {
        return;
    }
    
    try {
        // Attempt to compute result for preview
        auto previewBody = Execute();
        if (previewBody) {
            // TODO: Implement preview rendering
            // This will be implemented when the graphics system supports preview modes
        }
    }
    catch (const Error&) {
        // Ignore errors during preview
    }
}

void SubtractTool::ValidateBodies() const {
    if (!m_mainBody) {
        throw Error(ErrorCode::CADError, "Main body not set");
    }
    
    if (m_toolBodies.empty()) {
        throw Error(ErrorCode::CADError, "No tool bodies set");
    }
    
    if (!m_mainBody->GetTopology().IsValid()) {
        throw Error(ErrorCode::GeometryError, "Main body has invalid topology");
    }
    
    for (const auto& body : m_toolBodies) {
        if (!body->GetTopology().IsValid()) {
            throw Error(ErrorCode::GeometryError, "Tool body has invalid topology");
        }
    }
}

bool SubtractTool::CheckIntersections() const {
    for (const auto& toolBody : m_toolBodies) {
        if (m_mainBody->IntersectsWith(toolBody)) {
            return true;
        }
    }
    return false;
}

Topology SubtractTool::ProcessSubtraction() const {
    // This is a simplified implementation. A full implementation would:
    // 1. Find all intersection curves between bodies
    // 2. Create new topology splitting faces at intersections
    // 3. Remove faces and volumes that are inside tool bodies
    // 4. Ensure resulting topology is manifold and valid
    
    Topology result = m_mainBody->GetTopology();
    
    // Process each tool body sequentially
    for (const auto& toolBody : m_toolBodies) {
        if (m_mainBody->IntersectsWith(toolBody)) {
            // Get new boundary faces where subtraction occurs
            std::vector<Face> boundaryFaces = CreateBoundaryFaces(toolBody);
            
            // Update topology vertices and edges
            for (const Face& face : boundaryFaces) {
                result.faces.push_back(face);
            }
            
            // Ensure topology remains valid
            if (!result.IsValid()) {
                throw Error(ErrorCode::GeometryError, "Invalid topology created during subtraction");
            }
        }
    }
    
    return result;
}

std::vector<Face> SubtractTool::CreateBoundaryFaces(const std::shared_ptr<SolidBody>& toolBody) const {
    std::vector<Face> boundaryFaces;
    
    // This is a placeholder implementation
    // A full implementation would:
    // 1. Calculate intersection curves between main body and tool body
    // 2. Create new faces along intersection boundaries
    // 3. Ensure proper orientation of new faces
    // 4. Handle multiple intersections and complex cases
    
    // For now, we'll create a simple planar face at the intersection
    if (m_mainBody->IntersectsWith(toolBody)) {
        Face newFace;
        // Add edges that form the intersection boundary
        // This is simplified and would need proper implementation
        boundaryFaces.push_back(newFace);
    }
    
    return boundaryFaces;
}

} // namespace Modeling
} // namespace RebelCAD
