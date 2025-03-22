#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace rebel::simulation {

// Initialize time integration
bool ThermalSolver::initializeTimeIntegration(
    std::shared_ptr<Mesh> mesh,
    const std::vector<double>& initialTemperatures,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Initializing time integration...");
    
    // Initialize temperature vector
    m_temperatures.resize(m_numDofs);
    
    // Use provided initial temperatures if available
    if (!initialTemperatures.empty()) {
        if (initialTemperatures.size() != m_numDofs) {
            REBEL_LOG_ERROR("Initial temperatures size does not match number of DOFs");
            return false;
        }
        
        for (int i = 0; i < m_numDofs; ++i) {
            m_temperatures[i] = initialTemperatures[i];
        }
    } else {
        // Use ambient temperature as initial condition
        double ambientTemperature = 20.0; // 20°C
        for (int i = 0; i < m_numDofs; ++i) {
            m_temperatures[i] = ambientTemperature;
        }
    }
    
    // Initialize heat flux and thermal gradient vectors
    m_heatFluxes.resize(m_numDofs * 3);
    m_thermalGradients.resize(m_numDofs * 3);
    
    return true;
}

// Perform a time step for transient analysis
bool ThermalSolver::timeStep(
    double time,
    double dt,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Performing time step at time " + std::to_string(time) + " with dt = " + std::to_string(dt));
    
    // We'll use the implicit Euler method for time integration
    // (C/dt + K) * T_{n+1} = C/dt * T_n + F_{n+1}
    
    // Create the effective stiffness matrix: (C/dt + K)
    Eigen::SparseMatrix<double> A = m_capacityMatrix / dt + m_conductivityMatrix;
    
    // Create the effective load vector: C/dt * T_n + F_{n+1}
    Eigen::VectorXd b = m_capacityMatrix * Eigen::Map<const Eigen::VectorXd>(m_temperatures.data(), m_temperatures.size()) / dt + m_heatLoadVector;
    
    // Solve the system
    Eigen::VectorXd x(m_numDofs);
    if (!solveLinearSystem(A, b, x)) {
        REBEL_LOG_ERROR("Failed to solve linear system for time step");
        return false;
    }
    
    // Update temperatures
    for (int i = 0; i < m_numDofs; ++i) {
        m_temperatures[i] = x(i);
    }
    
    // Update progress
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Time step completed successfully");
    return true;
}

// Compute critical time step for transient analysis
double ThermalSolver::computeCriticalTimeStep(std::shared_ptr<Mesh> mesh) {
    REBEL_LOG_INFO("Computing critical time step...");
    
    // For thermal analysis, the critical time step is related to the thermal diffusivity
    // dt_crit = min(dx^2 / (2 * alpha))
    // where alpha = k / (rho * c) is the thermal diffusivity
    // k is thermal conductivity, rho is density, c is specific heat
    // dx is the characteristic element size
    
    double minTimeStep = std::numeric_limits<double>::max();
    
    // Loop over all elements
    for (int elemIdx = 0; elemIdx < mesh->getElementCount(); ++elemIdx) {
        const Element& element = mesh->getElement(elemIdx);
        
        // Get element material
        const Material& material = mesh->getMaterial(element.materialId);
        
        // Get thermal properties
        double k = material.getThermalConductivity();
        double rho = material.getDensity();
        double c = material.getSpecificHeat();
        
        // Compute thermal diffusivity
        double alpha = k / (rho * c);
        
        // Compute characteristic element size
        double dx = computeCharacteristicElementSize(element, mesh);
        
        // Compute critical time step for this element
        double dt_crit = dx * dx / (2.0 * alpha);
        
        // Update minimum time step
        minTimeStep = std::min(minTimeStep, dt_crit);
    }
    
    // Apply safety factor
    minTimeStep *= m_thermalSettings.timeStepSafetyFactor;
    
    REBEL_LOG_INFO("Critical time step: " + std::to_string(minTimeStep));
    return minTimeStep;
}

// Helper function to compute characteristic element size
double ThermalSolver::computeCharacteristicElementSize(const Element& element, std::shared_ptr<Mesh> mesh) {
    // Get element nodes
    std::vector<Node> nodes;
    for (int nodeIdx : element.nodeIds) {
        nodes.push_back(mesh->getNode(nodeIdx));
    }
    
    // Compute minimum edge length
    double minEdgeLength = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = i + 1; j < nodes.size(); ++j) {
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double dz = nodes[i].z - nodes[j].z;
            double edgeLength = std::sqrt(dx*dx + dy*dy + dz*dz);
            minEdgeLength = std::min(minEdgeLength, edgeLength);
        }
    }
    
    return minEdgeLength;
}

} // namespace rebel::simulation
