# RebelCAD Build and Debug Scripts

This directory contains scripts for building, debugging, and managing logs for the RebelCAD project. These scripts are designed to help manage the complexity of the build process and handle large error logs that might exceed token or content limits.

## Table of Contents

1. [Build Scripts](#build-scripts)
2. [Testing Scripts](#testing-scripts)
3. [Debugging Scripts](#debugging-scripts)
4. [Log Management Scripts](#log-management-scripts)
5. [Usage Examples](#usage-examples)

## Build Scripts

### build_component.ps1

Builds a specific component of RebelCAD.

```powershell
.\scripts\build_component.ps1 -Component simulation -BuildType Debug -LogLevel VERBOSE
```

Parameters:
- `-Component`: (Required) The component to build (e.g., simulation, core, modeling, ui, sketching, assembly, constraints, graphics)
- `-BuildType`: (Optional) The build type (Debug, Release, RelWithDebInfo, MinSizeRel). Default: Release
- `-LogLevel`: (Optional) The log level (ERROR, WARNING, INFO, DEBUG, VERBOSE). Default: INFO

### rebuild_modified.ps1

Rebuilds only the targets affected by modified files.

```powershell
.\scripts\rebuild_modified.ps1 -Component simulation -Verbose
```

Parameters:
- `-BuildDir`: (Optional) The build directory. Default: build
- `-Component`: (Optional) Only rebuild targets in this component
- `-Verbose`: (Optional) Show verbose output
- `-BuildType`: (Optional) The build type. Default: Release

## Testing Scripts

### test_component.ps1

Runs tests for a specific component.

```powershell
.\scripts\test_component.ps1 -Component simulation -TestFilter "FEASystem*" -Verbose
```

Parameters:
- `-Component`: (Required) The component to test
- `-TestFilter`: (Optional) Filter for specific tests using Google Test filter syntax
- `-Verbose`: (Optional) Show verbose output

### run_tests_by_category.ps1

Runs tests by category.

```powershell
.\scripts\run_tests_by_category.ps1 -Category SIMULATION -Verbose
```

Parameters:
- `-Category`: (Required) The test category (SMALL, MEDIUM, LARGE, SIMULATION, CORE, etc.)
- `-Verbose`: (Optional) Show verbose output
- `-BuildType`: (Optional) The build type. Default: Release

## Debugging Scripts

### debug_incremental.ps1

Runs tests incrementally for debugging, with the option to save state.

```powershell
.\scripts\debug_incremental.ps1 -Component simulation -StartTest "FEASystem.Initialize" -EndTest "FEASystem.Solve" -SaveState
```

Parameters:
- `-Component`: (Required) The component to debug
- `-StartTest`: (Optional) The test to start with
- `-EndTest`: (Optional) The test to end with
- `-SaveState`: (Optional) Save state for each test

## Log Management Scripts

### filter_logs.ps1

Filters and manages build logs.

```powershell
.\scripts\filter_logs.ps1 -BuildCommand "cmake --build build --config Debug" -ErrorLevel WARNING -ShowProgress
```

Parameters:
- `-BuildCommand`: (Required) The build command to run
- `-OutputFile`: (Optional) The output file. Default: build_log.txt
- `-ErrorLevel`: (Optional) The error level to filter (ERROR, WARNING, INFO). Default: ERROR
- `-ShowProgress`: (Optional) Show progress during the build

### analyze_log_chunks.ps1

Analyzes large log files in manageable chunks.

```powershell
.\scripts\analyze_log_chunks.ps1 -LogFile "logs/build_log_2025-03-22_15-30-45.txt" -ChunkSize 500
```

Parameters:
- `-LogFile`: (Required) The log file to analyze
- `-ChunkSize`: (Optional) The number of lines per chunk. Default: 1000
- `-OutputDir`: (Optional) The output directory. Default: log_chunks

### rotate_logs.ps1

Rotates and archives log files.

```powershell
.\scripts\rotate_logs.ps1 -KeepDays 14 -Compress
```

Parameters:
- `-KeepDays`: (Optional) Number of days to keep logs. Default: 7
- `-LogDir`: (Optional) The log directory. Default: logs
- `-Compress`: (Optional) Compress logs into zip files

## Usage Examples

### Building and Testing a Component

```powershell
# Build the simulation component in debug mode
.\scripts\build_component.ps1 -Component simulation -BuildType Debug

# Run all simulation tests
.\scripts\test_component.ps1 -Component simulation

# Run only FEA system tests
.\scripts\test_component.ps1 -Component simulation -TestFilter "FEASystem*"
```

### Debugging a Failed Test

```powershell
# Run tests incrementally, saving state for debugging
.\scripts\debug_incremental.ps1 -Component simulation -SaveState

# Analyze a large log file
.\scripts\analyze_log_chunks.ps1 -LogFile "logs/build_log.txt"
```

### Managing a Large Build

```powershell
# Build with filtered logs
.\scripts\filter_logs.ps1 -BuildCommand "cmake --build build --config Release" -ErrorLevel WARNING

# Rebuild only modified files
.\scripts\rebuild_modified.ps1 -Verbose

# Rotate and archive logs
.\scripts\rotate_logs.ps1 -Compress
```

### Running Tests by Category

```powershell
# Run small tests
.\scripts\run_tests_by_category.ps1 -Category SMALL

# Run simulation tests
.\scripts\run_tests_by_category.ps1 -Category SIMULATION -Verbose
```

## Additional Information

For more details on the build and debug strategy, see the [BuildAndDebugStrategy.md](../docs/BuildAndDebugStrategy.md) document.
