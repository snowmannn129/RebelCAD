#include "simulation/FEASystem.h"
#include <iostream>

int main() {
    // Get the FEA system instance
    rebel::simulation::FEASystem& feaSystem = rebel::simulation::FEASystem::getInstance();
    
    // Initialize the FEA system
    if (!feaSystem.initialize()) {
        std::cerr << "Failed to initialize FEA system" << std::endl;
        return 1;
    }
    
    std::cout << "FEA system initialized successfully" << std::endl;
    
    // Set FEA settings
    rebel::simulation::FEASettings settings;
    settings.solverType = rebel::simulation::SolverType::Linear;
    settings.elementType = rebel::simulation::ElementType::Tetra;
    settings.meshRefinementLevel = 1;
    settings.convergenceTolerance = 1e-6;
    settings.maxIterations = 1000;
    feaSystem.setSettings(settings);
    
    std::cout << "FEA settings set successfully" << std::endl;
    
    // Shutdown the FEA system
    feaSystem.shutdown();
    
    std::cout << "FEA system shut down successfully" << std::endl;
    
    return 0;
}
