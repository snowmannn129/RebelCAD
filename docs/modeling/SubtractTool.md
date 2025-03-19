# SubtractTool

The SubtractTool is a modeling operation that removes the volume of one or more tool bodies from a main body. This boolean operation is essential for creating complex shapes by removing material from a base shape.

## Usage

```cpp
// Create the subtract tool
SubtractTool subtractTool;

// Set the main body to subtract from
subtractTool.SetMainBody(mainBody);

// Set one or more tool bodies to subtract
std::vector<std::shared_ptr<SolidBody>> toolBodies = {toolBody1, toolBody2};
subtractTool.SetToolBodies(toolBodies);

// Execute the subtraction operation
auto result = subtractTool.Execute();
```

## Features

- Supports subtracting multiple tool bodies in a single operation
- Handles complex intersections between bodies
- Creates proper boundary faces at intersection regions
- Maintains valid topology throughout the operation
- Provides preview capability for visualization before execution

## Technical Details

### Input Requirements

- Main body must be a valid solid body with manifold topology
- Tool bodies must be valid solid bodies with manifold topology
- At least one tool body must be provided
- All bodies must be non-null

### Operation Process

1. Validation
   - Checks for null bodies
   - Verifies topology validity of all bodies
   - Ensures proper initialization state

2. Intersection Detection
   - Determines if tool bodies intersect with main body
   - Returns unchanged copy of main body if no intersections exist
   - Identifies intersection regions for processing

3. Boundary Processing
   - Creates new faces at intersection boundaries
   - Ensures proper face orientation
   - Maintains topological validity

4. Sequential Processing
   - Handles multiple tool bodies one at a time
   - Updates topology after each subtraction
   - Validates intermediate results

### Error Handling

The tool throws `Error` exceptions in the following cases:
- Null main body provided
- Empty tool bodies vector
- Null tool body in vector
- Invalid topology in any body
- Failed topology operations during execution

### Preview Support

The tool provides a preview capability that:
- Shows the expected result before execution
- Handles preview failures gracefully
- Works with the graphics system for visualization

## Implementation Notes

### Topology Management

The tool maintains topology validity by:
- Creating proper boundary faces at intersections
- Ensuring manifold topology throughout operations
- Validating topology after each modification

### Performance Considerations

- Sequential processing of multiple tool bodies
- Efficient intersection detection
- Optimized topology operations

## Example: Complex Subtraction

```cpp
// Create main body (e.g., a cube)
auto mainBody = CreateCube(Point3D{0,0,0}, 10.0);

// Create tool bodies (e.g., cylinders for holes)
auto cylinder1 = CreateCylinder(Point3D{2,2,0}, 1.0, 10.0);
auto cylinder2 = CreateCylinder(Point3D{7,7,0}, 1.0, 10.0);

// Set up subtraction
SubtractTool subtractTool;
subtractTool.SetMainBody(mainBody);
subtractTool.SetToolBodies({cylinder1, cylinder2});

// Execute to create cube with two holes
auto result = subtractTool.Execute();
```

## Future Enhancements

- Volume calculation and comparison
- Advanced intersection curve handling
- Optimization for complex cases
- Enhanced preview capabilities

## See Also

- UnionTool - For combining solid bodies
- IntersectTool - For finding common volumes
- Topology - For understanding topological operations
