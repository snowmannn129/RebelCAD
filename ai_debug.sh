#!/bin/bash
# AI-Assisted Development Workflow Shell Script for RebelCAD
# This shell script runs the PowerShell script for AI-assisted development

# Default values
MODULE="simulation"
SUBMODULE="fea"
TESTTYPE="unit"
BUILDDIR="build_ai_debug"
CLEAN=false
RUNTESTS=true
RUNINTEGRATIONTESTS=false

# Function to display help
show_help() {
    echo "AI-Assisted Development Workflow for RebelCAD"
    echo "Usage: ./ai_debug.sh [options]"
    echo "Options:"
    echo "  -m, --module MODULE            Module to build (default: simulation)"
    echo "  -s, --submodule SUBMODULE      Submodule to build (default: fea)"
    echo "  -t, --testtype TESTTYPE        Test type to run (unit, integration, benchmark, all, none) (default: unit)"
    echo "  -b, --builddir BUILDDIR        Build directory (default: build_ai_debug)"
    echo "  -c, --clean                    Clean build directory"
    echo "  -n, --notests                  Don't run tests"
    echo "  -i, --integration              Run integration tests"
    echo "  -h, --help                     Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./ai_debug.sh --module simulation --submodule fea --clean"
    echo "  ./ai_debug.sh -m modeling -s geometry -t integration"
    exit 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--module)
            MODULE="$2"
            shift 2
            ;;
        -s|--submodule)
            SUBMODULE="$2"
            shift 2
            ;;
        -t|--testtype)
            TESTTYPE="$2"
            shift 2
            ;;
        -b|--builddir)
            BUILDDIR="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -n|--notests)
            RUNTESTS=false
            shift
            ;;
        -i|--integration)
            RUNINTEGRATIONTESTS=true
            shift
            ;;
        -h|--help)
            show_help
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            ;;
    esac
done

# Check if pwsh is installed
if ! command -v pwsh &> /dev/null; then
    echo "Error: PowerShell (pwsh) is not installed."
    echo "Please install PowerShell for Linux/macOS:"
    echo "  https://docs.microsoft.com/en-us/powershell/scripting/install/installing-powershell"
    exit 1
fi

# Construct PowerShell command
PS_CMD="pwsh -ExecutionPolicy Bypass -File scripts/ai_debug_workflow.ps1 -Module $MODULE -Submodule $SUBMODULE -TestType $TESTTYPE -BuildDir $BUILDDIR"

if [ "$CLEAN" = true ]; then
    PS_CMD="$PS_CMD -Clean"
fi

if [ "$RUNTESTS" = false ]; then
    PS_CMD="$PS_CMD -RunTests:\$false"
fi

if [ "$RUNINTEGRATIONTESTS" = true ]; then
    PS_CMD="$PS_CMD -RunIntegrationTests"
fi

# Run PowerShell script
echo "Running AI-Assisted Development Workflow with the following parameters:"
echo "  Module:    $MODULE"
echo "  Submodule: $SUBMODULE"
echo "  Test Type: $TESTTYPE"
echo "  Build Dir: $BUILDDIR"
echo "  Clean:     $CLEAN"
echo "  Run Tests: $RUNTESTS"
echo "  Run Integration Tests: $RUNINTEGRATIONTESTS"
echo ""
echo "Executing: $PS_CMD"
echo ""

$PS_CMD

echo ""
echo "AI-Assisted Development Workflow Complete"
