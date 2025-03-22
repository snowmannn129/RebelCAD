#!/usr/bin/env pwsh
# AI-Assisted Development Workflow Script for RebelCAD
# This script demonstrates how to use the modular CMake structure for AI-assisted development

param (
    [Parameter(Mandatory=$false)]
    [string]$Module = "simulation",
    
    [Parameter(Mandatory=$false)]
    [string]$Submodule = "fea",
    
    [Parameter(Mandatory=$false)]
    [string]$TestType = "unit",
    
    [Parameter(Mandatory=$false)]
    [string]$BuildDir = "build_ai_debug",
    
    [Parameter(Mandatory=$false)]
    [switch]$Clean = $false,
    
    [Parameter(Mandatory=$false)]
    [switch]$RunTests = $true,
    
    [Parameter(Mandatory=$false)]
    [switch]$RunIntegrationTests = $false
)

# Convert module and submodule to uppercase for CMake options
$ModuleUpper = $Module.ToUpper()
$SubmoduleUpper = $Submodule.ToUpper()

# Create build directory if it doesn't exist
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Write-Host "Created build directory: $BuildDir"
}

# Clean build directory if requested
if ($Clean) {
    Write-Host "Cleaning build directory: $BuildDir"
    Remove-Item -Path "$BuildDir\*" -Recurse -Force
}

# Construct CMake configuration command
$cmakeConfigCmd = "cmake -B $BuildDir"

# Add module options
$modules = @("SIMULATION", "MODELING", "UI", "SKETCHING", "ASSEMBLY", "CONSTRAINTS", "GRAPHICS")
foreach ($mod in $modules) {
    if ($mod -eq $ModuleUpper) {
        $cmakeConfigCmd += " -DBUILD_$mod=ON"
    } else {
        $cmakeConfigCmd += " -DBUILD_$mod=OFF"
    }
}

# Add submodule options based on the selected module
if ($ModuleUpper -eq "SIMULATION") {
    $submodules = @("FEA", "THERMAL", "DYNAMIC", "NONLINEAR")
    foreach ($submod in $submodules) {
        if ($submod -eq $SubmoduleUpper) {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=ON"
        } else {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=OFF"
        }
    }
} elseif ($ModuleUpper -eq "MODELING") {
    $submodules = @("GEOMETRY", "TOPOLOGY", "OPERATIONS")
    foreach ($submod in $submodules) {
        if ($submod -eq $SubmoduleUpper) {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=ON"
        } else {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=OFF"
        }
    }
} elseif ($ModuleUpper -eq "UI") {
    $submodules = @("WIDGETS", "DIALOGS", "VIEWPORT")
    foreach ($submod in $submodules) {
        if ($submod -eq $SubmoduleUpper) {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=ON"
        } else {
            $cmakeConfigCmd += " -DBUILD_${ModuleUpper}_$submod=OFF"
        }
    }
}

# Add test options
if ($TestType -eq "unit") {
    $cmakeConfigCmd += " -DBUILD_UNIT_TESTS=ON -DBUILD_INTEGRATION_TESTS=OFF -DBUILD_BENCHMARK_TESTS=OFF"
} elseif ($TestType -eq "integration") {
    $cmakeConfigCmd += " -DBUILD_UNIT_TESTS=OFF -DBUILD_INTEGRATION_TESTS=ON -DBUILD_BENCHMARK_TESTS=OFF"
} elseif ($TestType -eq "benchmark") {
    $cmakeConfigCmd += " -DBUILD_UNIT_TESTS=OFF -DBUILD_INTEGRATION_TESTS=OFF -DBUILD_BENCHMARK_TESTS=ON"
} elseif ($TestType -eq "all") {
    $cmakeConfigCmd += " -DBUILD_UNIT_TESTS=ON -DBUILD_INTEGRATION_TESTS=ON -DBUILD_BENCHMARK_TESTS=ON"
} else {
    $cmakeConfigCmd += " -DBUILD_UNIT_TESTS=OFF -DBUILD_INTEGRATION_TESTS=OFF -DBUILD_BENCHMARK_TESTS=OFF"
}

# Configure the build
Write-Host "Configuring build with command: $cmakeConfigCmd"
Invoke-Expression $cmakeConfigCmd

# Determine the target to build
$targetName = ""
if ($ModuleUpper -eq "SIMULATION") {
    if ($SubmoduleUpper -eq "FEA") {
        $targetName = "SimulationFEA"
    } elseif ($SubmoduleUpper -eq "THERMAL") {
        $targetName = "SimulationThermal"
    } elseif ($SubmoduleUpper -eq "DYNAMIC") {
        $targetName = "SimulationDynamic"
    } elseif ($SubmoduleUpper -eq "NONLINEAR") {
        $targetName = "SimulationNonLinear"
    } else {
        $targetName = "simulation"
    }
} elseif ($ModuleUpper -eq "MODELING") {
    if ($SubmoduleUpper -eq "GEOMETRY") {
        $targetName = "ModelingGeometry"
    } elseif ($SubmoduleUpper -eq "TOPOLOGY") {
        $targetName = "ModelingTopology"
    } elseif ($SubmoduleUpper -eq "OPERATIONS") {
        $targetName = "ModelingOperations"
    } else {
        $targetName = "modeling"
    }
} elseif ($ModuleUpper -eq "UI") {
    if ($SubmoduleUpper -eq "WIDGETS") {
        $targetName = "UIWidgets"
    } elseif ($SubmoduleUpper -eq "DIALOGS") {
        $targetName = "UIDialogs"
    } elseif ($SubmoduleUpper -eq "VIEWPORT") {
        $targetName = "UIViewport"
    } else {
        $targetName = "ui"
    }
} else {
    $targetName = $Module.ToLower()
}

# Build the target
Write-Host "Building target: $targetName"
Invoke-Expression "cmake --build $BuildDir --target $targetName"

# Run tests if requested
if ($RunTests) {
    $testName = "${ModuleUpper}_${SubmoduleUpper}"
    Write-Host "Running tests for: $testName"
    Invoke-Expression "ctest --test-dir $BuildDir -R $testName"
}

# Run integration tests if requested
if ($RunIntegrationTests) {
    Write-Host "Running integration tests"
    Invoke-Expression "ctest --test-dir $BuildDir -L INTEGRATION"
}

Write-Host "AI-Assisted Development Workflow Complete"
Write-Host "----------------------------------------"
Write-Host "Module:    $Module"
Write-Host "Submodule: $Submodule"
Write-Host "Target:    $targetName"
Write-Host "Test Type: $TestType"
Write-Host "Build Dir: $BuildDir"
Write-Host ""
Write-Host "To modify and test this component again, run:"
Write-Host "  cmake --build $BuildDir --target $targetName"
if ($RunTests) {
    Write-Host "  ctest --test-dir $BuildDir -R ${ModuleUpper}_${SubmoduleUpper}"
}
if ($RunIntegrationTests) {
    Write-Host "  ctest --test-dir $BuildDir -L INTEGRATION"
}
