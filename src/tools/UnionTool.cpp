#include "modeling/UnionTool.h"
#include "core/Log.h"
#include "core/Error.h"
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

using namespace rebel::core;

UnionTool::UnionTool() : m_isValid(false) {}

UnionTool::~UnionTool() = default;

void UnionTool::SetBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies) {
    if (bodies.size() < 2) {
        throw Error(ErrorCode::CADError, "UnionTool requires at least 2 bodies for union operation");
    }

    // Validate that all bodies are non-null
    if (std::any_of(bodies.begin(), bodies.end(), 
        [](const auto& body) { return !body; })) {
        throw Error(ErrorCode::CADError, "UnionTool received null body in input");
    }

    m_bodies = bodies;
    m_isValid = true;
}

std::shared_ptr<SolidBody> UnionTool::Execute() {
    if (!m_isValid || m_bodies.empty()) {
        throw Error(ErrorCode::CADError, "UnionTool::Execute called without valid bodies");
    }

    ValidateBodies();

    // If bodies don't intersect, create a compound body
    if (!CheckIntersection()) {
        REBEL_LOG_INFO("Bodies don't intersect, creating compound body");
        return SolidBody::CreateCompound(m_bodies);
    }

    // Merge the topology of intersecting bodies
    auto mergedTopology = MergeTopology();
    
    // Create new solid body from merged topology
    auto result = SolidBody::FromTopology(mergedTopology);
    if (!result) {
        throw Error(ErrorCode::CADError, "Failed to create solid body from merged topology");
    }

    return result;
}

void UnionTool::Preview(std::shared_ptr<Graphics::GraphicsSystem> graphics) {
    if (!m_isValid || !graphics) {
        return;
    }

    try {
        // Attempt to create preview geometry
        auto previewBody = Execute();
        
        // Begin preview rendering mode
        graphics->beginPreview();
        
        // TODO: Implement actual preview rendering using previewBody
        // This would involve getting mesh data from the body and rendering it
        
        // End preview rendering mode
        graphics->endPreview();
    }
    catch (const Error& e) {
        REBEL_LOG_WARNING(std::string("Failed to generate union preview: ") + e.what());
    }
}

void UnionTool::ValidateBodies() const {
    if (!m_isValid) {
        throw Error(ErrorCode::CADError, "UnionTool not properly initialized");
    }

    // Check each body's topology is valid
    for (const auto& body : m_bodies) {
        if (!body->GetTopology().IsValid()) {
            throw Error(ErrorCode::GeometryError, "Invalid body topology detected");
        }
    }
}

bool UnionTool::CheckIntersection() const {
    // Check for intersections between all pairs of bodies
    for (size_t i = 0; i < m_bodies.size(); ++i) {
        for (size_t j = i + 1; j < m_bodies.size(); ++j) {
            if (m_bodies[i]->IntersectsWith(m_bodies[j])) {
                return true;
            }
        }
    }
    return false;
}

Topology UnionTool::MergeTopology() const {
    if (m_bodies.empty()) {
        throw Error(ErrorCode::CADError, "No bodies to merge");
    }

    // Start with the first body's topology
    Topology result = m_bodies[0]->GetTopology();

    // Merge with remaining bodies
    for (size_t i = 1; i < m_bodies.size(); ++i) {
        result = result.Merge(m_bodies[i]->GetTopology());
    }

    return result;
}

} // namespace Modeling
} // namespace RebelCAD
