# ExtrudeTool

The ExtrudeTool is a fundamental 3D modeling tool in RebelCAD that creates 3D solids by extruding 2D profiles along a specified direction. It supports both straight and tapered extrusions, as well as additive and subtractive operations.

## Features

- Create 3D solids from closed 2D profiles
- Support for straight and tapered extrusions
- Configurable extrusion direction and distance
- Additive and subtractive operations
- Real-time preview capability
- Robust error handling and validation

## Usage

### Basic Extrusion

```cpp
#include "modeling/ExtrudeTool.h"

// Create the tool
RebelCAD::Modeling::ExtrudeTool extrudeTool;

// Define a profile (must be a closed loop of 3D vertices)
std::vector<float> profile = {
    0.0f, 0.0f, 0.0f,  // First vertex
    1.0f, 0.0f, 0.0f,  // Second vertex
    1.0f, 1.0f, 0.0f,  // Third vertex
    0.0f, 1.0f, 0.0f,  // Fourth vertex
    0.0f, 0.0f, 0.0f   // Close the loop
};

// Set the profile
auto result = extrudeTool.SetProfile(profile);
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}

// Configure the extrusion
ExtrudeTool::ExtrudeParams params;
params.distance = 2.0f;              // Extrude 2 units
params.direction[0] = 0.0f;          // Extrude along Z-axis
params.direction[1] = 0.0f;
params.direction[2] = 1.0f;
params.isSubtractive = false;        // Additive operation
params.taper_angle = 0.0f;           // No tapering

result = extrudeTool.Configure(params);
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}

// Execute the extrusion
result = extrudeTool.Execute();
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}
```

### Tapered Extrusion

To create a tapered extrusion, simply set the taper_angle parameter:

```cpp
params.taper_angle = 15.0f;  // 15 degree taper
```

The taper angle must be between -89 and 89 degrees. Positive angles create an outward taper, while negative angles create an inward taper.

### Subtractive Operation

To subtract material instead of adding it:

```cpp
params.isSubtractive = true;
```

### Preview

Before committing an extrusion, you can preview it:

```cpp
auto graphics = GetGraphicsSystem();  // Get your graphics system instance
result = extrudeTool.Preview(graphics);
if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
}
```

### Canceling an Operation

To cancel an in-progress extrusion:

```cpp
extrudeTool.Cancel();
```

## Technical Details

### Profile Requirements

- Must be a closed loop
- Vertices must be specified in counter-clockwise order for proper face orientation
- Each vertex is specified as three consecutive float values (x, y, z)
- Minimum of 3 vertices (9 float values) required
- First and last vertices must match to close the loop

### Validation

The tool performs several validations:

1. Profile validation:
   - Non-empty vertex list
   - Vertex count is multiple of 3
   - Forms a closed loop
   - Minimum vertex requirement met

2. Configuration validation:
   - Non-zero extrusion distance
   - Valid direction vector (non-zero length)
   - Taper angle within valid range (-89 to 89 degrees)

### Error Handling

The tool uses RebelCAD's error handling system, returning Error objects with specific error codes:

- `ErrorCode::None`: Operation successful
- `ErrorCode::InvalidVertex`: Invalid profile data
- `ErrorCode::CADError`: General CAD operation error
- `ErrorCode::InvalidMesh`: Generated mesh is invalid

## Performance Considerations

- The tool uses efficient mesh generation algorithms
- Memory allocation is optimized through pre-allocation of vertex buffers
- Supports cancellation of long-running operations

## Future Enhancements

- Support for non-planar profiles
- Advanced triangulation for complex shapes
- Multiple extrusion paths
- Pattern-based extrusions
- Integration with boolean operations
