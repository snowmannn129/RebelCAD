# Mesh Generator

## Overview

The RebelCAD Mesh Generator is a robust system for creating high-quality tetrahedral meshes for finite element analysis (FEA). It provides automatic mesh generation with quality controls, optimization capabilities, and comprehensive validation.

## Features

- Automatic tetrahedral mesh generation from 3D geometry
- Configurable mesh quality parameters
- Mesh optimization and refinement
- Quality metrics and validation
- Comprehensive error handling
- Statistical analysis of mesh quality

## Usage

### Basic Mesh Generation

```cpp
#include "simulation/mesh/MeshGenerator.h"

using namespace RebelCAD::Simulation;

// Create mesh generator instance
MeshGenerator generator;

// Configure mesh parameters (optional)
MeshParameters params;
params.maxElementSize = 0.5f;
params.minElementSize = 0.1f;
params.aspectRatioLimit = 4.0f;
generator.setParameters(params);

// Generate mesh from input geometry
std::vector<glm::vec3> vertices = /* your input vertices */;
std::vector<uint32_t> indices = /* your input triangle indices */;

try {
    size_t elementCount = generator.generateMesh(vertices, indices);
    std::cout << "Generated " << elementCount << " elements" << std::endl;
    
    // Optimize mesh quality
    float improvement = generator.optimizeMesh();
    std::cout << "Quality improved by " << improvement << "%" << std::endl;
    
    // Get mesh statistics
    std::cout << generator.getMeshStatistics() << std::endl;
} catch (const MeshGenerationError& e) {
    std::cerr << "Mesh generation failed: " << e.what() << std::endl;
}
```

### Mesh Parameters

The `MeshParameters` struct allows fine-tuning of the mesh generation process:

| Parameter | Description | Default |
|-----------|-------------|---------|
| maxElementSize | Maximum size of mesh elements | 1.0 |
| minElementSize | Minimum size of mesh elements | 0.1 |
| aspectRatioLimit | Maximum allowed aspect ratio | 5.0 |
| gradingRate | Maximum size ratio between adjacent elements | 1.2 |
| preserveFeatures | Preserve sharp features and boundaries | true |
| optimizationPasses | Number of optimization passes | 3 |

### Quality Metrics

The mesh generator uses several metrics to assess mesh quality:

1. **Element Quality (0-1)**
   - 0: Poor quality
   - 1: Perfect quality
   - Based on shape measures including:
     - Volume ratio
     - Dihedral angles
     - Edge length ratios

2. **Aspect Ratio**
   - Ratio of circumradius to inradius
   - Lower values indicate better quality
   - Limited by `aspectRatioLimit` parameter

### Best Practices

1. **Parameter Configuration**
   - Start with default parameters
   - Adjust based on specific requirements
   - Balance quality vs performance

2. **Mesh Generation**
   - Ensure input geometry is clean and manifold
   - Use appropriate element size for your analysis
   - Validate mesh before simulation

3. **Optimization**
   - Run optimization when high quality is critical
   - Monitor quality improvements
   - Consider multiple optimization passes

4. **Validation**
   - Always check mesh statistics
   - Verify element quality metrics
   - Ensure mesh meets simulation requirements

## Error Handling

The mesh generator uses the `MeshGenerationError` class for error reporting. Common errors include:

- Invalid input geometry
- Insufficient vertices
- Invalid parameter values
- Quality constraint violations

## Future Improvements

The current implementation includes placeholders for several advanced features:

1. **Delaunay Tetrahedralization**
   - Will implement Bowyer-Watson algorithm
   - Improved quality guarantees

2. **Adaptive Refinement**
   - Size-based refinement
   - Feature-based refinement
   - Quality-based refinement

3. **Advanced Optimization**
   - Smoothing algorithms
   - Edge/face swapping
   - Topology optimization

4. **Quality Metrics**
   - Additional quality measures
   - Performance optimization
   - Parallel processing support

## Integration with RebelCAD

The mesh generator is designed to integrate seamlessly with RebelCAD's simulation pipeline:

1. **Pre-processing**
   - Automatic geometry cleanup
   - Feature detection
   - Boundary condition preparation

2. **Mesh Generation**
   - Quality-controlled tetrahedral mesh
   - Boundary layer handling
   - Multi-region support

3. **Post-processing**
   - Quality analysis
   - Visualization
   - Export capabilities

## Contributing

When extending the mesh generator:

1. Follow existing code structure
2. Maintain comprehensive unit tests
3. Update documentation
4. Consider performance implications
5. Preserve backward compatibility

## See Also

- [Finite Element Analysis](../simulation/FEA.md)
- [Simulation Setup](../simulation/SimulationSetup.md)
- [Mesh Quality Guidelines](../simulation/MeshQuality.md)
