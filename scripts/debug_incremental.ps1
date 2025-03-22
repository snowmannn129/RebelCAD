param (
    [Parameter(Mandatory=$true)]
    [string]$Component,
    
    [string]$StartTest = "",
    
    [string]$EndTest = "",
    
    [switch]$SaveState = $false
)

# Verify the component is valid
$validComponents = @("simulation", "core", "modeling", "ui", "sketching", "assembly", "constraints", "graphics")
if ($validComponents -notcontains $Component.ToLower()) {
    Write-Host "Error: Invalid component '$Component'" -ForegroundColor Red
    Write-Host "Valid components: $($validComponents -join ", ")" -ForegroundColor Yellow
    exit 1
}

# Determine the test executable
$testExecutable = "build_$Component\bin\$($Component)_tests"
if (!(Test-Path $testExecutable)) {
    $testExecutable = "build_$Component\bin\Debug\$($Component)_tests.exe"
    if (!(Test-Path $testExecutable)) {
        Write-Host "Error: Test executable not found at $testExecutable" -ForegroundColor Red
        Write-Host "Please build the component first using build_component.ps1" -ForegroundColor Yellow
        exit 1
    }
}

# Create debug states directory if saving state
if ($SaveState) {
    $debugStatesDir = "debug_states"
    if (!(Test-Path $debugStatesDir)) {
        New-Item -ItemType Directory -Path $debugStatesDir | Out-Null
    }
    
    $componentDebugDir = Join-Path $debugStatesDir $Component
    if (!(Test-Path $componentDebugDir)) {
        New-Item -ItemType Directory -Path $componentDebugDir | Out-Null
    }
    
    # Create a timestamp subdirectory
    $timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
    $sessionDir = Join-Path $componentDebugDir $timestamp
    New-Item -ItemType Directory -Path $sessionDir | Out-Null
    
    Write-Host "Debug session directory: $sessionDir" -ForegroundColor Cyan
}

# Get all tests for the component
Write-Host "Getting list of tests for $Component..." -ForegroundColor Cyan
$allTestsOutput = & $testExecutable --gtest_list_tests
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Failed to list tests for $Component" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Parse the test list output
$runTests = @()
$currentTestSuite = ""

foreach ($line in $allTestsOutput) {
    if ($line -match "^([^.]+)\.$") {
        $currentTestSuite = $matches[1]
    }
    elseif ($line -match "^\s+(\S+)") {
        $testName = $matches[1]
        $fullTestName = "$currentTestSuite.$testName"
        
        # Filter tests based on start and end points if specified
        $shouldInclude = $true
        
        if ($StartTest -ne "" -and $EndTest -ne "") {
            # Both start and end specified - include tests between them
            if ($fullTestName -eq $StartTest) {
                $capturing = $true
            }
            
            $shouldInclude = $capturing
            
            if ($fullTestName -eq $EndTest) {
                $capturing = $false
            }
        }
        elseif ($StartTest -ne "") {
            # Only start specified - include tests from start onwards
            $shouldInclude = $capturing -or ($fullTestName -eq $StartTest)
            if ($fullTestName -eq $StartTest) {
                $capturing = $true
            }
        }
        elseif ($EndTest -ne "") {
            # Only end specified - include tests up to end
            $shouldInclude = $capturing
            if ($fullTestName -eq $EndTest) {
                $capturing = $false
            }
        }
        
        if ($shouldInclude) {
            $runTests += $fullTestName
        }
    }
}

# If no tests were found or filtered, exit
if ($runTests.Count -eq 0) {
    Write-Host "No tests found to run." -ForegroundColor Yellow
    if ($StartTest -ne "" -or $EndTest -ne "") {
        Write-Host "Check your StartTest and EndTest parameters." -ForegroundColor Yellow
        Write-Host "Available tests:" -ForegroundColor Cyan
        $allTestsOutput | ForEach-Object {
            if ($_ -match "^([^.]+)\.$") {
                Write-Host ""
                Write-Host $_ -ForegroundColor Gray
            } else {
                Write-Host "  $($_.Trim())" -ForegroundColor Gray
            }
        }
    }
    exit 1
}

# Create a session log file if saving state
$sessionLogFile = $null
if ($SaveState) {
    $sessionLogFile = Join-Path $sessionDir "session_log.txt"
    "Debug Session for $Component - $(Get-Date)" | Set-Content $sessionLogFile
    "Tests to run: $($runTests.Count)" | Add-Content $sessionLogFile
    $runTests | ForEach-Object { "  - $_" | Add-Content $sessionLogFile }
    "" | Add-Content $sessionLogFile
}

