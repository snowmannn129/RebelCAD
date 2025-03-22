/**
 * @file ParameterEditorPanel.cpp
 * @brief Implementation of the ParameterEditorPanel class
 */

#include "ParameterEditorPanel.h"
#include "../modeling/parametric/ParametricFeature.h"
#include "../modeling/parametric/ParameterManager.h"
#include "../modeling/parametric/DependencyGraph.h"
#include "../modeling/parametric/FeatureHistory.h"
#include "../modeling/parametric/FeatureHistory.h"
#include <imgui.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace rebel_cad {
namespace ui {

ParameterEditorPanel::ParameterEditorPanel(modeling::ParameterManager& paramManager,
                                         modeling::DependencyGraph& graph,
                                         modeling::FeatureHistory& history)
    : m_paramManager(paramManager)
    , m_graph(graph)
    , m_history(history)
    , m_selectedFeature(nullptr)
    , m_visible(true)
    , m_nextCallbackId(0)
{
    // Nothing to initialize
}

void ParameterEditorPanel::setSelectedFeature(modeling::ParametricFeature* feature) {
    m_selectedFeature = feature;
}

modeling::ParametricFeature* ParameterEditorPanel::getSelectedFeature() const {
    return m_selectedFeature;
}

void ParameterEditorPanel::draw() {
    if (!m_visible || !m_selectedFeature) {
        return;
    }

    // Begin the parameter editor panel
    if (ImGui::Begin("Parameter Editor", &m_visible)) {
        // Display the feature name
        ImGui::Text("Feature: %s", m_selectedFeature->getName().c_str());
        ImGui::Separator();

        // Get the parameters for the selected feature
        std::vector<modeling::Parameter*> parameters = m_paramManager.getParameters(m_selectedFeature->getId());

        // Sort parameters by name
        std::sort(parameters.begin(), parameters.end(), [](const modeling::Parameter* a, const modeling::Parameter* b) {
            return a->getName() < b->getName();
        });

        // Display each parameter
        for (modeling::Parameter* param : parameters) {
            if (drawParameterEditor(param)) {
                // Parameter was changed, update the feature
                m_graph.updateFeature(m_selectedFeature->getId());
            }
        }
    }
    ImGui::End();
}

bool ParameterEditorPanel::isVisible() const {
    return m_visible;
}

void ParameterEditorPanel::setVisible(bool visible) {
    m_visible = visible;
}

size_t ParameterEditorPanel::addParameterChangeCallback(std::function<void(const std::string&, const std::string&, double)> callback) {
    size_t callbackId = m_nextCallbackId++;
    m_changeCallbacks[callbackId] = std::move(callback);
    return callbackId;
}

bool ParameterEditorPanel::removeParameterChangeCallback(size_t callbackId) {
    return m_changeCallbacks.erase(callbackId) > 0;
}

bool ParameterEditorPanel::drawParameterEditor(modeling::Parameter* param) {
    if (!param) {
        return false;
    }

    bool changed = false;
    const std::string& name = param->getName();
    double value = param->getValue();
    double newValue = value;

    // Display the parameter name
    ImGui::PushID(name.c_str());
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", name.c_str());
    ImGui::SameLine();

    // Display the appropriate editor based on the parameter type
    switch (param->getType()) {
        case modeling::Parameter::Type::Length:
        case modeling::Parameter::Type::Angle:
        case modeling::Parameter::Type::Percentage:
        case modeling::Parameter::Type::Custom: {
            // Use a slider if the parameter has min and max values
            if (param->isConstrained()) {
                float min = param->getMin().value_or(0.0f);
                float max = param->getMax().value_or(100.0f);
                float step = param->getStep().value_or(0.1f);
                float val = static_cast<float>(value);
                if (ImGui::SliderFloat("##value", &val, min, max, "%.3f")) {
                    newValue = val;
                    changed = true;
                }
            } else {
                // Use a drag control for unconstrained parameters
                float val = static_cast<float>(value);
                if (ImGui::DragFloat("##value", &val, 0.1f)) {
                    newValue = val;
                    changed = true;
                }
            }
            break;
        }
        case modeling::Parameter::Type::Count: {
            // Use an integer slider for count parameters
            int min = static_cast<int>(param->getMin().value_or(0.0));
            int max = static_cast<int>(param->getMax().value_or(100.0));
            int val = static_cast<int>(value);
            if (ImGui::SliderInt("##value", &val, min, max)) {
                newValue = static_cast<double>(val);
                changed = true;
            }
            break;
        }
        case modeling::Parameter::Type::Boolean: {
            // Use a checkbox for boolean parameters
            bool val = value != 0.0;
            if (ImGui::Checkbox("##value", &val)) {
                newValue = val ? 1.0 : 0.0;
                changed = true;
            }
            break;
        }
    }

    ImGui::PopID();

    // If the parameter was changed, update it and create an operation
    if (changed) {
        // Create an operation for the parameter change
        createParameterChangeOperation(name, value, newValue);

        // Update the parameter value
        param->setValue(newValue);

        // Notify callbacks
        notifyParameterChanged(m_selectedFeature->getId(), name, newValue);
    }

    return changed;
}

void ParameterEditorPanel::createParameterChangeOperation(const std::string& paramName, double oldValue, double newValue) {
    // Create a parameter change operation
    auto operation = std::make_unique<modeling::SetParameterOperation>(
        m_selectedFeature->getId(), paramName, oldValue, newValue);

    // Add the operation to the history
    m_history.addOperation(std::move(operation));
}

void ParameterEditorPanel::notifyParameterChanged(const std::string& featureId, const std::string& paramName, double newValue) {
    for (const auto& [id, callback] : m_changeCallbacks) {
        callback(featureId, paramName, newValue);
    }
}

} // namespace ui
} // namespace rebel_cad
