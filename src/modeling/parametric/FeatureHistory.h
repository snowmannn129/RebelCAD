/**
 * @file FeatureHistory.h
 * @brief History tracking for parametric features
 * 
 * This file defines the FeatureHistory class, which is responsible for
 * tracking the history of parametric features and providing undo/redo
 * functionality.
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
class ParametricFeature;
class DependencyGraph;
class ParameterManager;

/**
 * @brief Operation class for tracking changes to parametric features
 * 
 * The ParametricOperation class represents a single operation in the history
 * of parametric features. It provides methods for undoing and redoing the
 * operation.
 */
class ParametricOperation {
public:
    /**
     * @brief Operation type enumeration
     */
    enum class Type {
        AddFeature,         ///< Add a new feature
        RemoveFeature,      ///< Remove a feature
        ModifyFeature,      ///< Modify a feature
        AddDependency,      ///< Add a dependency between features
        RemoveDependency,   ///< Remove a dependency between features
        SetParameter,       ///< Set a parameter value
        Custom              ///< Custom operation type
    };

    /**
     * @brief Constructor
     * 
     * @param type The type of operation
     * @param featureId The ID of the feature affected by the operation
     * @param description A human-readable description of the operation
     */
    ParametricOperation(Type type, const std::string& featureId, const std::string& description);

    /**
     * @brief Virtual destructor
     */
    virtual ~ParametricOperation() = default;

    /**
     * @brief Get the type of operation
     * 
     * @return The operation type
     */
    Type getType() const { return m_type; }

    /**
     * @brief Get the ID of the feature affected by the operation
     * 
     * @return The feature ID
     */
    const std::string& getFeatureId() const { return m_featureId; }

    /**
     * @brief Get the description of the operation
     * 
     * @return The operation description
     */
    const std::string& getDescription() const { return m_description; }

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    virtual bool undo(DependencyGraph& graph, ParameterManager& paramManager) = 0;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    virtual bool redo(DependencyGraph& graph, ParameterManager& paramManager) = 0;

protected:
    Type m_type;                ///< Operation type
    std::string m_featureId;    ///< ID of the feature affected by the operation
    std::string m_description;  ///< Human-readable description of the operation
};

/**
 * @brief History class for tracking parametric operations
 * 
 * The FeatureHistory class is responsible for tracking the history of
 * parametric features and providing undo/redo functionality.
 */
class FeatureHistory {
public:
    /**
     * @brief Constructor
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     */
    FeatureHistory(DependencyGraph& graph, ParameterManager& paramManager);

    /**
     * @brief Destructor
     */
    ~FeatureHistory() = default;

    /**
     * @brief Add an operation to the history
     * 
     * @param operation The operation to add
     */
    void addOperation(std::unique_ptr<ParametricOperation> operation);

    /**
     * @brief Undo the last operation
     * 
     * @return True if an operation was undone, false if there are no operations to undo
     */
    bool undo();

    /**
     * @brief Redo the last undone operation
     * 
     * @return True if an operation was redone, false if there are no operations to redo
     */
    bool redo();

    /**
     * @brief Check if there are operations to undo
     * 
     * @return True if there are operations to undo, false otherwise
     */
    bool canUndo() const;

    /**
     * @brief Check if there are operations to redo
     * 
     * @return True if there are operations to redo, false otherwise
     */
    bool canRedo() const;

    /**
     * @brief Get the number of operations in the history
     * 
     * @return The number of operations
     */
    size_t getOperationCount() const;

    /**
     * @brief Get the current position in the history
     * 
     * @return The current position
     */
    size_t getCurrentPosition() const;

    /**
     * @brief Clear the history
     */
    void clear();

    /**
     * @brief Add a callback function to be called when the history changes
     * 
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addHistoryChangeCallback(std::function<void()> callback);

    /**
     * @brief Remove a callback function
     * 
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeHistoryChangeCallback(size_t callbackId);

private:
    /**
     * @brief Notify callbacks that the history has changed
     */
    void notifyHistoryChanged();

