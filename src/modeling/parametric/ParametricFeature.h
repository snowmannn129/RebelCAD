/**
 * @file ParametricFeature.h
 * @brief Base class for all parametric features in RebelCAD
 * 
 * This file defines the ParametricFeature class, which serves as the base class
 * for all parametric features in the RebelCAD modeling system. It provides the
 * interface for parameter management, dependency tracking, and update propagation.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ParameterManager;
class DependencyGraph;
class FeatureHistory;

/**
 * @brief Base class for all parametric features in RebelCAD
 * 
 * The ParametricFeature class serves as the base class for all parametric features
 * in the RebelCAD modeling system. It provides the interface for parameter management,
 * dependency tracking, and update propagation.
 */
class ParametricFeature {
public:
    /**
     * @brief Unique identifier for a feature
     */
    using FeatureId = std::string;

    /**
     * @brief Status of a feature
     */
    enum class FeatureStatus {
        Valid,      ///< Feature is valid and up-to-date
        Invalid,    ///< Feature is invalid and needs to be updated
        Suppressed, ///< Feature is suppressed and not included in the model
        Error       ///< Feature has an error and cannot be updated
    };

    /**
     * @brief Constructor
     * 
     * @param id Unique identifier for the feature
     * @param name Human-readable name for the feature
     */
    ParametricFeature(const FeatureId& id, const std::string& name);

    /**
     * @brief Virtual destructor
     */
    virtual ~ParametricFeature() = default;

    /**
     * @brief Get the unique identifier for the feature
     * 
     * @return The feature's unique identifier
     */
    const FeatureId& getId() const { return m_id; }

    /**
     * @brief Get the human-readable name for the feature
     * 
     * @return The feature's name
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Set the human-readable name for the feature
     * 
     * @param name The new name for the feature
     */
    void setName(const std::string& name) { m_name = name; }

    /**
     * @brief Get the status of the feature
     * 
     * @return The feature's status
     */
    FeatureStatus getStatus() const { return m_status; }

    /**
     * @brief Set the status of the feature
     * 
     * @param status The new status for the feature
     */
    void setStatus(FeatureStatus status) { m_status = status; }

    /**
     * @brief Check if the feature is valid
     * 
     * @return True if the feature is valid, false otherwise
     */
    bool isValid() const { return m_status == FeatureStatus::Valid; }

    /**
     * @brief Check if the feature is suppressed
     * 
     * @return True if the feature is suppressed, false otherwise
     */
    bool isSuppressed() const { return m_status == FeatureStatus::Suppressed; }

    /**
     * @brief Suppress the feature
     * 
     * Suppressing a feature removes it from the model without deleting it.
     * The feature can be unsuppressed later to include it in the model again.
     */
    void suppress() { m_status = FeatureStatus::Suppressed; }

    /**
     * @brief Unsuppress the feature
     * 
     * Unsuppressing a feature includes it in the model again.
     */
    void unsuppress() { m_status = FeatureStatus::Invalid; }

    /**
     * @brief Update the feature
     * 
     * This method is called when the feature needs to be updated due to
     * changes in its parameters or dependencies. It should recompute the
     * feature's geometry and update its status.
     * 
     * @return True if the update was successful, false otherwise
     */
    virtual bool update() = 0;

    /**
     * @brief Get the dependencies of the feature
     * 
     * @return A vector of feature IDs that this feature depends on
     */
    virtual std::vector<FeatureId> getDependencies() const = 0;

    /**
     * @brief Add a dependency to the feature
     * 
     * @param dependencyId The ID of the feature to depend on
     */
    virtual void addDependency(const FeatureId& dependencyId) = 0;

    /**
     * @brief Remove a dependency from the feature
     * 
     * @param dependencyId The ID of the feature to remove dependency from
     */
    virtual void removeDependency(const FeatureId& dependencyId) = 0;

    /**
     * @brief Get the parameters of the feature
     * 
     * @return A map of parameter names to values
     */
    virtual std::unordered_map<std::string, double> getParameters() const = 0;

    /**
     * @brief Set a parameter value
     * 
     * @param name The name of the parameter
     * @param value The new value for the parameter
     * @return True if the parameter was set successfully, false otherwise
     */
    virtual bool setParameter(const std::string& name, double value) = 0;

    /**
     * @brief Serialize the feature to a string
     * 
     * @return A string representation of the feature
     */
    virtual std::string serialize() const = 0;

    /**
     * @brief Deserialize the feature from a string
     * 
     * @param serialized The serialized string representation of the feature
     * @return True if deserialization was successful, false otherwise
     */
    virtual bool deserialize(const std::string& serialized) = 0;

protected:
    FeatureId m_id;                 ///< Unique identifier for the feature
    std::string m_name;             ///< Human-readable name for the feature
    FeatureStatus m_status;         ///< Status of the feature
};

} // namespace modeling
} // namespace rebel_cad
