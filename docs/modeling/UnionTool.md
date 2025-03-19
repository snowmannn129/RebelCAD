# UnionTool

The UnionTool is a CAD operation that combines two or more solid bodies into a single body. It performs a Boolean union operation, merging the volumes of the input bodies and removing any internal faces where the bodies intersect.

## Usage

```cpp
// Create the tool
UnionTool unionTool;

// Set the bodies to unite
std::vector<std::shared_ptr<SolidBody>> bodies = {body1, body2, body3};
unionTool.SetBodies(bodies);

// Execute the union operation
auto result = unionTool.Execute();
```

## Features

- Supports union of two or more solid bodies
- Handles both intersecting and non-intersecting bodies
- Maintains valid topology throughout the operation
- Provides preview functionality for interactive modeling
- Throws appropriate errors for invalid operations

## Error Handling

The tool throws `rebel::core::Error` with specific error codes in the following cases:

- `CADError`: When fewer than 2 bodies are provided
- `CADError`: When a null body is provided
- `CADError`: When executing without setting bodies
- `GeometryError`: When a body has invalid topology
- `CADError`: When topology merging fails

## Implementation Details

### Body Intersection Detection

The tool first checks if the input bodies intersect. This is done by performing intersection tests between all pairs of bodies. For non-intersecting bodies, a compound body is created without merging topology.

### Topology Merging

When bodies intersect, the tool:
1. Validates all input bodies have valid topology
2. Merges vertices, edges, and faces from all bodies
3. Removes internal faces at intersection boundaries
4. Creates a new unified topology
5. Generates a new solid body from the merged topology

### Preview Rendering

The tool supports interactive preview rendering through the GraphicsSystem:
- Shows the expected result before committing the operation
- Uses wireframe or transparent rendering for better visualization
- Updates in real-time as parameters change

## Performance Considerations

- Intersection detection uses spatial partitioning for efficiency
- Topology merging is optimized to handle complex intersections
- Preview rendering uses efficient graphics pipeline

## Example

```cpp
try {
    UnionTool unionTool;
    
    // Create some test bodies
    auto cube1 = CreateCube({0, 0, 0}, 1.0);
    auto cube2 = CreateCube({0.5, 0.5, 0.5}, 1.0);
    
    // Perform the union
    unionTool.SetBodies({cube1, cube2});
    auto result = unionTool.Execute();
    
    // Use the resulting unified body
    scene.AddBody(result);
}
catch (const rebel::core::Error& e) {
    REBEL_LOG_ERROR(e.what());
}
```

## See Also

- [Boolean Operations Overview](../modeling/BooleanOperations.md)
- [SubtractTool](SubtractTool.md)
- [IntersectTool](IntersectTool.md)
