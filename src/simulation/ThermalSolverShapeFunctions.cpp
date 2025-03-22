#include "simulation/ThermalSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "core/Log.h"
#include <algorithm>
#include <cmath>

namespace rebel::simulation {

// Compute shape functions
bool ThermalSolver::computeShapeFunctions(
    const Element& element,
    const std::vector<Node>& nodes,
    double xi, double eta, double zeta,
    Eigen::VectorXd& N,
    Eigen::MatrixXd& dN,
    Eigen::Matrix3d& J,
    double& detJ,
    Eigen::Matrix3d& invJ) {
    
    // Get number of nodes in element
    int numNodes = static_cast<int>(nodes.size());
    
    // Initialize shape function vector and derivative matrix
    N.resize(numNodes);
    dN.resize(numNodes, 3);
    
    // Compute shape functions and derivatives based on element type
    switch (element.type) {
        case ElementType::Triangle:
            if (numNodes == 3) {
                // Linear triangle
                N(0) = 1.0 - xi - eta;
                N(1) = xi;
                N(2) = eta;
                
                dN(0, 0) = -1.0; dN(0, 1) = -1.0; dN(0, 2) = 0.0;
                dN(1, 0) = 1.0;  dN(1, 1) = 0.0;  dN(1, 2) = 0.0;
                dN(2, 0) = 0.0;  dN(2, 1) = 1.0;  dN(2, 2) = 0.0;
            } else if (numNodes == 6) {
                // Quadratic triangle
                double zeta1 = 1.0 - xi - eta;
                
                N(0) = zeta1 * (2.0 * zeta1 - 1.0);
                N(1) = xi * (2.0 * xi - 1.0);
                N(2) = eta * (2.0 * eta - 1.0);
                N(3) = 4.0 * xi * zeta1;
                N(4) = 4.0 * xi * eta;
                N(5) = 4.0 * eta * zeta1;
                
                dN(0, 0) = -1.0 * (4.0 * zeta1 - 1.0); dN(0, 1) = -1.0 * (4.0 * zeta1 - 1.0); dN(0, 2) = 0.0;
                dN(1, 0) = 4.0 * xi - 1.0;             dN(1, 1) = 0.0;                         dN(1, 2) = 0.0;
                dN(2, 0) = 0.0;                         dN(2, 1) = 4.0 * eta - 1.0;             dN(2, 2) = 0.0;
                dN(3, 0) = 4.0 * (zeta1 - xi);          dN(3, 1) = -4.0 * xi;                   dN(3, 2) = 0.0;
                dN(4, 0) = 4.0 * eta;                   dN(4, 1) = 4.0 * xi;                    dN(4, 2) = 0.0;
                dN(5, 0) = -4.0 * eta;                  dN(5, 1) = 4.0 * (zeta1 - eta);         dN(5, 2) = 0.0;
            } else {
                REBEL_LOG_ERROR("Unsupported number of nodes for triangle element");
                return false;
            }
            break;
            
        case ElementType::Quad:
            if (numNodes == 4) {
                // Linear quad
                N(0) = 0.25 * (1.0 - xi) * (1.0 - eta);
                N(1) = 0.25 * (1.0 + xi) * (1.0 - eta);
                N(2) = 0.25 * (1.0 + xi) * (1.0 + eta);
                N(3) = 0.25 * (1.0 - xi) * (1.0 + eta);
                
                dN(0, 0) = -0.25 * (1.0 - eta); dN(0, 1) = -0.25 * (1.0 - xi); dN(0, 2) = 0.0;
                dN(1, 0) = 0.25 * (1.0 - eta);  dN(1, 1) = -0.25 * (1.0 + xi); dN(1, 2) = 0.0;
                dN(2, 0) = 0.25 * (1.0 + eta);  dN(2, 1) = 0.25 * (1.0 + xi);  dN(2, 2) = 0.0;
                dN(3, 0) = -0.25 * (1.0 + eta); dN(3, 1) = 0.25 * (1.0 - xi);  dN(3, 2) = 0.0;
            } else if (numNodes == 8) {
                // Quadratic quad (serendipity)
                N(0) = 0.25 * (1.0 - xi) * (1.0 - eta) * (-xi - eta - 1.0);
                N(1) = 0.25 * (1.0 + xi) * (1.0 - eta) * (xi - eta - 1.0);
                N(2) = 0.25 * (1.0 + xi) * (1.0 + eta) * (xi + eta - 1.0);
                N(3) = 0.25 * (1.0 - xi) * (1.0 + eta) * (-xi + eta - 1.0);
                N(4) = 0.5 * (1.0 - xi*xi) * (1.0 - eta);
                N(5) = 0.5 * (1.0 + xi) * (1.0 - eta*eta);
                N(6) = 0.5 * (1.0 - xi*xi) * (1.0 + eta);
                N(7) = 0.5 * (1.0 - xi) * (1.0 - eta*eta);
                
                // Derivatives are complex for quadratic quad, omitted for brevity
                // In a real implementation, these would be computed
                REBEL_LOG_ERROR("Derivatives for quadratic quad not implemented");
                return false;
            } else {
                REBEL_LOG_ERROR("Unsupported number of nodes for quad element");
                return false;
            }
            break;
            
        case ElementType::Tetra:
            if (numNodes == 4) {
                // Linear tetrahedron
                N(0) = 1.0 - xi - eta - zeta;
                N(1) = xi;
                N(2) = eta;
                N(3) = zeta;
                
                dN(0, 0) = -1.0; dN(0, 1) = -1.0; dN(0, 2) = -1.0;
                dN(1, 0) = 1.0;  dN(1, 1) = 0.0;  dN(1, 2) = 0.0;
                dN(2, 0) = 0.0;  dN(2, 1) = 1.0;  dN(2, 2) = 0.0;
                dN(3, 0) = 0.0;  dN(3, 1) = 0.0;  dN(3, 2) = 1.0;
            } else {
                REBEL_LOG_ERROR("Unsupported number of nodes for tetrahedron element");
                return false;
            }
            break;
            
        case ElementType::Hexa:
            if (numNodes == 8) {
                // Linear hexahedron
                N(0) = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 - zeta);
                N(1) = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 - zeta);
                N(2) = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 - zeta);
                N(3) = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 - zeta);
                N(4) = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 + zeta);
                N(5) = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 + zeta);
                N(6) = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 + zeta);
                N(7) = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 + zeta);
                
                dN(0, 0) = -0.125 * (1.0 - eta) * (1.0 - zeta); dN(0, 1) = -0.125 * (1.0 - xi) * (1.0 - zeta); dN(0, 2) = -0.125 * (1.0 - xi) * (1.0 - eta);
                dN(1, 0) = 0.125 * (1.0 - eta) * (1.0 - zeta);  dN(1, 1) = -0.125 * (1.0 + xi) * (1.0 - zeta); dN(1, 2) = -0.125 * (1.0 + xi) * (1.0 - eta);
                dN(2, 0) = 0.125 * (1.0 + eta) * (1.0 - zeta);  dN(2, 1) = 0.125 * (1.0 + xi) * (1.0 - zeta);  dN(2, 2) = -0.125 * (1.0 + xi) * (1.0 + eta);
                dN(3, 0) = -0.125 * (1.0 + eta) * (1.0 - zeta); dN(3, 1) = 0.125 * (1.0 - xi) * (1.0 - zeta);  dN(3, 2) = -0.125 * (1.0 - xi) * (1.0 + eta);
                dN(4, 0) = -0.125 * (1.0 - eta) * (1.0 + zeta); dN(4, 1) = -0.125 * (1.0 - xi) * (1.0 + zeta); dN(4, 2) = 0.125 * (1.0 - xi) * (1.0 - eta);
                dN(5, 0) = 0.125 * (1.0 - eta) * (1.0 + zeta);  dN(5, 1) = -0.125 * (1.0 + xi) * (1.0 + zeta); dN(5, 2) = 0.125 * (1.0 + xi) * (1.0 - eta);
                dN(6, 0) = 0.125 * (1.0 + eta) * (1.0 + zeta);  dN(6, 1) = 0.125 * (1.0 + xi) * (1.0 + zeta);  dN(6, 2) = 0.125 * (1.0 + xi) * (1.0 + eta);
                dN(7, 0) = -0.125 * (1.0 + eta) * (1.0 + zeta); dN(7, 1) = 0.125 * (1.0 - xi) * (1.0 + zeta);  dN(7, 2) = 0.125 * (1.0 - xi) * (1.0 + eta);
            } else {
                REBEL_LOG_ERROR("Unsupported number of nodes for hexahedron element");
                return false;
            }
            break;
            
        default:
            REBEL_LOG_ERROR("Unsupported element type for shape function computation");
            return false;
    }
    
    // Compute Jacobian matrix
    J.setZero();
    for (int i = 0; i < numNodes; ++i) {
        J(0, 0) += dN(i, 0) * nodes[i].x;
        J(0, 1) += dN(i, 1) * nodes[i].x;
        J(0, 2) += dN(i, 2) * nodes[i].x;
        J(1, 0) += dN(i, 0) * nodes[i].y;
        J(1, 1) += dN(i, 1) * nodes[i].y;
        J(1, 2) += dN(i, 2) * nodes[i].y;
        J(2, 0) += dN(i, 0) * nodes[i].z;
        J(2, 1) += dN(i, 1) * nodes[i].z;
        J(2, 2) += dN(i, 2) * nodes[i].z;
    }
    
    // Compute determinant of Jacobian
    detJ = J.determinant();
    if (std::abs(detJ) < 1e-10) {
        REBEL_LOG_ERROR("Jacobian determinant is too small");
        return false;
    }
    
    // Compute inverse of Jacobian
    invJ = J.inverse();
    
    return true;
}

