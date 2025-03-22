/**
 * @file DependencyGraph.h
 * @brief Dependency tracking for parametric features
 * 
 * This file defines the DependencyGraph class, which is responsible for
 * tracking dependencies between parametric features and propagating updates
 * when features change.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>

namespace rebel_cad {
namespace modeling {

// Forward declarations
class ParametricFeature;

/**
 * @brief Graph class for tracking dependencies between parametric features
 * 
 * The DependencyGraph class is responsible for tracking dependencies between
 * parametric features and propagating updates when features change. It maintains
 * a directed graph where each node is a feature and edges represent dependencies.
 */
class DependencyGraph {
public:
    /**
     * @brief Constructor
     */
    DependencyGraph();

    /**
     * @brief Destructor
     */
    ~DependencyGraph() = default;

    /**
     * @brief Add a feature to the graph
     * 
     * @param feature The feature to add
     * @return True if the feature was added successfully, false otherwise
     */
    bool addFeature(ParametricFeature* feature);

    /**
     * @brief Remove a feature from the graph
     * 
     * @param featureId The ID of the feature to remove
     * @return True if the feature was removed successfully, false otherwise
     */
    bool removeFeature(const std::string& featureId);

    /**
     * @brief Add a dependency between two features
     * 
     * @param dependentId The ID of the dependent feature
     * @param dependencyId The ID of the dependency feature
     * @return True if the dependency was added successfully, false otherwise
     */
    bool addDependency(const std::string& dependentId, const std::string& dependencyId);

    /**
     * @brief Remove a dependency between two features
     * 
     * @param dependentId The ID of the dependent feature
     * @param dependencyId The ID of the dependency feature
     * @return True if the dependency was removed successfully, false otherwise
     */
    bool removeDependency(const std::string& dependentId, const std::string& dependencyId);

    /**
     * @brief Get the dependencies of a feature
     * 
     * @param featureId The ID of the feature
     * @return A vector of feature IDs that the feature depends on
     */
    std::vector<std::string> getDependencies(const std::string& featureId) const;

    /**
     * @brief Get the dependents of a feature
     * 
     * @param featureId The ID of the feature
     * @return A vector of feature IDs that depend on the feature
     */
    std::vector<std::string> getDependents(const std::string& featureId) const;

    /**
     * @brief Check if a feature has dependencies
     * 
     * @param featureId The ID of the feature
     * @return True if the feature has dependencies, false otherwise
     */
    bool hasDependencies(const std::string& featureId) const;

    /**
     * @brief Check if a feature has dependents
     * 
     * @param featureId The ID of the feature
     * @return True if the feature has dependents, false otherwise
     */
    bool hasDependents(const std::string& featureId) const;

    /**
     * @brief Get all features in the graph
     * 
     * @return A vector of all feature IDs in the graph
     */
    std::vector<std::string> getAllFeatures() const;

    /**
     * @brief Get a feature by ID
     * 
     * @param featureId The ID of the feature
     * @return A pointer to the feature, or nullptr if not found
     */
    ParametricFeature* getFeature(const std::string& featureId) const;

    /**
     * @brief Update a feature and propagate changes to dependents
     * 
     * @param featureId The ID of the feature to update
     * @return True if the update was successful, false otherwise
     */
    bool updateFeature(const std::string& featureId);

    /**
     * @brief Get the features that need to be updated
     * 
     * @param featureId The ID of the feature that changed
     * @return A vector of feature IDs that need to be updated, in dependency order
     */
    std::vector<std::string> getUpdateOrder(const std::string& featureId) const;

    /**
     * @brief Check if adding a dependency would create a cycle
     * 
     * @param dependentId The ID of the dependent feature
     * @param dependencyId The ID of the dependency feature
     * @return True if adding the dependency would create a cycle, false otherwise
     */
    bool wouldCreateCycle(const std::string& dependentId, const std::string& dependencyId) const;

    /**
     * @brief Add a callback function to be called when a feature is updated
     * 
     * @param featureId The ID of the feature
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addUpdateCallback(const std::string& featureId, std::function<void()> callback);

    /**
     * @brief Remove a callback function
     * 
     * @param featureId The ID of the feature
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeUpdateCallback(const std::string& featureId, size_t callbackId);

private:
    /**
     * @brief Perform a topological sort of the dependency graph
     * 
     * @param startFeatureId The ID of the feature to start from
     * @return A vector of feature IDs in topological order
     */
    std::vector<std::string> topologicalSort(const std::string& startFeatureId) const;

    /**
     * @brief Check if there is a path from one feature to another
     * 
     * @param fromId The ID of the starting feature
     * @param toId The ID of the target feature
     * @return True if there is a path, false otherwise
     */
    bool hasPath(const std::string& fromId, const std::string& toId) const;

    std::unordered_map<std::string, ParametricFeature*> m_features; ///< Map of feature IDs to features
    std::unordered_map<std::string, std::unordered_set<std::string>> m_dependencies; ///< Map of feature IDs to their dependencies
    std::unordered_map<std::string, std::unordered_set<std::string>> m_dependents; ///< Map of feature IDs to their dependents
    std::unordered_map<std::string, std::unordered_map<size_t, std::function<void()>>> m_updateCallbacks; ///< Map of feature IDs to update callbacks
    std::unordered_map<std::string, size_t> m_nextCallbackIds; ///< Map of feature IDs to next callback IDs
};

} // namespace modeling
} // namespace rebel_cad
