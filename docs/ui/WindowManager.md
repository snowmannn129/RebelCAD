# Window Manager

The WindowManager class provides multi-window support for RebelCAD, allowing users to work with multiple views and documents simultaneously. It manages window creation, destruction, and state tracking while ensuring proper coordination between windows.

## Features

- Multiple window support with independent views
- Window state management (create, destroy, activate)
- Automatic handling of window closure
- Active window tracking
- Coordinated rendering across all windows

## Usage

### Creating Windows

```cpp
auto& manager = WindowManager::getInstance();
auto window = manager.createWindow("My Window", 1280, 720);
```

### Managing Windows

```cpp
// Get all windows
auto windows = manager.getWindows();

// Get active window
auto activeWindow = manager.getActiveWindow();

// Get window count
size_t count = manager.getWindowCount();

// Destroy specific window
manager.destroyWindow(window);

// Destroy all windows
manager.destroyAllWindows();
```

### Window Updates and Rendering

```cpp
// Update all windows (handles window closure, focus changes)
manager.updateAll();

// Render all windows
manager.renderAll();
```

## Implementation Details

### Singleton Pattern

The WindowManager is implemented as a singleton to ensure a single point of control for all window operations:

```cpp
WindowManager& manager = WindowManager::getInstance();
```

### Window Lifecycle

1. **Creation**: Windows are created with a title and optional dimensions. The first created window automatically becomes the active window.

2. **Active Window**: Only one window can be active at a time. The active window is automatically updated when:
   - A new window is created (if it's the first window)
   - The active window is destroyed (next available window becomes active)
   - A window gains focus

3. **Destruction**: Windows can be destroyed individually or all at once. The WindowManager handles proper cleanup and active window management.

### Memory Management

- Windows are managed using `std::shared_ptr` for automatic memory management
- The WindowManager maintains a list of all windows and tracks their state
- Closed windows are automatically removed during updates

### ImGui Integration

The WindowManager integrates with Dear ImGui for:
- Window focus detection
- Multi-viewport support
- Docking capabilities

## Best Practices

1. **Window Creation**
   - Always check if window creation was successful
   - Provide meaningful window titles
   - Consider screen resolution when specifying dimensions

2. **Window Management**
   - Regularly call `updateAll()` to handle window state changes
   - Call `renderAll()` in your render loop
   - Clean up windows when they're no longer needed

3. **Error Handling**
   - Handle null window pointers
   - Check window existence before operations
   - Monitor window creation failures

## Example Workflow

```cpp
// Get the window manager
auto& manager = WindowManager::getInstance();

// Create main window
auto mainWindow = manager.createWindow("Main View");
if (!mainWindow) {
    // Handle creation failure
    return;
}

// Create additional views as needed
auto sideView = manager.createWindow("Side View", 800, 600);
auto topView = manager.createWindow("Top View", 800, 600);

// Main application loop
while (!mainWindow->shouldClose()) {
    // Update all windows
    manager.updateAll();
    
    // Render all windows
    manager.renderAll();
}

// Cleanup
manager.destroyAllWindows();
```

## Future Enhancements

- Window layout persistence
- Custom window behaviors and event handling
- Enhanced window synchronization
- Window grouping and relationships
- Additional window states and modes

## See Also

- [MainWindow Documentation](MainWindow.md)
- [UI System Overview](../ui/README.md)
- [ImGui Integration](../graphics/ImGuiIntegration.md)
