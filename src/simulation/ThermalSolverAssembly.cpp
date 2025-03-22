#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "simulation/Load.h"
#include "simulation/ThermalLoads.h"
#include "core/Log.h"
#include <algorithm>

namespace rebel::simulation {

// Assemble conductivity matrix
bool ThermalSolver::assembleConductivityMatrix(
    std::shared_ptr<Mesh> mesh,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Assembling conductivity matrix...");
    
    // Initialize conductivity matrix
    m_conductivityMatrix.resize(m_numDofs, m_numDofs);
    m_conductivityMatrix.setZero();
    
    // Get the number of elements
    int numElements = mesh->getElementCount();
    if (numElements == 0) {
        REBEL_LOG_ERROR("Mesh has no elements");
        return false;
    }
    
    // Create triplet list for sparse matrix assembly
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(numElements * 64); // Estimate: 8 nodes per element, 8x8 matrix
    
    // Loop over all elements
    for (int elemIdx = 0; elemIdx < numElements; ++elemIdx) {
        // Update progress
        if (progressCallback && (elemIdx % 100 == 0)) {
            float progress = static_cast<float>(elemIdx) / static_cast<float>(numElements);
            progressCallback(progress);
        }
        
        // Get element
        const Element& element = mesh->getElement(elemIdx);
        
        // Get element nodes
        std::vector<Node> nodes;
        for (int nodeIdx : element.nodeIds) {
            nodes.push_back(mesh->getNode(nodeIdx));
        }
        
        // Get element material
        const Material& material = mesh->getMaterial(element.materialId);
        
        // Compute element conductivity matrix
        Eigen::MatrixXd Ke = computeElementConductivityMatrix(element, material, nodes);
        
        // Add element conductivity matrix to global conductivity matrix
        for (size_t i = 0; i < element.nodeIds.size(); ++i) {
            int rowIdx = element.nodeIds[i];
            for (size_t j = 0; j < element.nodeIds.size(); ++j) {
                int colIdx = element.nodeIds[j];
                triplets.push_back(Eigen::Triplet<double>(rowIdx, colIdx, Ke(i, j)));
            }
        }
    }
    
    // Set matrix from triplets
    m_conductivityMatrix.setFromTriplets(triplets.begin(), triplets.end());
    
    // Final progress update
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Conductivity matrix assembled successfully");
    return true;
}

// Assemble capacity matrix
bool ThermalSolver::assembleCapacityMatrix(
    std::shared_ptr<Mesh> mesh,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Assembling capacity matrix...");
    
    // Initialize capacity matrix
    m_capacityMatrix.resize(m_numDofs, m_numDofs);
    m_capacityMatrix.setZero();
    
    // Get the number of elements
    int numElements = mesh->getElementCount();
    if (numElements == 0) {
        REBEL_LOG_ERROR("Mesh has no elements");
        return false;
    }
    
    // Create triplet list for sparse matrix assembly
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(numElements * 64); // Estimate: 8 nodes per element, 8x8 matrix
    
    // Loop over all elements
    for (int elemIdx = 0; elemIdx < numElements; ++elemIdx) {
        // Update progress
        if (progressCallback && (elemIdx % 100 == 0)) {
            float progress = static_cast<float>(elemIdx) / static_cast<float>(numElements);
            progressCallback(progress);
        }
        
        // Get element
        const Element& element = mesh->getElement(elemIdx);
        
        // Get element nodes
        std::vector<Node> nodes;
        for (int nodeIdx : element.nodeIds) {
            nodes.push_back(mesh->getNode(nodeIdx));
        }
        
        // Get element material
        const Material& material = mesh->getMaterial(element.materialId);
        
        // Compute element capacity matrix
        Eigen::MatrixXd Ce = computeElementCapacityMatrix(element, material, nodes);
        
        // Add element capacity matrix to global capacity matrix
        for (size_t i = 0; i < element.nodeIds.size(); ++i) {
            int rowIdx = element.nodeIds[i];
            for (size_t j = 0; j < element.nodeIds.size(); ++j) {
                int colIdx = element.nodeIds[j];
                triplets.push_back(Eigen::Triplet<double>(rowIdx, colIdx, Ce(i, j)));
            }
        }
    }
    
    // Set matrix from triplets
    m_capacityMatrix.setFromTriplets(triplets.begin(), triplets.end());
    
    // Final progress update
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Capacity matrix assembled successfully");
    return true;
}

// Assemble heat load vector
bool ThermalSolver::assembleHeatLoadVector(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<Load>>& loads,
    double time,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Assembling heat load vector at time " + std::to_string(time) + "...");
    
    // Initialize heat load vector
    m_heatLoadVector.resize(m_numDofs);
    m_heatLoadVector.setZero();
    
    // Filter thermal loads
    std::vector<std::shared_ptr<Load>> thermalLoads;
    for (const auto& load : loads) {
        if (load->getType() == LoadType::Thermal || 
            load->getType() == LoadType::HeatGeneration) {
            thermalLoads.push_back(load);
        }
    }
    
    if (thermalLoads.empty()) {
        REBEL_LOG_WARNING("No thermal loads found");
        return true;
    }
    
    // Process each thermal load
    int loadCount = 0;
    for (const auto& load : thermalLoads) {
        // Update progress
        if (progressCallback) {
            float progress = static_cast<float>(loadCount) / static_cast<float>(thermalLoads.size());
            progressCallback(progress);
        }
        
        if (load->getType() == LoadType::Thermal) {
            // Handle thermal load (temperature change)
            const auto& thermalLoad = std::dynamic_pointer_cast<ThermalLoad>(load);
            if (!thermalLoad) {
                REBEL_LOG_ERROR("Failed to cast to ThermalLoad");
                continue;
            }
            
            // Get node group
            const NodeGroup* group = mesh->getNodeGroupByName(thermalLoad->getGroupName());
            if (!group) {
                REBEL_LOG_ERROR("Node group '" + thermalLoad->getGroupName() + "' not found");
                continue;
            }
            
            // Apply thermal load to each node in the group
            double temperature = thermalLoad->getTemperature();
            double refTemperature = thermalLoad->getReferenceTemperature();
            double deltaT = temperature - refTemperature;
            
            for (int nodeId : group->nodeIds) {
                // For thermal loads, we add to the heat load vector
                // The actual implementation depends on the element formulation
                // For now, we'll use a simplified approach
                m_heatLoadVector(nodeId) += deltaT;
            }
        }
        else if (load->getType() == LoadType::HeatGeneration) {
            // Handle heat generation load
            const auto& heatGenLoad = std::dynamic_pointer_cast<HeatGenerationLoad>(load);
            if (!heatGenLoad) {
                REBEL_LOG_ERROR("Failed to cast to HeatGenerationLoad");
                continue;
            }
            
            // Get element group
            const ElementGroup* group = mesh->getElementGroupByName(heatGenLoad->getGroupName());
            if (!group) {
                REBEL_LOG_ERROR("Element group '" + heatGenLoad->getGroupName() + "' not found");
                continue;
            }
            
            // Apply heat generation load to each element in the group
            for (int elemId : group->elementIds) {
                const Element& element = mesh->getElement(elemId);
                
                // Get element nodes
                std::vector<Node> nodes;
                for (int nodeIdx : element.nodeIds) {
                    nodes.push_back(mesh->getNode(nodeIdx));
                }
                
                // Compute element heat load vector
                Eigen::VectorXd Fe = computeElementHeatLoadVector(element, *heatGenLoad, nodes, time);
                
                // Add element heat load vector to global heat load vector
                for (size_t i = 0; i < element.nodeIds.size(); ++i) {
                    int nodeId = element.nodeIds[i];
                    m_heatLoadVector(nodeId) += Fe(i);
                }
            }
        }
        
        loadCount++;
    }
    
    // Final progress update
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Heat load vector assembled successfully");
    return true;
}

} // namespace rebel::simulation
