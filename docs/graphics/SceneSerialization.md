# Scene Serialization

The Scene Serialization system in RebelCAD provides functionality to save and load scene graphs, enabling persistent storage of 3D scenes and their hierarchies.

## Features

- Save entire scene graphs to JSON format
- Load scenes from JSON files
- Preserve node hierarchies and transforms
- Maintain selection states
- Version control for file format compatibility

## Usage

### Saving a Scene

```cpp
RebelCAD::Graphics::SceneGraph scene;
// ... build your scene ...

// Save to file
if (scene.saveToFile("my_scene.json")) {
    std::cout << "Scene saved successfully\n";
}
```

### Loading a Scene

```cpp
RebelCAD::Graphics::SceneGraph scene;

// Load from file
if (scene.loadFromFile("my_scene.json")) {
    std::cout << "Scene loaded successfully\n";
}
```

### Working with JSON Directly

For more control over the serialization process, you can work with the JSON data directly:

```cpp
// Serialize to JSON
nlohmann::json sceneData = scene.serialize();

// Modify the JSON data if needed
sceneData["metadata"]["author"] = "John Doe";

// Deserialize from JSON
scene.deserialize(sceneData);
```

## File Format Specification

Scene files use a structured JSON format:

```json
{
    "version": "1.0",
    "root": {
        "name": "root",
        "selected": false,
        "position": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        },
        "rotation": {
            "x": 0.0,
            "y": 0.0,
            "z": 0.0
        },
        "scale": {
            "x": 1.0,
            "y": 1.0,
            "z": 1.0
        },
        "children": [
            {
                "name": "child1",
                "selected": false,
                "position": {"x": 1.0, "y": 0.0, "z": 0.0},
                "rotation": {"x": 0.0, "y": 0.0, "z": 0.0},
                "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
                "children": []
            }
        ]
    }
}
```

### Version Control

The scene file format includes version information to maintain compatibility as the format evolves:

- Version 1.0: Initial format supporting basic node properties and hierarchy

### Node Properties

Each node in the scene graph includes:

- `name`: Unique identifier for the node
- `selected`: Boolean indicating selection state
- `position`: 3D vector for node position
- `rotation`: 3D vector for node rotation (in radians)
- `scale`: 3D vector for node scale
- `children`: Array of child nodes

## Error Handling

The serialization system includes robust error handling:

- Version compatibility checks
- File I/O error handling
- Data validation
- Graceful failure recovery

Example error handling:

```cpp
try {
    scene.deserialize(jsonData);
} catch (const std::runtime_error& e) {
    std::cerr << "Failed to deserialize scene: " << e.what() << std::endl;
}
```

## Best Practices

1. **Version Control**: Always check the version when loading files to ensure compatibility.

2. **Error Handling**: Implement proper error handling when loading scenes.

3. **File Management**: Use appropriate file extensions (.json) and implement backup strategies.

4. **Validation**: Validate scene data after loading to ensure integrity.

## Implementation Details

The serialization system is implemented in:
- `include/graphics/SceneGraph.h`
- `src/graphics/SceneGraph.cpp`

Key classes and methods:
- `SceneNode::serialize()`: Serializes a single node
- `SceneNode::deserialize()`: Deserializes a single node
- `SceneGraph::serialize()`: Serializes the entire scene
- `SceneGraph::deserialize()`: Deserializes the entire scene
- `SceneGraph::saveToFile()`: Saves scene to disk
- `SceneGraph::loadFromFile()`: Loads scene from disk

## Testing

The serialization system includes comprehensive unit tests in `tests/graphics/SceneGraphTests.cpp`:

- Node serialization/deserialization
- Scene graph serialization/deserialization
- File I/O operations
- Error handling
- Transform preservation
- Hierarchy maintenance

## Future Enhancements

Planned improvements for the serialization system:

1. Support for custom node types and properties
2. Binary file format for improved performance
3. Compression support for large scenes
4. Differential updates for real-time collaboration
5. Scene merging capabilities
