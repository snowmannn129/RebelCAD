@echo off
REM AI-Assisted Development Workflow Batch File for RebelCAD
REM This batch file runs the PowerShell script for AI-assisted development

setlocal enabledelayedexpansion

REM Default values
set MODULE=simulation
set SUBMODULE=fea
set TESTTYPE=unit
set BUILDDIR=build_ai_debug
set CLEAN=false
set RUNTESTS=true
set RUNINTEGRATIONTESTS=false

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse_args
if /i "%~1"=="-module" (
    set MODULE=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="-submodule" (
    set SUBMODULE=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="-testtype" (
    set TESTTYPE=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="-builddir" (
    set BUILDDIR=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="-clean" (
    set CLEAN=true
    shift
    goto :parse_args
)
if /i "%~1"=="-notests" (
    set RUNTESTS=false
    shift
    goto :parse_args
)
if /i "%~1"=="-integration" (
    set RUNINTEGRATIONTESTS=true
    shift
    goto :parse_args
)
if /i "%~1"=="-help" (
    echo AI-Assisted Development Workflow for RebelCAD
    echo Usage: ai_debug.bat [options]
    echo Options:
    echo   -module MODULE            Module to build (default: simulation)
    echo   -submodule SUBMODULE      Submodule to build (default: fea)
    echo   -testtype TESTTYPE        Test type to run (unit, integration, benchmark, all, none) (default: unit)
    echo   -builddir BUILDDIR        Build directory (default: build_ai_debug)
    echo   -clean                    Clean build directory
    echo   -notests                  Don't run tests
    echo   -integration              Run integration tests
    echo   -help                     Show this help message
    echo.
    echo Examples:
    echo   ai_debug.bat -module simulation -submodule fea -clean
    echo   ai_debug.bat -module modeling -submodule geometry -testtype integration
    exit /b 0
)
shift
goto :parse_args
:end_parse_args

REM Construct PowerShell command
set PS_CMD=powershell -ExecutionPolicy Bypass -File scripts\ai_debug_workflow.ps1 -Module %MODULE% -Submodule %SUBMODULE% -TestType %TESTTYPE% -BuildDir %BUILDDIR%

if "%CLEAN%"=="true" (
    set PS_CMD=!PS_CMD! -Clean
)

if "%RUNTESTS%"=="false" (
    set PS_CMD=!PS_CMD! -RunTests:$false
)

if "%RUNINTEGRATIONTESTS%"=="true" (
    set PS_CMD=!PS_CMD! -RunIntegrationTests
)

REM Run PowerShell script
echo Running AI-Assisted Development Workflow with the following parameters:
echo   Module:    %MODULE%
echo   Submodule: %SUBMODULE%
echo   Test Type: %TESTTYPE%
echo   Build Dir: %BUILDDIR%
echo   Clean:     %CLEAN%
echo   Run Tests: %RUNTESTS%
echo   Run Integration Tests: %RUNINTEGRATIONTESTS%
echo.
echo Executing: %PS_CMD%
echo.

%PS_CMD%

echo.
echo AI-Assisted Development Workflow Complete
