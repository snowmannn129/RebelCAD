# ExtendTool

The ExtendTool provides functionality for extending sketch elements (lines, curves, polygon edges) to meet other elements in the sketch. This is a fundamental CAD operation that helps create precise geometric constructions.

## Features

- Extend lines to intersect with other elements
- Extend Bezier curves and splines with tangent continuity
- Extend polygon edges while maintaining closed geometry
- Automatic detection of nearest extension points
- Support for extending from either end of an element
- Handles both float and double precision coordinates
- Maintains geometric relationships and constraints

## Usage

### Line Extension

```cpp
// Create an ExtendTool instance
ExtendTool extendTool;

// Extend a line to meet another element
Line extendedLine = extendTool.extendLine(originalLine, targetX, targetY, fromStart);
```

### Curve Extension

```cpp
// Extend a Bezier curve
BezierCurve curve(controlPoints);  // controlPoints is vector<pair<double, double>>
BezierCurve extended = extendTool.extendCurve(curve, targetX, targetY, fromStart);

// Extend a spline
Spline spline(controlPoints);  // controlPoints is vector<pair<double, double>>
Spline extended = extendTool.extendCurve(spline, targetX, targetY, fromStart);
```

### Polygon Edge Extension

```cpp
// Extend a polygon edge
Polygon polygon(vertices);  // vertices is vector<array<float, 2>>
size_t edgeIndex = 1;  // Index of the edge to extend
Polygon extended = extendTool.extendPolygonEdge(polygon, edgeIndex, targetX, targetY, fromStart);
```

### Finding Extension Points

```cpp
// Get all possible extension points for a line
std::vector<std::shared_ptr<Line>> otherLines = /* ... */;
auto extensionPoints = extendTool.findExtensionPoints(line, otherLines);

// Process extension points
for (const auto& point : extensionPoints) {
    std::cout << "Extension point at (" << point.x << ", " << point.y << ")\n";
    std::cout << "Distance to extend: " << point.extensionDistance << "\n";
    std::cout << "Extends from " << (point.isStart ? "start" : "end") << "\n";
}

// Works with curves too
std::vector<std::shared_ptr<BezierCurve>> curves = /* ... */;
auto curveExtensionPoints = extendTool.findExtensionPoints(line, curves);
```

### Finding Nearest Extension Point

```cpp
// Find nearest extension point to a clicked position
auto nearestPoint = extendTool.findNearestExtensionPoint(clickX, clickY, elements);
if (nearestPoint) {
    // Use the nearest extension point for lines
    Line extendedLine = extendTool.extendLine(
        originalLine,
        nearestPoint->x,
        nearestPoint->y,
        nearestPoint->isStart
    );
    
    // Or for curves
    BezierCurve extendedCurve = extendTool.extendCurve(
        originalCurve,
        nearestPoint->x,
        nearestPoint->y,
        nearestPoint->isStart
    );
}
```

## Error Handling

The ExtendTool throws exceptions in the following cases:

- Invalid coordinates (NaN values)
- Attempting to shorten a line instead of extending it
- Invalid extension points

Example error handling:

```cpp
try {
    Line extended = extendTool.extendLine(line, x, y, fromStart);
} catch (const rebel::core::Error& e) {
    if (e.code() == rebel::core::ErrorCode::GeometryError) {
        // Handle geometry error
        std::cerr << "Geometry error: " << e.message() << std::endl;
    }
}
```

## Best Practices

1. Always validate input coordinates before extending elements
2. Use findNearestExtensionPoint for interactive extension operations
3. Consider using findExtensionPoints when you need to analyze all possible extension options
4. Handle errors appropriately to provide meaningful feedback to users

## Integration with Constraints

When extending elements that have associated constraints:

1. The ExtendTool maintains geometric relationships
2. Existing constraints are preserved
3. New constraints can be added to the extended elements

## Performance Considerations

- The findExtensionPoints method performs intersection tests with all provided elements
- For large sketches, consider filtering the element list to nearby elements first
- The findNearestExtensionPoint method is optimized for interactive use

## See Also

- [TrimTool](TrimTool.md) - Complementary tool for trimming elements
- [Line](Line.md) - Line element documentation
- [BezierCurve](BezierCurve.md) - Curve element documentation
