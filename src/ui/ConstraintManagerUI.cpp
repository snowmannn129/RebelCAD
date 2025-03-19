#include "constraints/ConstraintManagerUI.h"
#include "core/Log.h"
#include <algorithm>
#include <imgui.h>

namespace RebelCAD {
namespace Constraints {

ConstraintManagerUI::ConstraintManagerUI(std::shared_ptr<AutoConstraintDetector> autoDetector)
    : m_autoDetector(std::move(autoDetector))
    , m_autoConstraintEnabled(true) {
    // Initialize constraint type filters
    m_constraintTypeFilters = {
        {"Parallel", true},
        {"Perpendicular", true},
        {"Concentric", true},
        {"Tangent", true},
        {"Length", true},
        {"Angle", true},
        {"Radius", true}
    };
}

bool ConstraintManagerUI::render() {
    bool modified = false;
    
    ImGui::Begin("Constraint Manager");
    
    // Auto-constraint settings section
    if (renderAutoConstraintSettings()) {
        modified = true;
    }
    
    ImGui::Separator();
    
    // Constraint filters
    if (ImGui::CollapsingHeader("Filters", ImGuiTreeNodeFlags_DefaultOpen)) {
        renderConstraintFilters();
    }
    
    ImGui::Separator();
    
    // Active constraints list
    if (renderConstraintList()) {
        modified = true;
    }
    
    ImGui::End();
    
    return modified;
}

bool ConstraintManagerUI::addConstraint(std::shared_ptr<Constraint> constraint) {
    if (!constraint) {
        Log::error("ConstraintManagerUI: Attempted to add null constraint");
        return false;
    }
    
    // Check if constraint already exists
    auto it = std::find_if(m_constraints.begin(), m_constraints.end(),
        [&constraint](const auto& existing) {
            return existing->equals(*constraint);
        });
        
    if (it != m_constraints.end()) {
        Log::warn("ConstraintManagerUI: Constraint already exists");
        return false;
    }
    
    m_constraints.push_back(std::move(constraint));
    return true;
}

bool ConstraintManagerUI::removeConstraint(std::shared_ptr<Constraint> constraint) {
    if (!constraint) {
        Log::error("ConstraintManagerUI: Attempted to remove null constraint");
        return false;
    }
    
    auto it = std::find_if(m_constraints.begin(), m_constraints.end(),
        [&constraint](const auto& existing) {
            return existing->equals(*constraint);
        });
        
    if (it == m_constraints.end()) {
        Log::warn("ConstraintManagerUI: Constraint not found");
        return false;
    }
    
    m_constraints.erase(it);
    return true;
}

void ConstraintManagerUI::setAutoConstraintEnabled(bool enabled) {
    m_autoConstraintEnabled = enabled;
    if (m_autoDetector) {
        m_autoDetector->setEnabled(enabled);
    }
}

bool ConstraintManagerUI::isAutoConstraintEnabled() const {
    return m_autoConstraintEnabled;
}

void ConstraintManagerUI::refresh() {
    // Remove any invalid constraints
    m_constraints.erase(
        std::remove_if(m_constraints.begin(), m_constraints.end(),
            [](const auto& constraint) {
                return !constraint || !constraint->isValid();
            }),
        m_constraints.end()
    );
}

bool ConstraintManagerUI::renderAutoConstraintSettings() {
    bool modified = false;
    
    ImGui::Text("Auto-Constraint Detection");
    
    bool enabled = m_autoConstraintEnabled;
    if (ImGui::Checkbox("Enable Auto-Constraints", &enabled)) {
        setAutoConstraintEnabled(enabled);
        modified = true;
    }
    
    if (enabled && m_autoDetector) {
        // Add additional auto-constraint settings here
        float sensitivity = m_autoDetector->getSensitivity();
        if (ImGui::SliderFloat("Detection Sensitivity", &sensitivity, 0.1f, 1.0f)) {
            m_autoDetector->setSensitivity(sensitivity);
            modified = true;
        }
    }
    
    return modified;
}

bool ConstraintManagerUI::renderConstraintList() {
    bool modified = false;
    
    ImGui::Text("Active Constraints");
    
    if (m_constraints.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No active constraints");
        return false;
    }
    
    // Create a temporary vector of visible constraints based on filters
    std::vector<std::shared_ptr<Constraint>> visibleConstraints;
    for (const auto& constraint : m_constraints) {
        if (m_constraintTypeFilters[constraint->getTypeName()]) {
            visibleConstraints.push_back(constraint);
        }
    }
    
    for (size_t i = 0; i < visibleConstraints.size(); i++) {
        const auto& constraint = visibleConstraints[i];
        
        ImGui::PushID(static_cast<int>(i));
        
        // Constraint header with type and remove button
        ImGui::BeginGroup();
        ImGui::Text("%s Constraint", constraint->getTypeName().c_str());
        ImGui::SameLine();
        
        if (ImGui::Button("Remove")) {
            removeConstraint(constraint);
            modified = true;
            ImGui::PopID();
            continue;
        }
        
        // Constraint-specific parameters
        if (ImGui::TreeNode("Parameters")) {
            modified |= constraint->renderUI();
            ImGui::TreePop();
        }
        
        ImGui::EndGroup();
        ImGui::Separator();
        
        ImGui::PopID();
    }
    
    return modified;
}

void ConstraintManagerUI::renderConstraintFilters() {
    ImGui::Text("Show Constraint Types:");
    
    for (auto& [type, enabled] : m_constraintTypeFilters) {
        ImGui::Checkbox(type.c_str(), &enabled);
    }
}

} // namespace Constraints
} // namespace RebelCAD
