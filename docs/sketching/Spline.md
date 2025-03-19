# Spline Tool

The Spline tool in RebelCAD enables users to create smooth, continuous curves defined by control points. It implements both B-spline and NURBS (Non-Uniform Rational B-Spline) curves which provide local control and maintains continuity, making it ideal for complex shape design. The tool supports both uniform and non-uniform knot vectors, as well as rational curves for exact representation of conic sections.

## Features

- Create smooth curves with precise control
- Support for both B-spline and NURBS curves
- Custom knot vector manipulation for non-uniform curves
- Weights for rational curves (NURBS)
- Clamped splines for endpoint interpolation
- Knot insertion and removal for local refinement
- Add, remove, and modify control points dynamically
- Adjustable curve degree for different levels of smoothness
- Visual feedback with customizable curve color
- Optimized performance with knot vector caching
- Robust validation and error handling

## Control Point Influence Visualization

The Spline tool now includes functionality to visualize how each control point influences the curve shape. This feature helps users understand the local control property of B-splines and makes editing more intuitive.

### Understanding Control Point Influence

Each control point in a B-spline curve has a limited region of influence determined by:
- The spline degree
- The knot vector
- The weights (for NURBS curves)

The influence region shows:
- Where on the curve the control point has effect (startParam to endParam)
- How strong the influence is at each point (maxInfluence)

Example code:
```cpp
// Get influence regions for all control points
auto influences = spline.calculateControlPointInfluences();

// Example usage for visualization
for (size_t i = 0; i < influences.size(); ++i) {
    const auto& influence = influences[i];
    
    // Influence region bounds
    double start = influence.startParam;  // Where influence begins
    double end = influence.endParam;      // Where influence ends
    double max = influence.maxInfluence;  // Maximum influence value
    
    // Use these values to visualize influence:
    // - Highlight curve segment between start and end
    // - Use maxInfluence to determine highlight intensity
    // - Show influence region when control point is selected
}
```

### Influence Properties

The influence characteristics vary by curve type:

#### Linear Splines (degree 1)
- Each control point influences the entire segment it's part of
- Influence decreases linearly from 1 to 0
- No overlap between non-adjacent control points

#### Quadratic Splines (degree 2)
- Each control point influences roughly 2/3 of the curve
- Smooth influence transition
- Partial overlap between adjacent control points

#### Cubic Splines (degree 3)
- Each control point influences roughly 3/4 of the curve
- Very smooth influence transition
- Significant overlap between adjacent control points

#### NURBS Curves
- Influence regions shaped by control point weights
- Higher weights increase influence magnitude
- Useful for precise shape control

### Applications

Control point influence visualization helps with:

1. **Editing**
   - Understand which control points affect a specific curve region
   - Predict how moving a control point will change the curve
   - Choose optimal points for local shape adjustments

2. **Learning**
   - Visualize B-spline local control property
   - Understand difference between degrees
   - See effect of weights in NURBS curves

3. **Debugging**
   - Identify control points causing unwanted behavior
   - Verify proper weight distribution
   - Check for appropriate influence overlap

### Best Practices

1. **Visualization**
   - Use different colors for each control point's influence region
   - Fade colors based on influence magnitude
   - Show regions on hover or selection

2. **Interaction**
   - Highlight influenced curve segments when selecting control points
   - Show influence region bounds
   - Display numerical influence values on demand

3. **Performance**
   - Cache influence calculations when possible
   - Adjust sampling rate based on curve complexity
   - Update visualizations efficiently during editing

## Usage

### Creating a Basic Spline

1. Select the Spline tool from the sketching toolbar
2. Click to place control points in the sketch plane
3. Double-click or press Enter to complete the spline

