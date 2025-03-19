# Context Menu System

The RebelCAD Context Menu System provides a flexible and extensible way to add right-click context menus throughout the application. This document describes how to use and extend the context menu functionality.

## Overview

The context menu system is built around the `ContextMenuManager` singleton class, which manages the registration and display of context menus for different parts of the application.

## Key Components

### MenuPosition

A structure representing a position for context menus:

```cpp
struct MenuPosition {
    float x;                    // X coordinate
    float y;                    // Y coordinate
    bool isValid() const;       // Checks if position is valid
};
```

### ContextMenuItem

A structure representing a single menu item in a context menu:

```cpp
struct ContextMenuItem {
    std::string label;          // Display text
    std::string shortcut;       // Keyboard shortcut hint (e.g., "Ctrl+C")
    std::function<void()> callback;  // Action to perform when clicked
    bool enabled;               // Whether the item is clickable
    bool separator;             // Whether this is a separator line
    std::vector<ContextMenuItem> subItems;  // Nested menu items
};
```

### ContextMenuManager

The main class managing all context menus:

```cpp
class ContextMenuManager {
public:
    static ContextMenuManager& getInstance();
    void registerContextMenu(const std::string& context, const std::vector<ContextMenuItem>& items);
    void showContextMenu(const std::string& context, const MenuPosition& pos = MenuPosition());
    void showRightClickMenu(const std::string& context);
    bool hasContextMenu(const std::string& context) const;
    void clearContextMenus();
};
```

## Usage Examples

### 1. Registering a Menu with Keyboard Shortcuts

```cpp
std::vector<ContextMenuItem> items = {
    ContextMenuItem("Copy", []() { /* Handle copy */ }, "Ctrl+C"),
    ContextMenuItem("Paste", []() { /* Handle paste */ }, "Ctrl+V")
};

contextManager.registerContextMenu("editContext", items);
```

### 2. Showing a Menu at a Specific Position

```cpp
MenuPosition pos(100.0f, 100.0f);
contextManager.showContextMenu("myContext", pos);
```

### 3. Registering a Simple Context Menu

```cpp
auto& contextManager = ContextMenuManager::getInstance();

std::vector<ContextMenuItem> items = {
    ContextMenuItem("Edit", []() { /* Handle edit action */ }),
    ContextMenuItem("Delete", []() { /* Handle delete action */ })
};

contextManager.registerContextMenu("myContext", items);
```

### 4. Creating a Menu with Separators

```cpp
std::vector<ContextMenuItem> items = {
    ContextMenuItem("Copy", []() { /* Handle copy */ }),
    ContextMenuItem("Cut", []() { /* Handle cut */ }),
    ContextMenuItem::Separator(),
    ContextMenuItem("Delete", []() { /* Handle delete */ })
};
```

### 5. Creating Nested Submenus

```cpp
ContextMenuItem subItem("Submenu Item", []() { /* Handle action */ });
ContextMenuItem mainItem("Main Item", []() {});
mainItem.subItems.push_back(subItem);

std::vector<ContextMenuItem> items = { mainItem };
```

### 6. Disabled Menu Items

```cpp
std::vector<ContextMenuItem> items = {
    ContextMenuItem("Enabled Item", []() { /* Handle action */ }, true),
    ContextMenuItem("Disabled Item", []() {}, false)
};
```

## Integration with MainWindow

The context menu system is integrated into the MainWindow class through two main methods:

1. `setupContextMenus()`: Initializes default context menus
2. `handleContextMenus()`: Handles right-click detection and menu display

## Default Context Menus

RebelCAD provides several default context menus:

### Sketch Context Menu
- Edit Sketch
- Delete Sketch
- Add Constraint
- Show/Hide

### 3D Model Context Menu
- Edit Feature
- Suppress Feature
- Hide/Show
- Properties

### Panel Context Menu
- Float
- Dock
- Close

## Best Practices

1. **Context Naming**: Use clear, descriptive names for context identifiers (e.g., "sketch", "3d_model", "panel").

2. **Menu Organization**: 
   - Group related items together
   - Use separators to create logical sections
   - Keep menus concise and focused

3. **Callbacks**:
   - Keep callback functions small and focused
   - Consider using command pattern for complex operations
   - Handle errors appropriately within callbacks

4. **Dynamic Menus**:
   - Update menu item enabled/disabled state based on current context
   - Consider using lambda captures to access relevant state
   - Re-register menus when their items need to change

## Error Handling

The context menu system includes built-in error handling:

- Invalid context names are logged with warnings
- Failed callback executions are caught and logged
- Menu registration failures are reported through exceptions

## Performance Considerations

1. **Menu Registration**:
   - Register menus during initialization when possible
   - Avoid frequent re-registration of static menus
   - Use dynamic registration only when menu content needs to change

2. **Callback Performance**:
   - Keep callbacks lightweight
   - Defer heavy processing to separate threads when necessary
   - Cache frequently accessed data

## Future Enhancements

Planned improvements to the context menu system include:

1. Custom styling support
2. Animation effects
3. Context-sensitive icons
4. Undo/redo support for menu actions

## Contributing

When adding new features to the context menu system:

1. Follow the existing naming conventions
2. Add appropriate unit tests
3. Update this documentation
4. Consider backward compatibility
