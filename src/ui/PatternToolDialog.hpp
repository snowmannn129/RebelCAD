#pragma once

#include <memory>
#include <string>
#include "../modeling/PatternTool.hpp"
#include "../core/EventBus.h"
#include "Events.hpp"

namespace RebelCAD {
namespace UI {

class PatternToolDialog {
public:
    PatternToolDialog();
    ~PatternToolDialog();

    // Initialize dialog with pattern tool instance
    void initialize(std::shared_ptr<Modeling::PatternTool> patternTool);

    // Render dialog UI
    void render();

    // Show/hide dialog
    void show() { m_isVisible = true; }
    void hide() { m_isVisible = false; }
    bool isVisible() const { return m_isVisible; }

private:
    // UI rendering methods for different pattern types
    void renderLinearPatternUI();
    void renderCircularPatternUI();
    void renderMirrorPatternUI();

    // Helper methods
    void renderPreviewControls();
    void renderErrorMessage();
    void updateParameters();
    void applyPattern();

protected:
    // Made protected for testing
    void cancelOperation();
    void onPreviewUpdate();
    void onValidationError(const std::string& message);

    // Friend declaration for test class
    friend class PatternToolDialogTests;

    // Member variables
    std::shared_ptr<Modeling::PatternTool> m_patternTool;
    Modeling::PatternType m_currentType;
    bool m_isVisible;
    bool m_hasValidationError;
    std::string m_errorMessage;

    // Pattern parameters
    union {
        Modeling::LinearPatternParameters m_linearParams;
        Modeling::CircularPatternParameters m_circularParams;
        Modeling::MirrorPatternParameters m_mirrorParams;
    };

    // Event system integration
    size_t m_previewUpdateSubscription;
    size_t m_validationErrorSubscription;

    // UI state
    bool m_includeOriginal;
    int m_instanceCount;
    float m_spacing;
    float m_angle;
    float m_direction[3];
    float m_center[3];
    float m_normal[3];
    float m_point[3];
};

} // namespace UI
} // namespace RebelCAD
