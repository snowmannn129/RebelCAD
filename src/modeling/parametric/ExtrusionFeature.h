/**
 * @file ExtrusionFeature.h
 * @brief Extrusion feature for parametric modeling
 * 
 * This file defines the ExtrusionFeature class, which represents an extrusion
 * operation in the parametric modeling system. It extrudes a 2D sketch into a
 * 3D solid.
 */

#pragma once

#include "ParametricFeature.h"
#include <vector>
#include <memory>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ParameterManager;

/**
 * @brief Extrusion feature for parametric modeling
 * 
 * The ExtrusionFeature class represents an extrusion operation in the parametric
 * modeling system. It extrudes a 2D sketch into a 3D solid.
 */
class ExtrusionFeature : public ParametricFeature {
public:
    /**
     * @brief Extrusion direction enumeration
     */
    enum class Direction {
        Normal,     ///< Extrude in the normal direction
        Reverse,    ///< Extrude in the reverse direction
        Symmetric,  ///< Extrude symmetrically in both directions
        Custom      ///< Extrude in a custom direction
    };

    /**
     * @brief Extrusion end condition enumeration
     */
    enum class EndCondition {
        Blind,      ///< Extrude to a specified distance
        ThroughAll, ///< Extrude through all existing geometry
        UpToSurface, ///< Extrude up to a specified surface
        UpToBody,   ///< Extrude up to a specified body
        UpToVertex  ///< Extrude up to a specified vertex
    };

    /**
     * @brief Constructor
     * 
     * @param id Unique identifier for the feature
     * @param name Human-readable name for the feature
     * @param sketchId ID of the sketch to extrude
     * @param paramManager Parameter manager for managing parameters
     */
    ExtrusionFeature(const FeatureId& id, const std::string& name, const FeatureId& sketchId,
                    ParameterManager& paramManager);

    /**
     * @brief Destructor
     */
    ~ExtrusionFeature() override = default;

    /**
     * @brief Update the feature
     * 
     * This method is called when the feature needs to be updated due to
     * changes in its parameters or dependencies. It recomputes the
     * feature's geometry and updates its status.
     * 
     * @return True if the update was successful, false otherwise
     */
    bool update() override;

    /**
     * @brief Get the dependencies of the feature
     * 
     * @return A vector of feature IDs that this feature depends on
     */
    std::vector<FeatureId> getDependencies() const override;

    /**
     * @brief Add a dependency to the feature
     * 
     * @param dependencyId The ID of the feature to depend on
     */
    void addDependency(const FeatureId& dependencyId) override;

    /**
     * @brief Remove a dependency from the feature
     * 
     * @param dependencyId The ID of the feature to remove dependency from
     */
    void removeDependency(const FeatureId& dependencyId) override;

    /**
     * @brief Get the parameters of the feature
     * 
     * @return A map of parameter names to values
     */
    std::unordered_map<std::string, double> getParameters() const override;

    /**
     * @brief Set a parameter value
     * 
     * @param name The name of the parameter
     * @param value The new value for the parameter
     * @return True if the parameter was set successfully, false otherwise
     */
    bool setParameter(const std::string& name, double value) override;

    /**
     * @brief Serialize the feature to a string
     * 
     * @return A string representation of the feature
     */
    std::string serialize() const override;

    /**
     * @brief Deserialize the feature from a string
     * 
     * @param serialized The serialized string representation of the feature
     * @return True if deserialization was successful, false otherwise
     */
    bool deserialize(const std::string& serialized) override;

    /**
     * @brief Get the sketch ID
     * 
     * @return The ID of the sketch being extruded
     */
    const FeatureId& getSketchId() const { return m_sketchId; }

    /**
     * @brief Set the sketch ID
     * 
     * @param sketchId The ID of the sketch to extrude
     */
    void setSketchId(const FeatureId& sketchId);

    /**
     * @brief Get the extrusion direction
     * 
     * @return The extrusion direction
     */
    Direction getDirection() const { return m_direction; }

    /**
     * @brief Set the extrusion direction
     * 
     * @param direction The new extrusion direction
     */
    void setDirection(Direction direction);

    /**
     * @brief Get the end condition
     * 
     * @return The end condition
     */
    EndCondition getEndCondition() const { return m_endCondition; }

    /**
     * @brief Set the end condition
     * 
     * @param endCondition The new end condition
     */
    void setEndCondition(EndCondition endCondition);

    /**
     * @brief Get the target feature ID for the end condition
     * 
     * @return The target feature ID
     */
    const FeatureId& getTargetFeatureId() const { return m_targetFeatureId; }

    /**
     * @brief Set the target feature ID for the end condition
     * 
     * @param targetFeatureId The new target feature ID
     */
    void setTargetFeatureId(const FeatureId& targetFeatureId);

private:
    FeatureId m_sketchId;                    ///< ID of the sketch to extrude
    std::vector<FeatureId> m_dependencies;   ///< Dependencies of the feature
    Direction m_direction;                   ///< Extrusion direction
    EndCondition m_endCondition;             ///< Extrusion end condition
    FeatureId m_targetFeatureId;             ///< Target feature ID for the end condition
    ParameterManager& m_paramManager;        ///< Reference to the parameter manager
};

} // namespace modeling
} // namespace rebel_cad
