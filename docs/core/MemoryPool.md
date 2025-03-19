# Memory Pool System

The Memory Pool system provides efficient memory management for fixed-size allocations in RebelCAD. It reduces memory fragmentation and improves performance by managing memory in blocks rather than individual allocations.

## Features

- Fixed-size block allocation
- Thread-safe operations
- Automatic memory block expansion
- Memory leak detection
- Efficient memory reuse
- Reduced fragmentation

## Usage

### Basic Usage

```cpp
// Create a memory pool for TestStruct with default block size (1024)
MemoryPool<TestStruct> pool;

// Allocate memory for a TestStruct
TestStruct* ptr = pool.allocate();

// Use the allocated memory
ptr->someMethod();

// Deallocate when done
pool.deallocate(ptr);
```

### Custom Block Size

```cpp
// Create a pool with custom block size of 128 elements
MemoryPool<TestStruct, 128> customPool;
```

### Thread Safety

The MemoryPool is thread-safe and can be safely used from multiple threads:

```cpp
// Create a shared pool
MemoryPool<TestStruct> sharedPool;

// Use from multiple threads
std::thread t1([&]() {
    auto ptr = sharedPool.allocate();
    // Use ptr...
    sharedPool.deallocate(ptr);
});

std::thread t2([&]() {
    auto ptr = sharedPool.allocate();
    // Use ptr...
    sharedPool.deallocate(ptr);
});
```

## API Reference

### Constructor

```cpp
explicit MemoryPool(size_t initialBlocks = 1)
```
Creates a new memory pool with the specified number of initial blocks.

### Methods

#### allocate()
```cpp
T* allocate()
```
Allocates memory for a single object of type T.
- Returns: Pointer to allocated memory
- Throws: std::bad_alloc if allocation fails

#### deallocate(T* ptr)
```cpp
void deallocate(T* ptr)
```
Deallocates previously allocated memory.
- Parameters: ptr - Pointer to memory to deallocate
- Throws: std::runtime_error if ptr is null or not from this pool

#### getActiveAllocations()
```cpp
size_t getActiveAllocations() const
```
Returns the number of currently active allocations.

#### getCapacity()
```cpp
size_t getCapacity() const
```
Returns the total capacity of the pool in number of objects.

## Best Practices

1. **Choose Appropriate Block Size**
   - Too small: Frequent block allocations
   - Too large: Wasted memory
   - Default (1024) works well for most cases

2. **Memory Leak Prevention**
   - Always pair allocate() with deallocate()
   - Use RAII wrappers when possible
   - The destructor will assert in debug builds if leaks are detected

3. **Performance Optimization**
   - Reuse pools for objects of the same type
   - Pre-allocate blocks if you know the required capacity
   - Monitor active allocations to detect potential leaks

## Implementation Details

The MemoryPool uses a block-based allocation strategy:

1. Memory is allocated in fixed-size blocks
2. Free memory is tracked in a list
3. When a block is full, a new one is automatically allocated
4. Deallocated memory is returned to the free list for reuse
5. Thread safety is ensured through mutex locks

## Example Use Cases

### Particle Systems
```cpp
struct Particle {
    Vector3 position;
    Vector3 velocity;
    float lifetime;
};

MemoryPool<Particle> particlePool;
```

### Geometry Caching
```cpp
struct GeometryCache {
    std::vector<Vector3> vertices;
    std::vector<int> indices;
};

MemoryPool<GeometryCache> geometryPool;
```

## Performance Considerations

- Allocation time: O(1) amortized
- Deallocation time: O(1)
- Memory overhead: One pointer per free block + block management
- Thread contention: Minimal, only during allocation/deallocation
