param (
    [int]$KeepDays = 7,
    [string]$LogDir = "logs",
    [switch]$Compress = $false
)

# Create log directory if it doesn't exist
if (!(Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir | Out-Null
    Write-Host "Created log directory: $LogDir"
}

# Get current date
$currentDate = Get-Date -Format "yyyy-MM-dd"

# Create a new log directory for today
$todayLogDir = Join-Path $LogDir $currentDate
if (!(Test-Path $todayLogDir)) {
    New-Item -ItemType Directory -Path $todayLogDir | Out-Null
    Write-Host "Created today's log directory: $todayLogDir"
}

# Find all log files in the current directory and subdirectories
$logFiles = Get-ChildItem -Path "." -Include "*.log", "*.txt" -Recurse | Where-Object {
    # Exclude files in the logs directory itself
    $_.FullName -notlike "*\$LogDir\*" -and
    # Exclude files in the test_results directory
    $_.FullName -notlike "*\test_results\*" -and
    # Exclude files in the debug_states directory
    $_.FullName -notlike "*\debug_states\*" -and
    # Only include files that look like logs
    ($_.Name -like "*.log" -or 
     $_.Name -like "*log*.txt" -or 
     $_.Name -like "build*.txt" -or
     $_.Name -like "error*.txt" -or
     $_.Name -like "output*.txt" -or
     $_.Name -like "debug*.txt")
}

# Count the number of log files found
$logFileCount = $logFiles.Count
Write-Host "Found $logFileCount log files to process"

# Move all log files to today's directory
foreach ($file in $logFiles) {
    # Create a subdirectory based on the file's parent directory
    $parentDir = Split-Path (Split-Path $file.FullName -Parent) -Leaf
    $subDir = Join-Path $todayLogDir $parentDir
    
    if (!(Test-Path $subDir)) {
        New-Item -ItemType Directory -Path $subDir | Out-Null
    }
    
    # Create a destination path that includes the original directory structure
    $destPath = Join-Path $subDir $file.Name
    
    # If a file with the same name already exists, append a timestamp
    if (Test-Path $destPath) {
        $timestamp = Get-Date -Format "HHmmss"
        $destPath = Join-Path $subDir "$($file.BaseName)_$timestamp$($file.Extension)"
    }
    
    # Move the file
    Move-Item $file.FullName -Destination $destPath
    Write-Host "Moved $($file.FullName) to $destPath"
}

# Compress today's log directory if requested
if ($Compress) {
    $compressedFile = "$todayLogDir.zip"
    
    if (Test-Path $compressedFile) {
        Remove-Item $compressedFile -Force
    }
    
    Write-Host "Compressing logs to $compressedFile..."
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::CreateFromDirectory($todayLogDir, $compressedFile)
    
    # Remove the uncompressed directory
    Remove-Item $todayLogDir -Recurse -Force
    Write-Host "Compressed logs and removed original directory"
}

# Remove old log directories
$cutoffDate = (Get-Date).AddDays(-$KeepDays)
$oldDirs = Get-ChildItem -Path $LogDir -Directory | Where-Object {
    try {
        $dirDate = [DateTime]::ParseExact($_.Name, "yyyy-MM-dd", $null)
        $dirDate -lt $cutoffDate
    } catch {
        # If the directory name is not a date, skip it
        $false
    }
}

foreach ($dir in $oldDirs) {
    Remove-Item $dir.FullName -Recurse -Force
    Write-Host "Removed old log directory: $($dir.Name)"
}

# Also remove old compressed logs if compression is enabled
if ($Compress) {
    $oldZips = Get-ChildItem -Path $LogDir -Filter "*.zip" | Where-Object {
        try {
            $zipDate = [DateTime]::ParseExact($_.BaseName, "yyyy-MM-dd", $null)
            $zipDate -lt $cutoffDate
        } catch {
            # If the zip file name is not a date, skip it
            $false
        }
    }
    
    foreach ($zip in $oldZips) {
        Remove-Item $zip.FullName -Force
        Write-Host "Removed old compressed logs: $($zip.Name)"
    }
}

# Generate a summary report
$summaryFile = Join-Path $LogDir "rotation_summary.txt"
"Log Rotation Summary" | Set-Content $summaryFile
"Generated: $(Get-Date)" | Add-Content $summaryFile
"Log files processed: $logFileCount" | Add-Content $summaryFile
"Logs stored in: $todayLogDir" | Add-Content $summaryFile
if ($Compress) {
    "Compressed to: $compressedFile" | Add-Content $summaryFile
}
"Retention policy: $KeepDays days" | Add-Content $summaryFile
"Old directories removed: $($oldDirs.Count)" | Add-Content $summaryFile
if ($Compress) {
    "Old compressed logs removed: $($oldZips.Count)" | Add-Content $summaryFile
}

Write-Host "`nLog rotation complete!" -ForegroundColor Green
Write-Host "Summary saved to: $summaryFile" -ForegroundColor Cyan
