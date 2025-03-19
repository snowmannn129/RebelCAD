# Garbage Collector

The RebelCAD Garbage Collector provides automatic memory management through a hybrid approach combining reference counting and cycle detection. It is designed to be efficient, thread-safe, and minimally intrusive while integrating seamlessly with the existing memory management infrastructure.

## Features

- Automatic memory management
- Thread-safe reference counting
- Cycle detection for handling circular references
- Incremental collection to minimize pause times
- Configurable collection triggers and intervals
- Integration with MemoryLeakDetector
- Background collection thread

## Usage

### Basic Object Management

```cpp
// Create a managed object
auto* obj = REBEL_GC_NEW(MyClass, constructor_args);

// Add reference from one object to another
obj->reference = REBEL_GC_REF(otherObj);

// Remove reference
REBEL_GC_UNREF(obj->reference);
```

### Configuration

```cpp
GCConfig config;
config.memoryThreshold = 1024 * 1024 * 100;  // 100MB
config.collectionInterval = std::chrono::milliseconds(2000);  // 2 seconds
config.maxPauseTime = 5;  // 5ms maximum pause
config.isIncremental = true;  // Enable incremental collection

GarbageCollector::getInstance().configure(config);
```

### Manual Collection

While the garbage collector runs automatically based on configured thresholds and intervals, you can manually trigger collection when needed:

```cpp
// Trigger incremental collection
GarbageCollector::getInstance().collect();

// Force full collection
GarbageCollector::getInstance().collect(true);
```

### Memory Statistics

```cpp
auto [totalMemory, objectCount] = GarbageCollector::getInstance().getStats();
std::cout << "Total managed memory: " << totalMemory << " bytes\n";
std::cout << "Managed objects: " << objectCount << "\n";
```

## Best Practices

1. **Use REBEL_GC Macros**
   - Always use `REBEL_GC_NEW` for object creation
   - Use `REBEL_GC_REF` when storing references
   - Use `REBEL_GC_UNREF` when removing references
   - These macros ensure proper tracking and memory management

2. **Handle Circular References**
   - The garbage collector can detect and collect circular references
   - Always use `REBEL_GC_REF`/`REBEL_GC_UNREF` for potential circular references
   - Clean up references in destructors when possible

3. **Performance Considerations**
   - Configure collection thresholds based on your application's memory usage patterns
   - Use incremental collection for large applications to minimize pause times
   - Monitor memory statistics to optimize collection parameters

4. **Thread Safety**
   - The garbage collector is thread-safe by design
   - No additional synchronization needed when using GC macros
   - Collection can occur concurrently with normal operation

## Integration with Memory Leak Detection

The garbage collector automatically integrates with RebelCAD's MemoryLeakDetector:

- All allocations are tracked for leak detection
- Collection events are properly recorded
- Memory leaks can be detected even with garbage collection enabled

## Example: Managing Complex Object Relationships

```cpp
class Node {
public:
    Node() = default;
    ~Node() {
        // Clean up references in destructor
        REBEL_GC_UNREF(next);
        REBEL_GC_UNREF(prev);
    }

    void setNext(Node* n) {
        REBEL_GC_UNREF(next);  // Remove old reference
        next = REBEL_GC_REF(n);  // Add new reference
    }

    void setPrev(Node* p) {
        REBEL_GC_UNREF(prev);
        prev = REBEL_GC_REF(p);
    }

private:
    Node* next = nullptr;
    Node* prev = nullptr;
};

// Usage
auto* node1 = REBEL_GC_NEW(Node);
auto* node2 = REBEL_GC_NEW(Node);

node1->setNext(node2);
node2->setPrev(node1);

// When nodes are no longer needed
REBEL_GC_UNREF(node1);
REBEL_GC_UNREF(node2);
// Garbage collector will handle cleanup, even with circular references
```

## Troubleshooting

1. **Memory Leaks**
   - Use MemoryLeakDetector's reporting
   - Check for missing REBEL_GC_UNREF calls
   - Verify circular references are properly managed

2. **Performance Issues**
   - Adjust collection thresholds
   - Enable incremental collection
   - Monitor pause times and adjust maxPauseTime

3. **Threading Issues**
   - Ensure proper use of GC macros
   - Check for manual memory management mixing with GC
   - Verify thread safety in custom destructors

## Implementation Details

The garbage collector uses a hybrid approach:

1. **Reference Counting**
   - Immediate reclamation of unreferenced objects
   - Efficient for most common cases
   - Handles simple object relationships

2. **Cycle Detection**
   - Mark-and-sweep for circular references
   - Runs periodically or on threshold
   - Ensures complete cleanup

3. **Incremental Collection**
   - Splits work into small chunks
   - Minimizes application pauses
   - Configurable pause times

4. **Background Thread**
   - Automatic collection based on thresholds
   - Non-blocking operation
   - Configurable intervals
