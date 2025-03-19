# FilletTool

The FilletTool is used to create smooth transitions between faces or edges in 3D models. It supports various types of fillets including constant radius, variable radius, chamfers, and setback fillets.

## Features

- Constant radius fillets
- Variable radius fillets along edges
- Chamfer creation
- Setback fillets (combination of offset and fillet)
- Preview capability before applying changes
- Validation of input parameters

## Usage

### Basic Constant Radius Fillet

```cpp
auto filletTool = std::make_unique<FilletTool>();
std::vector<int> selectedEdges = {0, 1}; // Edge indices to fillet

FilletTool::FilletOptions options;
options.radius = 0.1;                    // 0.1 unit radius
options.type = FilletTool::FilletOptions::Type::Round;

// Create the fillet
filletTool->createFillet(body, selectedEdges, options);
```

### Variable Radius Fillet

```cpp
FilletTool::FilletOptions options;
options.isVariableRadius = true;
options.radiusPoints = {0.1, 0.2, 0.1}; // Varying radius along edge
options.type = FilletTool::FilletOptions::Type::Round;

// Create variable radius fillet
filletTool->createFillet(body, selectedEdges, options);
```

### Chamfer

```cpp
FilletTool::FilletOptions options;
options.radius = 0.1;                    // Chamfer distance
options.type = FilletTool::FilletOptions::Type::Chamfer;

// Create chamfer
filletTool->createFillet(body, selectedEdges, options);
```

### Setback Fillet

```cpp
FilletTool::FilletOptions options;
options.radius = 0.2;                    // Total setback distance
options.type = FilletTool::FilletOptions::Type::Setback;

// Create setback fillet
filletTool->createFillet(body, selectedEdges, options);
```

### Preview

```cpp
// Preview the fillet before applying
auto previewBody = filletTool->previewFillet(body, selectedEdges, options);
if (previewBody) {
    // Visualize the preview
    // If satisfied, apply the actual fillet
    filletTool->createFillet(body, selectedEdges, options);
}
```

## Parameters

### FilletOptions

| Parameter | Type | Description |
|-----------|------|-------------|
| radius | double | The radius of the fillet or distance for chamfer |
| isVariableRadius | bool | Whether to use variable radius along edges |
| radiusPoints | std::vector<double> | Radius values for variable radius fillets |
| type | FilletOptions::Type | Type of fillet (Round, Chamfer, Setback) |

### Fillet Types

- **Round**: Standard rounded fillet with constant or variable radius
- **Chamfer**: Angled transition instead of rounded
- **Setback**: Creates an offset face with a fillet between original and offset

## Validation

The tool performs several validations:
- Non-empty selection of edges/faces
- Valid radius values (> 0)
- Valid element indices
- For variable radius: non-empty radius points with valid values

## Error Handling

The tool uses the RebelCAD error handling system:
- Returns false on validation failures
- Logs errors through the logging system
- Throws exceptions for critical failures during operation

## Implementation Details

The FilletTool uses a rolling ball algorithm for round fillets:
1. Analyzes the selected edges/faces
2. Creates appropriate transition geometry
3. Updates the model topology
4. Maintains tangent continuity at transitions

For variable radius fillets, it creates a swept surface with varying profile.

## Performance Considerations

- Preview operations are optimized to provide quick feedback
- Complex fillets (variable radius, multiple edges) may require more processing time
- Memory usage scales with the complexity of the input geometry

## Limitations

- Maximum fillet radius is limited by local geometry
- Variable radius fillets require sufficient edge length
- Setback fillets need adequate space for offset
- Complex intersections may require manual intervention

## See Also

- [ExtrudeTool](ExtrudeTool.md)
- [RevolveTool](RevolveTool.md)
- [ShellTool](ShellTool.md)
