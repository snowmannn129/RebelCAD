# RebelCAD Docking System

The RebelCAD docking system provides a flexible and user-friendly interface for managing panels and workspaces. Built on top of Dear ImGui's docking functionality, it allows for dynamic panel arrangement, customizable layouts, and persistent workspace configurations.

## Features

- Dockable panels that can be arranged freely
- Support for floating windows
- Customizable workspace layouts
- Panel visibility management
- Layout saving and loading
- Default panel configurations

## Default Panels

The following panels are provided by default:

- **Properties Panel**: Displays and edits properties of selected objects
- **Feature Tree**: Shows the hierarchical structure of the model
- **Console**: Displays system messages and command output
- **Layer Manager**: Manages visibility and properties of layers
- **Toolbox**: Quick access to commonly used tools

## Using the Docking System

### Panel Management

Panels can be manipulated in several ways:

1. **Dragging**: Click and drag panel headers to dock them in different locations
2. **Floating**: Double-click panel headers to make them float
3. **Resizing**: Drag panel edges to resize
4. **Visibility**: Toggle panel visibility through the View > Panels menu

### Workspace Management

Workspaces can be managed through the View > Workspaces menu:

- **Default**: Standard layout optimized for general use
- **Modeling**: Layout focused on 3D modeling tools
- **Assembly**: Layout optimized for assembly work
- **Drafting**: Layout configured for 2D drafting

### Saving/Loading Layouts

1. Arrange panels as desired
2. Go to View > Workspaces > Save Current Layout
3. Enter a name for the layout
4. Load saved layouts through View > Workspaces menu

## Creating Custom Panels

To create a custom panel, inherit from the `Panel` base class:

```cpp
class CustomPanel : public UI::Panel {
public:
    CustomPanel(const std::string& title) : Panel(title) {}
    
    void render() override {
        // Implement panel rendering here using ImGui
        ImGui::Text("Custom Panel Content");
        // Add widgets, controls, etc.
    }
};
```

Register the panel with the docking system:

```cpp
auto customPanel = std::make_shared<CustomPanel>("My Panel");
mainWindow.registerPanel("myPanel", customPanel);
```

## Default Layout

The default layout arranges panels as follows:

```
+----------------+------------------+
|                |                 |
| Feature Tree   |   Properties    |
|                |                 |
+----------------+   Main View     |
|                |                 |
|   Toolbox      |                 |
|                |                 |
+----------------+-----------------+
|           Console               |
+--------------------------------+
```

## Technical Details

### DockingManager

The `DockingManager` class handles:

- Panel registration and management
- Layout serialization/deserialization
- Dock node management
- Panel visibility state
- Default layout configuration

### Panel Base Class

The `Panel` base class provides:

- Virtual render method for panel content
- Visibility management
- Title and state handling

### Integration with ImGui

The docking system integrates with ImGui's native docking features:

- Uses ImGui docking branch
- Supports multi-viewport
- Handles window flags and styling
- Manages dock node hierarchy

## Best Practices

1. **Panel Design**
   - Keep panels modular and focused
   - Use consistent styling
   - Handle resize events gracefully
   - Implement proper cleanup

2. **Layout Management**
   - Save layouts for different workflows
   - Use meaningful layout names
   - Consider monitor configurations
   - Maintain reasonable default sizes

3. **Performance**
   - Optimize panel rendering
   - Cache frequently used data
   - Handle large datasets efficiently
   - Use appropriate update frequencies

## Example Usage

```cpp
// Creating a custom panel
class ModelInfoPanel : public UI::Panel {
public:
    ModelInfoPanel() : Panel("Model Info") {}
    
    void render() override {
        ImGui::Text("Model Statistics");
        ImGui::Separator();
        ImGui::Text("Vertices: %d", vertexCount);
        ImGui::Text("Faces: %d", faceCount);
        ImGui::Text("Materials: %d", materialCount);
    }
private:
    int vertexCount = 0;
    int faceCount = 0;
    int materialCount = 0;
};

// Registering the panel
auto modelInfo = std::make_shared<ModelInfoPanel>();
mainWindow.registerPanel("modelInfo", modelInfo);
```

## Troubleshooting

Common issues and solutions:

1. **Panel Not Visible**
   - Check panel registration
   - Verify visibility state
   - Ensure proper initialization

2. **Layout Not Saving**
   - Check write permissions
   - Verify layout serialization
   - Ensure unique layout names

3. **Docking Issues**
   - Verify ImGui docking enabled
   - Check window flags
   - Ensure proper viewport setup

## Future Enhancements

Planned improvements:

- Layout import/export
- Panel templates
- Advanced workspace management
- Multi-monitor optimization
- Custom docking zones
- Layout presets for different disciplines
