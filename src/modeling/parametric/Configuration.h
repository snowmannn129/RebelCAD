/**
 * @file Configuration.h
 * @brief Configuration for parametric models
 * 
 * This file defines the Configuration class, which represents a specific
 * configuration of a parametric model with a set of parameter values.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

namespace rebel_cad {
namespace modeling {

/**
 * @brief Configuration class for parametric models
 * 
 * The Configuration class represents a specific configuration of a parametric
 * model with a set of parameter values. It provides methods for getting and
 * setting parameter values for the configuration.
 */
class Configuration {
public:
    /**
     * @brief Constructor
     * 
     * @param name The name of the configuration
     * @param description The description of the configuration
     */
    Configuration(const std::string& name, const std::string& description = "");

    /**
     * @brief Destructor
     */
    ~Configuration() = default;

    /**
     * @brief Get the name of the configuration
     * 
     * @return The configuration name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Set the name of the configuration
     * 
     * @param name The new name for the configuration
     */
    void setName(const std::string& name) { m_name = name; }

    /**
     * @brief Get the description of the configuration
     * 
     * @return The configuration description
     */
    const std::string& getDescription() const { return m_description; }

    /**
     * @brief Set the description of the configuration
     * 
     * @param description The new description for the configuration
     */
    void setDescription(const std::string& description) { m_description = description; }

    /**
     * @brief Set a parameter value for the configuration
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @param value The value for the parameter in this configuration
     * @return True if the parameter was set successfully, false otherwise
     */
    bool setParameterValue(const std::string& featureId, const std::string& paramName, double value);

    /**
     * @brief Get a parameter value for the configuration
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @param defaultValue The default value to return if the parameter is not found
     * @return The parameter value, or defaultValue if not found
     */
    double getParameterValue(const std::string& featureId, const std::string& paramName, double defaultValue = 0.0) const;

    /**
     * @brief Check if the configuration has a value for a parameter
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @return True if the configuration has a value for the parameter, false otherwise
     */
    bool hasParameterValue(const std::string& featureId, const std::string& paramName) const;

    /**
     * @brief Remove a parameter value from the configuration
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @return True if the parameter was removed, false if it wasn't found
     */
    bool removeParameterValue(const std::string& featureId, const std::string& paramName);

    /**
     * @brief Get all parameter values for the configuration
     * 
     * @return A map of parameter keys to values
     */
    const std::unordered_map<std::string, double>& getAllParameterValues() const { return m_parameterValues; }

    /**
     * @brief Clear all parameter values for the configuration
     */
    void clearParameterValues() { m_parameterValues.clear(); }

    /**
     * @brief Serialize the configuration to a string
     * 
     * @return A string representation of the configuration
     */
    std::string serialize() const;

    /**
     * @brief Deserialize the configuration from a string
     * 
     * @param serialized The serialized string representation of the configuration
     * @return True if deserialization was successful, false otherwise
     */
    bool deserialize(const std::string& serialized);

private:
    /**
     * @brief Get the full parameter key
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @return The full parameter key
     */
    std::string getParameterKey(const std::string& featureId, const std::string& paramName) const;

    std::string m_name;                                  ///< Configuration name
    std::string m_description;                           ///< Configuration description
    std::unordered_map<std::string, double> m_parameterValues; ///< Map of parameter keys to values
};

} // namespace modeling
} // namespace rebel_cad
