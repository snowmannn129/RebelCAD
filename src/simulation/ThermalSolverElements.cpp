#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "simulation/Load.h"
#include "simulation/ThermalLoads.h"
#include "core/Log.h"
#include <algorithm>
#include <cmath>

namespace rebel::simulation {

// Compute element conductivity matrix
Eigen::MatrixXd ThermalSolver::computeElementConductivityMatrix(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize element conductivity matrix
    Eigen::MatrixXd Ke = Eigen::MatrixXd::Zero(numNodes, numNodes);
    
    // Get thermal conductivity
    double k = material.getThermalConductivity();
    
    // For simplicity, we'll assume isotropic thermal conductivity
    Eigen::MatrixXd D = computeThermalDMatrix(material);
    
    // Gauss quadrature points and weights
    std::vector<double> gaussPoints;
    std::vector<double> gaussWeights;
    
    // Set up Gauss quadrature based on element type
    switch (element.type) {
        case ElementType::Triangle:
            // 3-point quadrature for triangles
            gaussPoints = {1.0/6.0, 2.0/3.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 2.0/3.0};
            gaussWeights = {1.0/6.0, 1.0/6.0, 1.0/6.0};
            break;
        case ElementType::Quad:
            // 2x2 quadrature for quads
            gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
            gaussWeights = {1.0, 1.0, 1.0, 1.0};
            break;
        case ElementType::Tetra:
            // 4-point quadrature for tetrahedra
            gaussPoints = {1.0/4.0, 1.0/4.0, 1.0/4.0,
                           1.0/2.0, 1.0/6.0, 1.0/6.0,
                           1.0/6.0, 1.0/2.0, 1.0/6.0,
                           1.0/6.0, 1.0/6.0, 1.0/2.0};
            gaussWeights = {1.0/24.0, 1.0/24.0, 1.0/24.0, 1.0/24.0};
            break;
        case ElementType::Hexa:
            // 2x2x2 quadrature for hexahedra
            gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
            gaussWeights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
            break;
        default:
            REBEL_LOG_ERROR("Unsupported element type for conductivity matrix computation");
            return Ke;
    }
    
    // Number of Gauss points
    int numGaussPoints = static_cast<int>(gaussWeights.size());
    
    // Loop over Gauss points
    for (int i = 0; i < numGaussPoints; ++i) {
        // Get Gauss point coordinates
        double xi, eta, zeta;
        if (element.type == ElementType::Triangle) {
            xi = gaussPoints[i*2];
            eta = gaussPoints[i*2+1];
            zeta = 0.0;
        } else if (element.type == ElementType::Quad) {
            xi = gaussPoints[i*2];
            eta = gaussPoints[i*2+1];
            zeta = 0.0;
        } else {
            xi = gaussPoints[i*3];
            eta = gaussPoints[i*3+1];
            zeta = gaussPoints[i*3+2];
        }
        
        // Compute shape functions and derivatives
        Eigen::VectorXd N;
        Eigen::MatrixXd dN;
        Eigen::Matrix3d J, invJ;
        double detJ;
        
        if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
            REBEL_LOG_ERROR("Failed to compute shape functions");
            return Eigen::MatrixXd();
        }
        
        // Compute B matrix
        Eigen::MatrixXd B = computeThermalBMatrix(dN, invJ);
        
        // Compute contribution to element conductivity matrix
        Ke += B.transpose() * D * B * detJ * gaussWeights[i];
    }
    
    return Ke;
}

