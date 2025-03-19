# ChamferTool

The ChamferTool is a specialized modeling tool in RebelCAD that creates angled transitions (chamfers) between faces or edges in 3D models. It supports various chamfer types and configurations for maximum flexibility in design.

## Features

- Equal distance chamfers
- Two-distance chamfers (asymmetric)
- Distance and angle based chamfers
- Variable distance chamfers along edges
- Preview functionality before applying changes
- Comprehensive validation of parameters
- Support for both face and edge chamfers

## Usage

### Basic Equal Distance Chamfer

```cpp
ChamferTool chamferTool;
std::vector<int> selectedElements = {1, 2}; // Face or edge indices

ChamferTool::ChamferOptions options;
options.type = ChamferTool::ChamferOptions::Type::EqualDistance;
options.distance1 = 2.0; // 2mm chamfer distance

chamferTool.createChamfer(body, selectedElements, options);
```

### Two-Distance Chamfer

```cpp
ChamferTool::ChamferOptions options;
options.type = ChamferTool::ChamferOptions::Type::TwoDistances;
options.distance1 = 2.0; // First face distance
options.distance2 = 3.0; // Second face distance

chamferTool.createChamfer(body, selectedElements, options);
```

### Distance and Angle Chamfer

```cpp
ChamferTool::ChamferOptions options;
options.type = ChamferTool::ChamferOptions::Type::DistanceAndAngle;
options.distance1 = 2.0; // Base distance
options.angle = 45.0;    // Angle in degrees

chamferTool.createChamfer(body, selectedElements, options);
```

### Variable Distance Chamfer

```cpp
ChamferTool::ChamferOptions options;
options.type = ChamferTool::ChamferOptions::Type::Variable;
options.variableDistances = {1.0, 2.0, 1.5, 1.0}; // Distances along edge

chamferTool.createChamfer(body, selectedElements, options);
```

### Preview Before Applying

```cpp
auto previewBody = chamferTool.previewChamfer(body, selectedElements, options);
if (previewBody) {
    // Inspect preview...
    // If satisfied, apply the actual chamfer
    chamferTool.createChamfer(body, selectedElements, options);
}
```

## Parameters

### ChamferOptions

- `type`: Type of chamfer operation
  - `EqualDistance`: Same distance on both faces
  - `TwoDistances`: Different distances for each face
  - `DistanceAndAngle`: One distance and an angle
  - `Variable`: Variable distance along edge
- `distance1`: Primary distance (or only distance for equal distance type)
- `distance2`: Secondary distance for two-distance type
- `angle`: Angle in degrees for distance-and-angle type
- `variableDistances`: Vector of distances for variable type

## Validation

The tool performs comprehensive validation of input parameters:

- Non-empty selection of elements
- Valid positive distances
- Angle between 0° and 90° for angle-based chamfers
- Valid element indices within topology bounds
- Proper number of distances for variable chamfers

## Error Handling

The tool uses RebelCAD's error handling system:

- Returns false on operation failure
- Logs detailed error messages
- Throws exceptions for critical failures
- Maintains model integrity on failure

## Best Practices

1. Always validate parameters before creating chamfers
2. Use preview functionality for complex operations
3. Consider using equal distance chamfers for symmetric features
4. Use variable distance chamfers for smooth transitions
5. Keep chamfer distances proportional to model size

## Limitations

- Maximum angle of 90 degrees
- Chamfer distance must be less than feature size
- Variable distance chamfers require sufficient edge length
- Complex topology may limit chamfer options

## Performance Considerations

- Preview operations are lightweight
- Complex variable distance chamfers may impact performance
- Multiple chamfers should be batched when possible
- Large numbers of variable points may affect system resources
