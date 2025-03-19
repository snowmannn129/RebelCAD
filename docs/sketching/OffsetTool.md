# Offset Tool

The Offset Tool creates parallel copies of existing sketch geometry at a specified distance. It supports offsetting lines, arcs, and circles, maintaining their geometric relationships while creating new elements at the specified offset distance.

## Features

- Create parallel copies of lines at a specified distance
- Create concentric arcs with modified radius
- Create concentric circles with modified radius
- Offset multiple connected elements while maintaining their relationships:
  - Automatically detects connected elements
  - Precise intersection calculation for connected elements
  - Handles corner intersections between connected elements
  - Maintains geometric continuity at connection points
  - Graceful fallback to midpoint method if intersection calculation fails
- Support for both positive (external) and negative (internal) offsets
- Robust validation with configurable tolerance:
  - Prevents creation of invalid geometry
  - Customizable tolerance for near-zero comparisons
  - Validates minimum radius requirements
- Type-safe template-based implementation for different geometry types
- Empty input handling
- Parallel element support
- Variable distance offset support:
  - Linear interpolation of offset distance
  - Smooth transitions between elements
  - Special handling for closed shapes
  - Support for variable radius arcs

## Configuration

The OffsetTool can be configured through its Config class to customize various aspects of its behavior:

```cpp
// Get current configuration
auto config = OffsetTool::getConfig();

// Customize settings
config.setGridCellSize(20.0f)              // Size of spatial partitioning grid cells
      .setTolerance(1e-5f)                 // Tolerance for geometric comparisons
      .setSpatialPartitioningThreshold(150); // When to use spatial partitioning

// Apply configuration
OffsetTool::setConfig(config);
```

### Configuration Options

- **gridCellSize** (default: 10.0f)
  - Controls the size of grid cells used in spatial partitioning
  - Larger values = fewer partitions but potentially more elements per cell
  - Smaller values = more partitions but fewer elements per cell
  - Adjust based on typical geometry scale in your application

- **tolerance** (default: 1e-6f)
  - Used for near-zero comparisons and geometric validations
  - Affects detection of:
    - Zero-length lines
    - Connected elements
    - Parallel lines
    - Minimum radius requirements
  - Adjust based on required precision level

- **spatialPartitioningThreshold** (default: 100)
  - Number of elements before switching to spatial partitioning
  - Affects performance optimization strategy
  - Lower values = more aggressive use of spatial partitioning
  - Higher values = stick to simple O(n²) approach longer

## Usage

### Basic Offset Operations

```cpp
// Create a line
Line line(0.0f, 0.0f, 10.0f, 0.0f);

// Create parallel line 5 units above the original
auto offsetLine = OffsetTool::offsetLine(line, 5.0f);

// Create parallel line 5 units below the original
auto offsetLine2 = OffsetTool::offsetLine(line, -5.0f);
```

### Offsetting an Arc

```cpp
// Create an arc
Arc arc(0.0f, 0.0f, 10.0f, 0.0f, M_PI_2);

// Create concentric arc with larger radius (external offset)
auto offsetArc = OffsetTool::offsetArc(arc, 5.0f);

// Create concentric arc with smaller radius (internal offset)
auto offsetArc2 = OffsetTool::offsetArc(arc, -5.0f);
```

### Offsetting a Circle

```cpp
// Create a circle
Circle circle(0.0f, 0.0f, 10.0f);

// Create concentric circle with larger radius
auto offsetCircle = OffsetTool::offsetCircle(circle, 5.0f);

// Create concentric circle with smaller radius
auto offsetCircle2 = OffsetTool::offsetCircle(circle, -5.0f);
```

### Offsetting Multiple Elements

```cpp
// Example 1: Connected lines forming a corner
std::vector<Line> elements;
elements.emplace_back(0.0f, 0.0f, 10.0f, 0.0f);    // Horizontal line
elements.emplace_back(10.0f, 0.0f, 10.0f, 10.0f);  // Vertical line

// Offset all elements by 5 units - corners will be properly handled
auto offsetElements = OffsetTool::offsetElements(elements, 5.0f);

// Example 2: Connected arcs
std::vector<Arc> arcs;
arcs.emplace_back(0.0f, 0.0f, 10.0f, 0.0f, M_PI_2);
arcs.emplace_back(0.0f, 0.0f, 10.0f, M_PI_2, M_PI);

// Offset arcs while maintaining their connection point
auto offsetArcs = OffsetTool::offsetElements(arcs, 5.0f);

// Example 3: Parallel lines
std::vector<Line> parallelLines;
parallelLines.emplace_back(0.0f, 0.0f, 10.0f, 0.0f);     // First line
parallelLines.emplace_back(0.0f, 10.0f, 10.0f, 10.0f);   // Parallel line

// Offset while maintaining parallel relationship
auto offsetParallel = OffsetTool::offsetElements(parallelLines, 5.0f);
```

### Variable Distance Offset

