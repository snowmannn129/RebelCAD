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
$cmakeOptions += " -DBUILD_SKETCHING=OFF -DBUILD_ASSEMBLY=OFF -DBUILD_CONSTRAINTS=OFF -DBUILD_GRAPHICS=OFF"

# Enable the specified component
$cmakeOptions += " -DBUILD_$($Component.ToUpper())=ON"

# Configure
Write-Host "Configuring $Component component with options: $cmakeOptions"
$configOutput = cmake -B $buildDir $cmakeOptions 2>&1
$configOutput | Out-File -FilePath "build_${Component}_config.log"

# Build
Write-Host "Building $Component component..."
$buildOutput = cmake --build $buildDir --config $BuildType 2>&1
$buildOutput | Out-File -FilePath "build_${Component}_build.log"

# Check if build was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build of $Component component completed successfully!" -ForegroundColor Green
    "Build of $Component component completed successfully!" | Out-File -FilePath "build_${Component}_status.log"
} else {
    Write-Host "Build of $Component component failed with exit code $LASTEXITCODE" -ForegroundColor Red
    "Build of $Component component failed with exit code $LASTEXITCODE" | Out-File -FilePath "build_${Component}_status.log"
}
