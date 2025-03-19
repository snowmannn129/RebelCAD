# Smart Resource Manager

The Smart Resource Manager provides efficient and safe resource management in RebelCAD, integrating with the Memory Pool system for optimal memory usage.

## Features

- RAII-style resource management
- Automatic reference counting
- Type-safe resource handling
- Memory Pool integration
- Custom cleanup handlers
- Thread-safe operation

## Usage

### Creating Resources

```cpp
// Create a managed resource
auto resource = SmartResourceManager::getInstance().createResource<MyType>(args...);

// Resource will be automatically cleaned up when no longer referenced
```

### Resource Tracking

```cpp
// Get count of active resources of a type
size_t count = SmartResourceManager::getInstance().getResourceCount<MyType>();
```

### Custom Cleanup

```cpp
// Register custom cleanup handler
SmartResourceManager::getInstance().registerCleanupHandler<MyType>(
    [](MyType* resource) {
        // Custom cleanup logic
    }
);
```

## Integration with Memory Pool

The Smart Resource Manager automatically uses the Memory Pool system for efficient memory allocation and deallocation. This provides:

- Reduced memory fragmentation
- Faster allocation/deallocation
- Memory usage tracking
- Automatic memory cleanup

## Thread Safety

All operations are thread-safe, protected by internal mutex locks where necessary.

## Best Practices

1. Always use createResource() instead of manual allocation
2. Let shared_ptr handle resource lifecycle
3. Register cleanup handlers for resources requiring special cleanup
4. Monitor resource counts during development to catch leaks

## Example

```cpp
class Mesh {
public:
    Mesh(const std::string& name) : m_name(name) {}
private:
    std::string m_name;
};

// Create managed mesh
auto mesh = SmartResourceManager::getInstance().createResource<Mesh>("MyMesh");

// Resource count increases
assert(SmartResourceManager::getInstance().getResourceCount<Mesh>() == 1);

// Resource automatically cleaned up when mesh goes out of scope
