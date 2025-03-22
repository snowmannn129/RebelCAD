/**
 * @file ExtrusionFeature.cpp
 * @brief Implementation of the ExtrusionFeature class
 */

#include "ExtrusionFeature.h"
#include "ParameterManager.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>

namespace rebel_cad {
namespace modeling {

ExtrusionFeature::ExtrusionFeature(const FeatureId& id, const std::string& name, const FeatureId& sketchId,
                                 ParameterManager& paramManager)
    : ParametricFeature(id, name)
    , m_sketchId(sketchId)
    , m_direction(Direction::Normal)
    , m_endCondition(EndCondition::Blind)
    , m_targetFeatureId("")
    , m_paramManager(paramManager)
{
    // Add the sketch as a dependency
    m_dependencies.push_back(sketchId);

    // Create parameters
    m_paramManager.createParameter(id, "depth", 10.0, Parameter::Type::Length, 0.0);
    m_paramManager.createParameter(id, "direction", static_cast<double>(m_direction), Parameter::Type::Count, 0.0, 3.0, 1.0);
    m_paramManager.createParameter(id, "endCondition", static_cast<double>(m_endCondition), Parameter::Type::Count, 0.0, 4.0, 1.0);
    m_paramManager.createParameter(id, "draft", 0.0, Parameter::Type::Angle, -90.0, 90.0, 1.0);
    m_paramManager.createParameter(id, "thinWalled", 0.0, Parameter::Type::Boolean);
    m_paramManager.createParameter(id, "wallThickness", 1.0, Parameter::Type::Length, 0.0);
}

bool ExtrusionFeature::update() {
    // Check if the sketch exists
    if (m_sketchId.empty()) {
        setStatus(FeatureStatus::Error);
        return false;
    }

    // In a real implementation, we would:
    // 1. Get the sketch geometry
    // 2. Perform the extrusion operation
    // 3. Update the feature's geometry

    // For this example, we'll just set the status to valid
    setStatus(FeatureStatus::Valid);
    return true;
}

std::vector<ParametricFeature::FeatureId> ExtrusionFeature::getDependencies() const {
    return m_dependencies;
}

void ExtrusionFeature::addDependency(const FeatureId& dependencyId) {
    // Check if the dependency already exists
    if (std::find(m_dependencies.begin(), m_dependencies.end(), dependencyId) != m_dependencies.end()) {
        return;
    }

    // Add the dependency
    m_dependencies.push_back(dependencyId);
}

void ExtrusionFeature::removeDependency(const FeatureId& dependencyId) {
    // Check if the dependency is the sketch
    if (dependencyId == m_sketchId) {
        return; // Can't remove the sketch dependency
    }

    // Remove the dependency
    m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), dependencyId), m_dependencies.end());
}

std::unordered_map<std::string, double> ExtrusionFeature::getParameters() const {
    std::unordered_map<std::string, double> params;
    
    // Get all parameters from the parameter manager
    std::vector<Parameter*> parameters = m_paramManager.getParameters(getId());
    for (Parameter* param : parameters) {
        params[param->getName()] = param->getValue();
    }
    
    return params;
}

bool ExtrusionFeature::setParameter(const std::string& name, double value) {
    // Set the parameter value
    bool success = m_paramManager.setParameterValue(getId(), name, value);
    
    // Update internal state based on parameter changes
    if (success) {
        if (name == "direction") {
            m_direction = static_cast<Direction>(static_cast<int>(value));
        } else if (name == "endCondition") {
            m_endCondition = static_cast<EndCondition>(static_cast<int>(value));
        }
    }
    
    return success;
}

