/**
 * @file ConfigurationManagerDialog.cpp
 * @brief Implementation of the ConfigurationManagerDialog class
 */

#include "ConfigurationManagerDialog.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>
#include <algorithm>
#include <sstream>

namespace rebel_cad {
namespace ui {

ConfigurationManagerDialog::ConfigurationManagerDialog(modeling::ConfigurationManager& configManager, modeling::DesignTable& designTable)
    : m_configManager(configManager), m_designTable(designTable), m_currentTab(0),
      m_showCreateConfigDialog(false), m_showRenameConfigDialog(false), m_showAddParameterDialog(false) {
    // Initialize the design table from the configurations
    m_designTable.updateFromConfigurations();
}

bool ConfigurationManagerDialog::show(void* parentWindowHandle) {
    bool open = true;
    bool result = false;

    // Set up the dialog window
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Configuration Manager", &open, ImGuiWindowFlags_NoCollapse)) {
        // Create tabs for configurations and design table
        if (ImGui::BeginTabBar("ConfigurationManagerTabs")) {
            if (ImGui::BeginTabItem("Configurations")) {
                m_currentTab = 0;
                renderConfigurationsTab();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Design Table")) {
                m_currentTab = 1;
                renderDesignTableTab();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        // Add OK and Cancel buttons at the bottom
        ImGui::Separator();
        ImGui::BeginGroup();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 200);
        if (ImGui::Button("OK", ImVec2(80, 0))) {
            result = true;
            open = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            result = false;
            open = false;
        }
        ImGui::EndGroup();
    }
    ImGui::End();

    // Show create configuration dialog
    if (m_showCreateConfigDialog) {
        ImGui::OpenPopup("Create Configuration");
        m_showCreateConfigDialog = false;
    }

    if (ImGui::BeginPopupModal("Create Configuration", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter the name and description for the new configuration:");
        ImGui::Separator();

        ImGui::InputText("Name", &m_newConfigName);
        ImGui::InputText("Description", &m_newConfigDescription);

        if (ImGui::Button("Create", ImVec2(120, 0))) {
            if (!m_newConfigName.empty()) {
                createConfiguration();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Show rename configuration dialog
    if (m_showRenameConfigDialog) {
        ImGui::OpenPopup("Rename Configuration");
        m_showRenameConfigDialog = false;
    }

    if (ImGui::BeginPopupModal("Rename Configuration", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter the new name for the configuration:");
        ImGui::Separator();

        ImGui::InputText("New Name", &m_newConfigName);

        if (ImGui::Button("Rename", ImVec2(120, 0))) {
            if (!m_newConfigName.empty()) {
                renameConfiguration(m_selectedConfigName, m_newConfigName);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Show add parameter dialog
    if (m_showAddParameterDialog) {
        ImGui::OpenPopup("Add Parameter");
        m_showAddParameterDialog = false;
    }

    if (ImGui::BeginPopupModal("Add Parameter", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter the feature ID and parameter name:");
        ImGui::Separator();

        ImGui::InputText("Feature ID", &m_newFeatureId);
        ImGui::InputText("Parameter Name", &m_newParamName);

        if (ImGui::Button("Add", ImVec2(120, 0))) {
            if (!m_newFeatureId.empty() && !m_newParamName.empty()) {
                addParameter();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return result;
}

void ConfigurationManagerDialog::setConfigurationActivatedCallback(std::function<void(const std::string&)> callback) {
    m_configActivatedCallback = callback;
}

void ConfigurationManagerDialog::setConfigurationModifiedCallback(std::function<void(const std::string&)> callback) {
    m_configModifiedCallback = callback;
}

void ConfigurationManagerDialog::setConfigurationCreatedCallback(std::function<void(const std::string&)> callback) {
    m_configCreatedCallback = callback;
}

void ConfigurationManagerDialog::setConfigurationDeletedCallback(std::function<void(const std::string&)> callback) {
    m_configDeletedCallback = callback;
}

void ConfigurationManagerDialog::setDesignTableImportedCallback(std::function<void()> callback) {
    m_designTableImportedCallback = callback;
}

void ConfigurationManagerDialog::setDesignTableExportedCallback(std::function<void()> callback) {
    m_designTableExportedCallback = callback;
}

void ConfigurationManagerDialog::renderConfigurationsTab() {
    // Split the tab into two columns
    ImGui::Columns(2, "ConfigurationsColumns", true);

    // Left column: Configuration list
    renderConfigurationList();
    ImGui::NextColumn();

    // Right column: Configuration details
    if (!m_selectedConfigName.empty()) {
        renderConfigurationDetails(m_selectedConfigName);
    } else {
        ImGui::TextDisabled("Select a configuration to view details");
    }
    ImGui::Columns(1);
}

void ConfigurationManagerDialog::renderDesignTableTab() {
    // Add toolbar at the top
    renderDesignTableToolbar();
    ImGui::Separator();

    // Render the design table grid
    renderDesignTableGrid();
}

void ConfigurationManagerDialog::renderConfigurationList() {
    // Add buttons for creating and deleting configurations
    if (ImGui::Button("Create Configuration")) {
        m_newConfigName.clear();
        m_newConfigDescription.clear();
        m_showCreateConfigDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Configuration") && !m_selectedConfigName.empty()) {
        deleteConfiguration(m_selectedConfigName);
    }
    ImGui::SameLine();
    if (ImGui::Button("Rename Configuration") && !m_selectedConfigName.empty()) {
        m_newConfigName = m_selectedConfigName;
        m_showRenameConfigDialog = true;
    }

    // Add a list of configurations
    ImGui::BeginChild("ConfigurationList", ImVec2(0, 0), true);
    auto configs = m_configManager.getAllConfigurations();
    for (auto config : configs) {
        bool isActive = (config->getName() == m_configManager.getActiveConfiguration()->getName());
        bool isSelected = (config->getName() == m_selectedConfigName);

        // Create a selectable item for each configuration
        if (ImGui::Selectable(config->getName().c_str(), isSelected)) {
            m_selectedConfigName = config->getName();
        }

        // Add a context menu for each configuration
        if (ImGui::BeginPopupContextItem(config->getName().c_str())) {
            if (ImGui::MenuItem("Activate")) {
                activateConfiguration(config->getName());
            }
            if (ImGui::MenuItem("Rename")) {
                m_newConfigName = config->getName();
                m_showRenameConfigDialog = true;
            }
            if (ImGui::MenuItem("Delete")) {
                deleteConfiguration(config->getName());
            }
            ImGui::EndPopup();
        }

        // Add an indicator for the active configuration
        if (isActive) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "(Active)");
        }
    }
    ImGui::EndChild();
}

void ConfigurationManagerDialog::renderConfigurationDetails(const std::string& configName) {
    // Get the configuration
    modeling::Configuration* config = m_configManager.getConfiguration(configName);
    if (!config) {
        return;
    }

    // Add a header with the configuration name
    ImGui::Text("Configuration: %s", configName.c_str());
    ImGui::Separator();

    // Add a button to activate the configuration
    if (configName != m_configManager.getActiveConfiguration()->getName()) {
        if (ImGui::Button("Activate Configuration")) {
            activateConfiguration(configName);
        }
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active Configuration");
    }

    // Add a text input for the description
    std::string description = config->getDescription();
    if (ImGui::InputText("Description", &description, ImGuiInputTextFlags_EnterReturnsTrue)) {
        config->setDescription(description);
        if (m_configModifiedCallback) {
            m_configModifiedCallback(configName);
        }
    }

    // Add a table of parameter values
    ImGui::Text("Parameters:");
    ImGui::BeginChild("ParameterTable", ImVec2(0, 0), true);
    ImGui::Columns(3, "ParameterColumns", true);
    ImGui::Text("Feature"); ImGui::NextColumn();
    ImGui::Text("Parameter"); ImGui::NextColumn();
    ImGui::Text("Value"); ImGui::NextColumn();
    ImGui::Separator();

    // Get all parameter values
    auto paramValues = config->getAllParameterValues();
    for (const auto& param : paramValues) {
        // Parse the parameter key to get the feature ID and parameter name
        std::string key = param.first;
        size_t colonPos = key.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string featureId = key.substr(0, colonPos);
        std::string paramName = key.substr(colonPos + 1);
        double value = param.second;

        // Add a row for each parameter
        ImGui::Text("%s", featureId.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", paramName.c_str()); ImGui::NextColumn();

        // Add an input field for the value
        std::string valueStr = std::to_string(value);
        if (ImGui::InputText(("##" + key).c_str(), &valueStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
            try {
                double newValue = std::stod(valueStr);
                config->setParameterValue(featureId, paramName, newValue);
                if (m_configModifiedCallback) {
                    m_configModifiedCallback(configName);
                }
            } catch (const std::exception&) {
                // Ignore invalid values
            }
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::EndChild();
}

void ConfigurationManagerDialog::renderDesignTableGrid() {
    // Get the parameters and configurations
    auto params = m_designTable.getParameters();
    auto configs = m_designTable.getConfigurations();

    // Calculate the column widths
    float paramWidth = 200.0f;
    float configWidth = 100.0f;
    float tableWidth = paramWidth + configWidth * configs.size();

    // Create a child window for the table
    ImGui::BeginChild("DesignTableGrid", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    // Create a table with a column for parameters and a column for each configuration
    ImGui::Columns(1 + configs.size(), "DesignTableColumns", true);
    ImGui::SetColumnWidth(0, paramWidth);
    ImGui::Text("Parameter");
    ImGui::NextColumn();

    // Add a header for each configuration
    for (size_t i = 0; i < configs.size(); i++) {
        ImGui::SetColumnWidth(i + 1, configWidth);
        ImGui::Text("%s", configs[i].c_str());
        ImGui::NextColumn();
    }
    ImGui::Separator();

    // Add a row for each parameter
    for (const auto& param : params) {
        // Parse the parameter key to get the feature ID and parameter name
        std::string key = param;
        size_t colonPos = key.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string featureId = key.substr(0, colonPos);
        std::string paramName = key.substr(colonPos + 1);

        // Add the parameter name
        ImGui::Text("%s:%s", featureId.c_str(), paramName.c_str());

        // Add a context menu for the parameter
        if (ImGui::BeginPopupContextItem(key.c_str())) {
            if (ImGui::MenuItem("Remove Parameter")) {
                removeParameter(featureId, paramName);
            }
            ImGui::EndPopup();
        }
        ImGui::NextColumn();

        // Add a cell for each configuration
        for (const auto& config : configs) {
            double value = m_designTable.getParameterValue(config, featureId, paramName);
            std::string valueStr = std::to_string(value);
            std::string id = "##" + config + ":" + key;

            // Add an input field for the value
            if (ImGui::InputText(id.c_str(), &valueStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
                try {
                    double newValue = std::stod(valueStr);
                    m_designTable.setParameterValue(config, featureId, paramName, newValue);
                } catch (const std::exception&) {
                    // Ignore invalid values
                }
            }
            ImGui::NextColumn();
        }
    }
    ImGui::Columns(1);
    ImGui::EndChild();
}

void ConfigurationManagerDialog::renderDesignTableToolbar() {
    // Add buttons for importing and exporting design tables
    if (ImGui::Button("Import CSV")) {
        importDesignTable();
    }
    ImGui::SameLine();
    if (ImGui::Button("Export CSV")) {
        exportDesignTable();
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Parameter")) {
        m_newFeatureId.clear();
        m_newParamName.clear();
        m_showAddParameterDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Update from Configurations")) {
        updateDesignTableFromConfigurations();
    }
    ImGui::SameLine();
    if (ImGui::Button("Update Configurations")) {
        updateConfigurationsFromDesignTable();
    }
}

void ConfigurationManagerDialog::createConfiguration() {
    if (m_configManager.createConfiguration(m_newConfigName, m_newConfigDescription)) {
        m_selectedConfigName = m_newConfigName;
        if (m_configCreatedCallback) {
            m_configCreatedCallback(m_newConfigName);
        }
    }
}

void ConfigurationManagerDialog::deleteConfiguration(const std::string& configName) {
    if (m_configManager.removeConfiguration(configName)) {
        if (m_selectedConfigName == configName) {
            m_selectedConfigName.clear();
        }
        if (m_configDeletedCallback) {
            m_configDeletedCallback(configName);
        }
    }
}

void ConfigurationManagerDialog::renameConfiguration(const std::string& oldName, const std::string& newName) {
    if (m_configManager.renameConfiguration(oldName, newName)) {
        if (m_selectedConfigName == oldName) {
            m_selectedConfigName = newName;
        }
        if (m_configModifiedCallback) {
            m_configModifiedCallback(newName);
        }
    }
}

void ConfigurationManagerDialog::activateConfiguration(const std::string& configName) {
    if (m_configManager.setActiveConfiguration(configName)) {
        if (m_configActivatedCallback) {
            m_configActivatedCallback(configName);
        }
    }
}

void ConfigurationManagerDialog::importDesignTable() {
    // Open a file dialog to select a CSV file
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_OpenDialog("csv", nullptr, &outPath);
    if (result == NFD_OKAY) {
        m_importExportPath = outPath;
        free(outPath);

        // Import the design table
        if (m_designTable.importFromCsv(m_importExportPath)) {
            if (m_designTableImportedCallback) {
                m_designTableImportedCallback();
            }
        }
    }
}

void ConfigurationManagerDialog::exportDesignTable() {
    // Open a file dialog to select a CSV file
    nfdchar_t* outPath = nullptr;
    nfdresult_t result = NFD_SaveDialog("csv", nullptr, &outPath);
    if (result == NFD_OKAY) {
        m_importExportPath = outPath;
        free(outPath);

        // Export the design table
        if (m_designTable.exportToCsv(m_importExportPath)) {
            if (m_designTableExportedCallback) {
                m_designTableExportedCallback();
            }
        }
    }
}

void ConfigurationManagerDialog::addParameter() {
    if (m_designTable.addParameter(m_newFeatureId, m_newParamName)) {
        // Parameter added successfully
    }
}

void ConfigurationManagerDialog::removeParameter(const std::string& featureId, const std::string& paramName) {
    if (m_designTable.removeParameter(featureId, paramName)) {
        // Parameter removed successfully
    }
}

void ConfigurationManagerDialog::updateDesignTableFromConfigurations() {
    if (m_designTable.updateFromConfigurations()) {
        // Design table updated successfully
    }
}

void ConfigurationManagerDialog::updateConfigurationsFromDesignTable() {
    if (m_designTable.updateConfigurations()) {
        // Configurations updated successfully
    }
}

} // namespace ui
} // namespace rebel_cad
