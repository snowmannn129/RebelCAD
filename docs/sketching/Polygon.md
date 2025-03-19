# Polygon Class

The Polygon class provides functionality for creating and manipulating 2D polygons in the RebelCAD sketching system. It supports both regular polygons (equal sides and angles) and irregular polygons.

## Features

- Create regular polygons with specified center, radius, and number of sides
- Create irregular polygons from a list of vertices
- Calculate geometric properties (area, perimeter, center)
- Transform polygons (translate, rotate, scale)
- Check point containment
- Render polygons with customizable appearance

## Usage

### Creating Regular Polygons

```cpp
// Create a regular hexagon centered at (0,0) with radius 1.0
Polygon hexagon(0.0f, 0.0f, 1.0f, 6);

// Create a regular pentagon with 45-degree rotation
Polygon pentagon(0.0f, 0.0f, 2.0f, 5, M_PI / 4.0f);
```

### Creating Irregular Polygons

```cpp
// Create a rectangle from vertices
std::vector<std::array<float, 2>> vertices = {
    {0.0f, 0.0f},  // Bottom-left
    {2.0f, 0.0f},  // Bottom-right
    {2.0f, 1.0f},  // Top-right
    {0.0f, 1.0f}   // Top-left
};
Polygon rectangle(vertices);
```

### Geometric Operations

```cpp
// Get polygon properties
float area = polygon.getArea();
float perimeter = polygon.getPerimeter();
auto center = polygon.getCenter();

// Transform the polygon
polygon.translate(1.0f, 2.0f);     // Move
polygon.rotate(M_PI / 2.0f);       // Rotate 90 degrees
polygon.scale(2.0f);               // Double the size

// Check if a point is inside the polygon
bool contains = polygon.containsPoint(x, y);
```

### Rendering

```cpp
// Render polygon outline
polygon.render(graphics, Color::Blue, 1.0f);

// Render filled polygon
polygon.render(graphics, Color::Red, 1.0f, true);

// Render with dashed lines
std::vector<float> dashPattern = {5.0f, 3.0f};  // 5px dash, 3px gap
polygon.render(graphics, Color::Green, 1.0f, false, &dashPattern);
```

## Error Handling

The Polygon class throws `Error` exceptions in the following cases:

- Invalid vertex coordinates (NaN or infinite values)
- Too few sides (minimum 3 sides required)
- Invalid radius (must be positive)
- Invalid scale factor (must be positive)

Example error handling:

```cpp
try {
    Polygon polygon(0.0f, 0.0f, -1.0f, 4);  // Invalid radius
} catch (const Error& e) {
    // Handle error
    std::cout << "Error: " << e.what() << std::endl;
}
```

## Implementation Details

### Regular Polygon Construction

Regular polygons are constructed by:
1. Validating input parameters (center coordinates, radius, number of sides)
2. Calculating vertex positions using polar coordinates
3. Applying any specified rotation

### Point Containment Testing

The point containment test uses the ray casting algorithm:
1. Cast a ray from the test point in any fixed direction
2. Count the number of intersections with polygon edges
3. Point is inside if number of intersections is odd

### Area Calculation

Area is calculated using the shoelace formula (also known as the surveyor's formula):
1. Calculate signed area by summing cross products of adjacent vertices
2. Take absolute value to handle clockwise/counterclockwise vertex ordering

## Performance Considerations

- Vertex coordinates are stored in a vector for efficient access and modification
- Geometric calculations cache results where beneficial
- Point containment testing is optimized for common cases (points far outside bounding box)

## Future Improvements

- Implement polygon boolean operations (union, intersection, difference)
- Add support for holes in polygons
- Optimize rendering for very large polygons
- Add polygon simplification algorithm for reducing vertex count
