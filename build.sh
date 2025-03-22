#!/bin/bash
# Build script for RebelCAD FEA system

echo "Building RebelCAD FEA system..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory
cd build

# Configure the project
echo "Configuring project..."
cmake ..

# Build the project
echo "Building project..."
cmake --build . --config Release

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo ""
echo "You can run the FEA test program with:"
echo "./bin/fea_test"
echo ""

cd ..
