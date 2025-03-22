/**
 * @file FeatureHistory.cpp
 * @brief Implementation of the FeatureHistory class
 */

#include "modeling/parametric/FeatureHistory.h"
#include "modeling/parametric/ParametricFeature.h"
#include "modeling/parametric/DependencyGraph.h"
#include "modeling/parametric/ParameterManager.h"
#include <sstream>

namespace rebel_cad {
namespace modeling {

// ParametricOperation implementation

ParametricOperation::ParametricOperation(Type type, const std::string& featureId, const std::string& description)
    : m_type(type)
    , m_featureId(featureId)
    , m_description(description)
{
    // Nothing to initialize
}

// FeatureHistory implementation

FeatureHistory::FeatureHistory(DependencyGraph& graph, ParameterManager& paramManager)
    : m_graph(graph)
    , m_paramManager(paramManager)
    , m_currentPosition(0)
    , m_nextCallbackId(0)
{
    // Nothing to initialize
}

void FeatureHistory::addOperation(std::unique_ptr<ParametricOperation> operation) {
    // If we're not at the end of the history, remove all operations after the current position
    if (m_currentPosition < m_operations.size()) {
        m_operations.erase(m_operations.begin() + m_currentPosition, m_operations.end());
    }

    // Add the operation
    m_operations.push_back(std::move(operation));
    m_currentPosition = m_operations.size();

    // Notify callbacks
    notifyHistoryChanged();
}

bool FeatureHistory::undo() {
    if (!canUndo()) {
        return false;
    }

    // Get the operation to undo
    ParametricOperation* operation = m_operations[m_currentPosition - 1].get();

    // Undo the operation
    bool success = operation->undo(m_graph, m_paramManager);
    if (success) {
        m_currentPosition--;
        notifyHistoryChanged();
    }

    return success;
}

bool FeatureHistory::redo() {
    if (!canRedo()) {
        return false;
    }

    // Get the operation to redo
    ParametricOperation* operation = m_operations[m_currentPosition].get();

    // Redo the operation
    bool success = operation->redo(m_graph, m_paramManager);
    if (success) {
        m_currentPosition++;
        notifyHistoryChanged();
    }

    return success;
}

bool FeatureHistory::canUndo() const {
    return m_currentPosition > 0;
}

bool FeatureHistory::canRedo() const {
    return m_currentPosition < m_operations.size();
}

size_t FeatureHistory::getOperationCount() const {
    return m_operations.size();
}

size_t FeatureHistory::getCurrentPosition() const {
    return m_currentPosition;
}

void FeatureHistory::clear() {
    m_operations.clear();
    m_currentPosition = 0;
    notifyHistoryChanged();
}

size_t FeatureHistory::addHistoryChangeCallback(std::function<void()> callback) {
    size_t callbackId = m_nextCallbackId++;
    m_changeCallbacks[callbackId] = std::move(callback);
    return callbackId;
}

bool FeatureHistory::removeHistoryChangeCallback(size_t callbackId) {
    return m_changeCallbacks.erase(callbackId) > 0;
}

void FeatureHistory::notifyHistoryChanged() {
    for (const auto& [id, callback] : m_changeCallbacks) {
        callback();
    }
}

// AddFeatureOperation implementation

AddFeatureOperation::AddFeatureOperation(ParametricFeature* feature)
    : ParametricOperation(Type::AddFeature, feature->getId(), "Add feature: " + feature->getName())
    , m_feature(feature)
{
    // Nothing to initialize
}

bool AddFeatureOperation::undo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.removeFeature(m_featureId);
}

bool AddFeatureOperation::redo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.addFeature(m_feature);
}

// RemoveFeatureOperation implementation

RemoveFeatureOperation::RemoveFeatureOperation(ParametricFeature* feature)
    : ParametricOperation(Type::RemoveFeature, feature->getId(), "Remove feature: " + feature->getName())
    , m_feature(feature)
{
    // Nothing to initialize
}

bool RemoveFeatureOperation::undo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.addFeature(m_feature);
}

bool RemoveFeatureOperation::redo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.removeFeature(m_featureId);
}

// SetParameterOperation implementation

SetParameterOperation::SetParameterOperation(const std::string& featureId, const std::string& paramName,
                                           double oldValue, double newValue)
    : ParametricOperation(Type::SetParameter, featureId, "Set parameter: " + paramName)
    , m_paramName(paramName)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
    // Nothing to initialize
}

bool SetParameterOperation::undo(DependencyGraph& graph, ParameterManager& paramManager) {
    bool success = paramManager.setParameterValue(m_featureId, m_paramName, m_oldValue);
    if (success) {
        graph.updateFeature(m_featureId);
    }
    return success;
}

bool SetParameterOperation::redo(DependencyGraph& graph, ParameterManager& paramManager) {
    bool success = paramManager.setParameterValue(m_featureId, m_paramName, m_newValue);
    if (success) {
        graph.updateFeature(m_featureId);
    }
    return success;
}

// AddDependencyOperation implementation

AddDependencyOperation::AddDependencyOperation(const std::string& dependentId, const std::string& dependencyId)
    : ParametricOperation(Type::AddDependency, dependentId, "Add dependency: " + dependencyId)
    , m_dependencyId(dependencyId)
{
    // Nothing to initialize
}

bool AddDependencyOperation::undo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.removeDependency(m_featureId, m_dependencyId);
}

bool AddDependencyOperation::redo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.addDependency(m_featureId, m_dependencyId);
}

// RemoveDependencyOperation implementation

RemoveDependencyOperation::RemoveDependencyOperation(const std::string& dependentId, const std::string& dependencyId)
    : ParametricOperation(Type::RemoveDependency, dependentId, "Remove dependency: " + dependencyId)
    , m_dependencyId(dependencyId)
{
    // Nothing to initialize
}

bool RemoveDependencyOperation::undo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.addDependency(m_featureId, m_dependencyId);
}

bool RemoveDependencyOperation::redo(DependencyGraph& graph, ParameterManager& paramManager) {
    return graph.removeDependency(m_featureId, m_dependencyId);
}

} // namespace modeling
} // namespace rebel_cad
