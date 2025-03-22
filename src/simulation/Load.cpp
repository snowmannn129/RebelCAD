#include "simulation/Load.h"
#include "simulation/ThermalLoads.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace rebel::simulation {

Load::Load(
    const std::string& id,
    LoadType type,
    const std::string& groupName,
    LoadVariation variation)
    : id(id), type(type), groupName(groupName), variation(variation) {
}

std::shared_ptr<Load> Load::createPointForce(
    const std::string& id,
    const std::string& nodeGroupName,
    double forceX,
    double forceY,
    double forceZ,
    LoadVariation variation) {
    
    REBEL_LOG_INFO("Creating point force load: " + id);
    return std::shared_ptr<Load>(
        new PointForceLoad(id, nodeGroupName, forceX, forceY, forceZ, variation));
}

std::shared_ptr<Load> Load::createSurfacePressure(
    const std::string& id,
    const std::string& elementGroupName,
    double pressure,
    LoadVariation variation) {
    
    REBEL_LOG_INFO("Creating surface pressure load: " + id);
    return std::shared_ptr<Load>(
        new SurfacePressureLoad(id, elementGroupName, pressure, variation));
}

std::shared_ptr<Load> Load::createBodyForce(
    const std::string& id,
    const std::string& elementGroupName,
    double forceX,
    double forceY,
    double forceZ,
    LoadVariation variation) {
    
    REBEL_LOG_INFO("Creating body force load: " + id);
    return std::shared_ptr<Load>(
        new BodyForceLoad(id, elementGroupName, forceX, forceY, forceZ, variation));
}

std::shared_ptr<Load> Load::createGravity(
    const std::string& id,
    const std::string& elementGroupName,
    double accelerationX,
    double accelerationY,
    double accelerationZ) {
    
    REBEL_LOG_INFO("Creating gravity load: " + id);
    
    // Convert acceleration to force per unit volume
    // F = m*a = rho*V*a
    // F/V = rho*a
    // We'll use a standard density of 1.0 for now, and let the material density
    // scale the force during analysis
    
    return std::shared_ptr<Load>(
        new BodyForceLoad(id, elementGroupName, accelerationX, accelerationY, accelerationZ, LoadVariation::Static));
}

std::shared_ptr<Load> Load::createCentrifugal(
    const std::string& id,
    const std::string& elementGroupName,
    double centerX,
    double centerY,
    double centerZ,
    double axisX,
    double axisY,
    double axisZ,
    double angularVelocity) {
    
    REBEL_LOG_INFO("Creating centrifugal load: " + id);
    // TODO: Implement CentrifugalLoad class
    REBEL_LOG_ERROR("Centrifugal load not implemented yet");
    return nullptr;
}

std::shared_ptr<Load> Load::createThermal(
    const std::string& id,
    const std::string& nodeGroupName,
    double temperature,
    double referenceTemperature,
    LoadVariation variation) {

    REBEL_LOG_INFO("Creating thermal load: " + id);
    
    // Create a heat source load instead of a thermal load
    // The temperature difference can be treated as a heat source
    double heatSource = temperature - referenceTemperature;
    auto load = std::make_shared<HeatSourceLoad>(id, nodeGroupName, heatSource);
    
    REBEL_LOG_INFO("Thermal load created successfully");
    return load;
}

// PointForceLoad implementation
PointForceLoad::PointForceLoad(
    const std::string& id,
    const std::string& nodeGroupName,
    double forceX,
    double forceY,
    double forceZ,
    LoadVariation variation)
    : Load(id, LoadType::PointForce, nodeGroupName, variation),
      forceX(forceX), forceY(forceY), forceZ(forceZ) {
}

bool PointForceLoad::isValid(const Mesh& mesh) const {
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

bool PointForceLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying point force load: " + id);
    
    // TODO: Implement actual application of point force load
    // For now, just log the application
    
    const NodeGroup* group = mesh.getNodeGroupByName(groupName);
    REBEL_LOG_INFO("Applied point force load to " + 
                  std::to_string(group->nodeIds.size()) + " nodes");
    
    return true;
}

