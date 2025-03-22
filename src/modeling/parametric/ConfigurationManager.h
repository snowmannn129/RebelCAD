/**
 * @file ConfigurationManager.h
 * @brief Management of configurations for parametric models
 * 
 * This file defines the ConfigurationManager class, which is responsible for
 * managing multiple configurations of a parametric model. It provides methods
 * for creating, retrieving, and updating configurations.
 */

#pragma once

#include "Configuration.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ParameterManager;
class DependencyGraph;

/**
 * @brief Manager class for configurations
 * 
 * The ConfigurationManager class is responsible for managing multiple configurations
 * of a parametric model. It provides methods for creating, retrieving, and updating
 * configurations.
 */
class ConfigurationManager {
public:
    /**
     * @brief Constructor
     * 
     * @param paramManager Reference to the parameter manager
     * @param graph Reference to the dependency graph
     */
    ConfigurationManager(ParameterManager& paramManager, DependencyGraph& graph);

    /**
     * @brief Destructor
     */
    ~ConfigurationManager() = default;

    /**
     * @brief Create a new configuration
     * 
     * @param name The name of the configuration
     * @param description The description of the configuration
     * @return True if the configuration was created successfully, false otherwise
     */
    bool createConfiguration(const std::string& name, const std::string& description = "");

    /**
     * @brief Get a configuration by name
     * 
     * @param name The name of the configuration
     * @return A pointer to the configuration, or nullptr if not found
     */
    Configuration* getConfiguration(const std::string& name);

    /**
     * @brief Get all configurations
     * 
     * @return A vector of pointers to all configurations
     */
    std::vector<Configuration*> getAllConfigurations();

    /**
     * @brief Remove a configuration
     * 
     * @param name The name of the configuration to remove
     * @return True if the configuration was removed, false if it wasn't found
     */
    bool removeConfiguration(const std::string& name);

    /**
     * @brief Rename a configuration
     * 
     * @param oldName The current name of the configuration
     * @param newName The new name for the configuration
     * @return True if the configuration was renamed, false if it wasn't found or the new name is already in use
     */
    bool renameConfiguration(const std::string& oldName, const std::string& newName);

    /**
     * @brief Get the active configuration
     * 
     * @return A pointer to the active configuration, or nullptr if none is active
     */
    Configuration* getActiveConfiguration();

    /**
     * @brief Set the active configuration
     * 
     * @param name The name of the configuration to activate
     * @return True if the configuration was activated, false if it wasn't found
     */
    bool setActiveConfiguration(const std::string& name);

    /**
     * @brief Apply a configuration to the model
     * 
     * This method applies the parameter values from a configuration to the model.
     * 
     * @param name The name of the configuration to apply
     * @return True if the configuration was applied successfully, false otherwise
     */
    bool applyConfiguration(const std::string& name);

    /**
     * @brief Update a configuration from the current model state
     * 
     * This method updates the parameter values in a configuration to match the current model state.
     * 
     * @param name The name of the configuration to update
     * @param parameterNames A vector of parameter names to update (empty for all parameters)
     * @return True if the configuration was updated successfully, false otherwise
     */
    bool updateConfigurationFromModel(const std::string& name, const std::vector<std::string>& parameterNames = {});

    /**
     * @brief Add a callback function to be called when the active configuration changes
     * 
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addActiveConfigurationChangeCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Remove a callback function
     * 
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeActiveConfigurationChangeCallback(size_t callbackId);

    /**
     * @brief Serialize all configurations to a string
     * 
     * @return A string representation of all configurations
     */
    std::string serialize() const;

    /**
     * @brief Deserialize configurations from a string
     * 
     * @param serialized The serialized string representation of configurations
     * @return True if deserialization was successful, false otherwise
     */
    bool deserialize(const std::string& serialized);

private:
    /**
     * @brief Notify callbacks that the active configuration has changed
     */
    void notifyActiveConfigurationChanged();

    ParameterManager& m_paramManager;                                ///< Reference to the parameter manager
    DependencyGraph& m_graph;                                        ///< Reference to the dependency graph
    std::unordered_map<std::string, std::unique_ptr<Configuration>> m_configurations; ///< Map of configuration names to configurations
    std::string m_activeConfigurationName;                           ///< Name of the active configuration
    std::unordered_map<size_t, std::function<void(const std::string&)>> m_changeCallbacks; ///< Callbacks for active configuration changes
    size_t m_nextCallbackId = 0;                                     ///< Next callback ID
};

} // namespace modeling
} // namespace rebel_cad
