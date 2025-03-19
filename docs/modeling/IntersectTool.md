# IntersectTool

The IntersectTool performs boolean intersection operations between two or more solid bodies in RebelCAD. It creates a new solid body that represents the volume shared between all input bodies.

## Overview

Boolean intersection is a fundamental operation in solid modeling that creates a new solid containing only the space that exists in ALL input bodies. This is useful for:
- Creating complex shapes by finding common volumes
- Analyzing interference between parts
- Generating mating features between components

## Usage

```cpp
#include "modeling/IntersectTool.h"

// Create the intersection tool
RebelCAD::Modeling::IntersectTool intersectTool;

// Set the bodies to intersect (minimum 2 bodies required)
std::vector<std::shared_ptr<SolidBody>> bodies = {body1, body2, body3};
intersectTool.SetBodies(bodies);

// Execute the intersection operation
auto result = intersectTool.Execute();
```

## Requirements

- At least two input bodies must be provided
- All input bodies must have valid topology
- Input bodies must intersect with at least one other body in the set
- Bodies must not be null

## Error Handling

The tool throws exceptions in the following cases:

- `std::invalid_argument`: When fewer than 2 bodies are provided
- `std::invalid_argument`: When a null body is provided
- `std::invalid_argument`: When a body has invalid topology
- `std::runtime_error`: When bodies don't intersect
- `std::runtime_error`: When intersection operation fails

## Examples

### Basic Intersection

```cpp
// Create two overlapping boxes
auto box1 = CreateBox(Point3D{0,0,0}, Point3D{2,2,2});
auto box2 = CreateBox(Point3D{1,1,1}, Point3D{3,3,3});

// Intersect the boxes
IntersectTool tool;
tool.SetBodies({box1, box2});
auto intersection = tool.Execute();
```

### Multiple Body Intersection

```cpp
// Create three overlapping cylinders
auto cyl1 = CreateCylinder(Point3D{0,0,0}, Vector3D{1,0,0}, 1.0, 3.0);
auto cyl2 = CreateCylinder(Point3D{0,0,0}, Vector3D{0,1,0}, 1.0, 3.0);
auto cyl3 = CreateCylinder(Point3D{0,0,0}, Vector3D{0,0,1}, 1.0, 3.0);

// Find the volume common to all three cylinders
IntersectTool tool;
tool.SetBodies({cyl1, cyl2, cyl3});
auto intersection = tool.Execute();
```

## Performance Considerations

- The computational complexity increases with the number of input bodies
- Complex body geometries will take longer to process
- Consider using simpler representations when possible
- The tool validates inputs before performing expensive operations

## See Also

- [UnionTool](UnionTool.md) - For combining bodies
- [SubtractTool](SubtractTool.md) - For removing material
- [Geometry](Geometry.md) - Base geometry system
