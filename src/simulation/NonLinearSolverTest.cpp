#include "simulation/NonLinearSolver.h"
#include "simulation/FEASystem.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "core/Log.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace rebel::simulation;

/**
 * @brief Simple test program for the NonLinearSolver
 * 
 * This program demonstrates the usage of the NonLinearSolver class.
 * It creates a simple mesh, applies boundary conditions and loads,
 * and solves the non-linear problem using different methods.
 */
int main(int argc, char** argv) {
    // Initialize the FEA system
    FEASystem& feaSystem = FEASystem::getInstance();
    if (!feaSystem.initialize()) {
        std::cerr << "Failed to initialize FEA system" << std::endl;
        return 1;
    }
    
    // Create a simple mesh
    std::shared_ptr<Mesh> mesh = feaSystem.createMesh("simple_cube", ElementType::Tetra, 1);
    if (!mesh) {
        std::cerr << "Failed to create mesh" << std::endl;
        return 1;
    }
    
    // Create boundary conditions
    std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
    // Add boundary conditions here
    
    // Create loads
    std::vector<std::shared_ptr<Load>> loads;
    // Add loads here
    
    // Create non-linear solver settings
    NonLinearSolverSettings settings;
    settings.method = NonLinearSolverMethod::NewtonRaphson;
    settings.nonLinearityType = NonLinearityType::Geometric;
    settings.convergenceTolerance = 1e-6;
    settings.maxIterations = 100;
    
    // Create non-linear solver
    NonLinearSolver solver(feaSystem.getSettings(), settings);
    
    // Progress callback
    auto progressCallback = [](float progress) {
        std::cout << "Progress: " << progress * 100.0f << "%" << std::endl;
    };
    
    // Solve the problem
    std::shared_ptr<FEAResult> result = solver.solve(mesh, boundaryConditions, loads, progressCallback);
    if (!result) {
        std::cerr << "Failed to solve the problem" << std::endl;
        return 1;
    }
    
    // Print results
    std::cout << "Non-linear analysis completed successfully" << std::endl;
    
    // Shutdown the FEA system
    feaSystem.shutdown();
    
    return 0;
}
