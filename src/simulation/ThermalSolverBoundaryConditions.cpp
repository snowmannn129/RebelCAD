#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/ThermalBoundaryConditions.h"
#include "core/Log.h"
#include <algorithm>

namespace rebel::simulation {

// Apply boundary conditions
bool ThermalSolver::applyBoundaryConditions(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    double time,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Applying boundary conditions at time " + std::to_string(time) + "...");
    
    // Filter thermal boundary conditions
    std::vector<std::shared_ptr<BoundaryCondition>> thermalBCs;
    for (const auto& bc : boundaryConditions) {
        if (bc->getType() == BoundaryConditionType::Temperature || 
            bc->getType() == BoundaryConditionType::HeatFlux || 
            bc->getType() == BoundaryConditionType::Convection) {
            thermalBCs.push_back(bc);
        }
    }
    
    if (thermalBCs.empty()) {
        REBEL_LOG_WARNING("No thermal boundary conditions found");
        return true;
    }
    
    // Process each thermal boundary condition
    int bcCount = 0;
    for (const auto& bc : thermalBCs) {
        // Update progress
        if (progressCallback) {
            float progress = static_cast<float>(bcCount) / static_cast<float>(thermalBCs.size());
            progressCallback(progress);
        }
        
        if (bc->getType() == BoundaryConditionType::Temperature) {
            // Handle temperature boundary condition
            const auto& tempBC = std::dynamic_pointer_cast<TemperatureBC>(bc);
            if (!tempBC) {
                REBEL_LOG_ERROR("Failed to cast to TemperatureBC");
                continue;
            }
            
            // Get node group
            const NodeGroup* group = mesh->getNodeGroupByName(tempBC->getGroupName());
            if (!group) {
                REBEL_LOG_ERROR("Node group '" + tempBC->getGroupName() + "' not found");
                continue;
            }
            
            // Apply temperature boundary condition to each node in the group
            double temperature = tempBC->getTemperature();
            
            for (int nodeId : group->nodeIds) {
                // For temperature BCs, we modify the conductivity matrix and heat load vector
                // using the penalty method
                double penalty = 1.0e10; // Large penalty value
                
                // Modify diagonal of conductivity matrix
                m_conductivityMatrix.coeffRef(nodeId, nodeId) = penalty;
                
                // Modify heat load vector
                m_heatLoadVector(nodeId) = penalty * temperature;
            }
        }
        else if (bc->getType() == BoundaryConditionType::HeatFlux) {
            // Handle heat flux boundary condition
            const auto& heatFluxBC = std::dynamic_pointer_cast<HeatFluxBC>(bc);
            if (!heatFluxBC) {
                REBEL_LOG_ERROR("Failed to cast to HeatFluxBC");
                continue;
            }
            
            // Get element group
            const ElementGroup* group = mesh->getElementGroupByName(heatFluxBC->getGroupName());
            if (!group) {
                REBEL_LOG_ERROR("Element group '" + heatFluxBC->getGroupName() + "' not found");
                continue;
            }
            
            // Apply heat flux boundary condition to each element in the group
            double heatFlux = heatFluxBC->getHeatFlux();
            
            for (int elemId : group->elementIds) {
                const Element& element = mesh->getElement(elemId);
                
                // For heat flux BCs, we add to the heat load vector
                // The actual implementation depends on the element formulation
                // For now, we'll use a simplified approach
                double elementArea = 1.0; // Placeholder, should compute actual element area
                double heatLoad = heatFlux * elementArea / element.nodeIds.size();
                
                for (int nodeId : element.nodeIds) {
                    m_heatLoadVector(nodeId) += heatLoad;
                }
            }
        }
        else if (bc->getType() == BoundaryConditionType::Convection) {
            // Handle convection boundary condition
            const auto& convectionBC = std::dynamic_pointer_cast<ConvectionBC>(bc);
            if (!convectionBC) {
                REBEL_LOG_ERROR("Failed to cast to ConvectionBC");
                continue;
            }
            
            // Get element group
            const ElementGroup* group = mesh->getElementGroupByName(convectionBC->getGroupName());
            if (!group) {
                REBEL_LOG_ERROR("Element group '" + convectionBC->getGroupName() + "' not found");
                continue;
            }
            
            // Apply convection boundary condition to each element in the group
            double h = convectionBC->getFilmCoefficient();
            double Tinf = convectionBC->getAmbientTemperature();
            
            for (int elemId : group->elementIds) {
                const Element& element = mesh->getElement(elemId);
                
                // For convection BCs, we modify both the conductivity matrix and heat load vector
                // The actual implementation depends on the element formulation
                // For now, we'll use a simplified approach
                double elementArea = 1.0; // Placeholder, should compute actual element area
                double hA = h * elementArea;
                
                // Add h*A/n to diagonal of conductivity matrix for each node
                for (int nodeId : element.nodeIds) {
                    m_conductivityMatrix.coeffRef(nodeId, nodeId) += hA / element.nodeIds.size();
                }
                
                // Add h*A*Tinf/n to heat load vector for each node
                for (int nodeId : element.nodeIds) {
                    m_heatLoadVector(nodeId) += hA * Tinf / element.nodeIds.size();
                }
            }
        }
        
        bcCount++;
    }
    
    // Final progress update
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Boundary conditions applied successfully");
    return true;
}

} // namespace rebel::simulation
