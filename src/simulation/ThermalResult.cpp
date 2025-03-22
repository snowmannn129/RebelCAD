#include "simulation/ThermalSolver.h"
#include "core/Log.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rebel::simulation {

// ThermalResult constructor
ThermalResult::ThermalResult(int numTimePoints) {
    timePoints.resize(numTimePoints);
    temperatures.resize(numTimePoints);
    heatFluxes.resize(numTimePoints);
    thermalGradients.resize(numTimePoints);
}

// Export results to file
bool ThermalResult::exportToFile(const std::string& filePath) const {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        // Write header
        file << "Time,Temperature,HeatFlux_X,HeatFlux_Y,HeatFlux_Z,ThermalGradient_X,ThermalGradient_Y,ThermalGradient_Z" << std::endl;
        
        // Write data
        for (size_t i = 0; i < timePoints.size(); ++i) {
            file << timePoints[i];
            
            // Write temperature (first node only for simplicity)
            if (!temperatures[i].empty()) {
                file << "," << temperatures[i][0];
            } else {
                file << ",0.0";
            }
            
            // Write heat flux (first node only for simplicity)
            if (!heatFluxes[i].empty()) {
                file << "," << heatFluxes[i][0];
                file << "," << (heatFluxes[i].size() > 1 ? heatFluxes[i][1] : 0.0);
                file << "," << (heatFluxes[i].size() > 2 ? heatFluxes[i][2] : 0.0);
            } else {
                file << ",0.0,0.0,0.0";
            }
            
            // Write thermal gradient (first node only for simplicity)
            if (!thermalGradients[i].empty()) {
                file << "," << thermalGradients[i][0];
                file << "," << (thermalGradients[i].size() > 1 ? thermalGradients[i][1] : 0.0);
                file << "," << (thermalGradients[i].size() > 2 ? thermalGradients[i][2] : 0.0);
            } else {
                file << ",0.0,0.0,0.0";
            }
            
            file << std::endl;
        }
        
        file.close();
        return true;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to export thermal results: " + std::string(e.what()));
        return false;
    }
}

} // namespace rebel::simulation
