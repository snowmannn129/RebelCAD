# DraftTool

The DraftTool is used to create draft angles on faces for manufacturing purposes. Draft angles are essential for parts that need to be removed from molds, such as in injection molding or casting processes.

## Overview

Draft angles ensure that parts can be easily removed from molds by slightly tapering the faces. The tool works by tilting selected faces at a specified angle relative to a pull direction (the direction in which the part is removed from the mold).

## Usage

```cpp
DraftTool draft_tool;

// Set up draft parameters
DraftTool::DraftParams params;
params.angle = 3.0;                    // 3 degree draft angle
params.pull_direction[0] = 0;          // Pull direction vector
params.pull_direction[1] = 1;          // (typically vertical, i.e. 0,1,0)
params.pull_direction[2] = 0;
params.neutral_plane = 0.5;            // Height where no draft is applied
params.is_internal = false;            // External draft (outward taper)

// Apply draft to selected faces
std::vector<std::shared_ptr<SolidBody>> faces = GetSelectedFaces();
Error result = draft_tool.ApplyDraft(faces, params);

if (result.code() != rebel::core::ErrorCode::None) {
    // Handle error
    std::cerr << "Draft operation failed: " << result.message() << std::endl;
}
```

## Parameters

### DraftParams Structure

- `angle` (double): Draft angle in degrees. Must be between -89 and 89 degrees.
- `pull_direction` (float[3]): Direction vector for draft application, typically vertical.
- `neutral_plane` (double): Height at which no draft is applied. Features above this height are drafted outward, below are drafted inward.
- `is_internal` (bool): If true, creates an internal draft (inward taper). If false, creates an external draft (outward taper).

## Material-Specific Draft Angles

The tool provides recommended minimum draft angles for common materials:

- Plastic: 0.5°
- Aluminum: 1.0°
- Steel: 1.5°
- Zinc: 0.75°

These values can be retrieved using the `GetMinimumDraftAngle()` function:

```cpp
double min_angle = draft_tool.GetMinimumDraftAngle("plastic");
```

## Validation

The tool performs several validations:

1. Draft angle must be between -89° and 89°
2. Pull direction vector cannot be zero
3. Faces must be suitable for drafting (not parallel to pull direction)
4. At least one face must be provided

## Error Handling

The tool returns an Error object with appropriate error codes:

- `ErrorCode::InvalidVertex`: No faces provided
- `ErrorCode::CADError`: Invalid parameters or operation failed
- `ErrorCode::None`: Operation completed successfully

## Implementation Details

The draft operation:

1. Validates input parameters and faces
2. Creates a new topology for each face
3. Applies draft transformation to vertices based on their height relative to the neutral plane
4. Creates new solid bodies with the modified topology
5. Combines the modified bodies into a final result

## Best Practices

1. Use the recommended minimum draft angles for the target manufacturing process
2. Set the neutral plane at a logical feature height (e.g., parting line)
3. Verify faces are draftable using `CanApplyDraft()` before applying draft
4. Consider both internal and external draft requirements
5. Maintain consistent draft directions for related features

## Limitations

- Only handles planar faces in current implementation
- Draft angle limited to ±89 degrees
- All faces must use the same pull direction
- Complex features may require multiple draft operations

## See Also

- [ExtrudeTool](ExtrudeTool.md)
- [ShellTool](ShellTool.md)
- [FilletTool](../sketching/FilletTool.md)
