#include <gtest/gtest.h>
#include <memory>
#include "../../include/ui/PatternToolDialog.hpp"
#include "../../include/modeling/PatternTool.hpp"

using namespace RebelCAD;
using namespace RebelCAD::UI;
using namespace RebelCAD::Modeling;

class PatternToolDialogTests : public ::testing::Test {
protected:
    void SetUp() override {
        m_patternTool = std::make_shared<PatternTool>();
        m_dialog = std::make_unique<PatternToolDialog>();
        m_dialog->initialize(m_patternTool);
    }

    std::shared_ptr<PatternTool> m_patternTool;
    std::unique_ptr<PatternToolDialog> m_dialog;
};

TEST_F(PatternToolDialogTests, InitializationTest) {
    EXPECT_FALSE(m_dialog->isVisible());
}

TEST_F(PatternToolDialogTests, ShowHideTest) {
    m_dialog->show();
    EXPECT_TRUE(m_dialog->isVisible());
    
    m_dialog->hide();
    EXPECT_FALSE(m_dialog->isVisible());
}

TEST_F(PatternToolDialogTests, ErrorHandlingTest) {
    // Simulate an error event
    ErrorEvent errorEvent("Test error message");
    EventSystem::getInstance().dispatchEvent(errorEvent);
    
    // Note: In a real test environment, we would need a way to check the error message
    // is displayed in the UI. For now, we just verify the dialog remains functional
    EXPECT_FALSE(m_dialog->isVisible());
    m_dialog->show();
    EXPECT_TRUE(m_dialog->isVisible());
}

TEST_F(PatternToolDialogTests, PreviewUpdateTest) {
    // Simulate a preview update event
    PreviewUpdateEvent updateEvent;
    EventSystem::getInstance().dispatchEvent(updateEvent);
    
    // Dialog should remain in its current state
    EXPECT_FALSE(m_dialog->isVisible());
    m_dialog->show();
    EXPECT_TRUE(m_dialog->isVisible());
}

TEST_F(PatternToolDialogTests, CancelOperationTest) {
    m_dialog->show();
    EXPECT_TRUE(m_dialog->isVisible());
    
    // Simulate cancel operation
    m_dialog->cancelOperation();
    EXPECT_FALSE(m_dialog->isVisible());
}

// Note: Additional UI interaction tests would typically be done with a UI testing framework
// For now, we focus on testing the dialog's core functionality and event handling
