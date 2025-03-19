# RebelCAD Development Procedure

## 1. Development Environment & Execution
- RebelCAD is developed in VSCode on Windows 11 using PowerShell
- Built using C++ with OpenGL/DirectX for rendering
- All development follows a test-driven approach with rigorous validation
- All UI elements must be rigorously tested and functional before submission
- All modules must connect properly before requesting approval

## 2. Project Structure
RebelCAD follows a modular structure:

```
RebelCAD/
├── src/                 # Source code
│   ├── core/            # Core functionality (memory management, event system)
│   ├── graphics/        # Graphics engine (rendering, viewport)
│   ├── modeling/        # Modeling tools (sketching, solid modeling)
│   ├── assembly/        # Assembly and constraints
│   ├── simulation/      # Simulation capabilities
│   ├── ui/              # User interface components
│   ├── utils/           # Utility functions
│   ├── main.cpp         # Program entry point
├── include/             # Header files
│   ├── core/            # Core header files
│   ├── graphics/        # Graphics header files
│   ├── modeling/        # Modeling header files
│   ├── assembly/        # Assembly header files
│   ├── simulation/      # Simulation header files
│   ├── ui/              # UI header files
├── tests/               # Test files
│   ├── core/            # Core tests
│   ├── graphics/        # Graphics tests
│   ├── modeling/        # Modeling tests
│   ├── assembly/        # Assembly tests
│   ├── simulation/      # Simulation tests
│   ├── ui/              # UI tests
│   ├── CMakeLists.txt   # Test build configuration
├── docs/                # Documentation
│   ├── core/            # Core documentation
│   ├── graphics/        # Graphics documentation
│   ├── modeling/        # Modeling documentation
│   ├── assembly/        # Assembly documentation
│   ├── simulation/      # Simulation documentation
│   ├── ui/              # UI documentation
│   ├── development/     # Development guides
├── external/            # External dependencies
├── build/               # Build output
├── CMakeLists.txt       # Build configuration
├── build.bat            # Build script
├── RebelCAD_Progress_Tracker.md  # Progress tracking
├── RebelCAD_Detailed_Development_Checklist.md  # Development checklist
```

## 3. Functional Testing & UI Verification
RebelCAD follows a rigorous testing process:

### Unit Tests for Core Components
- Each function/class must have unit tests before submission
- Tests should cover edge cases, exceptions, and normal behavior
- Store all test scripts in tests/ directory
- Use Google Test framework for C++ unit testing

### UI Component Testing
- Ensure all buttons, menus, and inputs work
- Verify UI elements correctly trigger backend functions
- Use automated UI testing frameworks
- Create visual regression tests for UI components

### Integration Testing
- After every UI implementation:
  - Test every event handler (button clicks, keyboard shortcuts)
  - Ensure UI updates reflect backend actions
  - Simulate user input to test flow
  - Test performance with large models

### Functional Feature Testing
- Write test scenarios for every feature:
  - Modeling: Test creation and modification of various shapes
  - Assembly: Test constraint application and solving
  - Graphics: Test rendering performance and accuracy
  - File operations: Test saving and loading models

### Regression Testing
- Do not break previous features when adding new code
- Before approving new code, re-run all tests
- Maintain a test suite that can be run automatically

## 4. C++ Coding Standards & Best Practices
- Follow modern C++ conventions (C++17/20)
- Use consistent naming conventions:
  - CamelCase for class names
  - snake_case for function and variable names
  - ALL_CAPS for constants
- Each file should be ≤ 500 lines
- If a file exceeds this, split it into multiple modules
- Use doxygen-style comments for all functions and classes

Example:
```cpp
/**
 * @brief Saves a model to the specified file path
 * @param filepath The path where the model will be saved
 * @param model The model to save
 * @return True if the save was successful, false otherwise
 */
bool save_model(const std::string& filepath, const Model& model) {
    try {
        // Implementation
        return true;
    } catch (const std::exception& e) {
        log_error("Failed to save model: {}", e.what());
        return false;
    }
}
```