# Run each test individually
$passedTests = 0
$failedTests = 0

Write-Host "`nRunning $($runTests.Count) tests for $Component..." -ForegroundColor Cyan

foreach ($test in $runTests) {
    Write-Host "`nRunning test: $test" -ForegroundColor Cyan
    
    # Log to session log if saving state
    if ($SaveState) {
        "=== Test: $test ($(Get-Date)) ===" | Add-Content $sessionLogFile
    }
    
    # Run the test
    $testOutput = & $testExecutable --gtest_filter=$test
    $testPassed = $LASTEXITCODE -eq 0
    
    # Display test output with color coding
    $testOutput | ForEach-Object {
        if ($_ -match "FAILED") {
            Write-Host $_ -ForegroundColor Red
        } elseif ($_ -match "PASSED") {
            Write-Host $_ -ForegroundColor Green
        } else {
            Write-Host $_
        }
    }
    
    # Log test output if saving state
    if ($SaveState) {
        $testOutput | Add-Content $sessionLogFile
        if ($testPassed) {
            "RESULT: PASSED" | Add-Content $sessionLogFile
        } else {
            "RESULT: FAILED" | Add-Content $sessionLogFile
        }
        "" | Add-Content $sessionLogFile
    }
    
    # Save state if requested
    if ($SaveState) {
        $testStateDir = Join-Path $sessionDir $test.Replace(".", "_")
        New-Item -ItemType Directory -Path $testStateDir | Out-Null
        
        # Save test output
        $testOutput | Set-Content "$testStateDir\output.txt"
        
        # Save relevant source files
        $sourceFiles = Get-ChildItem -Path "src\$Component" -Filter "*.cpp" -Recurse
        if ($sourceFiles) {
            $sourceDir = Join-Path $testStateDir "src"
            New-Item -ItemType Directory -Path $sourceDir | Out-Null
            
            foreach ($file in $sourceFiles) {
                $relativePath = $file.FullName.Substring((Get-Location).Path.Length + 1)
                $destPath = Join-Path $testStateDir $relativePath
                $destDir = Split-Path $destPath -Parent
                
                if (!(Test-Path $destDir)) {
                    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
                }
                
                Copy-Item $file.FullName -Destination $destPath
            }
        }
        
        # Save header files
        $headerFiles = Get-ChildItem -Path "include\$Component" -Filter "*.h" -Recurse
        if ($headerFiles) {
            $includeDir = Join-Path $testStateDir "include"
            New-Item -ItemType Directory -Path $includeDir | Out-Null
            
            foreach ($file in $headerFiles) {
                $relativePath = $file.FullName.Substring((Get-Location).Path.Length + 1)
                $destPath = Join-Path $testStateDir $relativePath
                $destDir = Split-Path $destPath -Parent
                
                if (!(Test-Path $destDir)) {
                    New-Item -ItemType Directory -Path $destDir -Force | Out-Null
                }
                
                Copy-Item $file.FullName -Destination $destPath
            }
        }
        
        Write-Host "Saved state to $testStateDir" -ForegroundColor Gray
    }
    
    # Update counters
    if ($testPassed) {
        $passedTests++
        Write-Host "Test passed: $test" -ForegroundColor Green
    } else {
        $failedTests++
        Write-Host "Test failed: $test" -ForegroundColor Red
        
        # Ask if we should continue
        $continue = Read-Host "Continue with next test? (y/n)"
        if ($continue -ne "y") {
            Write-Host "Stopping debug session." -ForegroundColor Yellow
            break
        }
    }
}

# Print summary
Write-Host "`nDebug Session Summary:" -ForegroundColor Cyan
Write-Host "  Component: $Component"
Write-Host "  Total tests run: $($passedTests + $failedTests)"
Write-Host "  Passed: $passedTests" -ForegroundColor Green
Write-Host "  Failed: $failedTests" -ForegroundColor $(if ($failedTests -gt 0) { "Red" } else { "Green" })

if ($SaveState) {
    Write-Host "`nDebug session saved to: $sessionDir" -ForegroundColor Cyan
    
    # Add summary to session log
    "`nSUMMARY:" | Add-Content $sessionLogFile
    "Total tests run: $($passedTests + $failedTests)" | Add-Content $sessionLogFile
    "Passed: $passedTests" | Add-Content $sessionLogFile
    "Failed: $failedTests" | Add-Content $sessionLogFile
    "Session completed: $(Get-Date)" | Add-Content $sessionLogFile
}

# Return success if all tests passed
if ($failedTests -gt 0) {
    exit 1
} else {
    exit 0
}
