# ChamferTool

The ChamferTool is a utility class for creating angled transitions (chamfers) between sketch entities. It currently supports creating chamfers between two intersecting lines, with planned support for line-arc and arc-arc chamfers.

## Features

- Create chamfers between two intersecting lines with specified distances
- Support for equal and unequal chamfer distances
- Automatic calculation of chamfer endpoints
- Robust error handling for invalid inputs and geometric conditions

## Usage

### Creating a Line-Line Chamfer

```cpp
// Create two intersecting lines
auto line1 = std::make_shared<Line>(0.0f, 0.0f, 10.0f, 0.0f);   // Horizontal
auto line2 = std::make_shared<Line>(10.0f, 0.0f, 10.0f, 10.0f); // Vertical

// Create chamfer tool instance
ChamferTool chamferTool;

// Create a chamfer with distances 2.0 from each line
try {
    auto chamfer = chamferTool.createLineLine(line1, line2, 2.0f, 2.0f);
    // chamfer is now a shared_ptr to a Line object representing the chamfer
} catch (const rebel::core::Error& e) {
    // Handle error cases (parallel lines, invalid distances, etc.)
    std::cerr << "Failed to create chamfer: " << e.what() << std::endl;
}
```

## API Reference

### `createLineLine`

Creates a chamfer between two intersecting lines.

```cpp
std::shared_ptr<Line> createLineLine(
    std::shared_ptr<Line> line1,
    std::shared_ptr<Line> line2,
    double distance1,
    double distance2
);
```

**Parameters:**
- `line1`: First line to chamfer
- `line2`: Second line to chamfer
- `distance1`: Distance from intersection along first line
- `distance2`: Distance from intersection along second line (both must be greater than 0)

**Returns:**
- `std::shared_ptr<Line>`: Line object representing the created chamfer

**Throws:**
- `rebel::core::Error` if:
  - Lines are parallel or coincident
  - Either distance is zero or negative
  - Other geometric calculation errors occur

### `createLineArc` (Planned)

Creates a chamfer between a line and an arc.

```cpp
std::shared_ptr<Line> createLineArc(
    std::shared_ptr<Line> line,
    std::shared_ptr<Arc> arc,
    double lineDistance,
    double arcDistance
);
```

**Status:** Not yet implemented

### `createArcArc` (Planned)

Creates a chamfer between two arcs.

```cpp
std::shared_ptr<Line> createArcArc(
    std::shared_ptr<Arc> arc1,
    std::shared_ptr<Arc> arc2,
    double distance1,
    double distance2
);
```

**Status:** Not yet implemented

## Technical Details

### Line-Line Chamfer Algorithm

1. Validates input distances and line geometry
2. Calculates intersection point of the two lines
3. Determines unit vectors along both lines
4. Computes chamfer endpoints using specified distances
5. Creates chamfer line connecting the calculated points

### Error Handling

The tool uses the RebelCAD error system to handle various error conditions:

- `CADError`: General CAD operation errors (invalid distances, etc.)
- `GeometryError`: Geometric impossibilities (parallel lines, etc.)

## Future Enhancements

1. Implementation of line-arc chamfering
2. Implementation of arc-arc chamfering
3. Support for angle-based chamfer definition
4. Automatic trimming of original entities
5. Preview functionality for interactive use

## See Also

- [Line Documentation](Line.md)
- [Arc Documentation](Arc.md)
- [FilletTool Documentation](FilletTool.md)
