# Workspace Manager

The WorkspaceManager is responsible for saving and loading UI workspace configurations in RebelCAD. It allows users to persist their preferred UI layouts and restore them later.

## Features

- Save current workspace configuration to a file
- Load workspace configuration from a file
- List available workspace configurations
- Delete saved workspace configurations
- Validation of workspace names
- Error handling for file operations

## Usage

### Creating a Workspace Manager

```cpp
auto window_manager = std::make_shared<WindowManager>();
auto docking_manager = std::make_shared<DockingManager>();
auto toolbar_manager = std::make_shared<ToolbarManager>();

auto workspace_manager = std::make_unique<WorkspaceManager>(
    window_manager,
    docking_manager,
    toolbar_manager
);
```

### Saving a Workspace

```cpp
// Save current UI configuration
auto result = workspace_manager->SaveWorkspace("my_workspace");
if (!result.IsSuccess()) {
    // Handle error
    std::cerr << "Failed to save workspace: " << result.GetMessage() << std::endl;
}
```

### Loading a Workspace

```cpp
// Load a saved configuration
auto result = workspace_manager->LoadWorkspace("my_workspace");
if (!result.IsSuccess()) {
    // Handle error
    std::cerr << "Failed to load workspace: " << result.GetMessage() << std::endl;
}
```

### Managing Workspaces

```cpp
// List available workspaces
auto workspaces = workspace_manager->GetAvailableWorkspaces();
for (const auto& workspace : workspaces) {
    std::cout << "Found workspace: " << workspace << std::endl;
}

// Delete a workspace
auto result = workspace_manager->DeleteWorkspace("old_workspace");
if (!result.IsSuccess()) {
    std::cerr << "Failed to delete workspace: " << result.GetMessage() << std::endl;
}
```

## File Format

Workspaces are stored as JSON files in the `workspaces` directory. Each file contains:

- Window positions and sizes
- Docking layout configuration
- Toolbar positions and visibility states
- Version information
- Timestamp

Example workspace file structure:
```json
{
    "version": "1.0",
    "timestamp": 1645430400000,
    "windows": {
        // Window manager state
    },
    "docking": {
        // Docking manager state
    },
    "toolbars": {
        // Toolbar manager state
    }
}
```

## Error Handling

The WorkspaceManager uses the Error class to report operation results. Common errors include:

- Invalid workspace names (empty or containing invalid characters)
- File I/O errors
- Missing workspace files
- Invalid workspace file format
- Version mismatches

## Implementation Details

### Workspace Names

- Must not be empty
- Can only contain letters, numbers, underscores, and hyphens
- Case-sensitive
- Stored as .json files in the workspaces directory

### State Management

The WorkspaceManager coordinates with three UI managers:
1. WindowManager - Handles window positions and sizes
2. DockingManager - Manages docking layout configuration
3. ToolbarManager - Controls toolbar positions and states

Each manager implements SerializeState() and DeserializeState() methods to handle their specific configuration data.

### File Operations

- Uses std::filesystem for file management
- Implements automatic workspace directory creation
- Handles file read/write operations with proper error checking
- Uses nlohmann::json for JSON serialization/deserialization

## Best Practices

1. Always check operation results using the returned Error object
2. Use descriptive workspace names
3. Implement proper error handling in the calling code
4. Clean up unused workspaces to prevent clutter
5. Consider implementing workspace auto-save functionality for crash recovery

## Drag & Drop Support

The WorkspaceManager now includes comprehensive drag & drop functionality for UI elements, enabling dynamic workspace configuration through user interaction.

### DraggableElement Structure

```cpp
struct DraggableElement {
    int id;                     // Unique identifier for the element
    std::string type;          // Type of UI element (window, toolbar, panel)
    float x, y;                // Current position
    float width, height;       // Current dimensions
    bool isDocked;             // Whether the element is currently docked
    std::string dockLocation;  // Dock location if docked
};
```

### Basic Drag & Drop Operations

```cpp
// Start dragging a UI element
DraggableElement element{
    .id = 1,
    .type = "window",
    .x = 100.0f,
    .y = 100.0f,
    .width = 200.0f,
    .height = 150.0f,
    .isDocked = false
};
auto result = workspace_manager->BeginDrag(element);

// Update position during drag
workspace_manager->UpdateDragPosition(200.0f, 150.0f);

// End drag operation with target location
DraggableElement target{
    .id = 2,
    .type = "window",
    .x = 200.0f,
    .y = 150.0f,
    .isDocked = true,
    .dockLocation = "right"
};
workspace_manager->EndDrag(target);
```

### Drag & Drop Callbacks

Register a callback to handle drag & drop events:

```cpp
workspace_manager->SetDragDropCallback(
    [](const DraggableElement& source, const DraggableElement& target) {
        // Handle drag & drop completion
        std::cout << "Dropped element " << source.id 
                  << " onto target " << target.id << std::endl;
    }
);
```

### Docking Support

Elements can be docked to specific locations:

```cpp
DraggableElement panel{
    .id = 1,
    .type = "panel",
    .isDocked = true,
    .dockLocation = "left"  // left, right, top, bottom
};
```

### Error Handling

Common drag & drop errors to handle:

- Attempting to start a drag while another is in progress
- Updating or ending a drag when none is active
- Invalid drop target locations
- Docking operation failures

```cpp
auto result = workspace_manager->BeginDrag(element);
if (!result.IsSuccess()) {
    // Handle error
    std::cerr << "Failed to start drag: " << result.GetMessage() << std::endl;
}
```

### State Persistence

Drag & drop state is automatically included in workspace serialization:

```json
{
    "version": "1.0",
    "timestamp": 1645430400000,
    "windows": { },
    "docking": { },
    "toolbars": { },
    "drag_state": {
        "element_id": 1,
        "element_type": "window",
        "position_x": 100.0,
        "position_y": 100.0,
        "is_docked": false,
        "dock_location": ""
    }
}
```

## Future Enhancements

Potential improvements to consider:

- Workspace categories/tags
- Workspace import/export
- Workspace templates
- Auto-save functionality
- Workspace previews
- Multi-monitor workspace support
- Cloud sync integration
- Advanced drag & drop previews
- Snap-to-grid functionality
- Custom docking zones
- Drag & drop gesture recognition
