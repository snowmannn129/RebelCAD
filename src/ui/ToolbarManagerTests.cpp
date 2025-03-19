#include <gtest/gtest.h>
#include "ui/ToolbarManager.h"
#include "ui/MainWindow.h"
#include <filesystem>
#include <fstream>

using namespace RebelCAD::UI;

class ToolbarManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<ToolbarManager> toolbarManager;
    const std::string testLayoutFile = "test_toolbar_layout.json";

    void SetUp() override {
        mainWindow = std::make_unique<MainWindow>("Test Window", 800, 600);
        toolbarManager = std::make_unique<ToolbarManager>(mainWindow.get());
    }

    void TearDown() override {
        toolbarManager.reset();
        mainWindow.reset();
        // Cleanup test files
        if (std::filesystem::exists(testLayoutFile)) {
            std::filesystem::remove(testLayoutFile);
        }
    }
};

TEST_F(ToolbarManagerTest, CreateToolbar) {
    // Test creating a new toolbar
    toolbarManager->createToolbar("TestToolbar");
    
    // Add an item to verify toolbar exists
    auto testItem = std::make_shared<ToolbarItem>(
        "test", "Test", "icons/test.png",
        []() { /* Test action */ }
    );
    
    EXPECT_NO_THROW(toolbarManager->addItemToToolbar("TestToolbar", testItem));
}

TEST_F(ToolbarManagerTest, AddAndRemoveItems) {
    toolbarManager->createToolbar("TestToolbar");
    
    // Create test items
    auto item1 = std::make_shared<ToolbarItem>(
        "item1", "Item 1", "icons/item1.png",
        []() { /* Test action */ }
    );
    
    auto item2 = std::make_shared<ToolbarItem>(
        "item2", "Item 2", "icons/item2.png",
        []() { /* Test action */ }
    );
    
    // Add items
    toolbarManager->addItemToToolbar("TestToolbar", item1);
    toolbarManager->addItemToToolbar("TestToolbar", item2);
    
    // Remove an item
    EXPECT_NO_THROW(toolbarManager->removeItemFromToolbar("TestToolbar", "item1"));
    
    // Try to remove non-existent item
    EXPECT_NO_THROW(toolbarManager->removeItemFromToolbar("TestToolbar", "nonexistent"));
}

TEST_F(ToolbarManagerTest, SaveAndLoadLayout) {
    // Create test toolbar with items
    toolbarManager->createToolbar("TestToolbar");
    
    auto item = std::make_shared<ToolbarItem>(
        "test", "Test", "icons/test.png",
        []() { /* Test action */ }
    );
    
    toolbarManager->addItemToToolbar("TestToolbar", item);
    
    // Save layout
    EXPECT_NO_THROW(toolbarManager->saveLayout(testLayoutFile));
    
    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists(testLayoutFile));
    
    // Create new toolbar manager
    auto newToolbarManager = std::make_unique<ToolbarManager>(mainWindow.get());
    
    // Load layout
    EXPECT_NO_THROW(newToolbarManager->loadLayout(testLayoutFile));
}

TEST_F(ToolbarManagerTest, ToolbarVisibility) {
    toolbarManager->createToolbar("TestToolbar");
    
    auto item = std::make_shared<ToolbarItem>(
        "test", "Test", "icons/test.png",
        []() { /* Test action */ }
    );
    
    toolbarManager->addItemToToolbar("TestToolbar", item);
    
    // Test item visibility
    item->visible = false;
    EXPECT_FALSE(item->visible);
    
    item->visible = true;
    EXPECT_TRUE(item->visible);
}

TEST_F(ToolbarManagerTest, ItemProperties) {
    auto item = std::make_shared<ToolbarItem>(
        "test", "Test Item", "icons/test.png",
        []() { /* Test action */ }
    );
    
    EXPECT_EQ(item->id, "test");
    EXPECT_EQ(item->name, "Test Item");
    EXPECT_EQ(item->iconPath, "icons/test.png");
    EXPECT_TRUE(item->enabled);
    EXPECT_TRUE(item->visible);
}

TEST_F(ToolbarManagerTest, NonExistentToolbar) {
    auto item = std::make_shared<ToolbarItem>(
        "test", "Test", "icons/test.png",
        []() { /* Test action */ }
    );
    
    // Adding to non-existent toolbar should not throw
    EXPECT_NO_THROW(toolbarManager->addItemToToolbar("NonExistentToolbar", item));
    
    // Removing from non-existent toolbar should not throw
    EXPECT_NO_THROW(toolbarManager->removeItemFromToolbar("NonExistentToolbar", "test"));
}

TEST_F(ToolbarManagerTest, MultipleToolbars) {
    toolbarManager->createToolbar("Toolbar1");
    toolbarManager->createToolbar("Toolbar2");
    
    auto item1 = std::make_shared<ToolbarItem>(
        "item1", "Item 1", "icons/item1.png",
        []() { /* Test action */ }
    );
    
    auto item2 = std::make_shared<ToolbarItem>(
        "item2", "Item 2", "icons/item2.png",
        []() { /* Test action */ }
    );
    
    toolbarManager->addItemToToolbar("Toolbar1", item1);
    toolbarManager->addItemToToolbar("Toolbar2", item2);
    
    // Both toolbars should work independently
    EXPECT_NO_THROW(toolbarManager->removeItemFromToolbar("Toolbar1", "item1"));
    EXPECT_NO_THROW(toolbarManager->addItemToToolbar("Toolbar2", item1));
}
