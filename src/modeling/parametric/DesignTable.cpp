/**
 * @file DesignTable.cpp
 * @brief Implementation of the DesignTable class
 */

#include "DesignTable.h"
#include "ConfigurationManager.h"
#include "Configuration.h"
#include "ParameterManager.h"
#include <sstream>
#include <fstream>
#include <algorithm>

namespace rebel_cad {
namespace modeling {

DesignTable::DesignTable(ConfigurationManager& configManager, ParameterManager& paramManager)
    : m_configManager(configManager), m_paramManager(paramManager) {
}

bool DesignTable::addParameter(const std::string& featureId, const std::string& paramName) {
    std::string key = getParameterKey(featureId, paramName);
    
    // Check if the parameter already exists in the table
    if (std::find(m_parameters.begin(), m_parameters.end(), key) != m_parameters.end()) {
        return false;
    }
    
    // Add the parameter to the table
    m_parameters.push_back(key);
    
    // Initialize the parameter value for all configurations
    for (const auto& configName : m_configurations) {
        // Get the current value from the configuration
        Configuration* config = m_configManager.getConfiguration(configName);
        if (config && config->hasParameterValue(featureId, paramName)) {
            m_values[configName][key] = config->getParameterValue(featureId, paramName);
        } else {
            // Use the default value from the parameter manager
            m_values[configName][key] = m_paramManager.getParameterValue(featureId, paramName);
        }
    }
    
    return true;
}

bool DesignTable::removeParameter(const std::string& featureId, const std::string& paramName) {
    std::string key = getParameterKey(featureId, paramName);
    
    // Find the parameter in the table
    auto it = std::find(m_parameters.begin(), m_parameters.end(), key);
    if (it == m_parameters.end()) {
        return false;
    }
    
    // Remove the parameter from the table
    m_parameters.erase(it);
    
    // Remove the parameter value from all configurations
    for (auto& configValues : m_values) {
        configValues.second.erase(key);
    }
    
    return true;
}

std::vector<std::string> DesignTable::getParameters() const {
    return m_parameters;
}

bool DesignTable::addConfiguration(const std::string& configName) {
    // Check if the configuration already exists in the table
    if (std::find(m_configurations.begin(), m_configurations.end(), configName) != m_configurations.end()) {
        return false;
    }
    
    // Add the configuration to the table
    m_configurations.push_back(configName);
    
    // Initialize the configuration values
    Configuration* config = m_configManager.getConfiguration(configName);
    if (config) {
        // Copy values from the existing configuration
        for (const auto& key : m_parameters) {
            std::string featureId, paramName;
            if (parseParameterKey(key, featureId, paramName)) {
                if (config->hasParameterValue(featureId, paramName)) {
                    m_values[configName][key] = config->getParameterValue(featureId, paramName);
                } else {
                    // Use the default value from the parameter manager
                    m_values[configName][key] = m_paramManager.getParameterValue(featureId, paramName);
                }
            }
        }
    } else {
        // Create a new configuration
        m_configManager.createConfiguration(configName);
        
        // Initialize with default values
        for (const auto& key : m_parameters) {
            std::string featureId, paramName;
            if (parseParameterKey(key, featureId, paramName)) {
                m_values[configName][key] = m_paramManager.getParameterValue(featureId, paramName);
            }
        }
    }
    
    return true;
}

bool DesignTable::removeConfiguration(const std::string& configName) {
    // Find the configuration in the table
    auto it = std::find(m_configurations.begin(), m_configurations.end(), configName);
    if (it == m_configurations.end()) {
        return false;
    }
    
    // Remove the configuration from the table
    m_configurations.erase(it);
    
    // Remove the configuration values
    m_values.erase(configName);
    
    return true;
}

std::vector<std::string> DesignTable::getConfigurations() const {
    return m_configurations;
}

bool DesignTable::setParameterValue(const std::string& configName, const std::string& featureId,
                                   const std::string& paramName, double value) {
    // Check if the configuration exists in the table
    if (std::find(m_configurations.begin(), m_configurations.end(), configName) == m_configurations.end()) {
        return false;
    }
    
    std::string key = getParameterKey(featureId, paramName);
    
    // Check if the parameter exists in the table
    if (std::find(m_parameters.begin(), m_parameters.end(), key) == m_parameters.end()) {
        // Add the parameter to the table
        if (!addParameter(featureId, paramName)) {
            return false;
        }
    }
    
    // Set the parameter value
    m_values[configName][key] = value;
    
    return true;
}

double DesignTable::getParameterValue(const std::string& configName, const std::string& featureId,
                                     const std::string& paramName, double defaultValue) const {
    // Check if the configuration exists in the table
    if (std::find(m_configurations.begin(), m_configurations.end(), configName) == m_configurations.end()) {
        return defaultValue;
    }
    
    std::string key = getParameterKey(featureId, paramName);
    
    // Check if the parameter exists in the table
    if (std::find(m_parameters.begin(), m_parameters.end(), key) == m_parameters.end()) {
        return defaultValue;
    }
    
    // Get the parameter value
    auto configIt = m_values.find(configName);
    if (configIt != m_values.end()) {
        auto paramIt = configIt->second.find(key);
        if (paramIt != configIt->second.end()) {
            return paramIt->second;
        }
    }
    
    return defaultValue;
}

bool DesignTable::updateFromConfigurations() {
    // Clear the current table
    m_parameters.clear();
    m_configurations.clear();
    m_values.clear();
    
    // Get all configurations from the configuration manager
    auto configs = m_configManager.getAllConfigurations();
    
    // Add all configurations to the table
    for (auto config : configs) {
        m_configurations.push_back(config->getName());
        
        // Add all parameters from the configuration
        for (const auto& param : config->getAllParameterValues()) {
            std::string featureId, paramName;
            if (parseParameterKey(param.first, featureId, paramName)) {
                std::string key = getParameterKey(featureId, paramName);
                
                // Add the parameter to the table if it doesn't exist
                if (std::find(m_parameters.begin(), m_parameters.end(), key) == m_parameters.end()) {
                    m_parameters.push_back(key);
                }
                
                // Set the parameter value
                m_values[config->getName()][key] = param.second;
            }
        }
    }
    
    return true;
}

bool DesignTable::updateConfigurations() {
    // Update all configurations in the configuration manager
    for (const auto& configName : m_configurations) {
        Configuration* config = m_configManager.getConfiguration(configName);
        if (!config) {
            // Create the configuration if it doesn't exist
            if (!m_configManager.createConfiguration(configName)) {
                return false;
            }
            config = m_configManager.getConfiguration(configName);
            if (!config) {
                return false;
            }
        }
        
        // Update all parameter values
        for (const auto& key : m_parameters) {
            std::string featureId, paramName;
            if (parseParameterKey(key, featureId, paramName)) {
                auto configIt = m_values.find(configName);
                if (configIt != m_values.end()) {
                    auto paramIt = configIt->second.find(key);
                    if (paramIt != configIt->second.end()) {
                        config->setParameterValue(featureId, paramName, paramIt->second);
                    }
                }
            }
        }
    }
    
    return true;
}

bool DesignTable::importFromCsv(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    // Clear the current table
    m_parameters.clear();
    m_configurations.clear();
    m_values.clear();
    
    // Read the header line
    std::string line;
    if (!std::getline(file, line)) {
        return false;
    }
    
    // Parse the header line to get the configuration names
    std::stringstream headerStream(line);
    std::string cell;
    
    // Skip the first cell (parameter name)
    if (!std::getline(headerStream, cell, ',')) {
        return false;
    }
    
    // Read the configuration names
    while (std::getline(headerStream, cell, ',')) {
        m_configurations.push_back(cell);
    }
    
    // Read the parameter lines
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string paramKey;
        
        // Read the parameter key
        if (!std::getline(lineStream, paramKey, ',')) {
            continue;
        }
        
        // Add the parameter to the table
        m_parameters.push_back(paramKey);
        
        // Read the parameter values for each configuration
        size_t configIndex = 0;
        while (std::getline(lineStream, cell, ',') && configIndex < m_configurations.size()) {
            try {
                double value = std::stod(cell);
                m_values[m_configurations[configIndex]][paramKey] = value;
            } catch (const std::exception&) {
                // Skip invalid values
            }
            configIndex++;
        }
    }
    
