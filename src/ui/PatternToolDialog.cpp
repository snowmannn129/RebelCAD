#include "../../include/ui/PatternToolDialog.hpp"
#include "../../include/ui/Events.hpp"
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace RebelCAD {
namespace UI {

PatternToolDialog::PatternToolDialog()
    : m_isVisible(false)
    , m_hasValidationError(false)
    , m_currentType(Modeling::PatternType::Linear)
    , m_includeOriginal(true)
    , m_instanceCount(2)
    , m_spacing(1.0f)
    , m_angle(360.0f)
{
    // Initialize vectors
    std::fill_n(m_direction, 3, 0.0f);
    m_direction[0] = 1.0f; // Default X direction
    
    std::fill_n(m_center, 3, 0.0f);
    
    std::fill_n(m_normal, 3, 0.0f);
    m_normal[2] = 1.0f; // Default Z normal
    
    std::fill_n(m_point, 3, 0.0f);
}

PatternToolDialog::~PatternToolDialog() {
    auto& eventBus = Core::EventBus::getInstance();
    eventBus.unsubscribe(m_previewUpdateSubscription);
    eventBus.unsubscribe(m_validationErrorSubscription);
}

void PatternToolDialog::initialize(std::shared_ptr<Modeling::PatternTool> patternTool) {
    m_patternTool = patternTool;
    
    // Register event handlers
    auto& eventBus = Core::EventBus::getInstance();
    
    m_previewUpdateSubscription = eventBus.subscribe<PreviewUpdateEvent>(
        [this](const PreviewUpdateEvent&, const Core::EventMetadata&) {
            onPreviewUpdate();
        }
    );
    
    m_validationErrorSubscription = eventBus.subscribe<ValidationErrorEvent>(
        [this](const ValidationErrorEvent& event, const Core::EventMetadata&) {
            onValidationError(event.message);
        }
    );
}

void PatternToolDialog::render() {
    if (!m_isVisible) return;

    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Pattern Tool", &m_isVisible)) {
        // Pattern type selection
        const char* patternTypes[] = { "Linear", "Circular", "Mirror" };
        int currentType = static_cast<int>(m_currentType);
        if (ImGui::Combo("Pattern Type", &currentType, patternTypes, IM_ARRAYSIZE(patternTypes))) {
            m_currentType = static_cast<Modeling::PatternType>(currentType);
            m_patternTool->initialize(m_currentType);
            updateParameters();
        }

        // Common parameters
        ImGui::Checkbox("Include Original", &m_includeOriginal);

        // Pattern-specific UI
        switch (m_currentType) {
            case Modeling::PatternType::Linear:
                renderLinearPatternUI();
                break;
            case Modeling::PatternType::Circular:
                renderCircularPatternUI();
                break;
            case Modeling::PatternType::Mirror:
                renderMirrorPatternUI();
                break;
        }

        // Error message
        renderErrorMessage();

        // Preview controls
        renderPreviewControls();

        ImGui::End();
    }
}

void PatternToolDialog::renderLinearPatternUI() {
    ImGui::InputInt("Instance Count", &m_instanceCount);
    m_instanceCount = std::max(2, m_instanceCount);
    
    ImGui::InputFloat("Spacing", &m_spacing);
    m_spacing = std::max(0.001f, m_spacing);
    
    if (ImGui::InputFloat3("Direction", m_direction)) {
        // Normalize direction vector
        glm::vec3 dir(m_direction[0], m_direction[1], m_direction[2]);
        if (glm::length(dir) > 0.0f) {
            dir = glm::normalize(dir);
            m_direction[0] = dir.x;
            m_direction[1] = dir.y;
            m_direction[2] = dir.z;
        }
    }
    
    updateParameters();
}

void PatternToolDialog::renderCircularPatternUI() {
    ImGui::InputInt("Instance Count", &m_instanceCount);
    m_instanceCount = std::max(2, m_instanceCount);
    
    ImGui::InputFloat("Total Angle", &m_angle);
    m_angle = std::clamp(m_angle, 0.0f, 360.0f);
    
    ImGui::InputFloat3("Center Point", m_center);
    
    if (ImGui::InputFloat3("Rotation Axis", m_normal)) {
        // Normalize axis vector
        glm::vec3 axis(m_normal[0], m_normal[1], m_normal[2]);
        if (glm::length(axis) > 0.0f) {
            axis = glm::normalize(axis);
            m_normal[0] = axis.x;
            m_normal[1] = axis.y;
            m_normal[2] = axis.z;
        }
    }
    
    updateParameters();
}

void PatternToolDialog::renderMirrorPatternUI() {
    if (ImGui::InputFloat3("Normal", m_normal)) {
        // Normalize normal vector
        glm::vec3 normal(m_normal[0], m_normal[1], m_normal[2]);
        if (glm::length(normal) > 0.0f) {
            normal = glm::normalize(normal);
            m_normal[0] = normal.x;
            m_normal[1] = normal.y;
            m_normal[2] = normal.z;
        }
    }
    
    ImGui::InputFloat3("Point on Plane", m_point);
    
    updateParameters();
}

void PatternToolDialog::renderPreviewControls() {
    ImGui::Separator();
    
    if (ImGui::Button("Apply")) {
        applyPattern();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Cancel")) {
        cancelOperation();
    }
}

void PatternToolDialog::renderErrorMessage() {
    if (m_hasValidationError) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::TextWrapped("%s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
    }
}

void PatternToolDialog::updateParameters() {
    if (!m_patternTool) return;

    std::shared_ptr<Modeling::PatternParameters> params;

    switch (m_currentType) {
        case Modeling::PatternType::Linear: {
            auto linearParams = std::make_shared<Modeling::LinearPatternParameters>();
            linearParams->direction = glm::vec3(m_direction[0], m_direction[1], m_direction[2]);
            linearParams->spacing = m_spacing;
            linearParams->count = m_instanceCount;
            linearParams->includeOriginal = m_includeOriginal;
            params = linearParams;
            break;
        }
        case Modeling::PatternType::Circular: {
            auto circularParams = std::make_shared<Modeling::CircularPatternParameters>();
            circularParams->axis = glm::vec3(m_normal[0], m_normal[1], m_normal[2]);
            circularParams->center = glm::vec3(m_center[0], m_center[1], m_center[2]);
            circularParams->angle = glm::radians(m_angle);
            circularParams->count = m_instanceCount;
            circularParams->includeOriginal = m_includeOriginal;
            params = circularParams;
            break;
        }
        case Modeling::PatternType::Mirror: {
            auto mirrorParams = std::make_shared<Modeling::MirrorPatternParameters>();
            mirrorParams->normal = glm::vec3(m_normal[0], m_normal[1], m_normal[2]);
            mirrorParams->point = glm::vec3(m_point[0], m_point[1], m_point[2]);
            mirrorParams->includeOriginal = m_includeOriginal;
            params = mirrorParams;
            break;
        }
    }

    m_patternTool->setParameters(params);
    m_patternTool->generatePreview();
}

void PatternToolDialog::applyPattern() {
    if (m_patternTool && m_patternTool->apply()) {
        hide();
    }
}

void PatternToolDialog::cancelOperation() {
    if (m_patternTool) {
        m_patternTool->cancel();
    }
    hide();
}

void PatternToolDialog::onPreviewUpdate() {
    m_hasValidationError = false;
    m_errorMessage.clear();
}

void PatternToolDialog::onValidationError(const std::string& message) {
    m_hasValidationError = true;
    m_errorMessage = message;
}

} // namespace UI
} // namespace RebelCAD
