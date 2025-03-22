/**
 * @file ParameterEditorPanel.h
 * @brief UI panel for editing parameters of parametric features
 * 
 * This file defines the ParameterEditorPanel class, which provides a UI
 * for editing parameters of parametric features.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace rebel_cad {

// Forward declarations for external dependencies
namespace modeling {
class ParametricFeature;
class ParameterManager;
class DependencyGraph;
class FeatureHistory;
class Parameter;
}

namespace ui {

/**
 * @brief UI panel for editing parameters of parametric features
 * 
 * The ParameterEditorPanel class provides a UI for editing parameters of
 * parametric features. It displays the parameters of a selected feature
 * and allows the user to edit them.
 */
class ParameterEditorPanel {
public:
    /**
     * @brief Constructor
     * 
     * @param paramManager Reference to the parameter manager
     * @param graph Reference to the dependency graph
     * @param history Reference to the feature history
     */
    ParameterEditorPanel(modeling::ParameterManager& paramManager,
                         modeling::DependencyGraph& graph,
                         modeling::FeatureHistory& history);

    /**
     * @brief Destructor
     */
    ~ParameterEditorPanel() = default;

    /**
     * @brief Set the selected feature
     * 
     * @param feature Pointer to the selected feature
     */
    void setSelectedFeature(modeling::ParametricFeature* feature);

    /**
     * @brief Get the selected feature
     * 
     * @return Pointer to the selected feature
     */
    modeling::ParametricFeature* getSelectedFeature() const;

    /**
     * @brief Draw the panel
     * 
     * This method draws the panel using ImGui.
     */
    void draw();

    /**
     * @brief Check if the panel is visible
     * 
     * @return True if the panel is visible, false otherwise
     */
    bool isVisible() const;

    /**
     * @brief Set the visibility of the panel
     * 
     * @param visible True to show the panel, false to hide it
     */
    void setVisible(bool visible);

    /**
     * @brief Add a callback function to be called when a parameter is changed
     * 
     * @param callback The callback function
     * @return An ID that can be used to remove the callback
     */
    size_t addParameterChangeCallback(std::function<void(const std::string&, const std::string&, double)> callback);

    /**
     * @brief Remove a callback function
     * 
     * @param callbackId The ID of the callback to remove
     * @return True if the callback was removed, false if it wasn't found
     */
    bool removeParameterChangeCallback(size_t callbackId);

private:
    /**
     * @brief Draw a parameter editor for a specific parameter
     * 
     * @param param The parameter to edit
     * @return True if the parameter was changed, false otherwise
     */
    bool drawParameterEditor(modeling::Parameter* param);

    /**
     * @brief Create an operation for a parameter change
     * 
     * @param paramName The name of the parameter
     * @param oldValue The old value of the parameter
     * @param newValue The new value of the parameter
     */
    void createParameterChangeOperation(const std::string& paramName, double oldValue, double newValue);

    /**
     * @brief Notify callbacks that a parameter has changed
     * 
     * @param featureId The ID of the feature
     * @param paramName The name of the parameter
     * @param newValue The new value of the parameter
     */
    void notifyParameterChanged(const std::string& featureId, const std::string& paramName, double newValue);

    modeling::ParameterManager& m_paramManager;  ///< Reference to the parameter manager
    modeling::DependencyGraph& m_graph;          ///< Reference to the dependency graph
    modeling::FeatureHistory& m_history;         ///< Reference to the feature history
    modeling::ParametricFeature* m_selectedFeature; ///< Pointer to the selected feature
    bool m_visible;                              ///< Visibility of the panel
    std::unordered_map<size_t, std::function<void(const std::string&, const std::string&, double)>> m_changeCallbacks; ///< Callbacks for parameter changes
    size_t m_nextCallbackId;                     ///< Next callback ID
};

} // namespace ui
} // namespace rebel_cad
