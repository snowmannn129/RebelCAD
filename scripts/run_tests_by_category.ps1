param (
    [Parameter(Mandatory=$true)]
    [string]$Category,
    
    [switch]$Verbose = $false,
    
    [string]$BuildType = "Release"
)

# Validate category
$validCategories = @("SMALL", "MEDIUM", "LARGE", "SIMULATION", "CORE", "MODELING", "UI", "SKETCHING", "ASSEMBLY", "CONSTRAINTS", "GRAPHICS")
if ($validCategories -notcontains $Category.ToUpper()) {
    Write-Host "Warning: Unrecognized category '$Category'. This may still work if it matches a test label." -ForegroundColor Yellow
    Write-Host "Common categories: $($validCategories -join ", ")" -ForegroundColor Gray
}

# Check if we have a build directory
$buildDir = "build"
if (!(Test-Path $buildDir)) {
    Write-Host "Error: Build directory not found. Please run a build first." -ForegroundColor Red
    exit 1
}

# Create a timestamp for the test results
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$testResultsDir = "test_results"
if (!(Test-Path $testResultsDir)) {
    New-Item -ItemType Directory -Path $testResultsDir | Out-Null
}

$testResultsFile = Join-Path $testResultsDir "category_${Category}_${timestamp}.xml"
$testLogFile = Join-Path $testResultsDir "category_${Category}_${timestamp}.log"

# Build all tests if needed
Write-Host "Building tests in $BuildType configuration..." -ForegroundColor Cyan
$buildOutput = cmake --build $buildDir --config $BuildType --target all
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    $buildOutput | ForEach-Object { Write-Host $_ }
    exit $LASTEXITCODE
}

# Run tests by category
Write-Host "Running tests with category label: $Category" -ForegroundColor Cyan

# Prepare CTest arguments
$ctestArgs = "-C $BuildType -L $Category"
if ($Verbose) {
    $ctestArgs += " -V"
}

# Add output to XML
$ctestArgs += " --output-junit $testResultsFile"

# Run CTest
Write-Host "cd $buildDir && ctest $ctestArgs" -ForegroundColor Gray
$testOutput = & cmd /c "cd $buildDir && ctest $ctestArgs 2>&1"
$testExitCode = $LASTEXITCODE

# Save test output to log file
$testOutput | Out-File -FilePath $testLogFile

# Display test output with color coding
$testOutput | ForEach-Object {
    if ($_ -match "Failed") {
        Write-Host $_ -ForegroundColor Red
    } elseif ($_ -match "Passed") {
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
        
        $totalTests = 0
        $totalFailures = 0
        $totalErrors = 0
        $totalTime = 0
        
        # JUnit XML format has multiple testsuite elements
        foreach ($testsuite in $testResults.testsuites.testsuite) {
            $totalTests += [int]$testsuite.tests
            $totalFailures += [int]$testsuite.failures
            $totalErrors += [int]$testsuite.errors
            $totalTime += [double]$testsuite.time
        }
        
        Write-Host "  Total tests: $totalTests"
        Write-Host "  Passed: $($totalTests - $totalFailures - $totalErrors)" -ForegroundColor Green
        
        if ($totalFailures -gt 0) {
            Write-Host "  Failed: $totalFailures" -ForegroundColor Red
        } else {
            Write-Host "  Failed: $totalFailures" -ForegroundColor Green
        }
        
        if ($totalErrors -gt 0) {
            Write-Host "  Errors: $totalErrors" -ForegroundColor Red
        } else {
            Write-Host "  Errors: $totalErrors" -ForegroundColor Green
        }
        
        Write-Host "  Time: $totalTime seconds"
        
        if ($totalFailures -gt 0 -or $totalErrors -gt 0) {
            Write-Host "`nFailed Tests:" -ForegroundColor Red
            foreach ($testsuite in $testResults.testsuites.testsuite) {
                foreach ($testcase in $testsuite.testcase) {
                    if ($testcase.failure -ne $null) {
                        Write-Host "  - $($testcase.classname).$($testcase.name): $($testcase.failure.message)" -ForegroundColor Red
                        if ($Verbose) {
                            Write-Host "    $($testcase.failure.InnerText)" -ForegroundColor Gray
                        }
                    }
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

# Generate an HTML report for better visualization
$htmlReportFile = Join-Path $testResultsDir "category_${Category}_${timestamp}.html"

$htmlContent = @"
<!DOCTYPE html>
<html>
<head>
    <title>Test Results - Category: $Category</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1, h2 { color: #333; }
        .summary { background-color: #f0f0f0; padding: 10px; border-radius: 5px; margin-bottom: 20px; }
        .test-suite { margin-bottom: 20px; border: 1px solid #ddd; padding: 10px; border-radius: 5px; }
        .test-suite-header { font-weight: bold; margin-bottom: 10px; }
        .test-case { margin: 5px 0; padding: 5px; border-radius: 3px; }
        .passed { background-color: #dff0d8; color: #3c763d; }
        .failed { background-color: #f2dede; color: #a94442; }
        .error-message { margin-top: 5px; font-family: monospace; white-space: pre-wrap; }
    </style>
</head>
<body>
    <h1>Test Results - Category: $Category</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p>Generated: $(Get-Date)</p>
        <p>Build Type: $BuildType</p>
        <p>Total Tests: $totalTests</p>
        <p>Passed: $($totalTests - $totalFailures - $totalErrors)</p>
        <p>Failed: $totalFailures</p>
        <p>Errors: $totalErrors</p>
        <p>Total Time: $totalTime seconds</p>
    </div>
"@

if (Test-Path $testResultsFile) {
    try {
        [xml]$testResults = Get-Content $testResultsFile
        
        foreach ($testsuite in $testResults.testsuites.testsuite) {
            $htmlContent += @"
    <div class="test-suite">
        <div class="test-suite-header">Test Suite: $($testsuite.name)</div>
"@
            
            foreach ($testcase in $testsuite.testcase) {
                $testStatus = "passed"
                $errorMessage = ""
                
                if ($testcase.failure -ne $null) {
                    $testStatus = "failed"
                    $errorMessage = $testcase.failure.message
                    if ($testcase.failure.InnerText) {
                        $errorMessage += "`n" + $testcase.failure.InnerText
                    }
                }
                
                $htmlContent += @"
        <div class="test-case $testStatus">
            <div>$($testcase.name) ($($testcase.time)s)</div>
"@
                
                if ($testStatus -eq "failed") {
                    $htmlContent += @"
            <div class="error-message">$errorMessage</div>
"@
                }
                
                $htmlContent += @"
        </div>
"@
            }
            
            $htmlContent += @"
    </div>
"@
        }
    } catch {
        $htmlContent += @"
    <div>Error parsing test results XML: $_</div>
"@
    }
}

$htmlContent += @"
</body>
</html>
"@

$htmlContent | Set-Content $htmlReportFile
Write-Host "HTML report saved to: $htmlReportFile" -ForegroundColor Gray

# Return the exit code from CTest
exit $testExitCode
