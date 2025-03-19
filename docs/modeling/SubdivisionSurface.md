# Subdivision Surface

The SubdivisionSurface class implements Catmull-Clark subdivision surfaces, providing a powerful tool for creating smooth, organic shapes from coarse control meshes.

## Overview

Subdivision surfaces are a method of creating smooth surfaces from simpler polygonal meshes. The Catmull-Clark algorithm:
- Preserves sharp features through edge tagging
- Handles arbitrary topology
- Produces C2 continuous surfaces (except at extraordinary vertices)
- Creates quad-based meshes ideal for animation and further modeling

## Usage

```cpp
// Create a control mesh
std::vector<Point3D> vertices = {
    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},  // Base vertices
    {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}   // Top vertices
};

std::vector<Edge> edges = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom edges
    {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top edges
    {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Side edges
};

std::vector<Face> faces;
Face face;

// Front face
face.edge_indices = {0, 9, 4, 8};
face.edge_directions = {true, true, false, false};
faces.push_back(face);

// Right face
face.edge_indices = {1, 10, 5, 9};
face.edge_directions = {true, true, false, false};
faces.push_back(face);

// Back face
face.edge_indices = {2, 11, 6, 10};
face.edge_directions = {true, true, false, false};
faces.push_back(face);

// Left face
face.edge_indices = {3, 8, 7, 11};
face.edge_directions = {true, true, false, false};
faces.push_back(face);

// Create subdivision surface
auto surface = SubdivisionSurface::Create(vertices, edges, faces);

// Mark edges as sharp if needed
surface->SetEdgeSharpness(0, true);  // Make bottom front edge sharp

// Perform subdivision
auto refined = surface->Subdivide();

// Convert to solid body after multiple subdivisions
auto body = surface->ToSolidBody(3);  // 3 iterations
```

## Features

### Sharp Edge Support
- Edges can be marked as sharp to preserve features
- Sharp edges maintain their character through subdivision
- Vertices where multiple sharp edges meet maintain their position

### Topology Handling
- Works with arbitrary polygonal meshes
- Automatically handles T-junctions and non-manifold geometry
- Preserves mesh connectivity through subdivision

### Performance
- Efficient implementation using indexed geometry
- Optimized vertex/edge/face point calculations
- Memory-efficient data structures

## Technical Details

### Subdivision Rules

The Catmull-Clark algorithm applies these rules at each subdivision step:

1. Face Points
   - Computed as average of all vertices in face
   - Creates new vertices at face centers

2. Edge Points
   - For smooth edges: average of:
     * Two edge endpoints
     * Two adjacent face points
   - For sharp edges: simple edge midpoint

3. Vertex Points
   - For regular vertices: weighted average of:
     * Original vertex position (n-3)
     * Average of edge midpoints (2)
     * Average of face points (1)
   - For vertices on sharp edges: average of:
     * Original vertex position
     * Sharp edge endpoints

### Implementation Notes

- Uses half-edge data structure for efficient topology queries
- Maintains consistent winding order for faces
- Handles boundary cases and non-manifold geometry
- Preserves UV coordinates and other attributes through subdivision

## Performance Considerations

- Memory usage scales with subdivision level (4^n faces)
- Consider limiting maximum subdivision iterations
- Use sharp edges strategically to reduce geometry
- Pre-compute subdivision for static meshes

## Example Applications

1. Organic Modeling
   - Character meshes
   - Natural forms
   - Smooth transitions

2. Industrial Design
   - Product surfaces
   - Automotive modeling
   - Architectural elements

3. Animation
   - Deformable meshes
   - Character rigging
   - Motion systems
