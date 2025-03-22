#pragma once

#include "simulation/BoundaryCondition.h"
#include <string>

namespace rebel::simulation {

/**
 * @class TemperatureBC
 * @brief Temperature boundary condition.
 */
class TemperatureBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new TemperatureBC object.
     * @param id The ID of the boundary condition.
     * @param groupName The name of the node group to apply the boundary condition to.
     * @param temperature The temperature value.
     */
    TemperatureBC(const std::string& id, const std::string& groupName, double temperature)
        : BoundaryCondition(id, BoundaryConditionType::Temperature, groupName),
          m_temperature(temperature) {
    }
    
    /**
     * @brief Gets the temperature value.
     * @return The temperature value.
     */
    double getTemperature() const {
        return m_temperature;
    }
    
    /**
     * @brief Sets the temperature value.
     * @param temperature The temperature value.
     */
    void setTemperature(double temperature) {
        m_temperature = temperature;
    }
    
/**
 * @brief Checks if the boundary condition is valid for a given mesh.
 * @param mesh The mesh to check against.
 * @return True if the boundary condition is valid, false otherwise.
 */
bool isValid(const Mesh& mesh) const override;

/**
 * @brief Applies the boundary condition to a mesh.
 * @param mesh The mesh to apply to.
 * @return True if application was successful, false otherwise.
 */
bool apply(Mesh& mesh) const override;

/**
 * @brief Gets a string representation of the boundary condition.
 * @return String representation of the boundary condition.
 */
std::string toString() const override;
    
private:
    double m_temperature;  ///< Temperature value
};

/**
 * @class HeatFluxBC
 * @brief Heat flux boundary condition.
 */
class HeatFluxBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new HeatFluxBC object.
     * @param id The ID of the boundary condition.
     * @param groupName The name of the element group to apply the boundary condition to.
     * @param heatFlux The heat flux value.
     */
    HeatFluxBC(const std::string& id, const std::string& groupName, double heatFlux)
        : BoundaryCondition(id, BoundaryConditionType::HeatFlux, groupName),
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
 * @brief Checks if the boundary condition is valid for a given mesh.
 * @param mesh The mesh to check against.
 * @return True if the boundary condition is valid, false otherwise.
 */
bool isValid(const Mesh& mesh) const override;

/**
 * @brief Applies the boundary condition to a mesh.
 * @param mesh The mesh to apply to.
 * @return True if application was successful, false otherwise.
 */
bool apply(Mesh& mesh) const override;

/**
 * @brief Gets a string representation of the boundary condition.
 * @return String representation of the boundary condition.
 */
std::string toString() const override;
    
private:
    double m_heatFlux;  ///< Heat flux value
};

/**
 * @class ConvectionBC
 * @brief Convection boundary condition.
 */
class ConvectionBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new ConvectionBC object.
     * @param id The ID of the boundary condition.
     * @param groupName The name of the element group to apply the boundary condition to.
     * @param filmCoefficient The film coefficient (heat transfer coefficient).
     * @param ambientTemperature The ambient temperature.
     */
    ConvectionBC(const std::string& id, const std::string& groupName, 
                 double filmCoefficient, double ambientTemperature)
        : BoundaryCondition(id, BoundaryConditionType::Convection, groupName),
          m_filmCoefficient(filmCoefficient),
          m_ambientTemperature(ambientTemperature) {
    }
    
    /**
     * @brief Gets the film coefficient.
     * @return The film coefficient.
     */
    double getFilmCoefficient() const {
        return m_filmCoefficient;
    }
    
    /**
     * @brief Sets the film coefficient.
     * @param filmCoefficient The film coefficient.
     */
    void setFilmCoefficient(double filmCoefficient) {
        m_filmCoefficient = filmCoefficient;
    }
    
    /**
     * @brief Gets the ambient temperature.
     * @return The ambient temperature.
     */
    double getAmbientTemperature() const {
        return m_ambientTemperature;
    }
    
    /**
     * @brief Sets the ambient temperature.
     * @param ambientTemperature The ambient temperature.
     */
    void setAmbientTemperature(double ambientTemperature) {
        m_ambientTemperature = ambientTemperature;
    }
    
/**
 * @brief Checks if the boundary condition is valid for a given mesh.
 * @param mesh The mesh to check against.
 * @return True if the boundary condition is valid, false otherwise.
 */
bool isValid(const Mesh& mesh) const override;

/**
 * @brief Applies the boundary condition to a mesh.
 * @param mesh The mesh to apply to.
 * @return True if application was successful, false otherwise.
 */
bool apply(Mesh& mesh) const override;

/**
 * @brief Gets a string representation of the boundary condition.
 * @return String representation of the boundary condition.
 */
std::string toString() const override;
    
private:
    double m_filmCoefficient;     ///< Film coefficient (heat transfer coefficient)
    double m_ambientTemperature;  ///< Ambient temperature
};

} // namespace rebel::simulation