    DependencyGraph& m_graph;                                   ///< Reference to the dependency graph
    ParameterManager& m_paramManager;                           ///< Reference to the parameter manager
    std::vector<std::unique_ptr<ParametricOperation>> m_operations; ///< List of operations in the history
    size_t m_currentPosition;                                   ///< Current position in the history
    std::unordered_map<size_t, std::function<void()>> m_changeCallbacks; ///< Callbacks for history changes
    size_t m_nextCallbackId;                                    ///< Next callback ID
};

/**
 * @brief Operation for adding a feature
 */
class AddFeatureOperation : public ParametricOperation {
public:
    /**
     * @brief Constructor
     * 
     * @param feature The feature to add
     */
    AddFeatureOperation(ParametricFeature* feature);

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    bool undo(DependencyGraph& graph, ParameterManager& paramManager) override;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    bool redo(DependencyGraph& graph, ParameterManager& paramManager) override;

private:
    ParametricFeature* m_feature; ///< The feature to add
};

/**
 * @brief Operation for removing a feature
 */
class RemoveFeatureOperation : public ParametricOperation {
public:
    /**
     * @brief Constructor
     * 
     * @param feature The feature to remove
     */
    RemoveFeatureOperation(ParametricFeature* feature);

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    bool undo(DependencyGraph& graph, ParameterManager& paramManager) override;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    bool redo(DependencyGraph& graph, ParameterManager& paramManager) override;

private:
    ParametricFeature* m_feature; ///< The feature to remove
};

/**
 * @brief Operation for setting a parameter value
 */
class SetParameterOperation : public ParametricOperation {
public:
    /**
     * @brief Constructor
     * 
     * @param featureId The ID of the feature
     * @param paramName The name of the parameter
     * @param oldValue The old value of the parameter
     * @param newValue The new value of the parameter
     */
    SetParameterOperation(const std::string& featureId, const std::string& paramName,
                          double oldValue, double newValue);

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    bool undo(DependencyGraph& graph, ParameterManager& paramManager) override;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    bool redo(DependencyGraph& graph, ParameterManager& paramManager) override;

private:
    std::string m_paramName; ///< The name of the parameter
    double m_oldValue;       ///< The old value of the parameter
    double m_newValue;       ///< The new value of the parameter
};

/**
 * @brief Operation for adding a dependency
 */
class AddDependencyOperation : public ParametricOperation {
public:
    /**
     * @brief Constructor
     * 
     * @param dependentId The ID of the dependent feature
     * @param dependencyId The ID of the dependency feature
     */
    AddDependencyOperation(const std::string& dependentId, const std::string& dependencyId);

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    bool undo(DependencyGraph& graph, ParameterManager& paramManager) override;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    bool redo(DependencyGraph& graph, ParameterManager& paramManager) override;

private:
    std::string m_dependencyId; ///< The ID of the dependency feature
};

/**
 * @brief Operation for removing a dependency
 */
class RemoveDependencyOperation : public ParametricOperation {
public:
    /**
     * @brief Constructor
     * 
     * @param dependentId The ID of the dependent feature
     * @param dependencyId The ID of the dependency feature
     */
    RemoveDependencyOperation(const std::string& dependentId, const std::string& dependencyId);

    /**
     * @brief Undo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was undone successfully, false otherwise
     */
    bool undo(DependencyGraph& graph, ParameterManager& paramManager) override;

    /**
     * @brief Redo the operation
     * 
     * @param graph The dependency graph
     * @param paramManager The parameter manager
     * @return True if the operation was redone successfully, false otherwise
     */
    bool redo(DependencyGraph& graph, ParameterManager& paramManager) override;

private:
    std::string m_dependencyId; ///< The ID of the dependency feature
};

} // namespace modeling
} // namespace rebel_cad
