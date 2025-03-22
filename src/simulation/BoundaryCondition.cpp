#include "simulation/BoundaryCondition.h"
#include "simulation/ThermalBoundaryConditions.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>
#include <iomanip>

namespace rebel::simulation {

BoundaryCondition::BoundaryCondition(
    const std::string& id,
    BoundaryConditionType type,
    const std::string& groupName)
    : id(id), type(type), groupName(groupName) {
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createDisplacement(
    const std::string& id,
    const std::string& nodeGroupName,
    DisplacementDirection direction,
    double value) {
    
    REBEL_LOG_INFO("Creating displacement boundary condition: " + id);
    return std::shared_ptr<BoundaryCondition>(
        new DisplacementBC(id, nodeGroupName, direction, value));
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createForce(
    const std::string& id,
    const std::string& nodeGroupName,
    double forceX,
    double forceY,
    double forceZ) {
    
    REBEL_LOG_INFO("Creating force boundary condition: " + id);
    return std::shared_ptr<BoundaryCondition>(
        new ForceBC(id, nodeGroupName, forceX, forceY, forceZ));
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createPressure(
    const std::string& id,
    const std::string& elementGroupName,
    double pressure) {
    
    REBEL_LOG_INFO("Creating pressure boundary condition: " + id);
    return std::shared_ptr<BoundaryCondition>(
        new PressureBC(id, elementGroupName, pressure));
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createTemperature(
    const std::string& id,
    const std::string& nodeGroupName,
    double temperature) {

    REBEL_LOG_INFO("Creating temperature boundary condition: " + id);
    
    // Create a new temperature boundary condition
    auto bc = std::make_shared<TemperatureBC>(id, nodeGroupName, temperature);
    
    REBEL_LOG_INFO("Temperature boundary condition created successfully");
    return bc;
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createHeatFlux(
    const std::string& id,
    const std::string& elementGroupName,
    double heatFlux) {

    REBEL_LOG_INFO("Creating heat flux boundary condition: " + id);
    
    // Create a new heat flux boundary condition
    auto bc = std::make_shared<HeatFluxBC>(id, elementGroupName, heatFlux);
    
    REBEL_LOG_INFO("Heat flux boundary condition created successfully");
    return bc;
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createConvection(
    const std::string& id,
    const std::string& elementGroupName,
    double filmCoefficient,
    double ambientTemperature) {

    REBEL_LOG_INFO("Creating convection boundary condition: " + id);
    
    // Create a new convection boundary condition
    auto bc = std::make_shared<ConvectionBC>(id, elementGroupName, filmCoefficient, ambientTemperature);
    
    REBEL_LOG_INFO("Convection boundary condition created successfully");
    return bc;
}

std::shared_ptr<BoundaryCondition> BoundaryCondition::createSymmetry(
    const std::string& id,
    const std::string& nodeGroupName,
    DisplacementDirection direction) {
    
    REBEL_LOG_INFO("Creating symmetry boundary condition: " + id);
    // TODO: Implement SymmetryBC class
    REBEL_LOG_ERROR("Symmetry boundary condition not implemented yet");
    return nullptr;
}

// DisplacementBC implementation
DisplacementBC::DisplacementBC(
    const std::string& id,
    const std::string& nodeGroupName,
    DisplacementDirection direction,
    double value)
    : BoundaryCondition(id, BoundaryConditionType::Displacement, nodeGroupName),
      direction(direction), value(value) {
}

bool DisplacementBC::isValid(const Mesh& mesh) const {
    const NodeGroup* group = mesh.getNodeGroupByName(groupName);
    if (!group) {
        REBEL_LOG_ERROR("Node group '" + groupName + "' not found");
        return false;
    }
    
    if (group->nodeIds.empty()) {
        REBEL_LOG_ERROR("Node group '" + groupName + "' is empty");
        return false;
    }
    
    return true;
}

bool DisplacementBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying displacement boundary condition: " + id);
    
    // TODO: Implement actual application of displacement boundary condition
    // For now, just log the application
    
    const NodeGroup* group = mesh.getNodeGroupByName(groupName);
    REBEL_LOG_INFO("Applied displacement boundary condition to " + 
                  std::to_string(group->nodeIds.size()) + " nodes");
    
    return true;
}

std::string DisplacementBC::toString() const {
    std::ostringstream oss;
    oss << "DisplacementBC(id=" << id << ", group=" << groupName << ", ";
    
    switch (direction) {
        case DisplacementDirection::X:
            oss << "direction=X";
            break;
        case DisplacementDirection::Y:
            oss << "direction=Y";
            break;
        case DisplacementDirection::Z:
            oss << "direction=Z";
            break;
        case DisplacementDirection::XY:
            oss << "direction=XY";
            break;
        case DisplacementDirection::YZ:
            oss << "direction=YZ";
            break;
        case DisplacementDirection::XZ:
            oss << "direction=XZ";
            break;
        case DisplacementDirection::XYZ:
            oss << "direction=XYZ";
            break;
        default:
            oss << "direction=Unknown";
            break;
    }
    
    oss << ", value=" << value << ")";
    return oss.str();
}

// ForceBC implementation
ForceBC::ForceBC(
    const std::string& id,
    const std::string& nodeGroupName,
    double forceX,
    double forceY,
    double forceZ)
    : BoundaryCondition(id, BoundaryConditionType::Force, nodeGroupName),
      forceX(forceX), forceY(forceY), forceZ(forceZ) {
}

bool ForceBC::isValid(const Mesh& mesh) const {
    const NodeGroup* group = mesh.getNodeGroupByName(groupName);
    if (!group) {
        REBEL_LOG_ERROR("Node group '" + groupName + "' not found");
        return false;
    }
    
    if (group->nodeIds.empty()) {
        REBEL_LOG_ERROR("Node group '" + groupName + "' is empty");
        return false;
    }
    
    return true;
}

bool ForceBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying force boundary condition: " + id);
    
    // TODO: Implement actual application of force boundary condition
    // For now, just log the application
    
    const NodeGroup* group = mesh.getNodeGroupByName(groupName);
    REBEL_LOG_INFO("Applied force boundary condition to " + 
                  std::to_string(group->nodeIds.size()) + " nodes");
    
    return true;
}

std::string ForceBC::toString() const {
    std::ostringstream oss;
    oss << "ForceBC(id=" << id << ", group=" << groupName << ", ";
    oss << "forceX=" << forceX << ", forceY=" << forceY << ", forceZ=" << forceZ << ")";
    return oss.str();
}

// PressureBC implementation
PressureBC::PressureBC(
    const std::string& id,
    const std::string& elementGroupName,
    double pressure)
    : BoundaryCondition(id, BoundaryConditionType::Pressure, elementGroupName),
      pressure(pressure) {
}

bool PressureBC::isValid(const Mesh& mesh) const {
    const ElementGroup* group = mesh.getElementGroupByName(groupName);
    if (!group) {
        REBEL_LOG_ERROR("Element group '" + groupName + "' not found");
        return false;
    }
    
    if (group->elementIds.empty()) {
        REBEL_LOG_ERROR("Element group '" + groupName + "' is empty");
        return false;
    }
    
    return true;
}

bool PressureBC::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying pressure boundary condition: " + id);
    
    // TODO: Implement actual application of pressure boundary condition
    // For now, just log the application
    
    const ElementGroup* group = mesh.getElementGroupByName(groupName);
    REBEL_LOG_INFO("Applied pressure boundary condition to " + 
                  std::to_string(group->elementIds.size()) + " elements");
    
    return true;
}

std::string PressureBC::toString() const {
    std::ostringstream oss;
    oss << "PressureBC(id=" << id << ", group=" << groupName << ", ";
    oss << "pressure=" << pressure << ")";
    return oss.str();
}

} // namespace rebel::simulation
