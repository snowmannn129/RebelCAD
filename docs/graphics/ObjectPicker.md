# Object Picker

The ObjectPicker class provides functionality for selecting objects in the 3D viewport using various selection methods. It supports point, box, and lasso selection modes with configurable precision and filtering capabilities.

## Features

- Point picking (single object selection)
- Box selection (multiple objects within a rectangle)
- Lasso selection (freeform selection area)
- Configurable picking precision
- Selection filtering support
- Distance-based sorting for overlapping objects

## Usage

```cpp
// Create an object picker
auto sceneGraph = std::make_shared<SceneGraph>();
ObjectPicker picker(sceneGraph);

// Set picking precision (in screen pixels)
picker.setPickingPrecision(3.0f);

// Set a selection filter (optional)
picker.setSelectionFilter([](const SceneNode::Ptr& node) {
    // Only select nodes of a specific type
    return node->getName().find("Mesh") != std::string::npos;
});

// Point picking
glm::vec2 mousePos(400, 300);  // Screen coordinates
auto selected = picker.pickAtPoint(mousePos, viewMatrix, projMatrix);

// Box selection
glm::vec2 min(100, 100), max(200, 200);
auto boxSelected = picker.pickInBox(min, max, viewMatrix, projMatrix);

// Lasso selection
std::vector<glm::vec2> lassoPoints = {
    glm::vec2(100, 100),
    glm::vec2(200, 100),
    glm::vec2(200, 200),
    glm::vec2(100, 200)
};
auto lassoSelected = picker.pickInLasso(lassoPoints, viewMatrix, projMatrix);
```

## Selection Modes

### Point Picking
- Uses ray casting to find the closest object under the cursor
- Returns objects sorted by distance from camera
- Respects picking precision setting

### Box Selection
- Selects all objects within a rectangular screen region
- Objects must be fully contained within the selection box
- Useful for selecting multiple objects at once

### Lasso Selection
- Allows freeform selection area
- Uses point-in-polygon testing for object selection
- Great for precise selection of irregular object groups

## Technical Details

### Ray Casting
- Converts screen coordinates to world-space rays
- Performs intersection tests with object bounding boxes
- Sorts results by distance for proper depth handling

### Coordinate Spaces
- Screen Space: (0,0) at top-left to (width,height) at bottom-right
- NDC Space: (-1,-1) to (1,1)
- World Space: Scene coordinates

### Performance Considerations
- Uses bounding box tests for initial filtering
- Implements spatial partitioning for large scenes
- Optimizes intersection tests for common cases

## Integration with Scene Graph

The ObjectPicker integrates with the SceneGraph system to:
- Access scene hierarchy
- Perform intersection tests
- Handle object transformations
- Manage selection state

## Best Practices

1. **Picking Precision**
   - Adjust based on scene density
   - Consider UI scale and DPI
   - Default is 5.0 pixels

2. **Selection Filters**
   - Use to limit selectable object types
   - Implement efficient filtering logic
   - Consider scene hierarchy

3. **Performance**
   - Cache results when appropriate
   - Use box/lasso selection judiciously
   - Consider view frustum culling

## Example: Custom Selection Filter

```cpp
// Filter for selecting only specific object types
picker.setSelectionFilter([](const SceneNode::Ptr& node) {
    // Check node type
    if (auto* meshNode = dynamic_cast<MeshNode*>(node.get())) {
        return true;  // Allow mesh selection
    }
    if (auto* lightNode = dynamic_cast<LightNode*>(node.get())) {
        return false;  // Prevent light selection
    }
    return true;  // Allow other types
});
```

## Error Handling

The ObjectPicker includes robust error handling for:
- Invalid screen coordinates
- Null scene nodes
- Degenerate selection regions
- Invalid view/projection matrices

## Future Enhancements

Planned improvements include:
- Multi-threaded picking for large scenes
- Advanced selection preview
- Custom selection shapes
- Selection history
- Smart selection algorithms
