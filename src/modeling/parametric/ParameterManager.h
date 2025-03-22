/**
 * @file ParameterManager.h
 * @brief Management of parameters for parametric features
 * 
 * This file defines the ParameterManager class, which is responsible for
 * managing the parameters of parametric features. It provides methods for
 * creating, retrieving, and updating parameters, as well as handling
 * parameter relationships and constraints.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <optional>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ParametricFeature;

/**
 * @brief Parameter class for storing parameter values and metadata
 */
class Parameter {
public:
    /**
     * @brief Parameter type enumeration
     */
    enum class Type {
        Length,     ///< Length parameter (mm)
        Angle,      ///< Angle parameter (degrees)
        Count,      ///< Count parameter (integer)
        Percentage, ///< Percentage parameter (0-100%)
        Boolean,    ///< Boolean parameter (true/false)
        Custom      ///< Custom parameter type
    };

    /**
     * @brief Constructor
     * 
     * @param name The name of the parameter
     * @param value The initial value of the parameter
     * @param type The type of the parameter
     * @param min The minimum allowed value (optional)
     * @param max The maximum allowed value (optional)
     * @param step The step size for incrementing/decrementing (optional)
     */
    Parameter(const std::string& name, double value, Type type,
              std::optional<double> min = std::nullopt,
              std::optional<double> max = std::nullopt,
              std::optional<double> step = std::nullopt);

    /**
     * @brief Get the name of the parameter
     * 
     * @return The parameter name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get the value of the parameter
     * 
     * @return The parameter value
     */
    double getValue() const { return m_value; }

    /**
     * @brief Set the value of the parameter
     * 
     * @param value The new value
     * @return True if the value was set successfully, false otherwise
     */
    bool setValue(double value);

    /**
     * @brief Get the type of the parameter
     * 
     * @return The parameter type
     */
    Type getType() const { return m_type; }

    /**
     * @brief Get the minimum allowed value
     * 
     * @return The minimum value, if set
     */
    std::optional<double> getMin() const { return m_min; }

    /**
     * @brief Get the maximum allowed value
     * 
     * @return The maximum value, if set
     */
    std::optional<double> getMax() const { return m_max; }

    /**
     * @brief Get the step size
     * 
     * @return The step size, if set
     */
    std::optional<double> getStep() const { return m_step; }

    /**
     * @brief Set the minimum allowed value
     * 
     * @param min The minimum value
     */
    void setMin(std::optional<double> min) { m_min = min; }

    /**
     * @brief Set the maximum allowed value
     * 
     * @param max The maximum value
     */
    void setMax(std::optional<double> max) { m_max = max; }

    /**
     * @brief Set the step size
     * 
     * @param step The step size
     */
    void setStep(std::optional<double> step) { m_step = step; }

    /**
     * @brief Check if the parameter is constrained
     * 
     * @return True if the parameter has min or max constraints
     */
    bool isConstrained() const { return m_min.has_value() || m_max.has_value(); }

    /**
     * @brief Add a callback function to be called when the parameter value changes
     * 
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addChangeCallback(std::function<void(double)> callback);

    /**
     * @brief Remove a callback function
     * 
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeChangeCallback(size_t callbackId);

private:
    std::string m_name;                                  ///< Parameter name
    double m_value;                                      ///< Parameter value
    Type m_type;                                         ///< Parameter type
    std::optional<double> m_min;                         ///< Minimum allowed value
    std::optional<double> m_max;                         ///< Maximum allowed value
    std::optional<double> m_step;                        ///< Step size
    std::unordered_map<size_t, std::function<void(double)>> m_changeCallbacks; ///< Callbacks for value changes
    size_t m_nextCallbackId = 0;                         ///< Next callback ID
};

/**
 * @brief Manager class for parameters
 * 
 * The ParameterManager class is responsible for managing the parameters of
 * parametric features. It provides methods for creating, retrieving, and
 * updating parameters, as well as handling parameter relationships and constraints.
 */
class ParameterManager {
public:
    /**
     * @brief Constructor
     */
    ParameterManager();

    /**
     * @brief Destructor
     */
    ~ParameterManager() = default;

    /**
     * @brief Create a new parameter
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param value The initial value of the parameter
     * @param type The type of the parameter
     * @param min The minimum allowed value (optional)
     * @param max The maximum allowed value (optional)
     * @param step The step size for incrementing/decrementing (optional)
     * @return True if the parameter was created successfully, false otherwise
     */
    bool createParameter(const std::string& featureId, const std::string& name,
                         double value, Parameter::Type type,
                         std::optional<double> min = std::nullopt,
                         std::optional<double> max = std::nullopt,
                         std::optional<double> step = std::nullopt);

    /**
     * @brief Get a parameter
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @return A pointer to the parameter, or nullptr if not found
     */
    Parameter* getParameter(const std::string& featureId, const std::string& name);

    /**
     * @brief Get all parameters for a feature
     * 
     * @param featureId The ID of the feature
     * @return A vector of pointers to the parameters
     */
    std::vector<Parameter*> getParameters(const std::string& featureId);

    /**
     * @brief Set a parameter value
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param value The new value for the parameter
     * @return True if the parameter was set successfully, false otherwise
     */
    bool setParameterValue(const std::string& featureId, const std::string& name, double value);

    /**
     * @brief Get a parameter value
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param defaultValue The default value to return if the parameter is not found
     * @return The parameter value, or defaultValue if not found
     */
    double getParameterValue(const std::string& featureId, const std::string& name, double defaultValue = 0.0);

    /**
     * @brief Create a parameter expression
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param expression The expression to evaluate
     * @param type The type of the parameter
     * @return True if the expression was created successfully, false otherwise
     */
    bool createExpression(const std::string& featureId, const std::string& name,
                          const std::string& expression, Parameter::Type type);

    /**
     * @brief Get an expression
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @return The expression, or an empty string if not found
     */
    std::string getExpression(const std::string& featureId, const std::string& name);

    /**
     * @brief Set an expression
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param expression The new expression
     * @return True if the expression was set successfully, false otherwise
     */
    bool setExpression(const std::string& featureId, const std::string& name, const std::string& expression);

    /**
     * @brief Evaluate an expression
     * 
     * @param expression The expression to evaluate
     * @return The result of the expression
     */
    double evaluateExpression(const std::string& expression);

    /**
     * @brief Add a parameter change callback
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addParameterChangeCallback(const std::string& featureId, const std::string& name,
                                     std::function<void(double)> callback);

    /**
     * @brief Remove a parameter change callback
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeParameterChangeCallback(const std::string& featureId, const std::string& name, size_t callbackId);

private:
    /**
     * @brief Get the full parameter key
     * 
     * @param featureId The ID of the feature that owns the parameter
     * @param name The name of the parameter
     * @return The full parameter key
     */
    std::string getParameterKey(const std::string& featureId, const std::string& name) const;

    std::unordered_map<std::string, std::unique_ptr<Parameter>> m_parameters; ///< Map of parameter keys to parameters
    std::unordered_map<std::string, std::string> m_expressions; ///< Map of parameter keys to expressions
};

} // namespace modeling
} // namespace rebel_cad