Example code:
```cpp
// Create a cubic spline with 4 control points
Spline spline;
spline.addControlPoint(0.0, 0.0);   // Start point
spline.addControlPoint(1.0, 1.0);   // First control point
spline.addControlPoint(2.0, -1.0);  // Second control point
spline.addControlPoint(3.0, 0.0);   // End point

// Get points for rendering
auto curvePoints = spline.calculateCurvePoints();

// Calculate properties
double length = spline.calculateArcLength();
double curvature = spline.calculateCurvature(0.5);  // at u=0.5
auto tangent = spline.evaluateFirstDerivative(0.5); // at u=0.5
```

### Creating a NURBS Curve

1. Create a basic spline as above
2. Use the properties panel to:
   - Set custom weights for rational curves
   - Modify the knot vector for non-uniform parameterization
   - Convert to clamped form for endpoint interpolation

Example code:
```cpp
// Create a NURBS circle quadrant
Spline circle;
circle.setDegree(2);  // Quadratic for conic sections

// Add control points for 90-degree arc
circle.addControlPoint(1.0, 0.0);    // Start at (1,0)
circle.addControlPoint(1.0, 1.0);    // Control point at (1,1)
circle.addControlPoint(0.0, 1.0);    // End at (0,1)

// Set weights for exact circular arc
std::vector<double> weights = {1.0, 0.7071067811865476, 1.0};  // sqrt(2)/2 for middle point
circle.setWeights(weights);

// Set custom knot vector for clamped ends
std::vector<double> knots = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
circle.setKnotVector(knots);

// Get points for rendering
auto arcPoints = circle.calculateCurvePoints(100, 200, 0.01);
```

### Creating Common Curves

#### Circle Arc (90 degrees)
1. Place three control points: (1,0), (1,1), (0,1)
2. Set weights to [1.0, 0.7071, 1.0] for exact circular arc
3. Use degree 2 for quadratic representation

Example code:
```cpp
// Helper function to create a 90-degree circular arc
Spline createQuarterCircle(double radius = 1.0) {
    Spline arc;
    arc.setDegree(2);
    
    // Scale control points by radius
    arc.addControlPoint(radius, 0.0);
    arc.addControlPoint(radius, radius);
    arc.addControlPoint(0.0, radius);
    
    // Set weights for exact circular shape
    arc.setWeights({1.0, 0.7071067811865476, 1.0});
    
    // Use clamped knot vector
    arc.convertToClamped();
    
    return arc;
}

// Usage:
Spline quarterCircle = createQuarterCircle(5.0);  // 5.0 radius circle
```

#### Ellipse
1. Place control points to define the ellipse shape
2. Set appropriate weights for exact conic representation
3. Use degree 2 for standard conic section

Example code:
```cpp
// Helper function to create an elliptical arc
Spline createEllipticalArc(double a, double b) {
    Spline ellipse;
    ellipse.setDegree(2);
    
    // Control points for quarter ellipse
    ellipse.addControlPoint(a, 0.0);      // Start point
    ellipse.addControlPoint(a, b);        // Control point
    ellipse.addControlPoint(0.0, b);      // End point
    
    // Weight calculation for ellipse
    double w = std::cos(M_PI/4.0);  // ~0.7071
    ellipse.setWeights({1.0, w, 1.0});
    
    // Ensure clamped ends
    ellipse.convertToClamped();
    
    return ellipse;
}

// Usage:
Spline ellipticalArc = createEllipticalArc(3.0, 2.0);  // 3x2 ellipse quadrant
```

### Editing a Spline

- **Add Control Point**: Click on the spline curve where you want to add a new control point
- **Remove Control Point**: Select a control point and press Delete
- **Move Control Point**: Click and drag any control point to a new position
- **Adjust Degree**: Use the properties panel to modify the spline's degree (2 for quadratic, 3 for cubic, etc.)

### Properties

- **Degree**: Controls the smoothness and complexity of the curve
  - Higher degrees result in smoother curves but require more control points
  - Default degree is 3 (cubic)
  - Minimum required control points = degree + 1
  
- **Control Points**: Define the shape of the curve
  - Each point influences a local region of the curve
  - Minimum number of control points needed = degree + 1
  - Adding/removing points triggers knot vector recalculation
  - Moving points preserves knot vector for efficiency

