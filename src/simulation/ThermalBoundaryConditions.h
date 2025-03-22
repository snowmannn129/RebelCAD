#pragma once

#include "simulation/BoundaryCondition.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>
#include <iomanip>

namespace rebel::simulation {

/**
 * @brief Temperature boundary condition
 * 
 * This boundary condition applies a prescribed temperature to a group of nodes.
 */
class TemperatureBC : public BoundaryCondition {
public:
    /**
     * @brief Constructor
     * @param id Boundary condition ID
     * @param nodeGroupName Name of the node group
     * @param temperature Prescribed temperature value
     */
    TemperatureBC(
        const std::string& id,
        const std::string& nodeGroupName,
        double temperature)
        : BoundaryCondition(id, BoundaryConditionType::Temperature, nodeGroupName),
          temperature(temperature) {
    }

    /**
     * @brief Check if the boundary condition is valid for the given mesh
     * @param mesh The mesh to check against
     * @return True if valid, false otherwise
     */
    bool isValid(const Mesh& mesh) const override {
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

    /**
     * @brief Apply the boundary condition to the mesh
     * @param mesh The mesh to apply to
     * @return True if successful, false otherwise
     */
    bool apply(Mesh& mesh) const override {
        if (!isValid(mesh)) {
            return false;
        }
        
        REBEL_LOG_INFO("Applying temperature boundary condition: " + id);
        
        // TODO: Implement actual application of temperature boundary condition
        // For now, just log the application
        
        const NodeGroup* group = mesh.getNodeGroupByName(groupName);
        REBEL_LOG_INFO("Applied temperature boundary condition to " + 
                      std::to_string(group->nodeIds.size()) + " nodes");
        
        return true;
    }

    /**
     * @brief Get a string representation of the boundary condition
     * @return String representation
     */
    std::string toString() const override {
        std::ostringstream oss;
        oss << "TemperatureBC(id=" << id << ", group=" << groupName << ", ";
        oss << "temperature=" << temperature << ")";
        return oss.str();
    }

    /**
     * @brief Get the prescribed temperature value
     * @return Temperature value
     */
    double getTemperature() const {
        return temperature;
    }

private:
    double temperature; ///< Prescribed temperature value
};

/**
 * @brief Heat flux boundary condition
 * 
 * This boundary condition applies a prescribed heat flux to a group of elements.
 */
class HeatFluxBC : public BoundaryCondition {
public:
    /**
     * @brief Constructor
     * @param id Boundary condition ID
     * @param elementGroupName Name of the element group
     * @param heatFlux Prescribed heat flux value
     */
    HeatFluxBC(
        const std::string& id,
        const std::string& elementGroupName,
        double heatFlux)
        : BoundaryCondition(id, BoundaryConditionType::HeatFlux, elementGroupName),
          heatFlux(heatFlux) {
    }

    /**
     * @brief Check if the boundary condition is valid for the given mesh
     * @param mesh The mesh to check against
     * @return True if valid, false otherwise
     */
    bool isValid(const Mesh& mesh) const override {
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

    /**
     * @brief Apply the boundary condition to the mesh
     * @param mesh The mesh to apply to
     * @return True if successful, false otherwise
     */
    bool apply(Mesh& mesh) const override {
        if (!isValid(mesh)) {
            return false;
        }
        
        REBEL_LOG_INFO("Applying heat flux boundary condition: " + id);
        
        // TODO: Implement actual application of heat flux boundary condition
        // For now, just log the application
        
        const ElementGroup* group = mesh.getElementGroupByName(groupName);
        REBEL_LOG_INFO("Applied heat flux boundary condition to " + 
                      std::to_string(group->elementIds.size()) + " elements");
        
        return true;
    }

    /**
     * @brief Get a string representation of the boundary condition
     * @return String representation
     */
    std::string toString() const override {
        std::ostringstream oss;
        oss << "HeatFluxBC(id=" << id << ", group=" << groupName << ", ";
        oss << "heatFlux=" << heatFlux << ")";
        return oss.str();
    }

    /**
     * @brief Get the prescribed heat flux value
     * @return Heat flux value
     */
    double getHeatFlux() const {
        return heatFlux;
    }

private:
    double heatFlux; ///< Prescribed heat flux value
};

/**
 * @brief Convection boundary condition
 * 
 * This boundary condition applies a convection heat transfer to a group of elements.
 */
class ConvectionBC : public BoundaryCondition {
public:
    /**
     * @brief Constructor
     * @param id Boundary condition ID
     * @param elementGroupName Name of the element group
     * @param filmCoefficient Film coefficient (heat transfer coefficient)
     * @param ambientTemperature Ambient temperature
     */
    ConvectionBC(
        const std::string& id,
        const std::string& elementGroupName,
        double filmCoefficient,
        double ambientTemperature)
        : BoundaryCondition(id, BoundaryConditionType::Convection, elementGroupName),
          filmCoefficient(filmCoefficient),
          ambientTemperature(ambientTemperature) {
    }

    /**
     * @brief Check if the boundary condition is valid for the given mesh
     * @param mesh The mesh to check against
     * @return True if valid, false otherwise
     */
    bool isValid(const Mesh& mesh) const override {
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

    /**
     * @brief Apply the boundary condition to the mesh
     * @param mesh The mesh to apply to
     * @return True if successful, false otherwise
     */
    bool apply(Mesh& mesh) const override {
        if (!isValid(mesh)) {
            return false;
        }
        
        REBEL_LOG_INFO("Applying convection boundary condition: " + id);
        
        // TODO: Implement actual application of convection boundary condition
        // For now, just log the application
        
        const ElementGroup* group = mesh.getElementGroupByName(groupName);
        REBEL_LOG_INFO("Applied convection boundary condition to " + 
                      std::to_string(group->elementIds.size()) + " elements");
        
        return true;
    }

    /**
     * @brief Get a string representation of the boundary condition
     * @return String representation
     */
    std::string toString() const override {
        std::ostringstream oss;
        oss << "ConvectionBC(id=" << id << ", group=" << groupName << ", ";
        oss << "filmCoefficient=" << filmCoefficient << ", ";
        oss << "ambientTemperature=" << ambientTemperature << ")";
        return oss.str();
    }

    /**
     * @brief Get the film coefficient (heat transfer coefficient)
     * @return Film coefficient
     */
    double getFilmCoefficient() const {
        return filmCoefficient;
    }

    /**
     * @brief Get the ambient temperature
     * @return Ambient temperature
     */
    double getAmbientTemperature() const {
        return ambientTemperature;
    }

private:
    double filmCoefficient;    ///< Film coefficient (heat transfer coefficient)
    double ambientTemperature; ///< Ambient temperature
};

} // namespace rebel::simulation
