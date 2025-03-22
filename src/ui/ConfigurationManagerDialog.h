/**
 * @file ConfigurationManagerDialog.h
 * @brief Dialog for managing configurations and design tables
 * 
 * This file defines the ConfigurationManagerDialog class, which provides a user
 * interface for managing configurations and design tables in the parametric
 * modeling system.
 */

#pragma once

#include "../modeling/parametric/ConfigurationManager.h"
#include "../modeling/parametric/DesignTable.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace rebel_cad {
namespace ui {

/**
 * @brief Dialog for managing configurations and design tables
 * 
 * The ConfigurationManagerDialog class provides a user interface for managing
 * configurations and design tables in the parametric modeling system. It allows
 * users to create, edit, and delete configurations, as well as import and export
 * design tables.
 */
class ConfigurationManagerDialog {
public:
    /**
     * @brief Constructor
     * 
     * @param configManager Reference to the configuration manager
     * @param designTable Reference to the design table
     */
    ConfigurationManagerDialog(modeling::ConfigurationManager& configManager, modeling::DesignTable& designTable);

    /**
     * @brief Destructor
     */
    ~ConfigurationManagerDialog() = default;

    /**
     * @brief Show the dialog
     * 
     * @param parentWindowHandle Handle to the parent window
     * @return True if the dialog was closed with OK, false if it was cancelled
     */
    bool show(void* parentWindowHandle = nullptr);

    /**
     * @brief Set the callback function to be called when a configuration is activated
     * 
     * @param callback The callback function
     */
    void setConfigurationActivatedCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Set the callback function to be called when a configuration is modified
     * 
     * @param callback The callback function
     */
    void setConfigurationModifiedCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Set the callback function to be called when a configuration is created
     * 
     * @param callback The callback function
     */
    void setConfigurationCreatedCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Set the callback function to be called when a configuration is deleted
     * 
     * @param callback The callback function
     */
    void setConfigurationDeletedCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Set the callback function to be called when a design table is imported
     * 
     * @param callback The callback function
     */
    void setDesignTableImportedCallback(std::function<void()> callback);

    /**
     * @brief Set the callback function to be called when a design table is exported
     * 
     * @param callback The callback function
     */
    void setDesignTableExportedCallback(std::function<void()> callback);

private:
    /**
     * @brief Render the configurations tab
     */
    void renderConfigurationsTab();

    /**
     * @brief Render the design table tab
     */
    void renderDesignTableTab();

    /**
     * @brief Render the configuration list
     */
    void renderConfigurationList();

    /**
     * @brief Render the configuration details
     * 
     * @param configName The name of the configuration to show details for
     */
    void renderConfigurationDetails(const std::string& configName);

    /**
     * @brief Render the design table grid
     */
    void renderDesignTableGrid();

    /**
     * @brief Render the design table toolbar
     */
    void renderDesignTableToolbar();

    /**
     * @brief Create a new configuration
     */
    void createConfiguration();

    /**
     * @brief Delete a configuration
     * 
     * @param configName The name of the configuration to delete
     */
    void deleteConfiguration(const std::string& configName);

    /**
     * @brief Rename a configuration
     * 
     * @param oldName The current name of the configuration
     * @param newName The new name for the configuration
     */
    void renameConfiguration(const std::string& oldName, const std::string& newName);

    /**
     * @brief Activate a configuration
     * 
     * @param configName The name of the configuration to activate
     */
    void activateConfiguration(const std::string& configName);

    /**
     * @brief Import a design table from a CSV file
     */
    void importDesignTable();

    /**
     * @brief Export the design table to a CSV file
     */
    void exportDesignTable();

    /**
     * @brief Add a parameter to the design table
     */
    void addParameter();

    /**
     * @brief Remove a parameter from the design table
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param paramName The name of the parameter
     */
    void removeParameter(const std::string& featureId, const std::string& paramName);

    /**
     * @brief Update the design table from the configurations
     */
    void updateDesignTableFromConfigurations();

    /**
     * @brief Update the configurations from the design table
     */
    void updateConfigurationsFromDesignTable();

    modeling::ConfigurationManager& m_configManager; ///< Reference to the configuration manager
    modeling::DesignTable& m_designTable;            ///< Reference to the design table
    std::string m_selectedConfigName;                ///< Name of the selected configuration
    std::string m_newConfigName;                     ///< Name for a new configuration
    std::string m_newConfigDescription;              ///< Description for a new configuration
    std::string m_importExportPath;                  ///< Path for importing/exporting design tables
    std::string m_newFeatureId;                      ///< Feature ID for a new parameter
    std::string m_newParamName;                      ///< Parameter name for a new parameter
    int m_currentTab;                                ///< Index of the current tab
    bool m_showCreateConfigDialog;                   ///< Whether to show the create configuration dialog
    bool m_showRenameConfigDialog;                   ///< Whether to show the rename configuration dialog
    bool m_showAddParameterDialog;                   ///< Whether to show the add parameter dialog
    std::function<void(const std::string&)> m_configActivatedCallback; ///< Callback for configuration activation
    std::function<void(const std::string&)> m_configModifiedCallback;  ///< Callback for configuration modification
    std::function<void(const std::string&)> m_configCreatedCallback;   ///< Callback for configuration creation
    std::function<void(const std::string&)> m_configDeletedCallback;   ///< Callback for configuration deletion
    std::function<void()> m_designTableImportedCallback;               ///< Callback for design table import
    std::function<void()> m_designTableExportedCallback;               ///< Callback for design table export
};

} // namespace ui
} // namespace rebel_cad
