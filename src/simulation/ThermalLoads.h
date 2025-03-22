#pragma once

#include "simulation/Load.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <sstream>
#include <iomanip>

namespace rebel::simulation {

/**
 * @brief Thermal load
 * 
 * This load applies a temperature change to a group of nodes, which can cause thermal expansion or contraction.
 */
class ThermalLoad : public Load {
public:
    /**
     * @brief Constructor
     * @param id Load ID
     * @param nodeGroupName Name of the node group
     * @param temperature Temperature value
     * @param referenceTemperature Reference temperature (zero thermal strain temperature)
     * @param variation Load variation type
     */
    ThermalLoad(
        const std::string& id,
        const std::string& nodeGroupName,
        double temperature,
        double referenceTemperature,
        LoadVariation variation)
        : Load(id, LoadType::Thermal, nodeGroupName, variation),
          temperature(temperature),
          referenceTemperature(referenceTemperature) {
    }

    /**
     * @brief Check if the load is valid for the given mesh
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
     * @brief Apply the load to the mesh
     * @param mesh The mesh to apply to
     * @return True if successful, false otherwise
     */
    bool apply(Mesh& mesh) const override {
        if (!isValid(mesh)) {
            return false;
        }
        
        REBEL_LOG_INFO("Applying thermal load: " + id);
        
        // TODO: Implement actual application of thermal load
        // For now, just log the application
        
        const NodeGroup* group = mesh.getNodeGroupByName(groupName);
        REBEL_LOG_INFO("Applied thermal load to " + 
                      std::to_string(group->nodeIds.size()) + " nodes");
        
        return true;
    }

    /**
     * @brief Get a string representation of the load
     * @return String representation
     */
    std::string toString() const override {
        std::ostringstream oss;
        oss << "ThermalLoad(id=" << id << ", group=" << groupName << ", ";
        oss << "temperature=" << temperature << ", ";
        oss << "referenceTemperature=" << referenceTemperature << ", ";
        
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

    /**
     * @brief Get the temperature value
     * @return Temperature value
     */
    double getTemperature() const {
        return temperature;
    }

    /**
     * @brief Get the reference temperature value
     * @return Reference temperature value
     */
    double getReferenceTemperature() const {
        return referenceTemperature;
    }

private:
    double temperature;          ///< Temperature value
    double referenceTemperature; ///< Reference temperature (zero thermal strain temperature)
};

/**
 * @brief Heat generation load
 * 
 * This load applies a heat generation rate to a group of elements, which can cause temperature changes.
 */
class HeatGenerationLoad : public Load {
public:
    /**
     * @brief Constructor
     * @param id Load ID
     * @param elementGroupName Name of the element group
     * @param heatGenerationRate Heat generation rate per unit volume
     * @param variation Load variation type
     */
    HeatGenerationLoad(
        const std::string& id,
        const std::string& elementGroupName,
        double heatGenerationRate,
        LoadVariation variation)
        : Load(id, LoadType::HeatGeneration, elementGroupName, variation),
          heatGenerationRate(heatGenerationRate) {
    }

    /**
     * @brief Check if the load is valid for the given mesh
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
     * @brief Apply the load to the mesh
     * @param mesh The mesh to apply to
     * @return True if successful, false otherwise
     */
    bool apply(Mesh& mesh) const override {
        if (!isValid(mesh)) {
            return false;
        }
        
        REBEL_LOG_INFO("Applying heat generation load: " + id);
        
        // TODO: Implement actual application of heat generation load
        // For now, just log the application
        
        const ElementGroup* group = mesh.getElementGroupByName(groupName);
        REBEL_LOG_INFO("Applied heat generation load to " + 
                      std::to_string(group->elementIds.size()) + " elements");
        
        return true;
    }

    /**
     * @brief Get a string representation of the load
     * @return String representation
     */
    std::string toString() const override {
        std::ostringstream oss;
        oss << "HeatGenerationLoad(id=" << id << ", group=" << groupName << ", ";
        oss << "heatGenerationRate=" << heatGenerationRate << ", ";
        
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

    /**
     * @brief Get the heat generation rate
     * @return Heat generation rate
     */
    double getHeatGenerationRate() const {
        return heatGenerationRate;
    }

private:
    double heatGenerationRate; ///< Heat generation rate per unit volume
};

} // namespace rebel::simulation
