@echo off
REM RebelCAD Build Tools Launcher
REM This script provides a menu-based interface to the build and debug scripts

setlocal enabledelayedexpansion

:MENU
cls
echo.
echo RebelCAD Build Tools
echo ===================
echo.
echo Build Options:
echo  1. Build Component
echo  2. Rebuild Modified Files
echo.
echo Test Options:
echo  3. Test Component
echo  4. Run Tests by Category
echo.
echo Debug Options:
echo  5. Debug Incrementally
echo.
echo Log Management:
echo  6. Filter Build Logs
echo  7. Analyze Log Chunks
echo  8. Rotate Logs
echo.
echo Other Options:
echo  9. View Documentation
echo  0. Exit
echo.
set /p choice=Enter your choice (0-9): 

if "%choice%"=="0" goto :EOF
if "%choice%"=="1" goto :BUILD_COMPONENT
if "%choice%"=="2" goto :REBUILD_MODIFIED
if "%choice%"=="3" goto :TEST_COMPONENT
if "%choice%"=="4" goto :RUN_TESTS_BY_CATEGORY
if "%choice%"=="5" goto :DEBUG_INCREMENTAL
if "%choice%"=="6" goto :FILTER_LOGS
if "%choice%"=="7" goto :ANALYZE_LOG_CHUNKS
if "%choice%"=="8" goto :ROTATE_LOGS
if "%choice%"=="9" goto :VIEW_DOCS
goto :MENU

:BUILD_COMPONENT
cls
echo.
echo Build Component
echo ==============
echo.
echo Available components:
echo  - simulation
echo  - core
echo  - modeling
echo  - ui
echo  - sketching
echo  - assembly
echo  - constraints
echo  - graphics
echo.
set /p component=Enter component name: 
set /p buildType=Enter build type (Debug/Release) [Release]: 
if "!buildType!"=="" set buildType=Release
set /p logLevel=Enter log level (ERROR/WARNING/INFO/DEBUG/VERBOSE) [INFO]: 
if "!logLevel!"=="" set logLevel=INFO

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\build_component.ps1 -Component !component! -BuildType !buildType! -LogLevel !logLevel!
echo.
powershell -ExecutionPolicy Bypass -File scripts\build_component.ps1 -Component !component! -BuildType !buildType! -LogLevel !logLevel!
echo.
pause
goto :MENU

:REBUILD_MODIFIED
cls
echo.
echo Rebuild Modified Files
echo =====================
echo.
set /p component=Enter component name (leave empty for all): 
set /p buildType=Enter build type (Debug/Release) [Release]: 
if "!buildType!"=="" set buildType=Release
set /p verbose=Show verbose output? (y/n) [n]: 
if "!verbose!"=="y" (
    set verboseFlag=-Verbose
) else (
    set verboseFlag=
)

echo.
if "!component!"=="" (
    echo Running: powershell -ExecutionPolicy Bypass -File scripts\rebuild_modified.ps1 -BuildType !buildType! !verboseFlag!
    powershell -ExecutionPolicy Bypass -File scripts\rebuild_modified.ps1 -BuildType !buildType! !verboseFlag!
) else (
    echo Running: powershell -ExecutionPolicy Bypass -File scripts\rebuild_modified.ps1 -Component !component! -BuildType !buildType! !verboseFlag!
    powershell -ExecutionPolicy Bypass -File scripts\rebuild_modified.ps1 -Component !component! -BuildType !buildType! !verboseFlag!
)
echo.
pause
goto :MENU

:TEST_COMPONENT
cls
echo.
echo Test Component
echo =============
echo.
echo Available components:
echo  - simulation
echo  - core
echo  - modeling
echo  - ui
echo  - sketching
echo  - assembly
echo  - constraints
echo  - graphics
echo.
set /p component=Enter component name: 
set /p testFilter=Enter test filter (leave empty for all): 
set /p verbose=Show verbose output? (y/n) [n]: 
if "!verbose!"=="y" (
    set verboseFlag=-Verbose
) else (
    set verboseFlag=
)

echo.
if "!testFilter!"=="" (
    echo Running: powershell -ExecutionPolicy Bypass -File scripts\test_component.ps1 -Component !component! !verboseFlag!
    powershell -ExecutionPolicy Bypass -File scripts\test_component.ps1 -Component !component! !verboseFlag!
) else (
    echo Running: powershell -ExecutionPolicy Bypass -File scripts\test_component.ps1 -Component !component! -TestFilter "!testFilter!" !verboseFlag!
    powershell -ExecutionPolicy Bypass -File scripts\test_component.ps1 -Component !component! -TestFilter "!testFilter!" !verboseFlag!
)
echo.
pause
goto :MENU

