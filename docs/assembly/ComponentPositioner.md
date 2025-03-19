# Component Positioning System

The ComponentPositioner is a core feature of RebelCAD's assembly system that enables precise positioning and manipulation of components in 3D space. It provides an intuitive interface for inserting, moving, and snapping components within assemblies.

## Features

- Interactive 3D component positioning
- Snap-to functionality for precise alignment
- Collision detection during positioning
- Support for both translation and rotation
- Automatic constraint detection (planned)

## Usage

### Basic Component Positioning

```cpp
// Create a component positioner
ComponentPositioner positioner;

// Start positioning a component
uint64_t componentId = /* your component ID */;
if (positioner.startPositioning(componentId)) {
    // Update position and rotation
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    glm::quat rotation = glm::angleAxis(
        glm::radians(45.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    positioner.updatePosition(position, rotation);
    
    // Try snapping to nearby geometry
    if (positioner.trySnap(0.01f)) {
        // Component snapped to a valid point
    }
    
    // Check for collisions
    if (!positioner.checkCollision()) {
        // Position is valid, finalize it
        positioner.finalizePosition();
    } else {
        // Cancel the operation
        positioner.cancelPositioning();
    }
}
```

### Snap-to Functionality

The snap system helps align components with existing geometry:

- Snaps to vertices, edges, and face centers
- Configurable snap threshold
- Returns true when snap is successful
- Automatically updates component position

```cpp
// Example with custom snap threshold
float snapThreshold = 0.02f; // 2cm threshold
if (positioner.trySnap(snapThreshold)) {
    // Component has snapped to nearby geometry
}
```

## Implementation Details

### Position Validation

The system performs several checks before finalizing a position:

1. Collision detection with other components
2. Workspace boundary validation
3. Assembly constraint validation
4. Component orientation validation

### Future Enhancements

- Spatial partitioning for improved collision detection
- Advanced constraint detection
- Preview of potential assembly constraints
- Support for temporary constraints during positioning
- Integration with the undo/redo system

## Best Practices

1. Always check the return value of `startPositioning()`
2. Use appropriate snap thresholds for your model scale
3. Implement proper error handling for positioning failures
4. Consider using the preview system for large assemblies
5. Clean up by calling either `finalizePosition()` or `cancelPositioning()`

## Technical Notes

- All positions and rotations use GLM types
- Positions are in model space coordinates
- Rotations use quaternions for smooth interpolation
- Snap thresholds are in model units
