# MirrorTool

The MirrorTool is a utility class for creating symmetrical copies of sketch entities about a specified mirror line. It supports mirroring of basic entities (lines, arcs, circles) and complex curves (bezier curves, splines).

## Features

- Mirror line definition using two points or an existing line entity
- Support for mirroring multiple entity types:
  - Lines
  - Arcs
  - Circles
  - Bezier curves
  - Splines
- Point mirroring utility for custom operations
- Robust error handling for invalid inputs

## Usage

### Setting up the Mirror Line

```cpp
// Create mirror tool instance
MirrorTool mirrorTool;

// Set mirror line using points (e.g., horizontal line at y=0)
try {
    mirrorTool.setMirrorLine(-10.0, 0.0, 10.0, 0.0);
} catch (const rebel::core::Error& e) {
    std::cerr << "Failed to set mirror line: " << e.what() << std::endl;
}

// Or use an existing line
auto line = std::make_shared<Line>(0.0, 0.0, 10.0, 10.0);
mirrorTool.setMirrorLine(line);
```

### Mirroring Entities

```cpp
// Mirror a line
auto originalLine = std::make_shared<Line>(0.0, 0.0, 5.0, 5.0);
try {
    auto mirroredLine = mirrorTool.mirrorLine(originalLine);
} catch (const rebel::core::Error& e) {
    std::cerr << "Failed to mirror line: " << e.what() << std::endl;
}

// Mirror a circle
auto circle = std::make_shared<Circle>(0.0, 2.0, 3.0);
auto mirroredCircle = mirrorTool.mirrorCircle(circle);

// Mirror a spline
std::vector<std::pair<double, double>> controlPoints = {
    {0.0, 0.0}, {2.0, 2.0}, {4.0, 2.0}, {6.0, 0.0}
};
auto spline = std::make_shared<Spline>(controlPoints);
auto mirroredSpline = mirrorTool.mirrorSpline(spline);
```

## API Reference

### `setMirrorLine` (Points)

Defines the mirror line using two points.

```cpp
void setMirrorLine(double x1, double y1, double x2, double y2);
```

**Parameters:**
- `x1, y1`: Coordinates of first point
- `x2, y2`: Coordinates of second point

**Throws:**
- `rebel::core::Error` if points are coincident

### `setMirrorLine` (Line)

Sets an existing line as the mirror line.

```cpp
void setMirrorLine(std::shared_ptr<Line> line);
```

**Parameters:**
- `line`: Line to use as mirror line

**Throws:**
- `rebel::core::Error` if line is null

### `mirrorPoint`

Mirrors a point about the mirror line.

```cpp
std::pair<double, double> mirrorPoint(double x, double y) const;
```

**Parameters:**
- `x, y`: Coordinates of point to mirror

**Returns:**
- Pair of x,y coordinates of mirrored point

**Throws:**
- `rebel::core::Error` if mirror line not set

### Entity Mirroring Methods

All entity mirroring methods follow a similar pattern:

```cpp
std::shared_ptr<EntityType> mirrorEntity(const std::shared_ptr<EntityType>& entity);
```

Available methods:
- `mirrorLine`
- `mirrorArc`
- `mirrorCircle`
- `mirrorBezierCurve`
- `mirrorSpline`

**Parameters:**
- Entity to mirror (must not be null)

**Returns:**
- Shared pointer to new mirrored entity

**Throws:**
- `rebel::core::Error` if:
  - Mirror line not set
  - Input entity is null

## Technical Details

### Point Mirroring Algorithm

1. Project point onto mirror line
2. Calculate perpendicular distance to mirror line
3. Reflect point across mirror line using vector mathematics:
   - For a horizontal mirror line (y=k), simply negate the y-coordinate relative to k
   - For other lines, use vector projection and reflection formulas

### Entity Mirroring Implementation

- **Lines**: Mirror start and end points
- **Arcs**: Mirror center, start, and end points; adjust angles for proper orientation
- **Circles**: Mirror center point (radius remains unchanged)
- **Curves**: Mirror all control points to maintain shape

### Error Handling

The tool uses the RebelCAD error system with specific error types:
- `GeometryError`: Invalid mirror line definition
- `CADError`: Invalid inputs or operation failures

## Future Enhancements

1. Support for mirroring multiple entities at once
2. Mirror preview functionality
3. Integration with selection system
4. Support for mirroring with constraints
5. Undo/redo support

## See Also

- [Line Documentation](Line.md)
- [Arc Documentation](Arc.md)
- [Circle Documentation](Circle.md)
- [BezierCurve Documentation](BezierCurve.md)
- [Spline Documentation](Spline.md)
