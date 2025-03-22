param (
    [string]$BuildDir = "build",
    [string]$Component = "",
    [switch]$Verbose = $false,
    [string]$BuildType = "Release"
)

# Check if git is available
$gitAvailable = $null -ne (Get-Command git -ErrorAction SilentlyContinue)
if (!$gitAvailable) {
    Write-Host "Error: Git is not available. This script requires git to identify modified files." -ForegroundColor Red
    exit 1
}

# Check if the build directory exists
if (!(Test-Path $BuildDir)) {
    Write-Host "Error: Build directory '$BuildDir' not found." -ForegroundColor Red
    Write-Host "Please run a full build first." -ForegroundColor Yellow
    exit 1
}

# Get list of modified files
Write-Host "Getting list of modified files from git..." -ForegroundColor Cyan
$modifiedFiles = git diff --name-only HEAD

if ($modifiedFiles.Count -eq 0) {
    Write-Host "No modified files found in the git working tree." -ForegroundColor Yellow
    
    # Check if there are staged changes
    $stagedFiles = git diff --name-only --staged
    if ($stagedFiles.Count -gt 0) {
        Write-Host "Found $($stagedFiles.Count) staged files. Include these? (y/n)" -ForegroundColor Yellow
        $includeStaged = Read-Host
        if ($includeStaged -eq "y") {
            $modifiedFiles = $stagedFiles
        }
    }
    
    # If still no files, check for untracked files
    if ($modifiedFiles.Count -eq 0) {
        $untrackedFiles = git ls-files --others --exclude-standard
        if ($untrackedFiles.Count -gt 0) {
            Write-Host "Found $($untrackedFiles.Count) untracked files. Include these? (y/n)" -ForegroundColor Yellow
            $includeUntracked = Read-Host
            if ($includeUntracked -eq "y") {
                $modifiedFiles = $untrackedFiles
            }
        }
    }
    
    # If still no files, exit
    if ($modifiedFiles.Count -eq 0) {
        Write-Host "No files to rebuild. Exiting." -ForegroundColor Yellow
        exit 0
    }
}

# Filter for C++ source files
$modifiedCppFiles = $modifiedFiles | Where-Object { 
    $_ -match "\.(cpp|h|hpp|c|cc|cxx)$" 
}

if ($modifiedCppFiles.Count -eq 0) {
    Write-Host "No C++ source files found among the modified files." -ForegroundColor Yellow
    exit 0
}

# Filter for files in the specified component if provided
if ($Component -ne "") {
    $componentPattern = "src/$Component/|include/$Component/"
    $modifiedCppFiles = $modifiedCppFiles | Where-Object { 
        $_ -match $componentPattern 
    }
    
    if ($modifiedCppFiles.Count -eq 0) {
        Write-Host "No modified C++ files found for component '$Component'." -ForegroundColor Yellow
        exit 0
    }
}

# Display the list of files to rebuild
Write-Host "`nFound $($modifiedCppFiles.Count) modified C++ files:" -ForegroundColor Cyan
$modifiedCppFiles | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }

# Create a timestamp for the build log
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$buildLogFile = "logs/rebuild_${timestamp}.log"

# Create logs directory if it doesn't exist
if (!(Test-Path "logs")) {
    New-Item -ItemType Directory -Path "logs" | Out-Null
}

# Determine targets to rebuild
$targetsToRebuild = @()
$fileToTargetMap = @{}

# Read the build system's dependency information
Write-Host "`nAnalyzing build system dependencies..." -ForegroundColor Cyan