    // Update the configurations in the configuration manager
    updateConfigurations();
    
    return true;
}

bool DesignTable::exportToCsv(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    // Write the header line
    file << "Parameter";
    for (const auto& configName : m_configurations) {
        file << "," << configName;
    }
    file << std::endl;
    
    // Write the parameter lines
    for (const auto& paramKey : m_parameters) {
        file << paramKey;
        
        for (const auto& configName : m_configurations) {
            file << ",";
            
            auto configIt = m_values.find(configName);
            if (configIt != m_values.end()) {
                auto paramIt = configIt->second.find(paramKey);
                if (paramIt != configIt->second.end()) {
                    file << paramIt->second;
                }
            }
        }
        
        file << std::endl;
    }
    
    return true;
}

std::string DesignTable::serialize() const {
    std::stringstream ss;
    ss << "DesignTable:" << std::endl;
    
    // Serialize parameters
    ss << "Parameters:" << std::endl;
    for (const auto& param : m_parameters) {
        ss << param << std::endl;
    }
    
    // Serialize configurations
    ss << "Configurations:" << std::endl;
    for (const auto& config : m_configurations) {
        ss << config << std::endl;
    }
    
    // Serialize values
    ss << "Values:" << std::endl;
    for (const auto& config : m_configurations) {
        auto configIt = m_values.find(config);
        if (configIt != m_values.end()) {
            for (const auto& param : m_parameters) {
                auto paramIt = configIt->second.find(param);
                if (paramIt != configIt->second.end()) {
                    ss << config << "," << param << "," << paramIt->second << std::endl;
                }
            }
        }
    }
    
    return ss.str();
}

bool DesignTable::deserialize(const std::string& serialized) {
    std::stringstream ss(serialized);
    std::string line;
    
    // Read "DesignTable:" line
    if (!std::getline(ss, line) || line != "DesignTable:") {
        return false;
    }
    
    // Clear the current table
    m_parameters.clear();
    m_configurations.clear();
    m_values.clear();
    
    // Read "Parameters:" line
    if (!std::getline(ss, line) || line != "Parameters:") {
        return false;
    }
    
    // Read parameters
    while (std::getline(ss, line) && !line.empty() && line != "Configurations:") {
        m_parameters.push_back(line);
    }
    
    // Check if we reached "Configurations:" line
    if (line != "Configurations:") {
        // Read "Configurations:" line
        if (!std::getline(ss, line) || line != "Configurations:") {
            return false;
        }
    }
    
    // Read configurations
    while (std::getline(ss, line) && !line.empty() && line != "Values:") {
        m_configurations.push_back(line);
    }
    
    // Check if we reached "Values:" line
    if (line != "Values:") {
        // Read "Values:" line
        if (!std::getline(ss, line) || line != "Values:") {
            return false;
        }
    }
    
    // Read values
    while (std::getline(ss, line) && !line.empty()) {
        std::stringstream lineStream(line);
        std::string configName, paramKey, valueStr;
        
        if (!std::getline(lineStream, configName, ',')) {
            continue;
        }
        
        if (!std::getline(lineStream, paramKey, ',')) {
            continue;
        }
        
        if (!std::getline(lineStream, valueStr)) {
            continue;
        }
        
        try {
            double value = std::stod(valueStr);
            m_values[configName][paramKey] = value;
        } catch (const std::exception&) {
            // Skip invalid values
        }
    }
    
    // Update the configurations in the configuration manager
    updateConfigurations();
    
    return true;
}

std::string DesignTable::getParameterKey(const std::string& featureId, const std::string& paramName) const {
    return featureId + ":" + paramName;
}

bool DesignTable::parseParameterKey(const std::string& key, std::string& featureId, std::string& paramName) const {
    size_t colonPos = key.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }
    
    featureId = key.substr(0, colonPos);
    paramName = key.substr(colonPos + 1);
    
    return true;
}

} // namespace modeling
} // namespace rebel_cad