- Use proper exception handling:
```cpp
try {
    // Code that might throw
} catch (const std::exception& e) {
    log_error("Error: {}", e.what());
    // Handle the error
}
```

- Use smart pointers instead of raw pointers:
```cpp
std::unique_ptr<Model> model = std::make_unique<Model>();
std::shared_ptr<Resource> resource = std::make_shared<Resource>();
```

- Use logging instead of print statements:
```cpp
#include "utils/logger.h"
// ...
Logger::debug("Model saved successfully");
Logger::error("Failed to load model: {}", error_message);
```

## 5. Managing Development Complexity
- Only implement one feature/component per development session
- Keep functions short and modular
- Use forward declarations to minimize header dependencies
- Implement proper memory management with RAII principles
- Track dependencies in CMakeLists.txt
- Use precompiled headers for common includes

## 6. Core Features & Modules
RebelCAD has the following core modules:

### Core Module (src/core/)
- Memory management system
- Event system
- Operation management (command pattern, undo/redo)
- File operations and data management

### Graphics Module (src/graphics/)
- Rendering engine
- Viewport management
- Camera system
- Lighting system
- Spatial partitioning

### Modeling Module (src/modeling/)
- Sketching tools
- Solid modeling operations
- Surface modeling
- Parametric design
- Feature history

### Assembly Module (src/assembly/)
- Component insertion and positioning
- Assembly tree management
- Constraints and joints
- Exploded view
- Motion analysis

### Simulation Module (src/simulation/)
- Finite Element Analysis
- Computational Fluid Dynamics
- Motion simulation
- Stress analysis

### UI Module (src/ui/)
- Window management
- Toolbars and menus
- Property panels
- Customizable workspaces
- Theme support

## 7. Automation for Testing
- Run All Tests Automatically:
```powershell
.\build\bin\run_tests.exe
```

- Generate Test Coverage Report:
```powershell
.\scripts\generate_coverage_report.ps1
```

- Run Performance Benchmarks:
```powershell
.\build\bin\run_benchmarks.exe
```

- Check Memory Leaks:
```powershell
.\scripts\check_memory_leaks.ps1
```

## 8. Development Workflow
- Task Breakdown:
  1. Break large tasks into smaller steps
  2. Create detailed implementation plan
  3. Write tests first (Test-Driven Development)
  4. Implement the feature
  5. Verify with tests
  6. Document the implementation

- Approval Workflow:
  1. Generate code and tests
  2. Test thoroughly before requesting approval
  3. Ensure UI elements properly connect to the backend
  4. Once approved, update progress tracker
  5. Move to the next task

## 9. Best Practices for Development
- Use Git for version control:
```powershell
git add .
git commit -m "Implemented constraint solver for assembly module"
```

- Create GitHub issues for tracking:
```powershell
.\scripts\create_github_issue.ps1 -title "Fix memory leak in graphics module" -body "Memory leak detected in the viewport rendering system" -labels "bug,high-priority"
```

- Keep modules focused:
  - No single file should exceed 500 lines
  - Split complex functionality into multiple files
  - Use proper abstraction and encapsulation

- Ensure proper memory management:
  - Use smart pointers (std::unique_ptr, std::shared_ptr)
  - Implement RAII (Resource Acquisition Is Initialization)
  - Run memory leak detection regularly

- Optimize performance:
  - Profile code to identify bottlenecks
  - Use appropriate data structures
  - Implement multi-threading for performance-critical operations
  - Use GPU acceleration where appropriate

## 10. Progress Tracking
- Update RebelCAD_Progress_Tracker.md after completing each task
- Run progress update script:
```powershell
.\scripts\update_progress.ps1
```

- Generate GitHub issues from progress tracker:
```powershell
.\scripts\generate_github_issues.ps1
```

- Update GitHub issues status:
```powershell
.\scripts\update_github_issues.ps1
```

## Final Notes
- Your goal is to develop a professional-grade CAD/3D modeling software
- Every UI element must be tested for functionality before requesting approval
- All features must be verified using unit, UI, and integration tests
- Do NOT generate untested or disconnected UI components
- DO ensure all modules connect properly before moving to the next task
