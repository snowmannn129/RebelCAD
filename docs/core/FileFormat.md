# RebelCAD File Format Specification (.rebelcad)

## Overview
The RebelCAD file format (.rebelcad) is designed for efficient storage and retrieval of CAD models, supporting version control, compression, and incremental updates.

## File Structure
```
[Header]
- Magic Number (8 bytes): "REBELCAD"
- Version (4 bytes): Major.Minor.Patch
- Flags (4 bytes): Compression, Encryption, etc.
- Timestamp (8 bytes): Creation/Modification time
- Checksum (32 bytes): SHA-256 hash

[Metadata Section]
- Project Info
- Units
- Materials
- Custom Properties
- Dependencies

[History Section]
- Version History
- Change Log
- Author Information
- Timestamps

[Model Data]
1. Scene Graph
   - Hierarchy
   - Transformations
   - Visibility
   - Instance References

2. Geometry Data
   - Vertices
   - Edges
   - Faces
   - UV Coordinates
   - Normals
   - Colors/Materials

3. Sketch Data
   - 2D Geometry
   - Constraints
   - Construction Lines
   - Dimensions

4. Feature Tree
   - Operations History
   - Parameters
   - Dependencies
   - Constraints

5. Assembly Data
   - Component References
   - Joints/Mates
   - Positions
   - Constraints

[Resource Section]
- Textures
- Materials
- Custom Components
- External References

[Footer]
- Section Offsets
- Integrity Check
- EOF Marker
```

## Data Organization

### Binary Format
- All numeric values are stored in little-endian format
- Floating-point numbers use IEEE 754 double precision
- Strings are UTF-8 encoded with length prefix
- Arrays are prefixed with 32-bit length
- Coordinates use right-handed coordinate system

### Compression
- Per-section compression using zstd
- Geometry data uses specialized mesh compression
- Optional whole-file compression for smaller files

### Version Control Support
- Incremental updates stored as deltas
- Reference counting for shared resources
- Efficient diff/merge operations
- Branch/variant support

### Performance Optimizations
- Memory-mapped file support
- Streaming-friendly structure
- Lazy loading capabilities
- Cache-friendly data layout

## Implementation Guidelines

### Reading Process
1. Validate header and checksum
2. Load metadata and resource map
3. Initialize memory pools
4. Stream geometry on demand
5. Build scene graph
6. Load feature history

### Writing Process
1. Generate metadata
2. Serialize scene graph
3. Compress geometry data
4. Write resources
5. Update checksums
6. Generate change log

### Error Handling
- CRC32 per section
- Redundant metadata
- Recovery markers
- Automatic backups
- Transaction logging

### Thread Safety
- Read-write locking
- Atomic operations
- Version stamping
- Resource pooling

## Compatibility

### Import/Export Support
- DXF
- STEP
- IGES
- STL
- OBJ
- FBX

### Extension System
- Plugin data sections
- Custom metadata
- User-defined features
- External references

## Security

### Data Integrity
- Section checksums
- Full file validation
- Corruption detection
- Auto-recovery support

### Access Control
- Optional encryption
- Digital signatures
- Permission metadata
- Audit logging

## Performance Requirements

### File Operations
- Load Time: < 100ms for typical files
- Save Time: < 500ms for incremental saves
- Memory Usage: < 1.5x file size
- Streaming: Support for files > 1GB

### Optimization Targets
- Minimize disk I/O
- Reduce memory fragmentation
- Enable parallel processing
- Support memory mapping

## Future Considerations

### Planned Features
- Cloud storage integration
- Real-time collaboration
- Version control system
- Asset management

### Extensibility
- Custom data sections
- Plugin architecture
- API versioning
- Migration tools