// Compute element capacity matrix
Eigen::MatrixXd ThermalSolver::computeElementCapacityMatrix(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize element capacity matrix
    Eigen::MatrixXd Ce = Eigen::MatrixXd::Zero(numNodes, numNodes);
    
    // Get material properties
    double rho = material.getDensity();
    double c = material.getSpecificHeat();
    
    // Gauss quadrature points and weights
    std::vector<double> gaussPoints;
    std::vector<double> gaussWeights;
    
    // Set up Gauss quadrature based on element type
    switch (element.type) {
        case ElementType::Triangle:
            // 3-point quadrature for triangles
            gaussPoints = {1.0/6.0, 2.0/3.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 2.0/3.0};
            gaussWeights = {1.0/6.0, 1.0/6.0, 1.0/6.0};
            break;
        case ElementType::Quad:
            // 2x2 quadrature for quads
            gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
            gaussWeights = {1.0, 1.0, 1.0, 1.0};
            break;
        case ElementType::Tetra:
            // 4-point quadrature for tetrahedra
            gaussPoints = {1.0/4.0, 1.0/4.0, 1.0/4.0,
                           1.0/2.0, 1.0/6.0, 1.0/6.0,
                           1.0/6.0, 1.0/2.0, 1.0/6.0,
                           1.0/6.0, 1.0/6.0, 1.0/2.0};
            gaussWeights = {1.0/24.0, 1.0/24.0, 1.0/24.0, 1.0/24.0};
            break;
        case ElementType::Hexa:
            // 2x2x2 quadrature for hexahedra
            gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                           1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
            gaussWeights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
            break;
        default:
            REBEL_LOG_ERROR("Unsupported element type for capacity matrix computation");
            return Ce;
    }
    
    // Number of Gauss points
    int numGaussPoints = static_cast<int>(gaussWeights.size());
    
    // Loop over Gauss points
    for (int i = 0; i < numGaussPoints; ++i) {
        // Get Gauss point coordinates
        double xi, eta, zeta;
        if (element.type == ElementType::Triangle) {
            xi = gaussPoints[i*2];
            eta = gaussPoints[i*2+1];
            zeta = 0.0;
        } else if (element.type == ElementType::Quad) {
            xi = gaussPoints[i*2];
            eta = gaussPoints[i*2+1];
            zeta = 0.0;
        } else {
            xi = gaussPoints[i*3];
            eta = gaussPoints[i*3+1];
            zeta = gaussPoints[i*3+2];
        }
        
        // Compute shape functions and derivatives
        Eigen::VectorXd N;
        Eigen::MatrixXd dN;
        Eigen::Matrix3d J, invJ;
        double detJ;
        
        if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
            REBEL_LOG_ERROR("Failed to compute shape functions");
            return Eigen::MatrixXd();
        }
        
        // Compute contribution to element capacity matrix
        Ce += N * N.transpose() * rho * c * detJ * gaussWeights[i];
    }
    
    return Ce;
}

