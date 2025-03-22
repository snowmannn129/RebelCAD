# RebelCAD Build and Debug Strategy

This document outlines the strategy for building and debugging RebelCAD, with a focus on handling large error logs and outputs that might exceed token or content limits.

## Table of Contents
1. [Build Configuration](#build-configuration)
2. [Incremental Building](#incremental-building)
3. [Error Log Management](#error-log-management)
4. [Debugging Strategies](#debugging-strategies)
5. [Automated Testing](#automated-testing)
6. [CI/CD Integration](#cicd-integration)

## Build Configuration

### Debug vs. Release Builds

Create separate build configurations with different levels of verbosity:

```powershell
# Debug build with full verbosity
cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug -DREBELCAD_LOG_LEVEL=VERBOSE

# Release build with minimal logging
cmake -B build_release -DCMAKE_BUILD_TYPE=Release -DREBELCAD_LOG_LEVEL=ERROR
```

### Custom CMake Options

Add the following options to the main CMakeLists.txt:

```cmake
# Log level options
set(REBELCAD_LOG_LEVEL "INFO" CACHE STRING "Log level (ERROR, WARNING, INFO, DEBUG, VERBOSE)")
set_property(CACHE REBELCAD_LOG_LEVEL PROPERTY STRINGS ERROR WARNING INFO DEBUG VERBOSE)

# Component-specific build options
option(BUILD_SIMULATION "Build simulation module" ON)
option(BUILD_MODELING "Build modeling module" ON)
option(BUILD_UI "Build UI module" ON)
option(BUILD_TESTS "Build tests" ON)
```

## Incremental Building

### Component-Based Building

Create scripts to build specific components:

```powershell
# build_component.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$Component,
    
    [string]$BuildType = "Release",
    
    [string]$LogLevel = "INFO"
)

$buildDir = "build_$Component"

# Create build directory if it doesn't exist
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir
}

# Configure with component-specific options
$cmakeOptions = "-DCMAKE_BUILD_TYPE=$BuildType -DREBELCAD_LOG_LEVEL=$LogLevel"

# Disable all components by default
$cmakeOptions += " -DBUILD_SIMULATION=OFF -DBUILD_MODELING=OFF -DBUILD_UI=OFF"

# Enable the specified component
$cmakeOptions += " -DBUILD_$($Component.ToUpper())=ON"

# Configure
cmake -B $buildDir $cmakeOptions

# Build
cmake --build $buildDir --config $BuildType
```

### Targeted Rebuilds

Create a script to rebuild only modified files:

```powershell
# rebuild_modified.ps1
param (
    [string]$BuildDir = "build",
    [string]$Component = ""
)

# Get list of modified files
$modifiedFiles = git diff --name-only HEAD

# Filter for C++ source files
$modifiedCppFiles = $modifiedFiles | Where-Object { $_ -match "\.(cpp|h)$" }

if ($Component -ne "") {
    # Filter for files in the specified component
    $modifiedCppFiles = $modifiedCppFiles | Where-Object { $_ -match "/$Component/" }
}

# Build only the targets affected by modified files
foreach ($file in $modifiedCppFiles) {
    $target = (Get-Item $file).BaseName
    Write-Host "Rebuilding target for $file: $target"
    cmake --build $BuildDir --target $target
}
```

## Error Log Management

### Log Filtering and Redirection

Create a PowerShell script to filter and manage build logs:

```powershell
# filter_logs.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$BuildCommand,
    
    [string]$OutputFile = "build_log.txt",
    
    [string]$ErrorLevel = "ERROR",
    
    [switch]$ShowProgress = $false
)

# Run the build command and capture output
$buildProcess = Start-Process -FilePath "powershell.exe" -ArgumentList "-Command `"$BuildCommand`"" -NoNewWindow -PassThru -RedirectStandardOutput "temp_output.txt" -RedirectStandardError "temp_error.txt"

# Show progress if requested
if ($ShowProgress) {
    while (!$buildProcess.HasExited) {
        Write-Host "." -NoNewline
        Start-Sleep -Seconds 1
    }
    Write-Host ""
}
else {
    $buildProcess.WaitForExit()
}

# Filter logs based on error level
$errorPattern = switch ($ErrorLevel) {
    "ERROR" { "error|Error|ERROR" }
    "WARNING" { "error|Error|ERROR|warning|Warning|WARNING" }
    "INFO" { "error|Error|ERROR|warning|Warning|WARNING|info|Info|INFO" }
    default { "" }
}

if ($errorPattern -ne "") {
    Get-Content "temp_output.txt", "temp_error.txt" | Select-String -Pattern $errorPattern | Set-Content $OutputFile
}
else {
    Get-Content "temp_output.txt", "temp_error.txt" | Set-Content $OutputFile
}

# Clean up temporary files
Remove-Item "temp_output.txt", "temp_error.txt"

# Display summary
$errorCount = (Select-String -Path $OutputFile -Pattern "error|Error|ERROR").Count
$warningCount = (Select-String -Path $OutputFile -Pattern "warning|Warning|WARNING").Count

Write-Host "Build completed with $errorCount errors and $warningCount warnings."
Write-Host "Full log saved to $OutputFile"

# Return the most critical errors (limited to 10)
if ($errorCount -gt 0) {
    Write-Host "`nTop errors:"
    Select-String -Path $OutputFile -Pattern "error|Error|ERROR" | Select-Object -First 10 | ForEach-Object { Write-Host $_.Line }
}
```

### Chunked Log Analysis

Create a script to analyze large log files in manageable chunks:

```powershell
# analyze_log_chunks.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$LogFile,
    
    [int]$ChunkSize = 1000,
    
    [string]$OutputDir = "log_chunks"
)

# Create output directory
if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir
}

# Read the log file
$logContent = Get-Content $LogFile

# Calculate number of chunks
$totalLines = $logContent.Count
$numChunks = [Math]::Ceiling($totalLines / $ChunkSize)

Write-Host "Splitting $totalLines lines into $numChunks chunks of $ChunkSize lines each."

# Split into chunks
for ($i = 0; $i -lt $numChunks; $i++) {
    $startLine = $i * $ChunkSize
    $endLine = [Math]::Min(($i + 1) * $ChunkSize - 1, $totalLines - 1)
    $chunkContent = $logContent[$startLine..$endLine]
    
    $outputFile = Join-Path $OutputDir "chunk_$($i + 1)_of_$numChunks.txt"
    $chunkContent | Set-Content $outputFile
    
    # Count errors and warnings in this chunk
    $errorCount = ($chunkContent | Select-String -Pattern "error|Error|ERROR").Count
    $warningCount = ($chunkContent | Select-String -Pattern "warning|Warning|WARNING").Count
    
    Write-Host "Chunk $($i + 1): Lines $startLine-$endLine, $errorCount errors, $warningCount warnings"
}

# Create a summary file
$summaryFile = Join-Path $OutputDir "summary.txt"
"Log Analysis Summary" | Set-Content $summaryFile
"Total lines: $totalLines" | Add-Content $summaryFile
"Total chunks: $numChunks" | Add-Content $summaryFile
"Chunk size: $ChunkSize" | Add-Content $summaryFile
"" | Add-Content $summaryFile

# Add error summary for each chunk
for ($i = 0; $i -lt $numChunks; $i++) {
    $chunkFile = Join-Path $OutputDir "chunk_$($i + 1)_of_$numChunks.txt"
    $chunkContent = Get-Content $chunkFile
    
    $errorCount = ($chunkContent | Select-String -Pattern "error|Error|ERROR").Count
    $warningCount = ($chunkContent | Select-String -Pattern "warning|Warning|WARNING").Count
    
    "Chunk $($i + 1): $errorCount errors, $warningCount warnings" | Add-Content $summaryFile
    
    if ($errorCount -gt 0) {
        "  Errors:" | Add-Content $summaryFile
        $chunkContent | Select-String -Pattern "error|Error|ERROR" | ForEach-Object {
            "    - $($_.Line)" | Add-Content $summaryFile
        }
    }
}

Write-Host "Analysis complete. Summary saved to $summaryFile"
```

## Debugging Strategies

### Component Isolation

Create a script to isolate and test specific components:

```powershell
# test_component.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$Component,
    
    [string]$TestFilter = "*",
    
    [switch]$Verbose = $false
)

# Determine the test executable
$testExecutable = switch ($Component.ToLower()) {
    "simulation" { "simulation_tests" }
    "core" { "core_tests" }
    "modeling" { "modeling_tests" }
    "ui" { "ui_tests" }
    default { throw "Unknown component: $Component" }
}

# Build the component and its tests
Write-Host "Building $Component component and tests..."
./build_component.ps1 -Component $Component -BuildType Debug

# Run the tests with the specified filter
$testCommand = "cd build_$Component/bin && ./$testExecutable"
if ($TestFilter -ne "*") {
    $testCommand += " --gtest_filter=$TestFilter"
}
if ($Verbose) {
    $testCommand += " --gtest_output=xml:test_results.xml"
}

Write-Host "Running tests for $Component component..."
Invoke-Expression $testCommand

# Parse test results if verbose
if ($Verbose -and (Test-Path "build_$Component/bin/test_results.xml")) {
    Write-Host "Parsing test results..."
    [xml]$testResults = Get-Content "build_$Component/bin/test_results.xml"
    
    $totalTests = $testResults.testsuites.tests
    $failures = $testResults.testsuites.failures
    $errors = $testResults.testsuites.errors
    
    Write-Host "Test Summary:"
    Write-Host "  Total tests: $totalTests"
    Write-Host "  Passed: $($totalTests - $failures - $errors)"
    Write-Host "  Failed: $failures"
    Write-Host "  Errors: $errors"
    
    if ($failures -gt 0 -or $errors -gt 0) {
        Write-Host "`nFailed Tests:"
        $testResults.testsuites.testsuite.testcase | Where-Object { $_.failure -ne $null } | ForEach-Object {
            Write-Host "  - $($_.name): $($_.failure.message)"
        }
    }
}
```

### Incremental Debugging

Create a script for incremental debugging with checkpoints:

```powershell
# debug_incremental.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$Component,
    
    [string]$StartTest = "",
    
    [string]$EndTest = "",
    
    [switch]$SaveState = $false
)

# Get all tests for the component
$testExecutable = "build_$Component/bin/$($Component)_tests"
$allTests = Invoke-Expression "$testExecutable --gtest_list_tests"

# Filter tests if start and end points are specified
$runTests = @()
$capturing = $StartTest -eq ""
foreach ($line in $allTests) {
    if ($line -match "^([^.]+)\.") {
        $testSuite = $matches[1]
    }
    elseif ($line -match "^\s+(\S+)") {
        $testName = $matches[1]
        $fullTestName = "$testSuite.$testName"
        
        if (!$capturing -and $fullTestName -eq $StartTest) {
            $capturing = $true
        }
        
        if ($capturing) {
            $runTests += $fullTestName
        }
        
        if ($capturing -and $fullTestName -eq $EndTest) {
            $capturing = $false
        }
    }
}

# Run each test individually and save state if requested
foreach ($test in $runTests) {
    Write-Host "Running test: $test"
    $testResult = Invoke-Expression "$testExecutable --gtest_filter=$test"
    
    if ($SaveState) {
        $stateDir = "debug_states/$Component/$test"
        if (!(Test-Path $stateDir)) {
            New-Item -ItemType Directory -Path $stateDir -Force
        }
        
        # Save test output
        $testResult | Set-Content "$stateDir/output.txt"
        
        # Save relevant source files
        $sourceFiles = Get-ChildItem -Path "src/$Component" -Filter "*.cpp" -Recurse
        foreach ($file in $sourceFiles) {
            $destPath = Join-Path $stateDir $file.FullName.Substring((Get-Location).Path.Length + 1)
            $destDir = Split-Path $destPath -Parent
            if (!(Test-Path $destDir)) {
                New-Item -ItemType Directory -Path $destDir -Force
            }
            Copy-Item $file.FullName -Destination $destPath
        }
        
        Write-Host "Saved state to $stateDir"
    }
    
    # Check if test passed
    if ($testResult -match "PASSED") {
        Write-Host "Test passed: $test" -ForegroundColor Green
    }
    else {
        Write-Host "Test failed: $test" -ForegroundColor Red
        Write-Host $testResult
        
        # Ask if we should continue
        $continue = Read-Host "Continue with next test? (y/n)"
        if ($continue -ne "y") {
            break
        }
    }
}
```

## Automated Testing

### Test Categorization

Modify the test CMakeLists.txt to categorize tests:

```cmake
# Add test categories
set(TEST_CATEGORIES
    SMALL
    MEDIUM
    LARGE
)

# Define test category for each test
set_property(TEST SimulationTests PROPERTY LABELS "MEDIUM;SIMULATION")
```

### Selective Test Running

Create a script to run tests by category:

```powershell
# run_tests_by_category.ps1
param (
    [Parameter(Mandatory=$true)]
    [string]$Category,
    
    [switch]$Verbose = $false
)

# Build all tests
cmake --build build --target all

# Run tests by category
$ctest_args = "-L $Category"
if ($Verbose) {
    $ctest_args += " -V"
}

cd build
ctest $ctest_args
```

## CI/CD Integration

### GitHub Actions Workflow

Create a GitHub Actions workflow for incremental building and testing:

```yaml
# .github/workflows/incremental-build.yml
name: Incremental Build and Test

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  determine-changes:
    runs-on: ubuntu-latest
    outputs:
      components: ${{ steps.filter.outputs.components }}
    steps:
      - uses: actions/checkout@v2
      - id: filter
        run: |
          CHANGED_FILES=$(git diff --name-only ${{ github.event.before }} ${{ github.sha }})
          COMPONENTS=""
          
          if echo "$CHANGED_FILES" | grep -q "src/core/"; then
            COMPONENTS="$COMPONENTS core"
          fi
          if echo "$CHANGED_FILES" | grep -q "src/simulation/"; then
            COMPONENTS="$COMPONENTS simulation"
          fi
          if echo "$CHANGED_FILES" | grep -q "src/modeling/"; then
            COMPONENTS="$COMPONENTS modeling"
          fi
          if echo "$CHANGED_FILES" | grep -q "src/ui/"; then
            COMPONENTS="$COMPONENTS ui"
          fi
          
          # If no specific component changed, build all
          if [ -z "$COMPONENTS" ]; then
            COMPONENTS="core simulation modeling ui"
          fi
          
          echo "::set-output name=components::$COMPONENTS"
  
  build-and-test:
    needs: determine-changes
    runs-on: windows-latest
    strategy:
      matrix:
        component: ${{ fromJson(needs.determine-changes.outputs.components) }}
    
    steps:
      - uses: actions/checkout@v2
      
      - name: Configure CMake
        run: |
          mkdir build_${{ matrix.component }}
          cd build_${{ matrix.component }}
          cmake .. -DBUILD_${{ matrix.component | upcase }}=ON
      
      - name: Build
        run: |
          cd build_${{ matrix.component }}
          cmake --build . --config Release
      
      - name: Test
        run: |
          cd build_${{ matrix.component }}
          ctest -C Release -L ${{ matrix.component | upcase }}
      
      - name: Upload Build Logs
        if: always()
        uses: actions/upload-artifact@v2
        with:
          name: build-logs-${{ matrix.component }}
          path: build_${{ matrix.component }}/CMakeFiles/CMakeOutput.log
```

### Log Rotation and Archiving

Create a script for log rotation and archiving:

```powershell
# rotate_logs.ps1
param (
    [int]$KeepDays = 7,
    [string]$LogDir = "logs"
)

# Create log directory if it doesn't exist
if (!(Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir
}

# Get current date
$currentDate = Get-Date -Format "yyyy-MM-dd"

# Create a new log directory for today
$todayLogDir = Join-Path $LogDir $currentDate
if (!(Test-Path $todayLogDir)) {
    New-Item -ItemType Directory -Path $todayLogDir
}

# Move all log files to today's directory
Get-ChildItem -Path "." -Filter "*.log" | ForEach-Object {
    Move-Item $_.FullName -Destination (Join-Path $todayLogDir $_.Name)
}

# Remove old log directories
$cutoffDate = (Get-Date).AddDays(-$KeepDays)
Get-ChildItem -Path $LogDir -Directory | ForEach-Object {
    $dirDate = [DateTime]::ParseExact($_.Name, "yyyy-MM-dd", $null)
    if ($dirDate -lt $cutoffDate) {
        Remove-Item $_.FullName -Recurse -Force
        Write-Host "Removed old log directory: $($_.Name)"
    }
}
```

## Implementation Plan

1. Add the custom CMake options to the main CMakeLists.txt
2. Create the build and debug scripts in a new `scripts` directory
3. Update the CI/CD configuration
4. Document the new build and debug process
5. Train the team on the new tools and processes
