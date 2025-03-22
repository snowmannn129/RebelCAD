/**
 * @file Configuration.cpp
 * @brief Implementation of the Configuration class
 */

#include "Configuration.h"
#include <sstream>
#include <iomanip>

namespace rebel_cad {
namespace modeling {

Configuration::Configuration(const std::string& name, const std::string& description)
    : m_name(name), m_description(description) {
}

bool Configuration::setParameterValue(const std::string& featureId, const std::string& paramName, double value) {
    std::string key = getParameterKey(featureId, paramName);
    m_parameterValues[key] = value;
    return true;
}

double Configuration::getParameterValue(const std::string& featureId, const std::string& paramName, double defaultValue) const {
    std::string key = getParameterKey(featureId, paramName);
    auto it = m_parameterValues.find(key);
    if (it != m_parameterValues.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Configuration::hasParameterValue(const std::string& featureId, const std::string& paramName) const {
    std::string key = getParameterKey(featureId, paramName);
    return m_parameterValues.find(key) != m_parameterValues.end();
}

bool Configuration::removeParameterValue(const std::string& featureId, const std::string& paramName) {
    std::string key = getParameterKey(featureId, paramName);
    auto it = m_parameterValues.find(key);
    if (it != m_parameterValues.end()) {
        m_parameterValues.erase(it);
        return true;
    }
    return false;
}

std::string Configuration::serialize() const {
    std::stringstream ss;
    ss << "Configuration:" << std::endl;
    ss << "Name: " << m_name << std::endl;
    ss << "Description: " << m_description << std::endl;
    ss << "Parameters:" << std::endl;
    for (const auto& param : m_parameterValues) {
        ss << param.first << " = " << std::fixed << std::setprecision(6) << param.second << std::endl;
    }
    return ss.str();
}

bool Configuration::deserialize(const std::string& serialized) {
    std::stringstream ss(serialized);
    std::string line;
    
    // Read "Configuration:" line
    if (!std::getline(ss, line) || line != "Configuration:") {
        return false;
    }
    
    // Read "Name:" line
    if (!std::getline(ss, line) || line.substr(0, 6) != "Name: ") {
        return false;
    }
    m_name = line.substr(6);
    
    // Read "Description:" line
    if (!std::getline(ss, line) || line.substr(0, 13) != "Description: ") {
        return false;
    }
    m_description = line.substr(13);
    
    // Read "Parameters:" line
    if (!std::getline(ss, line) || line != "Parameters:") {
        return false;
    }
    
    // Read parameter lines
    m_parameterValues.clear();
    while (std::getline(ss, line)) {
        size_t equalsPos = line.find(" = ");
        if (equalsPos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, equalsPos);
        std::string valueStr = line.substr(equalsPos + 3);
        
        try {
            double value = std::stod(valueStr);
            m_parameterValues[key] = value;
        } catch (const std::exception&) {
            // Skip invalid values
        }
    }
    
    return true;
}

std::string Configuration::getParameterKey(const std::string& featureId, const std::string& paramName) const {
    return featureId + ":" + paramName;
}

} // namespace modeling
} // namespace rebel_cad
