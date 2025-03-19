#include "modeling/IntersectTool.h"
#include "core/Error.h"
#include "core/Log.h"

namespace RebelCAD {
namespace Modeling {

IntersectTool::IntersectTool() = default;
IntersectTool::~IntersectTool() = default;

void IntersectTool::SetBodies(const std::vector<std::shared_ptr<SolidBody>>& bodies) {
    if (bodies.size() < 2) {
        throw std::invalid_argument("IntersectTool requires at least 2 bodies");
    }
    
    // Validate each body
    for (const auto& body : bodies) {
        if (!body) {
            throw std::invalid_argument("Null body provided to IntersectTool");
        }
        if (!body->GetTopology().IsValid()) {
            throw std::invalid_argument("Invalid body topology provided to IntersectTool");
        }
    }
    
    m_Bodies = bodies;
}

bool IntersectTool::Validate() const {
    if (m_Bodies.size() < 2) {
        return false;
    }

    // Check that all bodies intersect with at least one other body
    for (size_t i = 0; i < m_Bodies.size(); ++i) {
        bool hasIntersection = false;
        for (size_t j = 0; j < m_Bodies.size(); ++j) {
            if (i != j && m_Bodies[i]->IntersectsWith(m_Bodies[j])) {
                hasIntersection = true;
                break;
            }
        }
        if (!hasIntersection) {
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<SolidBody> IntersectTool::IntersectPair(
    const std::shared_ptr<SolidBody>& body1,
    const std::shared_ptr<SolidBody>& body2) {
    
    // Get topologies
    const Topology& topo1 = body1->GetTopology();
    const Topology& topo2 = body2->GetTopology();
    
    // Create merged topology containing both bodies
    Topology mergedTopo = topo1.Merge(topo2);
    
    // Create compound body from merged topology
    auto compound = SolidBody::FromTopology(mergedTopo);
    
    // The intersection will be the regions where both bodies exist
    // This is handled internally by the FromTopology method which
    // analyzes the merged topology to identify shared regions
    
    return compound;
}

std::shared_ptr<SolidBody> IntersectTool::Execute() {
    if (m_Bodies.empty()) {
        throw std::runtime_error("No bodies set for intersection operation");
    }
    
    if (!Validate()) {
        throw std::runtime_error("Invalid intersection configuration - bodies may not intersect");
    }
    
    // Start with first body
    auto result = m_Bodies[0];
    
    // Intersect with each subsequent body
    for (size_t i = 1; i < m_Bodies.size(); ++i) {
        result = IntersectPair(result, m_Bodies[i]);
        if (!result || !result->GetTopology().IsValid()) {
            throw std::runtime_error("Failed to create valid intersection");
        }
    }
    
    return result;
}

} // namespace Modeling
} // namespace RebelCAD