- **Weights**: Control the influence of control points (NURBS only)
  - Default weight is 1.0 for regular B-splines
  - Weights must be positive
  - Used for exact representation of conic sections
  - Enables creation of circles, ellipses, etc.

- **Knot Vector**: Controls parameterization of the curve
  - Uniform: Evenly spaced knots (default)
  - Non-uniform: Custom knot spacing for more control
  - Clamped: Repeated knots at endpoints for interpolation
  - Must be non-decreasing sequence

- **Color**: Customize the visual appearance of the spline
  - Default color is blue (0, 0, 1)
  - Fully customizable through the properties panel

## Technical Details

The Spline tool implements both B-spline and NURBS curves with the following characteristics:

- Support for both uniform and non-uniform knot vectors
- Rational curves with control point weights
- Local curve control and shape modification
- C(n-1) continuity where n is the degree
- De Boor's algorithm for curve evaluation
- Knot insertion/removal algorithms
- Adaptive point generation based on curvature
- Numerical curvature calculation using finite differences

### Performance Optimizations

The implementation includes several optimizations for better performance:

1. **Knot Vector Caching**
   - Knot vector is cached and only regenerated when necessary
   - Adding/removing control points triggers recalculation
   - Moving control points preserves the cache
   - Degree changes update the cache as needed
   - Custom knot vectors bypass cache mechanism

2. **NURBS Optimizations**
   - Efficient weight handling in curve evaluation
   - Smart caching of rational basis functions
   - Optimized knot insertion/removal algorithms
   - Minimal recomputation during weight changes

2. **Memory Management**
   - Pre-allocation of vectors to minimize reallocations
   - Efficient vector operations
   - Smart memory reuse in calculations

3. **Validation**
   - Early validation of control points
   - Proper error handling with descriptive messages
   - Graceful handling of edge cases

### Adaptive Point Generation

The spline curve is rendered using an adaptive point generation algorithm that:
- Starts with a minimum number of uniformly spaced points
- Calculates curvature at each point
- Adds more points in regions of high curvature
- Respects minimum and maximum point count constraints
- Uses a curvature tolerance parameter to control adaptation

Benefits:
- More efficient rendering by using fewer points in straight sections
- Better accuracy in curved regions where more detail is needed
- Customizable balance between performance and visual quality

Parameters:
- `minPoints`: Minimum number of points to generate (default: 50)
- `maxPoints`: Maximum number of points to generate (default: 500)
- `curvatureTolerance`: Controls point density in curved regions (default: 0.1)
  - Lower values create more points in curved areas
  - Higher values reduce point count but may decrease visual quality

### Curvature Calculation

The tool includes accurate curvature calculation using:
- Numerical differentiation for derivatives
- Standard curvature formula for planar curves
- Robust handling of special cases and numerical stability

Uses:
- Guides adaptive point generation
- Can be used for geometric analysis
- Helps in detecting sharp corners or smooth transitions

### Curve Splitting

The Spline tool provides functionality to split a curve into two separate splines at any parameter value:

#### Splitting Curves
- `splitAtParameter(u)`: Splits the spline into two curves at parameter value u
  - Creates two new splines that together represent the original curve
  - Maintains curve properties:
    - Degree preservation
    - Continuity at split point
    - Shape preservation
    - Weight preservation for NURBS
  - Parameters:
    - `u`: Parameter value between 0 and 1 where to split
  - Returns:
    - Pair of splines representing the two parts
  - Applications:
    - Curve trimming
    - Creating partial curves
    - Breaking complex curves into simpler parts
    - Isolating specific curve segments

#### Implementation Details
- Inserts knot at split point with multiplicity equal to degree
- Ensures C^(-1) continuity at split point
- Properly handles:
  - Rational curves (NURBS)
  - Custom knot vectors
  - Weights and control points
- Normalizes parameter ranges of resulting curves to [0,1]

#### Best Practices
- Choose split points carefully to maintain curve quality
- Consider using parameter values where knots already exist
- Verify continuity at split point when needed
- Handle resulting curves independently

