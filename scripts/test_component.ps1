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
    "sketching" { "sketching_tests" }
    "assembly" { "assembly_tests" }
    "constraints" { "constraints_tests" }
    "graphics" { "graphics_tests" }
    default { 
        Write-Host "Unknown component: $Component" -ForegroundColor Red
        Write-Host "Available components: simulation, core, modeling, ui, sketching, assembly, constraints, graphics" -ForegroundColor Yellow
        exit 1
    }
}

# Check if build_component.ps1 exists
$buildComponentScript = Join-Path (Get-Location) "scripts\build_component.ps1"
if (!(Test-Path $buildComponentScript)) {
    Write-Host "Error: build_component.ps1 script not found at $buildComponentScript" -ForegroundColor Red
    exit 1
}

# Build the component and its tests
Write-Host "Building $Component component and tests..." -ForegroundColor Cyan
& $buildComponentScript -Component $Component -BuildType Debug

# Check if build was successful
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed with exit code $LASTEXITCODE. Cannot run tests." -ForegroundColor Red
    exit $LASTEXITCODE
}

# Create a timestamp for the test results
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$testResultsDir = "test_results"
if (!(Test-Path $testResultsDir)) {
    New-Item -ItemType Directory -Path $testResultsDir | Out-Null
}

$testResultsFile = Join-Path $testResultsDir "${Component}_tests_${timestamp}.xml"
$testLogFile = Join-Path $testResultsDir "${Component}_tests_${timestamp}.log"

# Run the tests with the specified filter
$testBinary = "build_$Component\bin\$testExecutable"
if (!(Test-Path $testBinary)) {
    $testBinary = "build_$Component\bin\Debug\$testExecutable.exe"
    if (!(Test-Path $testBinary)) {
        Write-Host "Error: Test executable not found at $testBinary" -ForegroundColor Red
        Write-Host "Please check the build output and ensure tests were built correctly." -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "Running tests for $Component component..." -ForegroundColor Cyan
Write-Host "Test executable: $testBinary" -ForegroundColor Gray
Write-Host "Test filter: $TestFilter" -ForegroundColor Gray

$testCommand = "$testBinary"
if ($TestFilter -ne "*") {
    $testCommand += " --gtest_filter=$TestFilter"
}

$testCommand += " --gtest_output=xml:$testResultsFile"

# Run the tests and capture output
$testOutput = & $testBinary --gtest_filter=$TestFilter --gtest_output=xml:$testResultsFile 2>&1
$testOutput | Out-File -FilePath $testLogFile

# Display test output
$testOutput | ForEach-Object {
    if ($_ -match "FAILED") {
        Write-Host $_ -ForegroundColor Red
    } elseif ($_ -match "PASSED") {
        Write-Host $_ -ForegroundColor Green
    } else {
        Write-Host $_
    }
}

# Parse test results if available
if (Test-Path $testResultsFile) {
    Write-Host "`nTest Results Summary:" -ForegroundColor Cyan
    
    try {
        [xml]$testResults = Get-Content $testResultsFile
        
        $totalTests = $testResults.testsuites.tests
        $failures = $testResults.testsuites.failures
        $errors = $testResults.testsuites.errors
        $time = $testResults.testsuites.time
        
        Write-Host "  Total tests: $totalTests"
        Write-Host "  Passed: $($totalTests - $failures - $errors)" -ForegroundColor Green
        
        if ($failures -gt 0) {
            Write-Host "  Failed: $failures" -ForegroundColor Red
        } else {
            Write-Host "  Failed: $failures" -ForegroundColor Green
        }
        
        if ($errors -gt 0) {
            Write-Host "  Errors: $errors" -ForegroundColor Red
        } else {
            Write-Host "  Errors: $errors" -ForegroundColor Green
        }
        
        Write-Host "  Time: $time seconds"
        
        if ($failures -gt 0 -or $errors -gt 0) {
            Write-Host "`nFailed Tests:" -ForegroundColor Red
            $testResults.testsuites.testsuite.testcase | Where-Object { $_.failure -ne $null } | ForEach-Object {
                Write-Host "  - $($_.classname).$($_.name): $($_.failure.message)" -ForegroundColor Red
                if ($Verbose) {
                    Write-Host "    $($_.failure.InnerText)" -ForegroundColor Gray
                }
            }
        }
    } catch {
        Write-Host "Error parsing test results XML: $_" -ForegroundColor Red
    }
    
    Write-Host "`nTest results saved to: $testResultsFile" -ForegroundColor Gray
    Write-Host "Test log saved to: $testLogFile" -ForegroundColor Gray
} else {
    Write-Host "`nWarning: No test results XML file was generated." -ForegroundColor Yellow
}

# Return success if no failures
if ($failures -gt 0 -or $errors -gt 0) {
    exit 1
} else {
    exit 0
}