std::string ExtrusionFeature::serialize() const {
    // Use a simple string-based serialization format
    std::stringstream ss;
    
    // Serialize basic properties
    ss << "id=" << getId() << std::endl;
    ss << "name=" << getName() << std::endl;
    ss << "status=" << static_cast<int>(getStatus()) << std::endl;
    ss << "sketchId=" << m_sketchId << std::endl;
    ss << "direction=" << static_cast<int>(m_direction) << std::endl;
    ss << "endCondition=" << static_cast<int>(m_endCondition) << std::endl;
    ss << "targetFeatureId=" << m_targetFeatureId << std::endl;
    
    // Serialize dependencies
    ss << "dependencies=";
    for (size_t i = 0; i < m_dependencies.size(); ++i) {
        if (i > 0) {
            ss << ",";
        }
        ss << m_dependencies[i];
    }
    ss << std::endl;
    
    // Serialize parameters
    auto params = getParameters();
    ss << "parameters=" << std::endl;
    for (const auto& [name, value] : params) {
        ss << name << "=" << value << std::endl;
    }
    
    return ss.str();
}

bool ExtrusionFeature::deserialize(const std::string& serialized) {
    try {
        std::stringstream ss(serialized);
        std::string line;
        std::string key, value;
        
        // Parse each line
        while (std::getline(ss, line)) {
            // Skip empty lines
            if (line.empty()) {
                continue;
            }
            
            // Find the key-value separator
            size_t pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }
            
            key = line.substr(0, pos);
            value = line.substr(pos + 1);
            
            // Parse the key-value pair
            if (key == "name") {
                setName(value);
            } else if (key == "status") {
                setStatus(static_cast<FeatureStatus>(std::stoi(value)));
            } else if (key == "sketchId") {
                m_sketchId = value;
            } else if (key == "direction") {
                m_direction = static_cast<Direction>(std::stoi(value));
            } else if (key == "endCondition") {
                m_endCondition = static_cast<EndCondition>(std::stoi(value));
            } else if (key == "targetFeatureId") {
                m_targetFeatureId = value;
            } else if (key == "dependencies") {
                m_dependencies.clear();
                std::stringstream deps(value);
                std::string dep;
                while (std::getline(deps, dep, ',')) {
                    if (!dep.empty()) {
                        m_dependencies.push_back(dep);
                    }
                }
            } else if (key != "id" && key != "parameters") {
                // Assume it's a parameter
                setParameter(key, std::stod(value));
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        // Deserialization failed
        return false;
    }
}

void ExtrusionFeature::setSketchId(const FeatureId& sketchId) {
    // Remove the old sketch from dependencies
    m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), m_sketchId), m_dependencies.end());
    
    // Set the new sketch ID
    m_sketchId = sketchId;
    
    // Add the new sketch to dependencies
    if (!m_sketchId.empty()) {
        m_dependencies.push_back(m_sketchId);
    }
}

void ExtrusionFeature::setDirection(Direction direction) {
    m_direction = direction;
    m_paramManager.setParameterValue(getId(), "direction", static_cast<double>(m_direction));
}

void ExtrusionFeature::setEndCondition(EndCondition endCondition) {
    m_endCondition = endCondition;
    m_paramManager.setParameterValue(getId(), "endCondition", static_cast<double>(m_endCondition));
}

void ExtrusionFeature::setTargetFeatureId(const FeatureId& targetFeatureId) {
    // Remove the old target from dependencies if it exists
    if (!m_targetFeatureId.empty()) {
        m_dependencies.erase(std::remove(m_dependencies.begin(), m_dependencies.end(), m_targetFeatureId), m_dependencies.end());
    }
    
    // Set the new target ID
    m_targetFeatureId = targetFeatureId;
    
    // Add the new target to dependencies if it exists and the end condition requires it
    if (!m_targetFeatureId.empty() && 
        (m_endCondition == EndCondition::UpToSurface || 
         m_endCondition == EndCondition::UpToBody || 
         m_endCondition == EndCondition::UpToVertex)) {
        m_dependencies.push_back(m_targetFeatureId);
    }
}

} // namespace modeling
} // namespace rebel_cad