### Closest Point Functionality

The Spline tool includes methods for finding the closest point on a curve to any given point in space:

#### Finding Closest Points
- `findClosestPoint(x, y, tolerance, maxIterations)`: Finds the closest point on the spline to a given point
  - Uses a hybrid approach:
    1. Initial sampling to find approximate closest point
    2. Newton iteration for precise refinement
  - Parameters:
    - `x, y`: Target point coordinates
    - `tolerance`: Convergence tolerance (default: 1e-6)
    - `maxIterations`: Maximum Newton iterations (default: 20)
  - Returns:
    - Closest point coordinates on the spline
    - Parameter value u where the point occurs
  - Applications:
    - User interaction (curve selection)
    - Snapping functionality
    - Point-on-curve constraints
    - Distance measurements

### Arc Length Functionality

The Spline tool includes arc length computation capabilities:

#### Arc Length Calculation
- `calculateArcLength(tolerance)`: Computes the total length of the spline curve
  - Uses Gaussian quadrature for efficient numerical integration
  - Adjustable tolerance for precision control
  - Returns exact length for linear splines
  - Handles degenerate cases (empty spline, insufficient control points)

#### Parameter by Length
- `parameterAtLength(distance, tolerance)`: Finds parameter value at a given arc length
  - Enables uniform spacing of points along the curve
  - Uses binary search with numerical integration
  - Handles out-of-bounds distances gracefully
  - Returns 0.0 for distance ≤ 0, 1.0 for distance ≥ curve length

#### Point by Length
- `pointAtLength(distance, tolerance)`: Gets point coordinates at a specific arc length
  - Combines parameterAtLength and evaluatePoint
  - Useful for:
    - Placing features at specific distances
    - Creating uniform point distributions
    - Path following applications
    - Distance measurements

### Mathematical Foundation

B-spline curves are defined by:

```
C(u) = Σ(i=0 to n) Pi * Ni,p(u)
```

Where:
- C(u) is the curve point at parameter u
- Pi are the control points
- Ni,p are the basis functions of degree p
- n is the number of control points minus 1

#### Derivatives

The first derivative of a B-spline curve is given by:

```
C'(u) = Σ(i=0 to n-1) p/(ui+p+1 - ui+1) * (Pi+1 - Pi) * Ni,p-1(u)
```

The second derivative follows a similar pattern:

```
C''(u) = Σ(i=0 to n-2) p(p-1)/((ui+p+1 - ui+1)(ui+p - ui)) * (Pi+2 - 2Pi+1 + Pi) * Ni,p-2(u)
```

These derivatives provide important geometric information:
- First derivative: Tangent vector and velocity
- Second derivative: Curvature and acceleration
- Together they enable:
  - G1/G2 continuity analysis
  - Curvature computation
  - Path planning
  - Surface normal calculation

## Best Practices

1. **Control Point Management**
   - Start with minimum required points for desired degree
   - Add points strategically for shape control
   - Avoid placing points too close together
   - Consider using lower degrees for simpler curves

2. **Performance Considerations**
   - Use appropriate degree for the task
   - Leverage knot vector caching
   - Pre-allocate vectors when possible
   - Choose suitable tolerances for calculations

3. **Error Handling**
   - Validate input parameters
   - Check for sufficient control points
   - Handle edge cases gracefully
   - Provide meaningful error messages

4. **Integration Tips**
   - Use with geometric constraints
   - Combine with other sketch tools
   - Consider curve continuity at connections
   - Leverage arc length for uniform distributions

## Error Handling

The tool includes robust error handling for common issues:

- Insufficient control points for degree
- Invalid degree values
- Out-of-bounds parameter values
- Memory management for large curves

Error messages are descriptive and suggest corrective actions.

## Performance Considerations

- Curve evaluation optimized with knot vector caching
- Adaptive point generation based on curve complexity
- Efficient memory management for control point storage
- Smart caching strategies for repeated calculations
