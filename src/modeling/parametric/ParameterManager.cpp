/**
 * @file ParameterManager.cpp
 * @brief Implementation of the ParameterManager class
 */

#include "modeling/parametric/ParameterManager.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace rebel_cad {
namespace modeling {

// Parameter class implementation

Parameter::Parameter(const std::string& name, double value, Type type,
                     std::optional<double> min,
                     std::optional<double> max,
                     std::optional<double> step)
    : m_name(name)
    , m_value(value)
    , m_type(type)
    , m_min(min)
    , m_max(max)
    , m_step(step)
{
    // Validate the initial value against constraints
    if (m_min.has_value() && m_value < m_min.value()) {
        m_value = m_min.value();
    }
    if (m_max.has_value() && m_value > m_max.value()) {
        m_value = m_max.value();
    }

    // For Count type, ensure the value is an integer
    if (m_type == Type::Count) {
        m_value = std::round(m_value);
    }

    // For Boolean type, ensure the value is 0 or 1
    if (m_type == Type::Boolean) {
        m_value = (m_value != 0.0) ? 1.0 : 0.0;
    }

    // For Percentage type, ensure the value is between 0 and 100
    if (m_type == Type::Percentage) {
        if (!m_min.has_value()) {
            m_min = 0.0;
        }
        if (!m_max.has_value()) {
            m_max = 100.0;
        }
        m_value = std::clamp(m_value, m_min.value(), m_max.value());
    }
}

bool Parameter::setValue(double value) {
    // Validate the value against constraints
    if (m_min.has_value() && value < m_min.value()) {
        return false;
    }
    if (m_max.has_value() && value > m_max.value()) {
        return false;
    }

    // For Count type, ensure the value is an integer
    if (m_type == Type::Count) {
        value = std::round(value);
    }

    // For Boolean type, ensure the value is 0 or 1
    if (m_type == Type::Boolean) {
        value = (value != 0.0) ? 1.0 : 0.0;
    }

    // Check if the value has actually changed
    if (std::abs(m_value - value) < 1e-10) {
        return true; // Value hasn't changed significantly
    }

    // Update the value
    m_value = value;

    // Notify callbacks
    for (const auto& [id, callback] : m_changeCallbacks) {
        callback(m_value);
    }

    return true;
}

size_t Parameter::addChangeCallback(std::function<void(double)> callback) {
    size_t id = m_nextCallbackId++;
    m_changeCallbacks[id] = std::move(callback);
    return id;
}

bool Parameter::removeChangeCallback(size_t callbackId) {
    return m_changeCallbacks.erase(callbackId) > 0;
}

// ParameterManager class implementation

ParameterManager::ParameterManager() {
    // Nothing to initialize
}

bool ParameterManager::createParameter(const std::string& featureId, const std::string& name,
                                      double value, Parameter::Type type,
                                      std::optional<double> min,
                                      std::optional<double> max,
                                      std::optional<double> step) {
    std::string key = getParameterKey(featureId, name);
    
    // Check if the parameter already exists
    if (m_parameters.find(key) != m_parameters.end()) {
        return false;
    }

    // Create the parameter
    m_parameters[key] = std::make_unique<Parameter>(name, value, type, min, max, step);
    return true;
}

Parameter* ParameterManager::getParameter(const std::string& featureId, const std::string& name) {
    std::string key = getParameterKey(featureId, name);
    auto it = m_parameters.find(key);
    if (it != m_parameters.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Parameter*> ParameterManager::getParameters(const std::string& featureId) {
    std::vector<Parameter*> result;
    std::string prefix = featureId + ":";
    
    for (const auto& [key, param] : m_parameters) {
        if (key.substr(0, prefix.length()) == prefix) {
            result.push_back(param.get());
        }
    }
    
    return result;
}

bool ParameterManager::setParameterValue(const std::string& featureId, const std::string& name, double value) {
    Parameter* param = getParameter(featureId, name);
    if (param) {
        return param->setValue(value);
    }
    return false;
}

double ParameterManager::getParameterValue(const std::string& featureId, const std::string& name, double defaultValue) {
    Parameter* param = getParameter(featureId, name);
    if (param) {
        return param->getValue();
    }
    return defaultValue;
}

bool ParameterManager::createExpression(const std::string& featureId, const std::string& name,
                                       const std::string& expression, Parameter::Type type) {
    std::string key = getParameterKey(featureId, name);
    
    // Check if the parameter already exists
    if (m_parameters.find(key) != m_parameters.end()) {
        return false;
    }

    // Evaluate the expression
    double value;
    try {
        value = evaluateExpression(expression);
    } catch (const std::exception&) {
        return false;
    }

    // Create the parameter
    if (!createParameter(featureId, name, value, type)) {
        return false;
    }

    // Store the expression
    m_expressions[key] = expression;
    return true;
}

std::string ParameterManager::getExpression(const std::string& featureId, const std::string& name) {
    std::string key = getParameterKey(featureId, name);
    auto it = m_expressions.find(key);
    if (it != m_expressions.end()) {
        return it->second;
    }
    return "";
}

bool ParameterManager::setExpression(const std::string& featureId, const std::string& name, const std::string& expression) {
    std::string key = getParameterKey(featureId, name);
    
    // Check if the parameter exists
    Parameter* param = getParameter(featureId, name);
    if (!param) {
        return false;
    }

    // Evaluate the expression
    double value;
    try {
        value = evaluateExpression(expression);
    } catch (const std::exception&) {
        return false;
    }

    // Update the parameter value
    if (!param->setValue(value)) {
        return false;
    }

    // Store the expression
    m_expressions[key] = expression;
    return true;
}

double ParameterManager::evaluateExpression(const std::string& expression) {
    // This is a simple expression evaluator for demonstration purposes
    // In a real implementation, you would use a proper expression parser/evaluator
    
    // For now, just try to convert the expression to a double
    try {
        return std::stod(expression);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid expression: " + expression);
    }
}

size_t ParameterManager::addParameterChangeCallback(const std::string& featureId, const std::string& name,
                                                  std::function<void(double)> callback) {
    Parameter* param = getParameter(featureId, name);
    if (param) {
        return param->addChangeCallback(std::move(callback));
    }
    return 0;
}

bool ParameterManager::removeParameterChangeCallback(const std::string& featureId, const std::string& name, size_t callbackId) {
    Parameter* param = getParameter(featureId, name);
    if (param) {
        return param->removeChangeCallback(callbackId);
    }
    return false;
}

std::string ParameterManager::getParameterKey(const std::string& featureId, const std::string& name) const {
    return featureId + ":" + name;
}

} // namespace modeling
} // namespace rebel_cad
