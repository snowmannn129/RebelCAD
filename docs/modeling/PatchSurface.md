# Patch Surface

The PatchSurface class provides functionality for creating and manipulating parametric surface patches in RebelCAD. It supports three types of patches:

- Bicubic Bezier patches
- Gregory patches for n-sided regions
- Blend surfaces between adjacent patches

## Features

- Parametric surface evaluation
- Normal vector computation
- G1/G2 continuity enforcement
- Mesh generation for visualization
- Sharp feature preservation

## Usage Examples

### Creating a Bicubic Bezier Patch

```cpp
// Create 4x4 control point grid
std::vector<std::vector<Point3D>> controlPoints = {
    {Point3D(0,0,0), Point3D(1,0,1), Point3D(2,0,1), Point3D(3,0,0)},
    {Point3D(0,1,1), Point3D(1,1,2), Point3D(2,1,2), Point3D(3,1,1)},
    {Point3D(0,2,1), Point3D(1,2,2), Point3D(2,2,2), Point3D(3,2,1)},
    {Point3D(0,3,0), Point3D(1,3,1), Point3D(2,3,1), Point3D(3,3,0)}
};

// Optional tangent scale factors
std::vector<double> tangentScale = {1.0, 1.0, 1.0, 1.0};

// Create the patch
auto patch = PatchSurface::CreateBicubic(controlPoints, tangentScale);
```

### Creating a Gregory Patch

```cpp
// Define boundary curves
std::vector<std::vector<Point3D>> boundaryPoints = {
    {Point3D(0,0,0), Point3D(1,0,0), Point3D(2,0,0)},
    {Point3D(2,0,0), Point3D(2,1,0), Point3D(2,2,0)},
    {Point3D(2,2,0), Point3D(1,2,0), Point3D(0,2,0)},
    {Point3D(0,2,0), Point3D(0,1,0), Point3D(0,0,0)}
};

// Define center point
Point3D centerPoint(1, 1, 1);

// Create the patch
auto patch = PatchSurface::CreateGregory(boundaryPoints, centerPoint);
```

### Creating a Blend Surface

```cpp
// Create two patches to blend between
auto patch1 = PatchSurface::CreateBicubic(controlPoints1);
auto patch2 = PatchSurface::CreateBicubic(controlPoints2);

// Create blend surface with 0.5 blend factor
auto blendPatch = PatchSurface::CreateBlend(patch1, patch2, 0.5);
```

### Surface Evaluation

```cpp
// Evaluate surface point at parameters (u,v)
Point3D point = patch->Evaluate(0.5, 0.5);

// Evaluate surface normal
Vector3D normal = patch->EvaluateNormal(0.5, 0.5);
```

### Mesh Generation

```cpp
// Generate triangle mesh with 20x20 resolution
auto mesh = patch->ToMesh(20);
```

## Technical Details

### Bicubic Bezier Patches

Bicubic Bezier patches use a 4x4 grid of control points and cubic Bernstein basis functions to define a smooth parametric surface. The surface is defined by:

```
P(u,v) = Σ(i=0..3) Σ(j=0..3) P[i,j] * B[i](u) * B[j](v)
```

where:
- P[i,j] are the control points
- B[i](t) are the cubic Bernstein basis functions

### Gregory Patches

Gregory patches extend the concept of Bezier patches to handle n-sided regions while maintaining G1 continuity. They use rational functions to blend between boundary curves.

### Blend Surfaces

Blend surfaces create smooth transitions between two existing patches. The blending is controlled by a blend factor that determines how the transition occurs.

## Performance Considerations

- Surface evaluation is O(1) for all patch types
- Mesh generation is O(n^2) where n is the resolution
- Memory usage is proportional to the number of control points

## Limitations

- Gregory patches may have singularities at extraordinary points
- Blend surfaces require compatible patch parameterizations
- G2 continuity is not guaranteed for all configurations
