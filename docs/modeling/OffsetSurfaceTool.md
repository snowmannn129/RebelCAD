# OffsetSurfaceTool

The OffsetSurfaceTool creates a new surface that is offset from an input surface by a specified distance. The offset can be positive (outward) or negative (inward).

## Features

- Create offset surfaces with specified distance
- Handle self-intersections automatically
- Preserve sharp features (optional)
- Adjustable tolerance for surface approximation
- Support for both positive (outward) and negative (inward) offsets

## Usage

```cpp
#include "modeling/OffsetSurfaceTool.h"

// Create the tool
auto offsetTool = std::make_unique<RebelCAD::Modeling::OffsetSurfaceTool>();

// Set the input surface
offsetTool->setInputSurface(inputSurface);

// Configure the offset parameters
offsetTool->setOffsetDistance(2.0);  // 2 units outward
offsetTool->setTolerance(0.001);     // Set precision
offsetTool->setPreserveSharpFeatures(true);

// Execute the operation
auto offsetSurface = offsetTool->execute();
```

## Parameters

### Input Surface
The surface to offset. Must be a valid surface object implementing the Surface interface.

### Offset Distance
- Positive values create an outward offset
- Negative values create an inward offset
- The magnitude determines the distance between the original and offset surfaces

### Tolerance
Controls the precision of the offset surface approximation:
- Smaller values produce more accurate results but may increase computation time
- Must be positive
- Default is 0.001 units

### Sharp Feature Preservation
When enabled, the tool attempts to maintain sharp features (edges and corners) in the offset surface:
- Useful for mechanical parts where edge definition is important
- May increase computation time
- Enabled by default

## Technical Details

### Self-Intersection Handling
The tool automatically detects and handles self-intersections that may occur during offsetting:
1. Detects potential intersections using mesh-based analysis
2. Adjusts offset distance locally if needed
3. Creates trimmed surfaces where necessary

### Surface Continuity
The tool maintains the same degree of continuity as the input surface:
- G0 continuous surfaces remain G0
- G1 continuous surfaces maintain tangent continuity
- G2 continuous surfaces preserve curvature continuity

### Implementation Notes
- Uses mesh-based validation for robustness
- Implements adaptive tessellation for efficient processing
- Handles degenerate cases gracefully

## Examples

### Basic Offset
```cpp
// Create a simple outward offset
offsetTool->setInputSurface(surface);
offsetTool->setOffsetDistance(1.0);
auto result = offsetTool->execute();
```

### Precise Offset with Sharp Features
```cpp
// Configure for precise mechanical parts
offsetTool->setInputSurface(surface);
offsetTool->setOffsetDistance(-0.5);    // Inward offset
offsetTool->setTolerance(0.0001);       // High precision
offsetTool->setPreserveSharpFeatures(true);
auto result = offsetTool->execute();
```

## Error Handling

The tool validates inputs and operations:
- Invalid input surfaces return false from setInputSurface()
- Invalid tolerance values throw std::invalid_argument
- Failed operations throw std::runtime_error with descriptive messages

## Performance Considerations

- Computation time increases with:
  - Surface complexity
  - Smaller tolerance values
  - Sharp feature preservation enabled
- Memory usage scales with mesh resolution
- Consider using larger tolerance values for preview operations

## Limitations

- May not handle extremely thin regions well
- Complex self-intersections might require manual intervention
- Sharp feature preservation may not be perfect for all geometries