// Compute B matrix for thermal analysis
Eigen::MatrixXd ThermalSolver::computeThermalBMatrix(
    const Eigen::MatrixXd& dN,
    const Eigen::Matrix3d& invJ) {
    
    // Get number of nodes
    int numNodes = static_cast<int>(dN.rows());
    
    // Initialize B matrix (3 x numNodes)
    // For thermal analysis, B matrix maps nodal temperatures to temperature gradients
    Eigen::MatrixXd B(3, numNodes);
    
    // Compute B matrix
    for (int i = 0; i < numNodes; ++i) {
        // Transform shape function derivatives from natural to global coordinates
        double dNdx = invJ(0, 0) * dN(i, 0) + invJ(0, 1) * dN(i, 1) + invJ(0, 2) * dN(i, 2);
        double dNdy = invJ(1, 0) * dN(i, 0) + invJ(1, 1) * dN(i, 1) + invJ(1, 2) * dN(i, 2);
        double dNdz = invJ(2, 0) * dN(i, 0) + invJ(2, 1) * dN(i, 1) + invJ(2, 2) * dN(i, 2);
        
        // Store in B matrix
        B(0, i) = dNdx;
        B(1, i) = dNdy;
        B(2, i) = dNdz;
    }
    
    return B;
}

// Compute D matrix for thermal analysis
Eigen::MatrixXd ThermalSolver::computeThermalDMatrix(
    const Material& material) {
    
    // Get thermal conductivity
    double k = material.getThermalConductivity();
    
    // For isotropic material, D is a diagonal matrix with thermal conductivity
    Eigen::MatrixXd D = Eigen::MatrixXd::Zero(3, 3);
    D(0, 0) = k;
    D(1, 1) = k;
    D(2, 2) = k;
    
    return D;
}

} // namespace rebel::simulation
