#include "simulation/DynamicSolver.h"
#include "simulation/FEASystem.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "core/Log.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

using namespace rebel::simulation;

// Helper function to print modal analysis results
void printModalResults(const std::shared_ptr<ModalResult>& result) {
    std::cout << "=== Modal Analysis Results ===" << std::endl;
    std::cout << std::setw(5) << "Mode" << std::setw(15) << "Frequency (Hz)" 
              << std::setw(15) << "Period (s)" << std::setw(20) << "Effective Mass (%)" << std::endl;
    
    for (int i = 0; i < result->frequencies.size(); ++i) {
        std::cout << std::setw(5) << i + 1 
                  << std::setw(15) << std::fixed << std::setprecision(4) << result->frequencies[i]
                  << std::setw(15) << std::fixed << std::setprecision(4) << result->periods[i]
                  << std::setw(20) << std::fixed << std::setprecision(2) 
                  << result->effectiveMassRatios[i] * 100.0 << "%" << std::endl;
    }
    
    std::cout << std::endl;
}

// Helper function to print time history results
void printTimeHistoryResults(const std::shared_ptr<TimeHistoryResult>& result) {
    std::cout << "=== Time History Analysis Results ===" << std::endl;
    std::cout << "Number of time points: " << result->timePoints.size() << std::endl;
    
    // Print first few time points
    int numPointsToPrint = std::min(5, static_cast<int>(result->timePoints.size()));
    std::cout << "First " << numPointsToPrint << " time points:" << std::endl;
    
    for (int i = 0; i < numPointsToPrint; ++i) {
        std::cout << "Time: " << std::fixed << std::setprecision(4) << result->timePoints[i] << " s" << std::endl;
        
        // Print displacement results for a specific node (if available)
        if (!result->displacements[i].empty()) {
            std::cout << "  Displacement at node 1: "
                      << "X=" << result->displacements[i][0].x
                      << ", Y=" << result->displacements[i][0].y
                      << ", Z=" << result->displacements[i][0].z << std::endl;
        }
    }
    
    std::cout << std::endl;
}

// Create a simple beam mesh for testing
std::shared_ptr<Mesh> createBeamMesh() {
    // Initialize FEA system
    auto& feaSystem = FEASystem::getInstance();
    
    // Create a simple beam mesh
    auto mesh = feaSystem.createMesh("beam", ElementType::Beam, 1);
    
    // TODO: Add nodes and elements to the mesh
    
    return mesh;
}

// Create boundary conditions for a cantilever beam
std::vector<std::shared_ptr<BoundaryCondition>> createBoundaryConditions() {
    std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
    
    // Fixed support at one end (all DOFs constrained)
    auto fixedBC = BoundaryCondition::createDisplacement("fixed", "node_group_1", 
                                                        DisplacementDirection::XYZ, 0.0);
    boundaryConditions.push_back(fixedBC);
    
    return boundaryConditions;
}

// Create loads for a cantilever beam
std::vector<std::shared_ptr<Load>> createLoads() {
    std::vector<std::shared_ptr<Load>> loads;
    
    // Point force at the free end
    auto staticForce = Load::createPointForce("static_force", "node_group_2", 
                                             0.0, -1000.0, 0.0, LoadVariation::Static);
    loads.push_back(staticForce);
    
    // Harmonic force at the free end
    auto harmonicForce = Load::createPointForce("harmonic_force", "node_group_2", 
                                               0.0, -1000.0, 0.0, LoadVariation::Harmonic);
    loads.push_back(harmonicForce);
    
    return loads;
}

int main() {
    try {
        // Initialize FEA system
        auto& feaSystem = FEASystem::getInstance();
        if (!feaSystem.initialize()) {
            std::cerr << "Failed to initialize FEA system" << std::endl;
            return 1;
        }
        
        // Create a simple beam mesh
        auto mesh = createBeamMesh();
        if (!mesh) {
            std::cerr << "Failed to create mesh" << std::endl;
            return 1;
        }
        
        // Create boundary conditions
        auto boundaryConditions = createBoundaryConditions();
        
        // Create loads
        auto loads = createLoads();
        
        // Create FEA settings
        FEASettings feaSettings;
        feaSettings.solverType = SolverType::Modal;
        feaSettings.elementType = ElementType::Beam;
        feaSettings.meshRefinementLevel = 2;
        
        // Create dynamic solver settings
        DynamicSolverSettings dynamicSettings;
        dynamicSettings.timeIntegrationMethod = TimeIntegrationMethod::Newmark;
        dynamicSettings.startTime = 0.0;
        dynamicSettings.endTime = 1.0;
        dynamicSettings.timeStep = 0.01;
        dynamicSettings.numModes = 10;
        dynamicSettings.useDamping = true;
        dynamicSettings.rayleighAlpha = 0.01;
        dynamicSettings.rayleighBeta = 0.001;
        
        // Create dynamic solver
        DynamicSolver dynamicSolver(feaSettings, dynamicSettings);
        
        // Perform modal analysis
        std::cout << "Performing modal analysis..." << std::endl;
        auto modalResult = dynamicSolver.solveModal(mesh, boundaryConditions, 
                                                   [](float progress) {
                                                       std::cout << "Modal analysis progress: " 
                                                                 << std::fixed << std::setprecision(1) 
                                                                 << progress * 100.0 << "%" << std::endl;
                                                   });
        
        if (modalResult) {
            printModalResults(modalResult);
        } else {
            std::cerr << "Modal analysis failed" << std::endl;
            return 1;
        }
        
        // Perform time history analysis
        std::cout << "Performing time history analysis..." << std::endl;
        auto timeHistoryResult = dynamicSolver.solveTimeHistory(mesh, boundaryConditions, loads, {}, {},
                                                              [](float progress) {
                                                                  std::cout << "Time history analysis progress: " 
                                                                            << std::fixed << std::setprecision(1) 
                                                                            << progress * 100.0 << "%" << std::endl;
                                                              });
        
        if (timeHistoryResult) {
            printTimeHistoryResults(timeHistoryResult);
        } else {
            std::cerr << "Time history analysis failed" << std::endl;
            return 1;
        }
        
        // Perform dynamic analysis
        std::cout << "Performing dynamic analysis..." << std::endl;
        auto dynamicResult = dynamicSolver.solve(mesh, boundaryConditions, loads,
                                               [](float progress) {
                                                   std::cout << "Dynamic analysis progress: " 
                                                             << std::fixed << std::setprecision(1) 
                                                             << progress * 100.0 << "%" << std::endl;
                                               });
        
        if (dynamicResult) {
            std::cout << "Dynamic analysis completed successfully" << std::endl;
        } else {
            std::cerr << "Dynamic analysis failed" << std::endl;
            return 1;
        }
        
        // Shutdown FEA system
        feaSystem.shutdown();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
