# Memory Leak Detector

The Memory Leak Detector is a core component of RebelCAD's memory management system, designed to track and identify memory leaks during development and testing. It works in conjunction with the Memory Pool and Smart Resource Manager to provide comprehensive memory management capabilities.

## Features

- Real-time memory allocation tracking
- Detailed leak reports with stack traces
- Thread-safe implementation
- Minimal performance overhead when disabled
- Integration with existing memory management tools
- Windows-native stack trace capture

## Usage

### Basic Usage

```cpp
// Memory tracking is enabled by default
// Use the REBEL_NEW and REBEL_DELETE macros for tracked allocations
void* ptr = REBEL_NEW(sizeof(MyClass));
// ... use the memory ...
REBEL_DELETE(ptr);
```

### Checking for Leaks

```cpp
auto& detector = MemoryLeakDetector::getInstance();

if (detector.checkForLeaks()) {
    std::string report = detector.generateLeakReport();
    std::cout << report << std::endl;
}
```

### Controlling Leak Detection

```cpp
auto& detector = MemoryLeakDetector::getInstance();

// Disable leak detection (e.g., for performance-critical sections)
detector.setEnabled(false);

// Re-enable leak detection
detector.setEnabled(true);

// Clear all tracking data
detector.reset();
```

## Integration with Memory Management System

The Memory Leak Detector complements RebelCAD's existing memory management tools:

- **Memory Pool**: For efficient allocation of small objects
- **Smart Resource Manager**: For RAII-based resource management
- **Memory Leak Detector**: For identifying memory leaks during development

## Leak Report Format

The leak report provides detailed information about each detected leak:

```
Memory Leak Report
=================

Found 2 memory leak(s):

Leak #1
Address: 0x7fff1234
Size: 100 bytes
Allocated in: src/modeling/Mesh.cpp:156
Stack trace:
  Mesh::allocateVertices in src/modeling/Mesh.cpp:156
  Mesh::resize in src/modeling/Mesh.cpp:89
  ModelLoader::loadMesh in src/io/ModelLoader.cpp:234

Leak #2
Address: 0x7fff5678
Size: 200 bytes
Allocated in: src/graphics/Texture.cpp:78
Stack trace:
  Texture::loadMipMaps in src/graphics/Texture.cpp:78
  TextureLoader::load in src/graphics/TextureLoader.cpp:145

Total memory leaked: 300 bytes
```

## Best Practices

1. **Use RAII When Possible**
   - Prefer smart pointers and containers
   - Use the Smart Resource Manager for complex resources

2. **Strategic Leak Detection**
   - Enable detailed tracking during development/testing
   - Disable for release builds or performance-critical sections

3. **Regular Checks**
   - Add leak checks to unit tests
   - Verify no leaks after major operations
   - Monitor total memory usage over time

4. **Debug Builds**
   - Define REBEL_MEMORY_TRACKING_ENABLED in debug builds
   - Keep disabled in release builds for optimal performance

## Implementation Details

### Memory Tracking

The detector uses a thread-safe hash map to track allocations:
- Key: Memory address
- Value: Allocation record (size, location, stack trace)

### Stack Traces

Windows-native stack trace capture using:
- DbgHelp API for symbol resolution
- Automatic symbol demangling
- File and line information when available

### Thread Safety

All operations are protected by a mutex to ensure thread-safe:
- Allocation recording
- Deallocation recording
- Report generation

## Performance Considerations

1. **Overhead**
   - Minimal when disabled (single bool check)
   - Stack trace capture has the highest overhead
   - Thread synchronization impact in multi-threaded scenarios

2. **Memory Usage**
   - Tracking data scales with number of active allocations
   - Stack traces can consume significant memory
   - Regular reset() calls can help manage memory usage

## Future Improvements

1. **Planned Enhancements**
   - Allocation pattern analysis
   - Memory usage visualization
   - Integration with debugger
   - Custom allocation categories
   - Leak detection in release builds

2. **Performance Optimizations**
   - Lock-free tracking for high-performance scenarios
   - Sampling-based tracking for production
   - Compressed stack traces

## See Also

- [Memory Pool Documentation](MemoryPool.md)
- [Smart Resource Manager Documentation](SmartResourceManager.md)
- [Command System Documentation](CommandSystem.md)
