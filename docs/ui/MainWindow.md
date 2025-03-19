# MainWindow Class Documentation

## Overview

The MainWindow class is the core window management component of RebelCAD. It provides a modern, dockable interface using Dear ImGui, with support for multiple viewports and a customizable workspace layout.

## Features

- Dockable interface with support for multiple viewports
- Main menu bar with standard CAD application options
- Dark theme by default
- Window management (size, title, etc.)
- Integration with RebelCAD's graphics system

## Usage

### Basic Implementation

```cpp
#include "ui/MainWindow.h"

int main() {
    RebelCAD::UI::MainWindow window("RebelCAD", 1280, 720);
    
    try {
        window.initialize();
        
        while (!window.shouldClose()) {
            window.render();
        }
        
        window.shutdown();
    }
    catch (const RebelCAD::Error& e) {
        // Handle error
    }
    
    return 0;
}
```

### Window Management

```cpp
// Change window size
window.setSize(1920, 1080);

// Change window title
window.setTitle("RebelCAD - My Project");

// Check if window should close
if (window.shouldClose()) {
    // Perform cleanup
}
```

## Class Methods

### Constructor
```cpp
MainWindow(const std::string& title = "RebelCAD", int width = 1280, int height = 720)
```
Creates a new MainWindow instance with the specified title and dimensions.

### Core Functions

#### `void initialize()`
Initializes the window, graphics system, and ImGui context. Must be called before any other operations.

#### `void render()`
Renders a new frame, including the main menu bar and dockable interface.

#### `void shutdown()`
Cleans up resources and shuts down the window. Called automatically by destructor.

#### `bool shouldClose() const`
Returns true if the window should close (e.g., user clicked close button or pressed Alt+F4).

### Property Accessors

#### `int getWidth() const`
Returns the current window width.

#### `int getHeight() const`
Returns the current window height.

#### `const std::string& getTitle() const`
Returns the current window title.

### Property Mutators

#### `void setSize(int width, int height)`
Sets the window dimensions.

#### `void setTitle(const std::string& title)`
Sets the window title.

## Implementation Details

### Docking System
The MainWindow implements a dockable interface system using ImGui's docking feature. This allows for:
- Floating windows
- Tab-based document management
- Customizable workspace layouts
- Multi-viewport support

### Menu System
The main menu bar provides a comprehensive set of CAD application options:

#### File Menu
- **New**: Create new projects, parts, or assemblies (Ctrl+N, Ctrl+Shift+N, Ctrl+Alt+N)
- **Open/Save**: File management operations (Ctrl+O, Ctrl+S)
- **Import/Export**: Support for industry-standard formats:
  - STEP
  - IGES
  - STL
  - DXF

#### Edit Menu
- Standard operations (Undo, Redo, Cut, Copy, Paste)
- Selection management (Select All, Deselect All)
- Keyboard shortcuts follow industry standards

#### View Menu
- **Workspaces**: Predefined layouts for different tasks
  - Default
  - Modeling
  - Assembly
  - Drafting
- **Toolbars**: Toggle visibility of tool panels
  - Standard
  - Sketch
  - Features
  - Assembly
- **Windows**: Manage auxiliary panels
  - Properties
  - Feature Tree
  - Layer Manager
- **Display**: View modes
  - Wireframe
  - Hidden Line
  - Shaded
  - Realistic

#### Sketch Menu
- Create/Edit sketches
- **Tools**: Basic sketch elements
  - Line (L)
  - Circle (C)
  - Arc (A)
  - Rectangle (R)
  - Polygon (P)
  - Spline (S)
- **Constraints**: Geometric relationships
  - Horizontal/Vertical
  - Parallel/Perpendicular
  - Coincident/Concentric
  - Equal

#### Model Menu
- **Features**: 3D modeling operations
  - Extrude
  - Revolve
  - Sweep
  - Loft
  - Shell
  - Fillet
  - Chamfer
- **Surface**: Advanced surface modeling
  - Create Plane
  - Extend/Trim Surface
  - Offset Surface
- **Pattern**: Feature multiplication
  - Linear Pattern
  - Circular Pattern
  - Mirror

#### Assembly Menu
- Component management
- **Constraints**: Assembly relationships
  - Mate
  - Align
  - Angle
  - Distance
- Pattern/Mirror operations
- Exploded view generation

#### Analysis Menu
- Mass Properties
- Interference Check
- **Simulation**:
  - Static Analysis
  - Dynamic Analysis
  - Thermal Analysis

#### Tools Menu
- Application options and customization
- **Macros**: Automation support
  - Record/Stop Recording
  - Edit Macros
- Plugin management

#### Help Menu
- Documentation access
- Tutorials
- Sample Models
- Updates
- About information

### Menu Implementation
The menu system is implemented using Dear ImGui's menu API, providing:
- Consistent styling with the rest of the application
- Native-looking menus across platforms
- Keyboard shortcut support
- Context-sensitive enabling/disabling of items
- Submenus for organizing related commands

### Graphics Integration
The MainWindow class integrates with RebelCAD's graphics system to provide:
- OpenGL context management
- ImGui rendering
- Viewport management
- Buffer swapping

## Best Practices

1. Always call `initialize()` before using the window
2. Call `render()` in your main loop
3. Handle any exceptions that may be thrown during initialization
4. Ensure proper cleanup by calling `shutdown()` or letting the destructor handle it
5. Check `shouldClose()` in your main loop to handle window closure properly

## Dependencies

- ImGui (Dear ImGui)
- GLFW
- OpenGL
- RebelCAD Graphics System

## Thread Safety

The MainWindow class is not thread-safe and should only be used from the main thread.
