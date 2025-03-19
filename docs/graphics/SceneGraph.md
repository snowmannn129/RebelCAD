# Scene Graph System

The Scene Graph system provides a hierarchical organization of 3D objects in the RebelCAD environment. It manages spatial relationships, transformations, and scene-wide operations efficiently.

## Overview

The Scene Graph is implemented as a tree structure where each node can have:
- A parent node (except for the root)
- Multiple child nodes
- Local and world space transformations
- Selection state

## Key Components

### SceneNode

The basic building block of the scene hierarchy with the following capabilities:

- **Hierarchy Management**
  - Parent-child relationships
  - Tree traversal
  - Node addition/removal

- **Transform Operations**
  - Local transform manipulation
  - World transform computation
  - Position, rotation, and scale control

- **Selection Support**
  - Individual node selection
  - Selection state management

### SceneGraph

The main manager class that provides:

- **Scene Management**
  - Node addition/removal
  - Node lookup by name
  - Selection management
  - Scene-wide updates

- **Picking System**
  - Ray-based object picking
  - Closest object selection
  - Hierarchical traversal

## Usage Examples

### Creating and Managing Nodes

```cpp
// Create a scene graph
SceneGraph sceneGraph;

// Create nodes
auto parentNode = std::make_shared<SceneNode>("parent");
auto childNode = std::make_shared<SceneNode>("child");

// Build hierarchy
sceneGraph.addNode(parentNode);
sceneGraph.addNode(childNode, parentNode);
```

### Transform Operations

```cpp
// Set node transforms
parentNode->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
childNode->setRotation(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
childNode->setScale(glm::vec3(2.0f));

// Update transforms
sceneGraph.update();
```

### Selection and Picking

```cpp
// Pick object with ray
auto picked = sceneGraph.pick(rayOrigin, rayDirection);
if (picked) {
    picked->setSelected(true);
}

// Get all selected nodes
auto selectedNodes = sceneGraph.getSelectedNodes();

// Clear selection
sceneGraph.clearSelection();
```

## Best Practices

1. **Memory Management**
   - Use shared_ptr for node ownership
   - Clean up nodes properly using removeNode()
   - Avoid circular references

2. **Transform Updates**
   - Call update() after changing transforms
   - Use local transforms for relative positioning
   - Access world transforms for global coordinates

3. **Scene Organization**
   - Keep a logical hierarchy
   - Use meaningful node names
   - Group related objects under common parents

4. **Performance Considerations**
   - Minimize deep hierarchies
   - Batch transform updates
   - Use picking judiciously

## Integration with Other Systems

The Scene Graph system integrates with:

- **Viewport System**: For rendering and camera management
- **Selection System**: For user interaction
- **Transform System**: For precise object manipulation
- **Serialization System**: For scene saving/loading

## Future Enhancements

Planned improvements include:

- Spatial partitioning for faster picking
- Instance rendering support
- Animation system integration
- Advanced culling techniques
