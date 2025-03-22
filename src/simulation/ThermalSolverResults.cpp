#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "core/Log.h"
#include <algorithm>
#include <cmath>

namespace rebel::simulation {

// Compute element heat fluxes
std::vector<double> ThermalSolver::computeElementHeatFluxes(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes,
    const std::vector<double>& temperatures) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize heat flux vector (3 components per node)
    std::vector<double> heatFluxes(numNodes * 3, 0.0);
    
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
            REBEL_LOG_ERROR("Unsupported element type for heat flux computation");
            return heatFluxes;
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
            return heatFluxes;
        }
        
        // Compute B matrix
        Eigen::MatrixXd B = computeThermalBMatrix(dN, invJ);
        
        // Create temperature vector for element
        Eigen::VectorXd T(numNodes);
        for (int j = 0; j < numNodes; ++j) {
            T(j) = temperatures[element.nodeIds[j]];
        }
        
        // Compute heat flux at Gauss point
        Eigen::Vector3d q = -D * B * T;
        
        // Extrapolate heat flux to nodes using shape functions
        for (int j = 0; j < numNodes; ++j) {
            heatFluxes[j*3 + 0] += N(j) * q(0) * gaussWeights[i];
            heatFluxes[j*3 + 1] += N(j) * q(1) * gaussWeights[i];
            heatFluxes[j*3 + 2] += N(j) * q(2) * gaussWeights[i];
        }
    }
    
    // Normalize heat fluxes
    double totalWeight = 0.0;
    for (int i = 0; i < numGaussPoints; ++i) {
        totalWeight += gaussWeights[i];
    }
    
    for (int i = 0; i < numNodes * 3; ++i) {
        heatFluxes[i] /= totalWeight;
    }
    
    return heatFluxes;
}

// Compute element thermal gradients
std::vector<double> ThermalSolver::computeElementThermalGradients(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes,
    const std::vector<double>& temperatures) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize thermal gradient vector (3 components per node)
    std::vector<double> thermalGradients(numNodes * 3, 0.0);
    
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
            REBEL_LOG_ERROR("Unsupported element type for thermal gradient computation");
            return thermalGradients;
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
            return thermalGradients;
        }
        
        // Compute B matrix
        Eigen::MatrixXd B = computeThermalBMatrix(dN, invJ);
        
        // Create temperature vector for element
        Eigen::VectorXd T(numNodes);
        for (int j = 0; j < numNodes; ++j) {
            T(j) = temperatures[element.nodeIds[j]];
        }
        
        // Compute thermal gradient at Gauss point
        Eigen::Vector3d grad = B * T;
        
        // Extrapolate thermal gradient to nodes using shape functions
        for (int j = 0; j < numNodes; ++j) {
            thermalGradients[j*3 + 0] += N(j) * grad(0) * gaussWeights[i];
            thermalGradients[j*3 + 1] += N(j) * grad(1) * gaussWeights[i];
            thermalGradients[j*3 + 2] += N(j) * grad(2) * gaussWeights[i];
        }
    }
    
    // Normalize thermal gradients
    double totalWeight = 0.0;
    for (int i = 0; i < numGaussPoints; ++i) {
        totalWeight += gaussWeights[i];
    }
    
    for (int i = 0; i < numNodes * 3; ++i) {
        thermalGradients[i] /= totalWeight;
    }
    
    return thermalGradients;
}

} // namespace rebel::simulation