:RUN_TESTS_BY_CATEGORY
cls
echo.
echo Run Tests by Category
echo ===================
echo.
echo Available categories:
echo  - SMALL
echo  - MEDIUM
echo  - LARGE
echo  - SIMULATION
echo  - CORE
echo  - MODELING
echo  - UI
echo  - SKETCHING
echo  - ASSEMBLY
echo  - CONSTRAINTS
echo  - GRAPHICS
echo.
set /p category=Enter category: 
set /p buildType=Enter build type (Debug/Release) [Release]: 
if "!buildType!"=="" set buildType=Release
set /p verbose=Show verbose output? (y/n) [n]: 
if "!verbose!"=="y" (
    set verboseFlag=-Verbose
) else (
    set verboseFlag=
)

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\run_tests_by_category.ps1 -Category !category! -BuildType !buildType! !verboseFlag!
powershell -ExecutionPolicy Bypass -File scripts\run_tests_by_category.ps1 -Category !category! -BuildType !buildType! !verboseFlag!
echo.
pause
goto :MENU

:DEBUG_INCREMENTAL
cls
echo.
echo Debug Incrementally
echo =================
echo.
echo Available components:
echo  - simulation
echo  - core
echo  - modeling
echo  - ui
echo  - sketching
echo  - assembly
echo  - constraints
echo  - graphics
echo.
set /p component=Enter component name: 
set /p startTest=Enter start test (leave empty to start from beginning): 
set /p endTest=Enter end test (leave empty to run to end): 
set /p saveState=Save state for each test? (y/n) [n]: 
if "!saveState!"=="y" (
    set saveStateFlag=-SaveState
) else (
    set saveStateFlag=
)

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\debug_incremental.ps1 -Component !component! -StartTest "!startTest!" -EndTest "!endTest!" !saveStateFlag!
powershell -ExecutionPolicy Bypass -File scripts\debug_incremental.ps1 -Component !component! -StartTest "!startTest!" -EndTest "!endTest!" !saveStateFlag!
echo.
pause
goto :MENU

:FILTER_LOGS
cls
echo.
echo Filter Build Logs
echo ===============
echo.
set /p buildCommand=Enter build command: 
set /p outputFile=Enter output file [build_log.txt]: 
if "!outputFile!"=="" set outputFile=build_log.txt
set /p errorLevel=Enter error level (ERROR/WARNING/INFO) [ERROR]: 
if "!errorLevel!"=="" set errorLevel=ERROR
set /p showProgress=Show progress? (y/n) [n]: 
if "!showProgress!"=="y" (
    set showProgressFlag=-ShowProgress
) else (
    set showProgressFlag=
)

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\filter_logs.ps1 -BuildCommand "!buildCommand!" -OutputFile !outputFile! -ErrorLevel !errorLevel! !showProgressFlag!
powershell -ExecutionPolicy Bypass -File scripts\filter_logs.ps1 -BuildCommand "!buildCommand!" -OutputFile !outputFile! -ErrorLevel !errorLevel! !showProgressFlag!
echo.
pause
goto :MENU

:ANALYZE_LOG_CHUNKS
cls
echo.
echo Analyze Log Chunks
echo ================
echo.
set /p logFile=Enter log file path: 
set /p chunkSize=Enter chunk size [1000]: 
if "!chunkSize!"=="" set chunkSize=1000
set /p outputDir=Enter output directory [log_chunks]: 
if "!outputDir!"=="" set outputDir=log_chunks

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\analyze_log_chunks.ps1 -LogFile "!logFile!" -ChunkSize !chunkSize! -OutputDir "!outputDir!"
powershell -ExecutionPolicy Bypass -File scripts\analyze_log_chunks.ps1 -LogFile "!logFile!" -ChunkSize !chunkSize! -OutputDir "!outputDir!"
echo.
pause
goto :MENU

:ROTATE_LOGS
cls
echo.
echo Rotate Logs
echo ==========
echo.
set /p keepDays=Enter number of days to keep logs [7]: 
if "!keepDays!"=="" set keepDays=7
set /p logDir=Enter log directory [logs]: 
if "!logDir!"=="" set logDir=logs
set /p compress=Compress logs? (y/n) [n]: 
if "!compress!"=="y" (
    set compressFlag=-Compress
) else (
    set compressFlag=
)

echo.
echo Running: powershell -ExecutionPolicy Bypass -File scripts\rotate_logs.ps1 -KeepDays !keepDays! -LogDir "!logDir!" !compressFlag!
powershell -ExecutionPolicy Bypass -File scripts\rotate_logs.ps1 -KeepDays !keepDays! -LogDir "!logDir!" !compressFlag!
echo.
pause
goto :MENU

:VIEW_DOCS
cls
echo.
echo View Documentation
echo ================
echo.
echo Available documentation:
echo  1. Build and Debug Strategy
echo  2. Scripts README
echo  3. Back to Main Menu
echo.
set /p docChoice=Enter your choice (1-3): 

if "!docChoice!"=="1" (
    start "" notepad.exe docs\BuildAndDebugStrategy.md
) else if "!docChoice!"=="2" (
    start "" notepad.exe scripts\README.md
)
goto :MENU
