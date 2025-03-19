# RevolveTool

The RevolveTool is a 3D modeling tool that creates solid objects by revolving a 2D sketch profile around a specified axis. This operation is commonly used to create cylindrical, conical, or other rotationally symmetric objects.

## Features

- Full 360-degree or partial angle revolutions
- Support for arbitrary rotation axes
- Configurable segment count for controlling mesh resolution
- Additive and subtractive operations
- Automatic profile validation
- Preview capability before committing changes

## Usage

### Basic Example

```cpp
RevolveTool tool;

// Create a simple rectangular profile
std::vector<float> profile = {
    1.0f, 0.0f, 0.0f,  // Point 1
    2.0f, 0.0f, 0.0f,  // Point 2
    2.0f, 1.0f, 0.0f,  // Point 3
    1.0f, 1.0f, 0.0f   // Point 4
};

// Set up revolution parameters
RevolveTool::RevolveParams params;
params.angle = 360.0f;              // Full revolution
params.isSubtractive = false;       // Add material
params.axis_start[0] = 0.0f;        // Axis start point (0,0,0)
params.axis_start[1] = 0.0f;
params.axis_start[2] = 0.0f;
params.axis_end[0] = 0.0f;          // Axis end point (0,1,0)
params.axis_end[1] = 1.0f;
params.axis_end[2] = 0.0f;
params.full_revolution = true;       // Optimize for 360°
params.segments = 32;               // Number of segments

// Execute the operation
tool.SetProfile(profile);
tool.Configure(params);
tool.Execute();
```

## Parameters

### Profile Requirements

- Must be a vector of 3D points (x,y,z coordinates)
- Points must form a valid, non-self-intersecting profile
- Profile must not intersect or be too close to the rotation axis
- Coordinates are provided as consecutive floats: [x1,y1,z1, x2,y2,z2, ...]

### Revolution Parameters

| Parameter | Type | Description |
|-----------|------|-------------|
| angle | float | Revolution angle in degrees (0-360) |
| isSubtractive | bool | True for cutting operation, false for adding material |
| axis_start[3] | float[3] | Start point of rotation axis |
| axis_end[3] | float[3] | End point of rotation axis |
| full_revolution | bool | Optimization flag for 360° revolutions |
| segments | int | Number of segments (minimum 4) |

## Error Handling

The tool returns ErrorCode values to indicate success or failure:

- `ErrorCode::None`: Operation successful
- `ErrorCode::InvalidMesh`: Invalid profile or parameters
- `ErrorCode::CADError`: Operation failed (e.g., no profile set)

## Best Practices

1. **Profile Design**
   - Keep profiles simple and well-formed
   - Ensure profile is on one side of the rotation axis
   - Maintain reasonable distance from axis (> 1e-4 units)

2. **Performance Optimization**
   - Use `full_revolution = true` for 360° revolutions
   - Adjust segments based on model complexity:
     - More segments for smooth curves
     - Fewer segments for simple shapes
   - Consider using Preview() before Execute()

3. **Error Checking**
   ```cpp
   ErrorCode err = tool.SetProfile(profile);
   if (err != ErrorCode::None) {
       // Handle error
   }
   ```

## Common Use Cases

### Creating a Cylinder
```cpp
std::vector<float> profile = {
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};
```

### Creating a Cone
```cpp
std::vector<float> profile = {
    0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};
```

### Partial Revolution
```cpp
RevolveTool::RevolveParams params;
params.angle = 180.0f;  // Half revolution
params.full_revolution = false;
params.segments = 16;   // Fewer segments needed
```

## Limitations

- Profile must not intersect rotation axis
- Minimum 4 segments required
- Maximum revolution angle is 360 degrees
- Profile points must be provided in counter-clockwise order for correct normal generation

## See Also

- [ExtrudeTool](ExtrudeTool.md) - For linear extrusion operations
- [LoftTool](LoftTool.md) - For creating transitions between profiles
- [SweepTool](SweepTool.md) - For sweeping profiles along paths
