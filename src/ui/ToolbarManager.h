#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

namespace RebelCAD {
namespace UI {

// Forward declarations
class MainWindow;

// Represents a single toolbar button/action
struct ToolbarItem {
    std::string id;              // Unique identifier
    std::string name;            // Display name
    std::string iconPath;        // Path to icon texture
    std::function<void()> action;// Callback when clicked
    bool enabled;                // Whether the item is enabled
    bool visible;                // Whether the item is visible
    
    ToolbarItem(const std::string& id, 
                const std::string& name,
                const std::string& iconPath,
                std::function<void()> action)
        : id(id), name(name), iconPath(iconPath), action(action),
          enabled(true), visible(true) {}
};

// Represents a toolbar that can contain multiple items
class Toolbar {
public:
    Toolbar(const std::string& name, bool visible = true)
        : m_Name(name), m_Visible(visible) {}

    void addItem(std::shared_ptr<ToolbarItem> item);
    void removeItem(const std::string& itemId);
    void setVisible(bool visible) { m_Visible = visible; }
    bool isVisible() const { return m_Visible; }
    const std::string& getName() const { return m_Name; }
    const std::vector<std::shared_ptr<ToolbarItem>>& getItems() const { return m_Items; }

private:
    std::string m_Name;
    bool m_Visible;
    std::vector<std::shared_ptr<ToolbarItem>> m_Items;
};

class ToolbarManager {
public:
    explicit ToolbarManager(MainWindow* mainWindow);
    ~ToolbarManager() = default;

    // Toolbar management
    void createToolbar(const std::string& name);
    void removeToolbar(const std::string& name);
    void addItemToToolbar(const std::string& toolbarName, std::shared_ptr<ToolbarItem> item);
    void removeItemFromToolbar(const std::string& toolbarName, const std::string& itemId);
    
    // Customization
    void saveLayout(const std::string& filename);
    void loadLayout(const std::string& filename);
    
    // Rendering
    void render();
    void renderCustomizationDialog();

private:
    MainWindow* m_MainWindow;
    std::unordered_map<std::string, std::shared_ptr<Toolbar>> m_Toolbars;
    bool m_ShowCustomizationDialog;

    // Helper functions
    void setupDefaultToolbars();
    void serializeLayout(const std::string& filename);
    void deserializeLayout(const std::string& filename);
};

} // namespace UI
} // namespace RebelCAD
