# NURBS Surface

## Overview

The NURBS (Non-Uniform Rational B-Spline) Surface implementation provides advanced surface modeling capabilities in RebelCAD. NURBS surfaces are mathematical models commonly used in CAD for representing curves and surfaces with a high degree of flexibility and precision.

## Features

- Creation of NURBS surfaces from control point grids
- Support for arbitrary degrees in U and V directions
- Weighted control points for rational surfaces
- Surface evaluation at arbitrary parameter values
- Surface derivative computation
- Conversion to tessellated solid bodies

## Usage

### Creating a NURBS Surface

```cpp
// Create a control point grid
std::vector<Point3D> controlPoints = {
    {0, 0, 0}, {1, 0, 0}, {2, 0, 0},
    {0, 1, 0}, {1, 1, 0}, {2, 1, 0},
    {0, 2, 0}, {1, 2, 0}, {2, 2, 0}
};

// Create a degree 2 surface with 3x3 control points
auto surface = NurbsSurface::Create(
    controlPoints,  // Control points in row-major order
    3,             // Number of control points in U direction
    3,             // Number of control points in V direction
    2,             // Degree in U direction
    2              // Degree in V direction
);
```

### Evaluating Points on the Surface

```cpp
// Evaluate surface at parameter values
Point3D point = surface->Evaluate(0.5, 0.5);  // Point at center of surface

// Compute surface derivatives
Point3D du = surface->EvaluateDerivative(0.5, 0.5, 1, 0);  // First derivative in U
Point3D dv = surface->EvaluateDerivative(0.5, 0.5, 0, 1);  // First derivative in V
```

### Modifying the Surface

```cpp
// Access and modify control points
Point3D point = surface->GetControlPoint(1, 1);
surface->SetControlPoint(1, 1, {1, 1, 1});

// Access and modify weights
double weight = surface->GetWeight(1, 1);
surface->SetWeight(1, 1, 2.0);
```

### Converting to Solid Body

```cpp
// Convert to tessellated solid body with specified resolution
auto body = surface->ToSolidBody(32, 32);  // 32x32 tessellation
```

## Technical Details

### Mathematical Foundation

NURBS surfaces are defined by the following components:
- A bidirectional grid of control points
- Two knot vectors (U and V directions)
- Weights for each control point
- Degrees in U and V directions

The surface point S(u,v) is computed as:

```
S(u,v) = ∑∑(N[i,p](u) * N[j,q](v) * w[i,j] * P[i,j]) / ∑∑(N[i,p](u) * N[j,q](v) * w[i,j])
```

where:
- N[i,p](u) are the basis functions of degree p in u direction
- N[j,q](v) are the basis functions of degree q in v direction
- w[i,j] are the weights
- P[i,j] are the control points

### Performance Considerations

- Surface evaluation complexity is O((p+1)(q+1)) where p,q are degrees
- Memory usage is O(mn) where m,n are number of control points
- Tessellation complexity is O(uDiv * vDiv) where uDiv,vDiv are tessellation divisions

## Integration with RebelCAD

The NURBS Surface implementation integrates with RebelCAD's core modeling system through:
- Conversion to SolidBody for Boolean operations
- Compatible Point3D geometry representation
- Standard error handling through RebelCAD::Error system

## Limitations

- Currently supports only first derivatives
- Uniform knot vectors only (non-uniform knot vectors planned for future)
- No direct surface-surface intersection computation