The OffsetTool supports creating offsets with varying distances along the path. This feature allows for:
- Linear interpolation of offset distance from start to end
- Smooth transitions between connected elements
- Support for all geometry types (lines, arcs, circles)
- Automatic handling of self-intersections
- Variable radius arcs for smooth transitions

```cpp
// Example 1: Single line with varying offset
std::vector<Line> line;
line.emplace_back(0.0f, 0.0f, 10.0f, 0.0f);
// Create offset that starts at 2 units and increases to 5 units
auto result = OffsetTool::offsetElementsWithVariableDistance(line, 2.0f, 5.0f);

// Example 2: Arc with varying radius
std::vector<Arc> arc;
arc.emplace_back(0.0f, 0.0f, 10.0f, 0.0f, M_PI_2);
// Create offset that varies from 2 to 5 units
auto result = OffsetTool::offsetElementsWithVariableDistance(arc, 2.0f, 5.0f);
// Result will be a VariableRadiusArc

// Example 3: Circle (uses average of start/end distances)
std::vector<Circle> circle;
circle.emplace_back(0.0f, 0.0f, 10.0f);
// Create offset using average of 2 and 5 units
auto result = OffsetTool::offsetElementsWithVariableDistance(circle, 2.0f, 5.0f);

// Example 4: Connected elements with varying offset
std::vector<Line> path;
path.emplace_back(0.0f, 0.0f, 10.0f, 0.0f);      // First line
path.emplace_back(10.0f, 0.0f, 10.0f, 10.0f);    // Second line

// Create offset with varying distance (2 units at start, 5 units at end)
auto variableOffset = OffsetTool::offsetElementsWithVariableDistance(path, 2.0f, 5.0f);
```

## Implementation Details

- Uses perpendicular vector calculation for line offsets
- Maintains center point and angles for arc offsets, only modifying radius
- Template-based implementation for type safety and efficiency
- Enhanced two-pass algorithm for handling connected elements:
  1. First pass: Offset each element individually
  2. Second pass: Calculate precise intersections and adjust connection points
- Precise intersection calculation:
  - Uses point-direction vector representation
  - Handles parallel elements gracefully
  - Falls back to midpoint method if intersection calculation fails
- All offset operations are non-destructive (create new elements)
- Efficient type checking using compile-time template specialization
- Empty input handling returns empty result vector
- Maintains parallel relationships between unconnected parallel elements
- Self-intersection detection for complex geometries
- Support for mixed element types (lines, arcs, circles)
- Spatial partitioning optimization for large element sets:
  - Configurable grid cell size
  - Automatic threshold-based activation
  - Efficient hash-based cell lookup

## Error Handling

The tool includes robust validation to prevent creation of invalid geometry:

- Throws `rebel::core::Error` with `ErrorCode::GeometryError` in the following cases:
  - Attempting to offset a zero-length line
  - Attempting to create an arc or circle with negative or near-zero radius
  - Attempting to offset with an invalid distance (including near-zero values)
  - Attempting to offset an unsupported geometry type
  - Parallel lines causing intersection calculation failure
  - Self-intersecting geometry after offset operation
  - Invalid combinations of mixed element types
- Configurable tolerance for validation:
  - Default tolerance of 1e-6f for near-zero comparisons
  - Can be customized for different precision requirements
- Validates offset distances against minimum radius for curved geometry
- Provides detailed error messages for debugging
- Graceful fallback mechanisms:
  - Uses midpoint method if intersection calculation fails
  - Maintains element connectivity even in edge cases

## Performance Considerations

- Spatial partitioning optimization for large element sets:
  - Automatically switches to grid-based partitioning based on configured threshold
  - Reduces intersection check complexity from O(n²) to O(n)
  - Configurable grid cell size for different geometry scales
  - Efficient hash-based cell lookup
- Memory optimizations:
  - Efficient memory allocation with vector reservation
  - Minimal dynamic memory allocation
  - Smart pointer management for geometry objects
- Computational optimizations:
  - Compile-time type checking using template specialization
  - Early exit conditions for unconnected elements
  - Bounding box pre-checks before detailed intersection tests
  - Optimized validation with configurable tolerance
- Multi-geometry support:
  - Efficient handling of mixed geometry types
  - Type-safe template instantiations
  - Parallel processing potential for large sets
- Benchmarked performance:
  - < 1 second for 1000 connected elements
  - < 500ms for 20x20 grid intersection checks
  - < 750ms for mixed geometry sets (300 total elements)

## Future Improvements

Potential areas for enhancement:

- Support for more complex geometry types (splines, ellipses)
- Advanced corner treatment options:
  - Configurable fillet radius at corners
  - Sharp vs. smooth corner handling
  - Custom corner resolution strategies
- Further performance optimizations:
  - Multi-threaded processing for very large sets
  - GPU acceleration for intersection checks
  - Adaptive grid sizing for spatial partitioning
- Additional features:
  - Non-uniform offset patterns (beyond linear interpolation)
  - Offset preview generation
  - Custom interpolation functions for variable offset
- Integration with other systems:
  - Constraint system integration
  - Undo/redo support
  - Real-time update capabilities
