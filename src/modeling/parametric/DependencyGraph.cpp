/**
 * @file DependencyGraph.cpp
 * @brief Implementation of the DependencyGraph class
 */

#include "modeling/parametric/DependencyGraph.h"
#include "modeling/parametric/ParametricFeature.h"
#include <algorithm>
#include <queue>
#include <stack>

namespace rebel_cad {
namespace modeling {

DependencyGraph::DependencyGraph() {
    // Nothing to initialize
}

bool DependencyGraph::addFeature(ParametricFeature* feature) {
    if (!feature) {
        return false;
    }

    const std::string& featureId = feature->getId();
    
    // Check if the feature already exists
    if (m_features.find(featureId) != m_features.end()) {
        return false;
    }

    // Add the feature
    m_features[featureId] = feature;
    m_dependencies[featureId] = std::unordered_set<std::string>();
    m_dependents[featureId] = std::unordered_set<std::string>();
    m_updateCallbacks[featureId] = std::unordered_map<size_t, std::function<void()>>();
    m_nextCallbackIds[featureId] = 0;
    
    return true;
}

bool DependencyGraph::removeFeature(const std::string& featureId) {
    // Check if the feature exists
    auto featureIt = m_features.find(featureId);
    if (featureIt == m_features.end()) {
        return false;
    }

    // Remove dependencies
    for (const std::string& dependencyId : m_dependencies[featureId]) {
        m_dependents[dependencyId].erase(featureId);
    }

    // Remove dependents
    for (const std::string& dependentId : m_dependents[featureId]) {
        m_dependencies[dependentId].erase(featureId);
    }

    // Remove the feature
    m_features.erase(featureId);
    m_dependencies.erase(featureId);
    m_dependents.erase(featureId);
    m_updateCallbacks.erase(featureId);
    m_nextCallbackIds.erase(featureId);
    
    return true;
}

bool DependencyGraph::addDependency(const std::string& dependentId, const std::string& dependencyId) {
    // Check if both features exist
    if (m_features.find(dependentId) == m_features.end() ||
        m_features.find(dependencyId) == m_features.end()) {
        return false;
    }

    // Check if the dependency already exists
    if (m_dependencies[dependentId].find(dependencyId) != m_dependencies[dependentId].end()) {
        return true; // Already exists, but not an error
    }

    // Check if adding the dependency would create a cycle
    if (wouldCreateCycle(dependentId, dependencyId)) {
        return false;
    }

    // Add the dependency
    m_dependencies[dependentId].insert(dependencyId);
    m_dependents[dependencyId].insert(dependentId);
    
    return true;
}

bool DependencyGraph::removeDependency(const std::string& dependentId, const std::string& dependencyId) {
    // Check if both features exist
    if (m_features.find(dependentId) == m_features.end() ||
        m_features.find(dependencyId) == m_features.end()) {
        return false;
    }

    // Check if the dependency exists
    if (m_dependencies[dependentId].find(dependencyId) == m_dependencies[dependentId].end()) {
        return false;
    }

    // Remove the dependency
    m_dependencies[dependentId].erase(dependencyId);
    m_dependents[dependencyId].erase(dependentId);
    
    return true;
}

std::vector<std::string> DependencyGraph::getDependencies(const std::string& featureId) const {
    std::vector<std::string> result;
    
    auto it = m_dependencies.find(featureId);
    if (it != m_dependencies.end()) {
        result.insert(result.end(), it->second.begin(), it->second.end());
    }
    
    return result;
}

std::vector<std::string> DependencyGraph::getDependents(const std::string& featureId) const {
    std::vector<std::string> result;
    
    auto it = m_dependents.find(featureId);
    if (it != m_dependents.end()) {
        result.insert(result.end(), it->second.begin(), it->second.end());
    }
    
    return result;
}

bool DependencyGraph::hasDependencies(const std::string& featureId) const {
    auto it = m_dependencies.find(featureId);
    return it != m_dependencies.end() && !it->second.empty();
}

bool DependencyGraph::hasDependents(const std::string& featureId) const {
    auto it = m_dependents.find(featureId);
    return it != m_dependents.end() && !it->second.empty();
}

std::vector<std::string> DependencyGraph::getAllFeatures() const {
    std::vector<std::string> result;
    result.reserve(m_features.size());
    
    for (const auto& [featureId, feature] : m_features) {
        result.push_back(featureId);
    }
    
    return result;
}

ParametricFeature* DependencyGraph::getFeature(const std::string& featureId) const {
    auto it = m_features.find(featureId);
    if (it != m_features.end()) {
        return it->second;
    }
    return nullptr;
}

bool DependencyGraph::updateFeature(const std::string& featureId) {
    // Check if the feature exists
    ParametricFeature* feature = getFeature(featureId);
    if (!feature) {
        return false;
    }

    // Update the feature
    bool success = feature->update();
    if (!success) {
        return false;
    }

    // Notify callbacks
    auto callbacksIt = m_updateCallbacks.find(featureId);
    if (callbacksIt != m_updateCallbacks.end()) {
        for (const auto& [id, callback] : callbacksIt->second) {
            callback();
        }
    }

    // Get the features that need to be updated
    std::vector<std::string> updateOrder = getUpdateOrder(featureId);
    
    // Update the dependent features
    for (const std::string& dependentId : updateOrder) {
        ParametricFeature* dependent = getFeature(dependentId);
        if (dependent) {
            dependent->update();
            
            // Notify callbacks
            auto callbacksIt = m_updateCallbacks.find(dependentId);
            if (callbacksIt != m_updateCallbacks.end()) {
                for (const auto& [id, callback] : callbacksIt->second) {
                    callback();
                }
            }
        }
    }
    
    return true;
}

std::vector<std::string> DependencyGraph::getUpdateOrder(const std::string& featureId) const {
    return topologicalSort(featureId);
}

bool DependencyGraph::wouldCreateCycle(const std::string& dependentId, const std::string& dependencyId) const {
    // If the dependent and dependency are the same, it would create a self-cycle
    if (dependentId == dependencyId) {
        return true;
    }

    // Check if there is a path from the dependency to the dependent
    return hasPath(dependencyId, dependentId);
}

size_t DependencyGraph::addUpdateCallback(const std::string& featureId, std::function<void()> callback) {
    // Check if the feature exists
    if (m_features.find(featureId) == m_features.end()) {
        return 0;
    }

    // Add the callback
    size_t callbackId = m_nextCallbackIds[featureId]++;
    m_updateCallbacks[featureId][callbackId] = std::move(callback);
    return callbackId;
}

bool DependencyGraph::removeUpdateCallback(const std::string& featureId, size_t callbackId) {
    // Check if the feature exists
    auto callbacksIt = m_updateCallbacks.find(featureId);
    if (callbacksIt == m_updateCallbacks.end()) {
        return false;
    }

    // Remove the callback
    return callbacksIt->second.erase(callbackId) > 0;
}

std::vector<std::string> DependencyGraph::topologicalSort(const std::string& startFeatureId) const {
    std::vector<std::string> result;
    std::unordered_set<std::string> visited;
    std::stack<std::string> stack;
    
    // Helper function for depth-first search
    std::function<void(const std::string&)> dfs = [&](const std::string& featureId) {
        visited.insert(featureId);
        
        // Visit all dependents
        auto dependentsIt = m_dependents.find(featureId);
        if (dependentsIt != m_dependents.end()) {
            for (const std::string& dependentId : dependentsIt->second) {
                if (visited.find(dependentId) == visited.end()) {
                    dfs(dependentId);
                }
            }
        }
        
        // Push the feature onto the stack
        stack.push(featureId);
    };
    
    // Start DFS from the given feature
    dfs(startFeatureId);
    
    // Pop the stack to get the topological order
    while (!stack.empty()) {
        result.push_back(stack.top());
        stack.pop();
    }
    
    // Remove the start feature from the result
    result.erase(std::remove(result.begin(), result.end(), startFeatureId), result.end());
    
    return result;
}

bool DependencyGraph::hasPath(const std::string& fromId, const std::string& toId) const {
    std::unordered_set<std::string> visited;
    std::queue<std::string> queue;
    
    // Start BFS from the 'from' feature
    queue.push(fromId);
    visited.insert(fromId);
    
    while (!queue.empty()) {
        std::string currentId = queue.front();
        queue.pop();
        
        // Check if we've reached the 'to' feature
        if (currentId == toId) {
            return true;
        }
        
        // Visit all dependents
        auto dependentsIt = m_dependents.find(currentId);
        if (dependentsIt != m_dependents.end()) {
            for (const std::string& dependentId : dependentsIt->second) {
                if (visited.find(dependentId) == visited.end()) {
                    visited.insert(dependentId);
                    queue.push(dependentId);
                }
            }
        }
    }
    
    return false;
}

} // namespace modeling
} // namespace rebel_cad
