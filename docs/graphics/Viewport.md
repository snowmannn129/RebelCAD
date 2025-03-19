# Viewport System

The Viewport system in RebelCAD provides functionality for managing multiple synchronized views of the 3D workspace. It supports both perspective and orthographic projections, standard view orientations, and interactive camera controls.

## Features

- Multiple viewport support with optional synchronization
- Perspective and orthographic projection modes
- Standard view presets (Front, Back, Top, Bottom, Left, Right, Isometric)
- Interactive camera controls (Pan, Rotate, Zoom)
- View synchronization between multiple viewports
- Orthographic scale management for 2D views

## Usage

### Creating a Viewport

```cpp
// Create a perspective viewport
auto viewport = rebel_cad::graphics::Viewport(800, 600);

// Create an orthographic viewport
auto orthoView = rebel_cad::graphics::Viewport(
    800, 600, 
    rebel_cad::graphics::ViewportProjection::Orthographic
);
```

### Camera Control

```cpp
// Pan the view
viewport.pan(deltaX, deltaY);  // deltaX/Y are in screen coordinates

// Rotate around the target point
viewport.rotate(deltaX, deltaY);  // deltaX/Y are in radians

// Zoom in/out (Perspective)
viewport.zoom(1.2f);  // Zoom in (factor > 1)
viewport.zoom(0.8f);  // Zoom out (factor < 1)

// Zoom in/out (Orthographic)
float currentScale = viewport.getOrthoScale();
viewport.setOrthographic(-scale * aspect, scale * aspect,
                        -scale, scale,
                        nearPlane, farPlane);

// Set to a standard view
viewport.setStandardView(rebel_cad::graphics::ViewportView::Front);
viewport.setStandardView(rebel_cad::graphics::ViewportView::Isometric);

// Fit all content in view
viewport.fitAll();
```

### Viewport Synchronization

Multiple viewports can be synchronized to maintain the same view orientation:

```cpp
// Create two viewports
auto viewport1 = rebel_cad::graphics::Viewport(800, 600);
auto viewport2 = rebel_cad::graphics::Viewport(800, 600);

// Synchronize the viewports
viewport1.synchronizeWith(viewport2);

// Camera operations on viewport1 will now automatically update viewport2
viewport1.pan(10, 0);  // Both viewports will pan
```

### Projection Settings

```cpp
// Switch to perspective projection
viewport.setPerspective(
    glm::radians(45.0f),  // Field of view
    aspectRatio,          // Width/Height ratio
    0.1f,                 // Near plane
    1000.0f              // Far plane
);

// Switch to orthographic projection
float scale = 5.0f;      // Orthographic scale
float aspect = viewport.getAspectRatio();
viewport.setOrthographic(
    -scale * aspect,     // Left
    scale * aspect,      // Right
    -scale,             // Bottom
    scale,              // Top
    0.1f,               // Near plane
    1000.0f             // Far plane
);

// Get current projection type
auto projType = viewport.getProjectionType();

// Get orthographic scale (when in orthographic mode)
float scale = viewport.getOrthoScale();

// Handle viewport resizing
viewport.setDimensions(1024, 768);
```

## Implementation Details

### Camera Properties

- Field of view: 45 degrees (perspective mode)
- Default orthographic scale: 5.0 units
- Near plane: 0.1 units
- Far plane: 1000 units
- Default position: (0, 0, 10)
- Default target: (0, 0, 0)
- Default up vector: (0, 1, 0)

### Projection Modes

#### Perspective Mode
- Uses perspective projection matrix (glm::perspective)
- Zoom changes camera distance from target
- Field of view determines perspective effect

#### Orthographic Mode
- Uses orthographic projection matrix (glm::ortho)
- Zoom changes orthographic scale
- Scale determines visible area in world units
- Maintains aspect ratio during resizing

### Performance Considerations

- View and projection matrices are cached and only updated when necessary
- Viewport synchronization uses pointers to avoid copying
- Camera operations include bounds checking to prevent invalid states
- Efficient matrix operations using GLM library
- Projection type switching preserves view orientation

## Best Practices

1. **Memory Management**
   - Viewports should be managed by a viewport manager class
   - Ensure synchronized viewports are properly disconnected when destroyed

2. **Performance**
   - Limit the number of synchronized viewports to maintain performance
   - Consider using a viewport manager to handle culling and rendering optimization
   - Cache projection-dependent calculations

3. **User Interface**
   - Implement smooth transitions when switching between standard views
   - Provide visual feedback for viewport synchronization status
   - Consider adding viewport-specific overlays for orientation indicators
   - Scale UI feedback based on projection mode

4. **Projection Management**
   - Maintain consistent zoom behavior between projection modes
   - Consider view-specific projection settings
   - Handle projection transitions smoothly
   - Preserve view center when switching projections

## Future Enhancements

- [x] Implement orthographic scale management
- [x] Add smooth projection transitions
- [ ] Implement bounding box calculation for fitAll()
- [ ] Add support for custom view presets
- [ ] Add viewport-specific rendering settings
- [ ] Support for stereoscopic rendering
- [ ] Add viewport splitting and merging operations
- [ ] Add projection-specific navigation modes
