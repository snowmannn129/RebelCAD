param (
    [Parameter(Mandatory=$true)]
    [string]$BuildCommand,
    
    [string]$OutputFile = "build_log.txt",
    
    [string]$ErrorLevel = "ERROR",
    
    [switch]$ShowProgress = $false
)

# Create a timestamp for the log file
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$logFileName = [System.IO.Path]::GetFileNameWithoutExtension($OutputFile)
$logFileExt = [System.IO.Path]::GetExtension($OutputFile)
$timestampedOutputFile = "${logFileName}_${timestamp}${logFileExt}"

Write-Host "Running build command: $BuildCommand"
Write-Host "Logging output to: $timestampedOutputFile"

# Run the build command and capture output
$buildProcess = Start-Process -FilePath "powershell.exe" -ArgumentList "-Command `"$BuildCommand`"" -NoNewWindow -PassThru -RedirectStandardOutput "temp_output.txt" -RedirectStandardError "temp_error.txt"

# Show progress if requested
if ($ShowProgress) {
    Write-Host "Build in progress" -NoNewline
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

# Create logs directory if it doesn't exist
$logsDir = "logs"
if (!(Test-Path $logsDir)) {
    New-Item -ItemType Directory -Path $logsDir | Out-Null
}

$fullOutputPath = Join-Path $logsDir $timestampedOutputFile

if ($errorPattern -ne "") {
    Write-Host "Filtering logs for pattern: $errorPattern"
    Get-Content "temp_output.txt", "temp_error.txt" | Select-String -Pattern $errorPattern | Set-Content $fullOutputPath
}
else {
    Write-Host "Saving full log output"
    Get-Content "temp_output.txt", "temp_error.txt" | Set-Content $fullOutputPath
}

# Clean up temporary files
Remove-Item "temp_output.txt", "temp_error.txt"

# Display summary
$errorCount = (Select-String -Path $fullOutputPath -Pattern "error|Error|ERROR").Count
$warningCount = (Select-String -Path $fullOutputPath -Pattern "warning|Warning|WARNING").Count

Write-Host "Build completed with exit code $($buildProcess.ExitCode)"
Write-Host "Found $errorCount errors and $warningCount warnings."
Write-Host "Full log saved to $fullOutputPath"

# Return the most critical errors (limited to 10)
if ($errorCount -gt 0) {
    Write-Host "`nTop errors:" -ForegroundColor Red
    Select-String -Path $fullOutputPath -Pattern "error|Error|ERROR" | Select-Object -First 10 | ForEach-Object { 
        Write-Host $_.Line -ForegroundColor Red
    }
    
    # If there are more than 10 errors, indicate this
    if ($errorCount -gt 10) {
        Write-Host "... and $($errorCount - 10) more errors. See log file for details." -ForegroundColor Red
    }
}

# Return the exit code from the build process
exit $buildProcess.ExitCode
