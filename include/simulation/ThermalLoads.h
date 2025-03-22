#pragma once

#include "simulation/Load.h"
#include <string>
#include <functional>

namespace rebel::simulation {

/**
 * @class HeatSourceLoad
 * @brief Heat source load.
 */
class HeatSourceLoad : public Load {
public:
    /**
     * @brief Constructs a new HeatSourceLoad object.
     * @param id The ID of the load.
     * @param groupName The name of the node group to apply the load to.
     * @param heatSource The heat source value.
     */
    HeatSourceLoad(const std::string& id, const std::string& groupName, double heatSource)
        : Load(id, LoadType::Thermal, groupName, LoadVariation::Static),
          m_heatSource(heatSource) {
    }
    
    /**
     * @brief Gets the heat source value.
     * @return The heat source value.
     */
    double getHeatSource() const {
        return m_heatSource;
    }
    
    /**
     * @brief Sets the heat source value.
     * @param heatSource The heat source value.
     */
    void setHeatSource(double heatSource) {
        m_heatSource = heatSource;
    }
    
    /**
     * @brief Checks if the load is valid for a given mesh.
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;
    
    /**
     * @brief Applies the load to a mesh.
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;
    
    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;
    
private:
    double m_heatSource;  ///< Heat source value
};

/**
 * @class HeatFluxLoad
 * @brief Heat flux load.
 */
class HeatFluxLoad : public Load {
public:
    /**
     * @brief Constructs a new HeatFluxLoad object.
     * @param id The ID of the load.
     * @param groupName The name of the element group to apply the load to.
     * @param heatFlux The heat flux value.
     */
    HeatFluxLoad(const std::string& id, const std::string& groupName, double heatFlux)
        : Load(id, LoadType::Thermal, groupName, LoadVariation::Static),
          m_heatFlux(heatFlux) {
    }
    
    /**
     * @brief Gets the heat flux value.
     * @return The heat flux value.
     */
    double getHeatFlux() const {
        return m_heatFlux;
    }
    
    /**
     * @brief Sets the heat flux value.
     * @param heatFlux The heat flux value.
     */
    void setHeatFlux(double heatFlux) {
        m_heatFlux = heatFlux;
    }
    
    /**
     * @brief Checks if the load is valid for a given mesh.
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;
    
    /**
     * @brief Applies the load to a mesh.
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;
    
    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;
    
private:
    double m_heatFlux;  ///< Heat flux value
};

/**
 * @class TimeVaryingHeatSourceLoad
 * @brief Time-varying heat source load.
 */
class TimeVaryingHeatSourceLoad : public Load {
public:
    /**
     * @brief Constructs a new TimeVaryingHeatSourceLoad object.
     * @param id The ID of the load.
     * @param groupName The name of the node group to apply the load to.
     * @param heatSourceFunction The heat source function.
     */
    TimeVaryingHeatSourceLoad(
        const std::string& id, 
        const std::string& groupName, 
        std::function<double(double)> heatSourceFunction)
        : Load(id, LoadType::Thermal, groupName, LoadVariation::Transient),
          m_heatSourceFunction(heatSourceFunction) {
    }
    
    /**
     * @brief Gets the heat source value at a given time.
     * @param time The time.
     * @return The heat source value.
     */
    double getHeatSource(double time) const {
        return m_heatSourceFunction(time);
    }
    
    /**
     * @brief Sets the heat source function.
     * @param heatSourceFunction The heat source function.
     */
    void setHeatSourceFunction(std::function<double(double)> heatSourceFunction) {
        m_heatSourceFunction = heatSourceFunction;
    }
    
    /**
     * @brief Checks if the load is valid for a given mesh.
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;
    
    /**
     * @brief Applies the load to a mesh.
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;
    
    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;
    
private:
    std::function<double(double)> m_heatSourceFunction;  ///< Heat source function
};

} // namespace rebel::simulation
