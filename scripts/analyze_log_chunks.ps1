param (
    [Parameter(Mandatory=$true)]
    [string]$LogFile,
    
    [int]$ChunkSize = 1000,
    
    [string]$OutputDir = "log_chunks"
)

# Verify the log file exists
if (!(Test-Path $LogFile)) {
    Write-Host "Error: Log file '$LogFile' not found." -ForegroundColor Red
    exit 1
}

# Create output directory with timestamp
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$outputDirWithTimestamp = "${OutputDir}_${timestamp}"

if (!(Test-Path $outputDirWithTimestamp)) {
    New-Item -ItemType Directory -Path $outputDirWithTimestamp -Force | Out-Null
    Write-Host "Created output directory: $outputDirWithTimestamp"
}

# Read the log file
Write-Host "Reading log file: $LogFile"
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
    
    $outputFile = Join-Path $outputDirWithTimestamp "chunk_$($i + 1)_of_$numChunks.txt"
    $chunkContent | Set-Content $outputFile
    
    # Count errors and warnings in this chunk
    $errorCount = ($chunkContent | Select-String -Pattern "error|Error|ERROR").Count
    $warningCount = ($chunkContent | Select-String -Pattern "warning|Warning|WARNING").Count
    
    # Use color coding based on error count
    $color = "Green"
    if ($errorCount -gt 0) {
        $color = "Red"
    } elseif ($warningCount -gt 0) {
        $color = "Yellow"
    }
    
    Write-Host "Chunk $($i + 1): Lines $startLine-$endLine, $errorCount errors, $warningCount warnings" -ForegroundColor $color
}

# Create a summary file
$summaryFile = Join-Path $outputDirWithTimestamp "summary.txt"
"Log Analysis Summary for $LogFile" | Set-Content $summaryFile
"Generated: $(Get-Date)" | Add-Content $summaryFile
"Total lines: $totalLines" | Add-Content $summaryFile
"Total chunks: $numChunks" | Add-Content $summaryFile
"Chunk size: $ChunkSize" | Add-Content $summaryFile
"" | Add-Content $summaryFile

# Add error summary for each chunk
$totalErrors = 0
$totalWarnings = 0

for ($i = 0; $i -lt $numChunks; $i++) {
    $chunkFile = Join-Path $outputDirWithTimestamp "chunk_$($i + 1)_of_$numChunks.txt"
    $chunkContent = Get-Content $chunkFile
    
    $errorCount = ($chunkContent | Select-String -Pattern "error|Error|ERROR").Count
    $warningCount = ($chunkContent | Select-String -Pattern "warning|Warning|WARNING").Count
    
    $totalErrors += $errorCount
    $totalWarnings += $warningCount
    
    "Chunk $($i + 1): $errorCount errors, $warningCount warnings" | Add-Content $summaryFile
    
    if ($errorCount -gt 0) {
        "  Errors:" | Add-Content $summaryFile
        $chunkContent | Select-String -Pattern "error|Error|ERROR" | ForEach-Object {
            "    - $($_.Line)" | Add-Content $summaryFile
        }
    }
}

# Add total counts to summary
"" | Add-Content $summaryFile
"Total errors: $totalErrors" | Add-Content $summaryFile
"Total warnings: $totalWarnings" | Add-Content $summaryFile

# Create an HTML report for easier viewing
$htmlReport = Join-Path $outputDirWithTimestamp "report.html"

$htmlContent = @"
<!DOCTYPE html>
<html>
<head>
    <title>Log Analysis Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .summary { background-color: #f0f0f0; padding: 10px; border-radius: 5px; }
        .chunk { margin-top: 20px; border: 1px solid #ddd; padding: 10px; border-radius: 5px; }
        .chunk-header { font-weight: bold; }
        .error { color: red; }
        .warning { color: orange; }
        .clean { color: green; }
        .error-list { margin-left: 20px; }
    </style>
</head>
<body>
    <h1>Log Analysis Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p>Log File: $LogFile</p>
        <p>Generated: $(Get-Date)</p>
        <p>Total Lines: $totalLines</p>
        <p>Total Chunks: $numChunks</p>
        <p>Chunk Size: $ChunkSize</p>
        <p>Total Errors: <span class="error">$totalErrors</span></p>
        <p>Total Warnings: <span class="warning">$totalWarnings</span></p>
    </div>
"@

for ($i = 0; $i -lt $numChunks; $i++) {
    $chunkFile = Join-Path $outputDirWithTimestamp "chunk_$($i + 1)_of_$numChunks.txt"
    $chunkContent = Get-Content $chunkFile
    
    $errorCount = ($chunkContent | Select-String -Pattern "error|Error|ERROR").Count
    $warningCount = ($chunkContent | Select-String -Pattern "warning|Warning|WARNING").Count
    
    $chunkClass = "clean"
    if ($errorCount -gt 0) {
        $chunkClass = "error"
    } elseif ($warningCount -gt 0) {
        $chunkClass = "warning"
    }
    
    $htmlContent += @"
    <div class="chunk">
        <div class="chunk-header $chunkClass">Chunk $($i + 1): Lines $($i * $ChunkSize)-$([Math]::Min(($i + 1) * $ChunkSize - 1, $totalLines - 1)), $errorCount errors, $warningCount warnings</div>
"@

    if ($errorCount -gt 0) {
        $htmlContent += @"
        <div class="error-list">
            <h3>Errors:</h3>
            <ul>
"@
        $chunkContent | Select-String -Pattern "error|Error|ERROR" | ForEach-Object {
            $htmlContent += @"
                <li class="error">$($_.Line)</li>
"@
        }
        $htmlContent += @"
            </ul>
        </div>
"@
    }

    $htmlContent += @"
    </div>
"@
}

$htmlContent += @"
</body>
</html>
"@

$htmlContent | Set-Content $htmlReport

Write-Host "Analysis complete." -ForegroundColor Green
Write-Host "Summary saved to: $summaryFile"
Write-Host "HTML report saved to: $htmlReport"
Write-Host "All chunks saved to: $outputDirWithTimestamp"