foreach ($file in $modifiedCppFiles) {
    # Convert file path to use backslashes for Windows
    $filePath = $file.Replace("/", "\")
    
    # Get the base name of the file (without extension)
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($file)
    
    # Try to find a target that matches the file name
    $potentialTargets = Get-ChildItem -Path $BuildDir -Recurse -Include "$baseName.dir", "$baseName.vcxproj" -ErrorAction SilentlyContinue
    
    if ($potentialTargets.Count -gt 0) {
        foreach ($target in $potentialTargets) {
            $targetName = [System.IO.Path]::GetFileNameWithoutExtension($target.Name)
            if ($targetName -ne $baseName) {
                $targetName = $baseName
            }
            
            if (!$targetsToRebuild.Contains($targetName)) {
                $targetsToRebuild += $targetName
                $fileToTargetMap[$file] = $targetName
            }
        }
    } else {
        # If no direct match, try to find the library/executable that contains this file
        
        # For header files, we need to find all targets that might include it
        if ($file -match "\.h(pp)?$") {
            # This is more complex - for now, we'll just note it
            Write-Host "  Header file: $file - determining affected targets..." -ForegroundColor Yellow
            
            # For simplicity, if it's in a component directory, rebuild that component
            if ($file -match "include/(\w+)/") {
                $headerComponent = $matches[1]
                $componentTarget = $headerComponent
                
                if (!$targetsToRebuild.Contains($componentTarget)) {
                    $targetsToRebuild += $componentTarget
                    $fileToTargetMap[$file] = $componentTarget
                    Write-Host "    Added component target: $componentTarget" -ForegroundColor Gray
                }
            }
        } else {
            # For source files, try to determine the component
            if ($file -match "src/(\w+)/") {
                $sourceComponent = $matches[1]
                $componentTarget = $sourceComponent
                
                if (!$targetsToRebuild.Contains($componentTarget)) {
                    $targetsToRebuild += $componentTarget
                    $fileToTargetMap[$file] = $componentTarget
                    Write-Host "    Added component target: $componentTarget" -ForegroundColor Gray
                }
            } else {
                Write-Host "  Could not determine target for: $file" -ForegroundColor Yellow
            }
        }
    }
}

# If no targets were found, exit
if ($targetsToRebuild.Count -eq 0) {
    Write-Host "Could not determine any targets to rebuild." -ForegroundColor Yellow
    Write-Host "Try running a full build instead." -ForegroundColor Yellow
    exit 0
}

# Display the targets to rebuild
Write-Host "`nTargets to rebuild:" -ForegroundColor Cyan
$targetsToRebuild | ForEach-Object { Write-Host "  $_" -ForegroundColor Gray }

# Ask for confirmation
Write-Host "`nReady to rebuild $($targetsToRebuild.Count) targets. Proceed? (y/n)" -ForegroundColor Yellow
$proceed = Read-Host
if ($proceed -ne "y") {
    Write-Host "Rebuild cancelled." -ForegroundColor Yellow
    exit 0
}

# Build each target
$buildStart = Get-Date
$successCount = 0
$failureCount = 0

foreach ($target in $targetsToRebuild) {
    Write-Host "`nRebuilding target: $target" -ForegroundColor Cyan
    
    # Build the target
    $buildCommand = "cmake --build $BuildDir --target $target --config $BuildType"
    if ($Verbose) {
        Write-Host "Running: $buildCommand" -ForegroundColor Gray
    }
    
    $buildOutput = Invoke-Expression $buildCommand 2>&1
    $buildSuccess = $LASTEXITCODE -eq 0
    
    # Log the build output
    "=== Build of $target ($(Get-Date)) ===" | Add-Content $buildLogFile
    $buildOutput | Add-Content $buildLogFile
    if ($buildSuccess) {
        "RESULT: SUCCESS" | Add-Content $buildLogFile
    } else {
        "RESULT: FAILURE (Exit code: $LASTEXITCODE)" | Add-Content $buildLogFile
    }
    "" | Add-Content $buildLogFile
    
    # Display build output if verbose or if there was an error
    if ($Verbose -or !$buildSuccess) {
        $buildOutput | ForEach-Object {
            if ($_ -match "error") {
                Write-Host $_ -ForegroundColor Red
            } elseif ($_ -match "warning") {
                Write-Host $_ -ForegroundColor Yellow
            } else {
                Write-Host $_
            }
        }
    }
    
    # Update counters
    if ($buildSuccess) {
        $successCount++
        Write-Host "Target $target built successfully." -ForegroundColor Green
    } else {
        $failureCount++
        Write-Host "Target $target build failed with exit code $LASTEXITCODE" -ForegroundColor Red
    }
}

# Calculate build duration
$buildEnd = Get-Date
$buildDuration = $buildEnd - $buildStart

# Print summary
Write-Host "`nRebuild Summary:" -ForegroundColor Cyan
Write-Host "  Total targets: $($targetsToRebuild.Count)"
Write-Host "  Successful: $successCount" -ForegroundColor Green
Write-Host "  Failed: $failureCount" -ForegroundColor $(if ($failureCount -gt 0) { "Red" } else { "Green" })
Write-Host "  Duration: $($buildDuration.TotalSeconds.ToString("0.00")) seconds"
Write-Host "  Build log: $buildLogFile" -ForegroundColor Gray

# Add summary to log file
"`nSUMMARY:" | Add-Content $buildLogFile
"Total targets: $($targetsToRebuild.Count)" | Add-Content $buildLogFile
"Successful: $successCount" | Add-Content $buildLogFile
"Failed: $failureCount" | Add-Content $buildLogFile
"Duration: $($buildDuration.TotalSeconds.ToString("0.00")) seconds" | Add-Content $buildLogFile
"Completed: $(Get-Date)" | Add-Content $buildLogFile

# Return success if all targets built successfully
if ($failureCount -gt 0) {
    exit 1
} else {
    exit 0
}
