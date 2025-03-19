# Toolbar System

The RebelCAD Toolbar System provides a flexible and customizable interface for organizing commonly used tools and commands. It supports multiple toolbars, drag-and-drop customization, and persistent layout saving.

## Features

- Multiple toolbars (Standard, Drawing, Modeling, etc.)
- Customizable toolbar visibility
- Drag-and-drop toolbar customization
- Persistent layout saving
- Support for icons and tooltips
- Keyboard shortcuts

## Default Toolbars

### Standard Toolbar
- New File (Ctrl+N)
- Open (Ctrl+O)
- Save (Ctrl+S)
- Save All (Ctrl+Shift+S)
- Undo (Ctrl+Z)
- Redo (Ctrl+Y)

### Drawing Toolbar
- Line Tool (L)
- Circle Tool (C)
- Rectangle Tool (R)
- Arc Tool (A)
- Spline Tool (S)
- Polygon Tool (P)

### Modeling Toolbar
- Extrude Tool (E)
- Revolve Tool (V)
- Sweep Tool (W)
- Loft Tool
- Shell Tool
- Fillet Tool (F)
- Chamfer Tool

## Usage

### Accessing Toolbars

1. View > Toolbars menu shows/hides individual toolbars
2. View > Customize Toolbars... opens the customization dialog
3. Right-click any toolbar to access context menu options

### Customizing Toolbars

1. Open the Customize Toolbars dialog (View > Customize Toolbars...)
2. Select a toolbar from the left panel
3. Use checkboxes to show/hide individual tools
4. Drag tools to reorder them within a toolbar
5. Click "Save Layout" to persist your customizations

### Toolbar Layout Persistence

Toolbar layouts are saved in JSON format with the following structure:
```json
{
  "toolbars": [
    {
      "name": "Standard",
      "visible": true,
      "items": [
        {
          "id": "new",
          "visible": true,
          "enabled": true
        },
        // ... more items
      ]
    },
    // ... more toolbars
  ]
}
```

### Creating Custom Toolbars

Developers can create custom toolbars programmatically:

```cpp
// Create a new toolbar
toolbarManager->createToolbar("CustomToolbar");

// Add items to the toolbar
toolbarManager->addItemToToolbar("CustomToolbar", 
    std::make_shared<ToolbarItem>(
        "custom_tool",      // Unique identifier
        "Custom Tool",      // Display name
        "icons/custom.png", // Icon path
        []() {             // Action callback
            // Tool implementation
        }
    )
);
```

## Implementation Details

### ToolbarManager Class

The `ToolbarManager` class handles:
- Toolbar creation and management
- Item addition and removal
- Layout persistence
- Rendering and interaction
- Customization dialog

### Toolbar Items

Each toolbar item contains:
- Unique identifier
- Display name
- Icon path
- Action callback
- Visibility state
- Enabled state

### Integration with ImGui

The toolbar system uses Dear ImGui for rendering and interaction:
- Toolbars are rendered as horizontal button strips
- Customization dialog uses ImGui windows and widgets
- Drag-and-drop uses ImGui's built-in DND system

## Best Practices

1. **Toolbar Organization**
   - Group related tools together
   - Most used tools should be easily accessible
   - Consider screen space efficiency

2. **Icon Design**
   - Use consistent icon style
   - 32x32 pixels recommended size
   - Clear visual metaphors
   - High contrast for visibility

3. **Customization**
   - Allow users to show/hide toolbars
   - Enable tool reordering
   - Persist user preferences
   - Provide reset to defaults option

4. **Performance**
   - Lazy load icons
   - Cache toolbar layouts
   - Optimize rendering for large toolbars

## Error Handling

The toolbar system includes robust error handling:
- Invalid toolbar/item operations are handled gracefully
- Layout file corruption recovery
- Missing icon fallbacks
- Invalid action handling

## Future Enhancements

Planned improvements include:
- Vertical toolbar orientation
- Nested toolbars/submenus
- Tool search functionality
- Keyboard shortcut customization
- Toolbar position memory
- Context-sensitive toolbars
