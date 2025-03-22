@echo off
REM Build script for RebelCAD FEA system

echo Building RebelCAD FEA system...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Navigate to build directory
cd build

REM Configure the project
echo Configuring project...
cmake .. -G "Visual Studio 17 2022" -A win32

REM Build the project
echo Building project...
cmake --build . --config Release

REM Check if build was successful
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build completed successfully!
echo.
echo You can run the FEA test program with:
echo bin\Release\fea_test.exe
echo.

cd ..
