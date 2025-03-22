#include "simulation/ThermalLoads.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>

namespace rebel::simulation {

// HeatSourceLoad implementation
bool HeatSourceLoad::isValid(const Mesh& mesh) const {
    // Check if the node group exists
    if (mesh.getNodeGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Node group '" + getGroupName() + "' not found for heat source load");
        return false;
    }
    
    return true;
}

bool HeatSourceLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the node group
    const NodeGroup* group = mesh.getNodeGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get node group '" + getGroupName() + "' for heat source load");
        return false;
    }
    
    // Apply heat source to all nodes in the group
    for (int nodeId : group->nodeIds) {
        // In a real implementation, we would set the heat source for the node
        // For now, we just log the action
        REBEL_LOG_INFO("Applied heat source " + std::to_string(getHeatSource()) + 
                      " to node " + std::to_string(nodeId));
    }
    
    return true;
}

std::string HeatSourceLoad::toString() const {
    std::stringstream ss;
    ss << "HeatSourceLoad(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", heatSource=" << getHeatSource() << ")";
    return ss.str();
}

// HeatFluxLoad implementation
bool HeatFluxLoad::isValid(const Mesh& mesh) const {
    // Check if the element group exists
    if (mesh.getElementGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Element group '" + getGroupName() + "' not found for heat flux load");
        return false;
    }
    
    return true;
}

bool HeatFluxLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the element group
    const ElementGroup* group = mesh.getElementGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get element group '" + getGroupName() + "' for heat flux load");
        return false;
    }
    
    // Apply heat flux to all elements in the group
    for (int elemId : group->elementIds) {
        // In a real implementation, we would set the heat flux for the element
        // For now, we just log the action
        REBEL_LOG_INFO("Applied heat flux " + std::to_string(getHeatFlux()) + 
                      " to element " + std::to_string(elemId));
    }
    
    return true;
}

std::string HeatFluxLoad::toString() const {
    std::stringstream ss;
    ss << "HeatFluxLoad(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", heatFlux=" << getHeatFlux() << ")";
    return ss.str();
}

// TimeVaryingHeatSourceLoad implementation
bool TimeVaryingHeatSourceLoad::isValid(const Mesh& mesh) const {
    // Check if the node group exists
    if (mesh.getNodeGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Node group '" + getGroupName() + "' not found for time-varying heat source load");
        return false;
    }
    
    return true;
}

bool TimeVaryingHeatSourceLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the node group
    const NodeGroup* group = mesh.getNodeGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get node group '" + getGroupName() + "' for time-varying heat source load");
        return false;
    }
    
    // Apply heat source to all nodes in the group
    // Note: In a real implementation, we would need to know the current time
    // For now, we just log the action with a placeholder time
    double time = 0.0;
    for (int nodeId : group->nodeIds) {
        REBEL_LOG_INFO("Applied time-varying heat source " + std::to_string(getHeatSource(time)) + 
                      " at time " + std::to_string(time) +
                      " to node " + std::to_string(nodeId));
    }
    
    return true;
}

std::string TimeVaryingHeatSourceLoad::toString() const {
    std::stringstream ss;
    ss << "TimeVaryingHeatSourceLoad(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", function=<time-varying function>)";
    return ss.str();
}

} // namespace rebel::simulation
