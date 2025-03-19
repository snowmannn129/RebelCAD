# FilletTool

The FilletTool is a utility class for creating smooth transitions (fillets) between sketch entities. It currently supports creating fillets between two intersecting lines, with planned support for line-arc and arc-arc fillets.

## Features

- Create fillets between two intersecting lines with a specified radius
- Automatic calculation of tangent points and fillet arc parameters
- Robust error handling for invalid inputs and geometric conditions

## Usage

### Creating a Line-Line Fillet

```cpp
// Create two intersecting lines
auto line1 = std::make_shared<Line>(0.0f, 0.0f, 10.0f, 0.0f);   // Horizontal
auto line2 = std::make_shared<Line>(10.0f, 0.0f, 10.0f, 10.0f); // Vertical

// Create fillet tool instance
FilletTool filletTool;

// Create a fillet with radius 2.0
try {
    auto fillet = filletTool.createLineLine(line1, line2, 2.0f);
    // fillet is now a shared_ptr to an Arc object representing the fillet
} catch (const rebel::core::Error& e) {
    // Handle error cases (parallel lines, invalid radius, etc.)
    std::cerr << "Failed to create fillet: " << e.what() << std::endl;
}
```

## API Reference

### `createLineLine`

Creates a fillet between two intersecting lines.

```cpp
std::shared_ptr<Arc> createLineLine(
    std::shared_ptr<Line> line1,
    std::shared_ptr<Line> line2,
    double radius
);
```

**Parameters:**
- `line1`: First line to fillet
- `line2`: Second line to fillet
- `radius`: Desired fillet radius (must be greater than 0)

**Returns:**
- `std::shared_ptr<Arc>`: Arc object representing the created fillet

**Throws:**
- `rebel::core::Error` if:
  - Lines are parallel or coincident
  - Radius is zero or negative
  - Other geometric calculation errors occur

### `createLineArc` (Planned)

Creates a fillet between a line and an arc.

```cpp
std::shared_ptr<Arc> createLineArc(
    std::shared_ptr<Line> line,
    std::shared_ptr<Arc> arc,
    double radius
);
```

**Status:** Not yet implemented

### `createArcArc` (Planned)

Creates a fillet between two arcs.

```cpp
std::shared_ptr<Arc> createArcArc(
    std::shared_ptr<Arc> arc1,
    std::shared_ptr<Arc> arc2,
    double radius
);
```

**Status:** Not yet implemented

## Technical Details

### Line-Line Fillet Algorithm

1. Validates input radius and line geometry
2. Calculates intersection point of the two lines
3. Determines angles of both lines
4. Calculates angle between lines and adjusts for obtuse angles
5. Computes tangent points based on radius and angle
6. Creates fillet arc tangent to both lines

### Error Handling

The tool uses the RebelCAD error system to handle various error conditions:

- `CADError`: General CAD operation errors (invalid radius, etc.)
- `GeometryError`: Geometric impossibilities (parallel lines, etc.)

## Future Enhancements

1. Implementation of line-arc filleting
2. Implementation of arc-arc filleting
3. Support for variable radius fillets
4. Automatic trimming of original entities
5. Preview functionality for interactive use

## See Also

- [Arc Documentation](Arc.md)
- [Line Documentation](Line.md)
