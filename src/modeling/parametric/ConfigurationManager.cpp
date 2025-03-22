/**
 * @file ConfigurationManager.cpp
 * @brief Implementation of the ConfigurationManager class
 */

#include "ConfigurationManager.h"
#include "ParameterManager.h"
#include "DependencyGraph.h"
#include <sstream>
#include <algorithm>

namespace rebel_cad {
namespace modeling {

ConfigurationManager::ConfigurationManager(ParameterManager& paramManager, DependencyGraph& graph)
    : m_paramManager(paramManager), m_graph(graph), m_nextCallbackId(0) {
    // Create a default configuration
    createConfiguration("Default", "Default configuration");
    setActiveConfiguration("Default");
}

bool ConfigurationManager::createConfiguration(const std::string& name, const std::string& description) {
    // Check if a configuration with this name already exists
    if (m_configurations.find(name) != m_configurations.end()) {
        return false;
    }
    
    // Create a new configuration
    auto config = std::make_unique<Configuration>(name, description);
    m_configurations[name] = std::move(config);
    
    // If this is the first configuration, make it active
    if (m_configurations.size() == 1) {
        m_activeConfigurationName = name;
        notifyActiveConfigurationChanged();
    }
    
    return true;
}

Configuration* ConfigurationManager::getConfiguration(const std::string& name) {
    auto it = m_configurations.find(name);
    if (it != m_configurations.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Configuration*> ConfigurationManager::getAllConfigurations() {
    std::vector<Configuration*> configs;
    configs.reserve(m_configurations.size());
    
    for (auto& pair : m_configurations) {
        configs.push_back(pair.second.get());
    }
    
    return configs;
}

bool ConfigurationManager::removeConfiguration(const std::string& name) {
    auto it = m_configurations.find(name);
    if (it == m_configurations.end()) {
        return false;
    }
    
    // Check if this is the active configuration
    bool wasActive = (m_activeConfigurationName == name);
    
    // Remove the configuration
    m_configurations.erase(it);
    
    // If this was the active configuration, set a new active configuration
    if (wasActive) {
        if (!m_configurations.empty()) {
            m_activeConfigurationName = m_configurations.begin()->first;
        } else {
            m_activeConfigurationName = "";
        }
        notifyActiveConfigurationChanged();
    }
    
    return true;
}

bool ConfigurationManager::renameConfiguration(const std::string& oldName, const std::string& newName) {
    // Check if the old name exists and the new name doesn't
    auto oldIt = m_configurations.find(oldName);
    if (oldIt == m_configurations.end() || m_configurations.find(newName) != m_configurations.end()) {
        return false;
    }
    
    // Create a new configuration with the new name
    auto config = std::make_unique<Configuration>(newName, oldIt->second->getDescription());
    
    // Copy all parameter values
    for (const auto& param : oldIt->second->getAllParameterValues()) {
        config->setParameterValue("", param.first, param.second);
    }
    
    // Add the new configuration
    m_configurations[newName] = std::move(config);
    
    // Update the active configuration name if needed
    if (m_activeConfigurationName == oldName) {
        m_activeConfigurationName = newName;
        notifyActiveConfigurationChanged();
    }
    
    // Remove the old configuration
    m_configurations.erase(oldIt);
    
    return true;
}

Configuration* ConfigurationManager::getActiveConfiguration() {
    if (m_activeConfigurationName.empty()) {
        return nullptr;
    }
    
    return getConfiguration(m_activeConfigurationName);
}

bool ConfigurationManager::setActiveConfiguration(const std::string& name) {
    if (m_configurations.find(name) == m_configurations.end()) {
        return false;
    }
    
    if (m_activeConfigurationName != name) {
        m_activeConfigurationName = name;
        notifyActiveConfigurationChanged();
    }
    
    return true;
}

bool ConfigurationManager::applyConfiguration(const std::string& name) {
    Configuration* config = getConfiguration(name);
    if (!config) {
        return false;
    }
    
    // Apply all parameter values from the configuration
    for (const auto& param : config->getAllParameterValues()) {
        // Parse the parameter key to get the feature ID and parameter name
        size_t colonPos = param.first.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }
        
        std::string featureId = param.first.substr(0, colonPos);
        std::string paramName = param.first.substr(colonPos + 1);
        
        // Set the parameter value
        m_paramManager.setParameterValue(featureId, paramName, param.second);
    }
    
    // Update all features that depend on the changed parameters
    for (const auto& featureId : m_graph.getAllFeatures()) {
        m_graph.updateFeature(featureId);
    }
    
    return true;
}

bool ConfigurationManager::updateConfigurationFromModel(const std::string& name, const std::vector<std::string>& parameterNames) {
    Configuration* config = getConfiguration(name);
    if (!config) {
        return false;
    }
    
    // If no specific parameters are specified, update all parameters
    if (parameterNames.empty()) {
        // Get all features
        for (const auto& featureId : m_graph.getAllFeatures()) {
            // Get all parameters for the feature
            auto params = m_paramManager.getParameters(featureId);
            
            // Update the configuration with the current parameter values
            for (auto param : params) {
                config->setParameterValue(featureId, param->getName(), param->getValue());
            }
        }
    } else {
        // Update only the specified parameters
        for (const auto& paramName : parameterNames) {
            // Parse the parameter name to get the feature ID and parameter name
            size_t colonPos = paramName.find(':');
            if (colonPos == std::string::npos) {
                continue;
            }
            
            std::string featureId = paramName.substr(0, colonPos);
            std::string paramName = paramName.substr(colonPos + 1);
            
            // Get the parameter value
            double value = m_paramManager.getParameterValue(featureId, paramName);
            
            // Update the configuration
            config->setParameterValue(featureId, paramName, value);
        }
    }
    
    return true;
}

size_t ConfigurationManager::addActiveConfigurationChangeCallback(std::function<void(const std::string&)> callback) {
    size_t callbackId = m_nextCallbackId++;
    m_changeCallbacks[callbackId] = callback;
    return callbackId;
}

bool ConfigurationManager::removeActiveConfigurationChangeCallback(size_t callbackId) {
    auto it = m_changeCallbacks.find(callbackId);
    if (it != m_changeCallbacks.end()) {
        m_changeCallbacks.erase(it);
        return true;
    }
    return false;
}

std::string ConfigurationManager::serialize() const {
    std::stringstream ss;
    ss << "ConfigurationManager:" << std::endl;
    ss << "ActiveConfiguration: " << m_activeConfigurationName << std::endl;
    ss << "Configurations:" << std::endl;
    
    for (const auto& pair : m_configurations) {
        ss << pair.second->serialize() << std::endl;
    }
    
    return ss.str();
}

bool ConfigurationManager::deserialize(const std::string& serialized) {
    std::stringstream ss(serialized);
    std::string line;
    
    // Read "ConfigurationManager:" line
    if (!std::getline(ss, line) || line != "ConfigurationManager:") {
        return false;
    }
    
    // Read "ActiveConfiguration:" line
    if (!std::getline(ss, line) || line.substr(0, 21) != "ActiveConfiguration: ") {
        return false;
    }
    m_activeConfigurationName = line.substr(21);
    
    // Read "Configurations:" line
    if (!std::getline(ss, line) || line != "Configurations:") {
        return false;
    }
    
    // Clear existing configurations
    m_configurations.clear();
    
    // Read configurations
    std::string configSerialized;
    std::string configLine;
    
    while (std::getline(ss, configLine)) {
        if (configLine == "Configuration:") {
            // Start of a new configuration
            configSerialized = configLine + "\n";
        } else if (configLine.empty() && !configSerialized.empty()) {
            // End of a configuration
            auto config = std::make_unique<Configuration>("Temp");
            if (config->deserialize(configSerialized)) {
                m_configurations[config->getName()] = std::move(config);
            }
            configSerialized.clear();
        } else if (!configSerialized.empty()) {
            // Part of a configuration
            configSerialized += configLine + "\n";
        }
    }
    
    // Process the last configuration if there is one
    if (!configSerialized.empty()) {
        auto config = std::make_unique<Configuration>("Temp");
        if (config->deserialize(configSerialized)) {
            m_configurations[config->getName()] = std::move(config);
        }
    }
    
    // Ensure the active configuration exists
    if (!m_activeConfigurationName.empty() && m_configurations.find(m_activeConfigurationName) == m_configurations.end()) {
        if (!m_configurations.empty()) {
            m_activeConfigurationName = m_configurations.begin()->first;
        } else {
            m_activeConfigurationName = "";
        }
    }
    
    notifyActiveConfigurationChanged();
    
    return true;
}

void ConfigurationManager::notifyActiveConfigurationChanged() {
    for (const auto& callback : m_changeCallbacks) {
        callback.second(m_activeConfigurationName);
    }
}

} // namespace modeling
} // namespace rebel_cad
