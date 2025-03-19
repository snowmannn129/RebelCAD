# ShellTool

The ShellTool is an advanced modeling tool in RebelCAD that creates hollow objects by removing material from solid bodies while maintaining a specified wall thickness. It supports selective face removal to create openings in the resulting shell.

## Features

- Create hollow objects from solid bodies
- Configurable wall thickness
- Selective face removal for creating openings
- Automatic topology handling
- Robust error handling and validation
- Maintains model integrity and manifold geometry

## Usage

### Basic Shell Operation

```cpp
#include "modeling/ShellTool.h"

// Create the tool
RebelCAD::Modeling::ShellTool shellTool;

// Get your solid body (e.g., from a previous modeling operation)
std::shared_ptr<SolidBody> body = GetSolidBody();

// Configure the shell operation
auto result = shellTool.Configure(body, 0.1); // 0.1 units wall thickness
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}

// Execute the shell operation
result = shellTool.Execute();
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}
```

### Creating Shells with Openings

To create a shell with openings, specify which faces to remove:

```cpp
// Remove faces 0 and 2 while creating the shell
std::vector<int> facesToRemove = {0, 2};
auto result = shellTool.Configure(body, 0.1, facesToRemove);
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}

result = shellTool.Execute();
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}
```

## Technical Details

### Input Requirements

1. Solid Body:
   - Must be a valid, manifold solid
   - Must have properly defined topology (vertices, edges, faces)
   - Must have consistent face orientations

2. Wall Thickness:
   - Must be positive
   - Must be less than half the smallest body dimension
   - Should maintain model integrity (not too thin or thick)

3. Face Indices:
   - Must be valid indices into the body's face array
   - Cannot remove all faces
   - Must maintain model integrity after removal

### Validation

The tool performs several validations:

1. Input validation:
   - Non-null solid body
   - Positive thickness value
   - Valid face indices
   - Thickness compatibility with body dimensions

2. Topology validation:
   - Manifold check after face removal
   - Shell integrity verification
   - Edge case handling

### Error Handling

The tool uses RebelCAD's error handling system, returning Error objects with specific error codes:

- `ErrorCode::None`: Operation successful
- `ErrorCode::CADError`: Various CAD operation errors including:
  - Invalid input body
  - Invalid thickness
  - Invalid face indices
  - Shell operation failure
  - Topology validation failure

## Implementation Details

### Shell Creation Process

1. Parameter Validation:
   - Verify input body validity
   - Check thickness constraints
   - Validate face indices

2. Offset Surface Creation:
   - Generate offset surfaces for each face
   - Apply thickness along face normals
   - Handle vertex and edge cases

3. Surface Connection:
   - Connect offset surfaces
   - Handle face removals
   - Create transitional geometry

4. Final Processing:
   - Verify shell integrity
   - Optimize resulting geometry
   - Clean up temporary data

## Performance Considerations

- Efficient topology handling algorithms
- Optimized surface offsetting
- Smart memory management
- Robust error recovery

## Limitations

- Minimum thickness constraints based on model size
- Maximum number of removable faces
- Geometry complexity limitations
- Potential issues with highly complex input geometry

## Future Enhancements

- Variable thickness support
- Advanced face selection tools
- Pattern-based face removal
- Automatic thickness optimization
- Integration with feature recognition
- Support for non-manifold input geometry
- Enhanced performance for large models
- Real-time preview capability
