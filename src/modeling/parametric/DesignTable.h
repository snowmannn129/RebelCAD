/**
 * @file DesignTable.h
 * @brief Design table for parametric models
 * 
 * This file defines the DesignTable class, which represents a table of
 * parameter values for different configurations of a parametric model.
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ConfigurationManager;
class ParameterManager;

/**
 * @brief Design table class for parametric models
 * 
 * The DesignTable class represents a table of parameter values for different
 * configurations of a parametric model. It provides methods for creating,
 * retrieving, and updating configurations based on the table.
 */
class DesignTable {
public:
    /**
     * @brief Constructor
     * 
     * @param configManager Reference to the configuration manager
     * @param paramManager Reference to the parameter manager
     */
    DesignTable(ConfigurationManager& configManager, ParameterManager& paramManager);

    /**
     * @brief Destructor
     */
    ~DesignTable() = default;

    /**
     * @brief Add a parameter to the design table
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @return True if the parameter was added successfully, false otherwise
     */
    bool addParameter(const std::string& featureId, const std::string& paramName);

    /**
     * @brief Remove a parameter from the design table
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @return True if the parameter was removed, false otherwise
     */
    bool removeParameter(const std::string& featureId, const std::string& paramName);

    /**
     * @brief Get all parameters in the design table
     * 
     * @return A vector of parameter keys (featureId:paramName)
     */
    std::vector<std::string> getParameters() const;

    /**
     * @brief Add a configuration to the design table
     * 
     * @param configName The name of the configuration
     * @return True if the configuration was added successfully, false otherwise
     */
    bool addConfiguration(const std::string& configName);

    /**
     * @brief Remove a configuration from the design table
     * 
     * @param configName The name of the configuration
     * @return True if the configuration was removed, false otherwise
     */
    bool removeConfiguration(const std::string& configName);

    /**
     * @brief Get all configurations in the design table
     * 
     * @return A vector of configuration names
     */
    std::vector<std::string> getConfigurations() const;

    /**
     * @brief Set a parameter value for a configuration
     * 
     * @param configName The name of the configuration
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @param value The value for the parameter
     * @return True if the parameter value was set successfully, false otherwise
     */
    bool setParameterValue(const std::string& configName, const std::string& featureId,
                          const std::string& paramName, double value);

    /**
     * @brief Get a parameter value for a configuration
     * 
     * @param configName The name of the configuration
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     * @param defaultValue The default value to return if the parameter is not found
     * @return The parameter value, or defaultValue if not found
     */
    double getParameterValue(const std::string& configName, const std::string& featureId,
                            const std::string& paramName, double defaultValue = 0.0) const;

    /**
     * @brief Update the design table from the configuration manager
     * 
     * This method updates the design table to match the current state of the
     * configurations in the configuration manager.
     * 
     * @return True if the update was successful, false otherwise
     */
    bool updateFromConfigurations();

    /**
     * @brief Update configurations from the design table
     * 
     * This method updates the configurations in the configuration manager to
     * match the current state of the design table.
     * 
     * @return True if the update was successful, false otherwise
     */
    bool updateConfigurations();

    /**
     * @brief Import a design table from a CSV file
     * 
     * @param filePath The path to the CSV file
     * @return True if the import was successful, false otherwise
     */
    bool importFromCsv(const std::string& filePath);

    /**
     * @brief Export the design table to a CSV file
     * 
     * @param filePath The path to the CSV file
     * @return True if the export was successful, false otherwise
     */
    bool exportToCsv(const std::string& filePath) const;

    /**
     * @brief Serialize the design table to a string
     * 
     * @return A string representation of the design table
     */
    std::string serialize() const;

    /**
     * @brief Deserialize the design table from a string
     * 
     * @param serialized The serialized string representation of the design table
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

    /**
     * @brief Parse a parameter key
     * 
     * @param key The parameter key
     * @param featureId Output parameter for the feature ID
     * @param paramName Output parameter for the parameter name
     * @return True if the key was parsed successfully, false otherwise
     */
    bool parseParameterKey(const std::string& key, std::string& featureId, std::string& paramName) const;

    ConfigurationManager& m_configManager; ///< Reference to the configuration manager
    ParameterManager& m_paramManager;      ///< Reference to the parameter manager
    std::vector<std::string> m_parameters; ///< Vector of parameter keys (featureId:paramName)
    std::vector<std::string> m_configurations; ///< Vector of configuration names
    std::unordered_map<std::string, std::unordered_map<std::string, double>> m_values; ///< Map of configuration names to parameter values
};

} // namespace modeling
} // namespace rebel_cad