// Compute element heat load vector
Eigen::VectorXd ThermalSolver::computeElementHeatLoadVector(
    const Element& element,
    const Load& load,
    const std::vector<Node>& nodes,
    double time) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize element heat load vector
    Eigen::VectorXd Fe = Eigen::VectorXd::Zero(numNodes);
    
    // Handle different load types
    if (load.getType() == LoadType::HeatGeneration) {
        // Cast to heat generation load
        const HeatGenerationLoad& heatGenLoad = dynamic_cast<const HeatGenerationLoad&>(load);
        
        // Get heat generation rate
        double q = heatGenLoad.getHeatGenerationRate();
        
        // Apply load variation
        q = computeHeatLoadValue(load, time);
        
        // Gauss quadrature points and weights
        std::vector<double> gaussPoints;
        std::vector<double> gaussWeights;
        
        // Set up Gauss quadrature based on element type
        switch (element.type) {
            case ElementType::Triangle:
                // 3-point quadrature for triangles
                gaussPoints = {1.0/6.0, 2.0/3.0, 1.0/6.0, 1.0/6.0, 1.0/6.0, 2.0/3.0};
                gaussWeights = {1.0/6.0, 1.0/6.0, 1.0/6.0};
                break;
            case ElementType::Quad:
                // 2x2 quadrature for quads
                gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 
                               1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                               -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                               1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
                gaussWeights = {1.0, 1.0, 1.0, 1.0};
                break;
            case ElementType::Tetra:
                // 4-point quadrature for tetrahedra
                gaussPoints = {1.0/4.0, 1.0/4.0, 1.0/4.0,
                               1.0/2.0, 1.0/6.0, 1.0/6.0,
                               1.0/6.0, 1.0/2.0, 1.0/6.0,
                               1.0/6.0, 1.0/6.0, 1.0/2.0};
                gaussWeights = {1.0/24.0, 1.0/24.0, 1.0/24.0, 1.0/24.0};
                break;
            case ElementType::Hexa:
                // 2x2x2 quadrature for hexahedra
                gaussPoints = {-1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                               1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                               -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                               1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0),
                               -1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                               1.0/std::sqrt(3.0), -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                               -1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0),
                               1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0), 1.0/std::sqrt(3.0)};
                gaussWeights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
                break;
            default:
                REBEL_LOG_ERROR("Unsupported element type for heat load vector computation");
                return Fe;
        }
        
        // Number of Gauss points
        int numGaussPoints = static_cast<int>(gaussWeights.size());
        
        // Loop over Gauss points
        for (int i = 0; i < numGaussPoints; ++i) {
            // Get Gauss point coordinates
            double xi, eta, zeta;
            if (element.type == ElementType::Triangle) {
                xi = gaussPoints[i*2];
                eta = gaussPoints[i*2+1];
                zeta = 0.0;
            } else if (element.type == ElementType::Quad) {
                xi = gaussPoints[i*2];
                eta = gaussPoints[i*2+1];
                zeta = 0.0;
            } else {
                xi = gaussPoints[i*3];
                eta = gaussPoints[i*3+1];
                zeta = gaussPoints[i*3+2];
            }
            
            // Compute shape functions and derivatives
            Eigen::VectorXd N;
            Eigen::MatrixXd dN;
            Eigen::Matrix3d J, invJ;
            double detJ;
            
            if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
                REBEL_LOG_ERROR("Failed to compute shape functions");
                return Eigen::VectorXd();
            }
            
            // Compute contribution to element heat load vector
            Fe += N * q * detJ * gaussWeights[i];
        }
    }
    
    return Fe;
}

// Compute heat load value
double ThermalSolver::computeHeatLoadValue(const Load& load, double time) {
    double value = 0.0;
    
    if (load.getType() == LoadType::HeatGeneration) {
        // Cast to heat generation load
        const HeatGenerationLoad& heatGenLoad = dynamic_cast<const HeatGenerationLoad&>(load);
        
        // Get base heat generation rate
        value = heatGenLoad.getHeatGenerationRate();
        
        // Apply load variation
        switch (load.getVariation()) {
            case LoadVariation::Static:
                // No time variation
                break;
            case LoadVariation::Transient:
                // Linear ramp from 0 to 1 over 1 second
                value *= std::min(time, 1.0);
                break;
            case LoadVariation::Harmonic:
                // Sinusoidal variation with 1 Hz frequency
                value *= std::sin(2.0 * M_PI * time);
                break;
            case LoadVariation::Random:
                // Not implemented for deterministic analysis
                REBEL_LOG_WARNING("Random load variation not implemented for deterministic analysis");
                break;
            default:
                REBEL_LOG_ERROR("Unknown load variation type");
                break;
        }
    }
    else if (load.getType() == LoadType::Thermal) {
        // Cast to thermal load
        const ThermalLoad& thermalLoad = dynamic_cast<const ThermalLoad&>(load);
        
        // Get base temperature
        value = thermalLoad.getTemperature() - thermalLoad.getReferenceTemperature();
        
        // Apply load variation
        switch (load.getVariation()) {
            case LoadVariation::Static:
                // No time variation
                break;
            case LoadVariation::Transient:
                // Linear ramp from 0 to 1 over 1 second
                value *= std::min(time, 1.0);
                break;
            case LoadVariation::Harmonic:
                // Sinusoidal variation with 1 Hz frequency
                value *= std::sin(2.0 * M_PI * time);
                break;
            case LoadVariation::Random:
                // Not implemented for deterministic analysis
                REBEL_LOG_WARNING("Random load variation not implemented for deterministic analysis");
                break;
            default:
                REBEL_LOG_ERROR("Unknown load variation type");
                break;
        }
    }
    
    return value;
}

} // namespace rebel::simulation
