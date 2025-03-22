# PowerShell script to download and extract Eigen library

# Configuration
$eigenVersion = "3.4.0"
$downloadUrl = "https://gitlab.com/libeigen/eigen/-/archive/$eigenVersion/eigen-$eigenVersion.zip"
$outputZip = "eigen-$eigenVersion.zip"
$extractDir = "external"
$eigenDir = "external/eigen"

# Create directories if they don't exist
if (-not (Test-Path $extractDir)) {
    New-Item -ItemType Directory -Path $extractDir | Out-Null
    Write-Host "Created directory: $extractDir"
}

# Download Eigen
Write-Host "Downloading Eigen $eigenVersion from $downloadUrl..."
Invoke-WebRequest -Uri $downloadUrl -OutFile $outputZip

# Extract Eigen
Write-Host "Extracting Eigen..."
Expand-Archive -Path $outputZip -DestinationPath $extractDir -Force

# Rename directory
if (Test-Path $eigenDir) {
    Remove-Item -Path $eigenDir -Recurse -Force
}
Rename-Item -Path "$extractDir/eigen-$eigenVersion" -NewName "eigen"

# Clean up
Remove-Item -Path $outputZip

Write-Host "Eigen $eigenVersion has been downloaded and extracted to $eigenDir"
