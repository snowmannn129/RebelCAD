#include "simulation/FEASystem.h"
#include "simulation/NonLinearSolver.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "core/Log.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

using namespace rebel::simulation;

/**
 * @brief Example program demonstrating the use of the NonLinearSolver
 * 
 * This program shows how to set up and use the NonLinearSolver for
 * non-linear finite element analysis.
 */
int main(int argc, char** argv) {
    // Initialize the FEA system
    FEASystem& feaSystem = FEASystem::getInstance();
    if (!feaSystem.initialize()) {
        std::cerr << "Failed to initialize FEA system" << std::endl;
        return 1;
    }
    
    // Set the solver type to NonLinear
    FEASettings settings = feaSystem.getSettings();
    settings.solverType = SolverType::NonLinear;
    feaSystem.setSettings(settings);
    
    // Create a simple mesh
    std::shared_ptr<Mesh> mesh = feaSystem.createMesh("simple_beam", ElementType::Beam, 2);
    if (!mesh) {
        std::cerr << "Failed to create mesh" << std::endl;
        return 1;
    }
    
    // Create boundary conditions
    std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
    // In a real application, you would add boundary conditions here
    
    // Create loads
    std::vector<std::shared_ptr<Load>> loads;
    // In a real application, you would add loads here
    
    // Create non-linear solver settings
    NonLinearSolverSettings nonLinearSettings;
    nonLinearSettings.method = NonLinearSolverMethod::NewtonRaphson;
    nonLinearSettings.nonLinearityType = NonLinearityType::Geometric;
    nonLinearSettings.convergenceTolerance = 1e-6;
    nonLinearSettings.maxIterations = 100;
    nonLinearSettings.loadIncrementFactor = 0.1;
    nonLinearSettings.numLoadSteps = 10;
    nonLinearSettings.adaptiveLoadStepping = true;
    
    // Create the non-linear solver
    NonLinearSolver solver(settings, nonLinearSettings);
    
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
    
    // Export results
    std::string resultFile = "nonlinear_result.fea";
    if (feaSystem.exportResults(result, resultFile)) {
        std::cout << "Results exported to " << resultFile << std::endl;
    } else {
        std::cerr << "Failed to export results" << std::endl;
    }
    
    // Shutdown the FEA system
    feaSystem.shutdown();
    
    return 0;
}