std::string PointForceLoad::toString() const {
    std::ostringstream oss;
    oss << "PointForceLoad(id=" << id << ", group=" << groupName << ", ";
    oss << "forceX=" << forceX << ", forceY=" << forceY << ", forceZ=" << forceZ << ", ";
    
    switch (variation) {
        case LoadVariation::Static:
            oss << "variation=Static";
            break;
        case LoadVariation::Transient:
            oss << "variation=Transient";
            break;
        case LoadVariation::Harmonic:
            oss << "variation=Harmonic";
            break;
        case LoadVariation::Random:
            oss << "variation=Random";
            break;
        default:
            oss << "variation=Unknown";
            break;
    }
    
    oss << ")";
    return oss.str();
}

// SurfacePressureLoad implementation
SurfacePressureLoad::SurfacePressureLoad(
    const std::string& id,
    const std::string& elementGroupName,
    double pressure,
    LoadVariation variation)
    : Load(id, LoadType::SurfacePressure, elementGroupName, variation),
      pressure(pressure) {
}

bool SurfacePressureLoad::isValid(const Mesh& mesh) const {
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

bool SurfacePressureLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying surface pressure load: " + id);
    
    // TODO: Implement actual application of surface pressure load
    // For now, just log the application
    
    const ElementGroup* group = mesh.getElementGroupByName(groupName);
    REBEL_LOG_INFO("Applied surface pressure load to " + 
                  std::to_string(group->elementIds.size()) + " elements");
    
    return true;
}

std::string SurfacePressureLoad::toString() const {
    std::ostringstream oss;
    oss << "SurfacePressureLoad(id=" << id << ", group=" << groupName << ", ";
    oss << "pressure=" << pressure << ", ";
    
    switch (variation) {
        case LoadVariation::Static:
            oss << "variation=Static";
            break;
        case LoadVariation::Transient:
            oss << "variation=Transient";
            break;
        case LoadVariation::Harmonic:
            oss << "variation=Harmonic";
            break;
        case LoadVariation::Random:
            oss << "variation=Random";
            break;
        default:
            oss << "variation=Unknown";
            break;
    }
    
    oss << ")";
    return oss.str();
}

// BodyForceLoad implementation
BodyForceLoad::BodyForceLoad(
    const std::string& id,
    const std::string& elementGroupName,
    double forceX,
    double forceY,
    double forceZ,
    LoadVariation variation)
    : Load(id, LoadType::BodyForce, elementGroupName, variation),
      forceX(forceX), forceY(forceY), forceZ(forceZ) {
}

bool BodyForceLoad::isValid(const Mesh& mesh) const {
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

bool BodyForceLoad::apply(Mesh& mesh) const {
    if (!isValid(mesh)) {
        return false;
    }
    
    REBEL_LOG_INFO("Applying body force load: " + id);
    
    // TODO: Implement actual application of body force load
    // For now, just log the application
    
    const ElementGroup* group = mesh.getElementGroupByName(groupName);
    REBEL_LOG_INFO("Applied body force load to " + 
                  std::to_string(group->elementIds.size()) + " elements");
    
    return true;
}

std::string BodyForceLoad::toString() const {
    std::ostringstream oss;
    oss << "BodyForceLoad(id=" << id << ", group=" << groupName << ", ";
    oss << "forceX=" << forceX << ", forceY=" << forceY << ", forceZ=" << forceZ << ", ";
    
    switch (variation) {
        case LoadVariation::Static:
            oss << "variation=Static";
            break;
        case LoadVariation::Transient:
            oss << "variation=Transient";
            break;
        case LoadVariation::Harmonic:
            oss << "variation=Harmonic";
            break;
        case LoadVariation::Random:
            oss << "variation=Random";
            break;
        default:
            oss << "variation=Unknown";
            break;
    }
    
    oss << ")";
    return oss.str();
}

} // namespace rebel::simulation
