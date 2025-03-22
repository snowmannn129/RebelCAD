#include "simulation/ThermalBoundaryConditions.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>

namespace rebel::simulation {

// TemperatureBC implementation
bool TemperatureBC::isValid(const Mesh& mesh) const {
    // Check if the node group exists
    if (mesh.getNodeGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Node group '" + getGroupName() + "' not found for temperature boundary condition");
        return false;
    }
    
    return true;
}

bool TemperatureBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the node group
    const NodeGroup* group = mesh.getNodeGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get node group '" + getGroupName() + "' for temperature boundary condition");
        return false;
    }
    
    // Apply temperature to all nodes in the group
    for (int nodeId : group->nodeIds) {
        // In a real implementation, we would set the temperature for the node
        // For now, we just log the action
        REBEL_LOG_INFO("Applied temperature " + std::to_string(getTemperature()) + 
                      " to node " + std::to_string(nodeId));
    }
    
    return true;
}

std::string TemperatureBC::toString() const {
    std::stringstream ss;
    ss << "TemperatureBC(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", temperature=" << getTemperature() << ")";
    return ss.str();
}

// HeatFluxBC implementation
bool HeatFluxBC::isValid(const Mesh& mesh) const {
    // Check if the element group exists
    if (mesh.getElementGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Element group '" + getGroupName() + "' not found for heat flux boundary condition");
        return false;
    }
    
    return true;
}

bool HeatFluxBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the element group
    const ElementGroup* group = mesh.getElementGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get element group '" + getGroupName() + "' for heat flux boundary condition");
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

std::string HeatFluxBC::toString() const {
    std::stringstream ss;
    ss << "HeatFluxBC(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", heatFlux=" << getHeatFlux() << ")";
    return ss.str();
}

// ConvectionBC implementation
bool ConvectionBC::isValid(const Mesh& mesh) const {
    // Check if the element group exists
    if (mesh.getElementGroupByName(getGroupName()) == nullptr) {
        REBEL_LOG_ERROR("Element group '" + getGroupName() + "' not found for convection boundary condition");
        return false;
    }
    
    return true;
}

bool ConvectionBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    // Get the element group
    const ElementGroup* group = mesh.getElementGroupByName(getGroupName());
    if (!group) {
        REBEL_LOG_ERROR("Failed to get element group '" + getGroupName() + "' for convection boundary condition");
        return false;
    }
    
    // Apply convection to all elements in the group
    for (int elemId : group->elementIds) {
        // In a real implementation, we would set the convection for the element
        // For now, we just log the action
        REBEL_LOG_INFO("Applied convection (h=" + std::to_string(getFilmCoefficient()) + 
                      ", T_inf=" + std::to_string(getAmbientTemperature()) + 
                      ") to element " + std::to_string(elemId));
    }
    
    return true;
}

std::string ConvectionBC::toString() const {
    std::stringstream ss;
    ss << "ConvectionBC(id=" << getId() 
       << ", group=" << getGroupName() 
       << ", filmCoefficient=" << getFilmCoefficient()
       << ", ambientTemperature=" << getAmbientTemperature() << ")";
    return ss.str();
}

} // namespace rebel::simulation
