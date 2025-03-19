# TrimTool

The TrimTool provides functionality for trimming sketch elements at intersection points or specified locations. It supports trimming lines, curves, and polygon edges while maintaining the mathematical validity of the trimmed elements.

## Features

- Line-to-line intersection detection
- Line-to-curve intersection detection (templated for different curve types)
- Line trimming at specified points
- Curve trimming at specified points
- Polygon edge trimming

## Usage Examples

### Finding Line Intersections

```cpp
TrimTool trimTool;
Line line1(0, 0, 4, 4);  // Line from (0,0) to (4,4)
Line line2(0, 4, 4, 0);  // Line from (0,4) to (4,0)

auto intersections = trimTool.findLineIntersections(line1, line2);
if (!intersections.empty()) {
    float x = intersections[0].x;  // x-coordinate of intersection
    float y = intersections[0].y;  // y-coordinate of intersection
    float t1 = intersections[0].parameter1;  // Parameter on first line (0-1)
    float t2 = intersections[0].parameter2;  // Parameter on second line (0-1)
}
```

### Trimming a Line

```cpp
Line line(0, 0, 4, 4);  // Original line
float trimX = 2.0f, trimY = 2.0f;  // Trim point
bool keepStart = true;  // Keep the portion from start to trim point

// Returns a new line from (0,0) to (2,2)
Line trimmedLine = trimTool.trimLine(line, trimX, trimY, keepStart);
```

### Trimming a Polygon Edge

```cpp
std::vector<std::array<float, 2>> vertices = {
    {0, 0}, {4, 0}, {4, 4}, {0, 4}
};
Polygon polygon(vertices);

size_t edgeIndex = 0;  // Index of edge to trim
float trimX = 2.0f, trimY = 0.0f;  // Trim point on edge
bool keepStart = true;  // Keep the portion from vertex to trim point

// Returns a new polygon with the specified edge trimmed
Polygon trimmedPolygon = trimTool.trimPolygonEdge(polygon, edgeIndex, trimX, trimY, keepStart);
```

## Error Handling

The TrimTool throws exceptions in the following cases:

- Invalid trim point coordinates (NaN or infinity)
- Trim point does not lie on the specified element
- Invalid edge index when trimming polygon edges

Example error handling:

```cpp
try {
    auto trimmed = trimTool.trimLine(line, x, y, keepStart);
} catch (const Error& e) {
    // Handle error (e.g., point not on line)
    std::cerr << e.what() << std::endl;
}
```

## Implementation Details

### Line Intersection

Line intersections are calculated using the parametric form of line equations. The algorithm:

1. Converts lines to parametric form
2. Solves for intersection parameters (t, u)
3. Checks if intersection point lies on both line segments (0 ≤ t,u ≤ 1)
4. Returns intersection point and parameters if found

### Point-on-Line Detection

Points are considered to lie on a line segment if:
- The sum of distances from point to endpoints equals the line length (within epsilon)
- Uses small epsilon (1e-6) for floating-point comparisons

### Polygon Edge Trimming

When trimming polygon edges:
1. Validates edge index and trim point
2. Creates new vertex list with trim point inserted
3. Maintains polygon closure and orientation
4. Handles both keep-start and keep-end cases

## Performance Considerations

- Efficient intersection calculations using parametric form
- Minimal memory allocation in core operations
- O(1) line trimming operations
- O(n) polygon edge trimming where n is number of vertices
