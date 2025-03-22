/**
 * @file FEATest.cpp
 * @brief Test program for the FEA system
 */

#include "simulation/FEASystem.h"
#include "simulation/Material.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "core/Log.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

using namespace rebel::simulation;

/**
 * @brief Creates a simple beam mesh for testing
 * @return Shared pointer to the created mesh
 */
std::shared_ptr<Mesh> createSimpleBeamMesh() {
    // Create a simple beam mesh
    auto mesh = std::make_shared<Mesh>("simple_beam");
    
    // Add nodes
    // Node 1: (0, 0, 0)
    // Node 2: (1, 0, 0)
    // Node 3: (0, 1, 0)
    // Node 4: (1, 1, 0)
    // Node 5: (0, 0, 1)
    // Node 6: (1, 0, 1)
    // Node 7: (0, 1, 1)
    // Node 8: (1, 1, 1)
    
    // Add elements
    // Element 1: Hexahedral element with nodes 1-8
    
    // Add node groups
    // Group "Left": Nodes 1, 3, 5, 7
    // Group "Right": Nodes 2, 4, 6, 8
    
    // Add element groups
    // Group "Beam": Element 1
    
    return mesh;
}

/**
 * @brief Progress callback function
 * @param progress Progress value (0.0 - 1.0)
 */
void progressCallback(float progress) {
    int barWidth = 70;
    int pos = static_cast<int>(barWidth * progress);
    
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
    
    if (progress >= 1.0) {
        std::cout << std::endl;
    }
}

/**
 * @brief Main function
 * @return Exit code
 */
int main() {
    // Initialize the FEA system
    auto& feaSystem = FEASystem::getInstance();
    if (!feaSystem.initialize()) {
        std::cerr << "Failed to initialize FEA system" << std::endl;
        return 1;
    }
    
    // Set FEA settings
    FEASettings settings;
    settings.solverType = SolverType::Linear;
    settings.elementType = ElementType::Hexa;
    settings.meshRefinementLevel = 1;
    settings.convergenceTolerance = 1e-6;
    settings.maxIterations = 1000;
    feaSystem.setSettings(settings);
    
    // Create a mesh
    auto mesh = feaSystem.createMesh("simple_beam", ElementType::Hexa, 1);
    if (!mesh) {
        std::cerr << "Failed to create mesh" << std::endl;
        return 1;
    }
    
    // Register materials
    auto steel = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
    if (!feaSystem.registerMaterial(steel)) {
        std::cerr << "Failed to register material" << std::endl;
        return 1;
    }
    
    // Set material for elements
    int groupId = mesh->createElementGroup("Beam", steel);
    for (size_t i = 0; i < mesh->getElementCount(); ++i) {
        mesh->addElementToGroup(groupId, static_cast<int>(i));
    }
    
    // Add boundary conditions
    auto fixedBC = BoundaryCondition::createDisplacement(
        "fixed",
        "Left",
        DisplacementDirection::XYZ,
        0.0
    );
    if (!feaSystem.addBoundaryCondition(fixedBC)) {
        std::cerr << "Failed to add boundary condition" << std::endl;
        return 1;
    }
    
    // Add loads
    auto pressureLoad = Load::createSurfacePressure(
        "pressure",
        "Right",
        1.0e6  // 1 MPa
    );
    if (!feaSystem.addLoad(pressureLoad)) {
        std::cerr << "Failed to add load" << std::endl;
        return 1;
    }
    
    // Solve the FEA problem
    std::cout << "Solving FEA problem..." << std::endl;
    auto result = feaSystem.solve(progressCallback);
    if (!result) {
        std::cerr << "Failed to solve FEA problem" << std::endl;
        return 1;
    }
    
    // Print results
    std::cout << "FEA problem solved successfully" << std::endl;
    std::cout << "Maximum displacement: " << result->getMaxDisplacementMagnitude() << " m" << std::endl;
    std::cout << "Maximum von Mises stress: " << result->getMaxStress(StressComponent::VonMises) << " Pa" << std::endl;
    
    // Export results
    if (!feaSystem.exportResults(result, "fea_results.json")) {
        std::cerr << "Failed to export results" << std::endl;
        return 1;
    }
    
    // Shutdown the FEA system
    feaSystem.shutdown();
    
    return 0;
}
