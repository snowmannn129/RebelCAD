/**
 * @file LinearStaticSolver.cpp
 * @brief Implementation of the LinearStaticSolver class
 */

#include "simulation/LinearStaticSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "simulation/FEASystem.h"
#include "core/Log.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

namespace rebel::simulation {

LinearStaticSolver::LinearStaticSolver(const FEASettings& settings)
    : settings(settings), numDofs(0) {
    // Nothing to initialize
}

std::shared_ptr<FEAResult> LinearStaticSolver::solve(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    const std::vector<std::shared_ptr<Load>>& loads,
    std::function<void(float)> progressCallback) {
    
    if (!mesh) {
        REBEL_LOG_ERROR("Cannot solve with null mesh");
        return nullptr;
    }
    
    if (boundaryConditions.empty()) {
        REBEL_LOG_ERROR("No boundary conditions defined");
        return nullptr;
    }
    
    if (loads.empty()) {
        REBEL_LOG_ERROR("No loads defined");
        return nullptr;
    }
    
    REBEL_LOG_INFO("Starting linear static solver...");
    
    // Initialize DOF mapping
    numDofs = 0;
    nodeIdToDofIndex.clear();
    nodeIdToDofs.clear();
    
    // Assign DOFs to nodes
    const auto& nodes = mesh->getNodes();
    for (const auto& node : nodes) {
        nodeIdToDofIndex[node.id] = numDofs;
        std::vector<int> dofs = {numDofs, numDofs + 1, numDofs + 2}; // X, Y, Z DOFs
        nodeIdToDofs[node.id] = dofs;
        numDofs += 3;
    }
    
    REBEL_LOG_INFO("Number of DOFs: " + std::to_string(numDofs));
    
    // Initialize matrices and vectors
    stiffnessMatrix.resize(numDofs, numDofs);
    loadVector = Eigen::VectorXd::Zero(numDofs);
    displacementVector = Eigen::VectorXd::Zero(numDofs);
    
    // Assemble the stiffness matrix
    if (progressCallback) {
        progressCallback(0.1f);
    }
    
    if (!assembleStiffnessMatrix(mesh, [&](float progress) {
        if (progressCallback) {
            progressCallback(0.1f + progress * 0.3f);
        }
    })) {
        REBEL_LOG_ERROR("Failed to assemble stiffness matrix");
        return nullptr;
    }
    
    // Assemble the load vector
    if (progressCallback) {
        progressCallback(0.4f);
    }
    
    if (!assembleLoadVector(mesh, loads, [&](float progress) {
        if (progressCallback) {
            progressCallback(0.4f + progress * 0.1f);
        }
    })) {
        REBEL_LOG_ERROR("Failed to assemble load vector");
        return nullptr;
    }
    
    // Apply boundary conditions
    if (progressCallback) {
        progressCallback(0.5f);
    }
    
    if (!applyBoundaryConditions(mesh, boundaryConditions, [&](float progress) {
        if (progressCallback) {
            progressCallback(0.5f + progress * 0.1f);
        }
    })) {
        REBEL_LOG_ERROR("Failed to apply boundary conditions");
        return nullptr;
    }
    
    // Solve the system of equations
    if (progressCallback) {
        progressCallback(0.6f);
    }
    
    if (!solveSystem([&](float progress) {
        if (progressCallback) {
            progressCallback(0.6f + progress * 0.2f);
        }
    })) {
        REBEL_LOG_ERROR("Failed to solve system of equations");
        return nullptr;
    }
    
    // Compute the results
    if (progressCallback) {
        progressCallback(0.8f);
    }
    
    auto result = computeResults(mesh, [&](float progress) {
        if (progressCallback) {
            progressCallback(0.8f + progress * 0.2f);
        }
    });
    
    if (!result) {
        REBEL_LOG_ERROR("Failed to compute results");
        return nullptr;
    }
    
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Linear static solver completed successfully");
    return result;
}

bool LinearStaticSolver::assembleStiffnessMatrix(
    std::shared_ptr<Mesh> mesh,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Assembling stiffness matrix...");
    
    // Initialize triplet list for sparse matrix assembly
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(numDofs * 10); // Estimate number of non-zeros
    
    // Get elements and nodes
    const auto& elements = mesh->getElements();
    const auto& nodes = mesh->getNodes();
    
    // Process each element
    size_t numElements = elements.size();
    for (size_t i = 0; i < numElements; ++i) {
        const auto& element = elements[i];
        
        // Get the material for the element
        auto material = element.material;
        if (!material) {
            REBEL_LOG_ERROR("Element " + std::to_string(element.id) + " has no material");
            return false;
        }
        
        // Get the nodes for the element
        std::vector<Node> elementNodes;
        for (int nodeId : element.nodeIds) {
            const Node* node = mesh->getNode(nodeId);
            if (!node) {
                REBEL_LOG_ERROR("Node " + std::to_string(nodeId) + " not found");
                return false;
            }
            elementNodes.push_back(*node);
        }
        
        // Compute the element stiffness matrix
        Eigen::MatrixXd Ke = computeElementStiffnessMatrix(element, *material, elementNodes);
        
        // Get the DOFs for the element
        std::vector<int> dofs;
        for (int nodeId : element.nodeIds) {
            const auto& nodeDofs = nodeIdToDofs[nodeId];
            dofs.insert(dofs.end(), nodeDofs.begin(), nodeDofs.end());
        }
        
        // Add the element stiffness matrix to the global stiffness matrix
        for (size_t j = 0; j < dofs.size(); ++j) {
            for (size_t k = 0; k < dofs.size(); ++k) {
                triplets.push_back(Eigen::Triplet<double>(dofs[j], dofs[k], Ke(j, k)));
            }
        }
        
        // Report progress
        if (progressCallback && i % 100 == 0) {
            progressCallback(static_cast<float>(i) / numElements);
        }
    }
    
    // Build the sparse matrix
    stiffnessMatrix.setFromTriplets(triplets.begin(), triplets.end());
    
    // Report completion
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Stiffness matrix assembled with " + std::to_string(triplets.size()) + " non-zero entries");
    return true;
}

bool LinearStaticSolver::assembleLoadVector(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<Load>>& loads,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Assembling load vector...");
    
    // Initialize the load vector to zero
    loadVector.setZero();
    
    // Process each load
    size_t numLoads = loads.size();
    for (size_t i = 0; i < numLoads; ++i) {
        const auto& load = loads[i];
        
        // Check if the load is valid
        if (!load->isValid(*mesh)) {
            REBEL_LOG_ERROR("Load " + load->getId() + " is not valid for the mesh");
            return false;
        }
        
        // Apply the load based on its type
        switch (load->getType()) {
            case LoadType::PointForce: {
                // Cast to point force load
                const auto* pointForce = dynamic_cast<const PointForceLoad*>(load.get());
                if (!pointForce) {
                    REBEL_LOG_ERROR("Failed to cast load to PointForceLoad");
                    return false;
                }
                
                // Get the node group
                const auto* nodeGroup = mesh->getNodeGroupByName(pointForce->getGroupName());
                if (!nodeGroup) {
                    REBEL_LOG_ERROR("Node group " + pointForce->getGroupName() + " not found");
                    return false;
                }
                
                // Apply the force to each node in the group
                for (int nodeId : nodeGroup->nodeIds) {
                    const auto& dofs = nodeIdToDofs[nodeId];
                    loadVector[dofs[0]] += pointForce->getForceX();
                    loadVector[dofs[1]] += pointForce->getForceY();
                    loadVector[dofs[2]] += pointForce->getForceZ();
                }
                break;
            }
            case LoadType::SurfacePressure: {
                // Cast to surface pressure load
                const auto* surfacePressure = dynamic_cast<const SurfacePressureLoad*>(load.get());
                if (!surfacePressure) {
                    REBEL_LOG_ERROR("Failed to cast load to SurfacePressureLoad");
                    return false;
                }
                
                // Get the element group
                const auto* elementGroup = mesh->getElementGroupByName(surfacePressure->getGroupName());
                if (!elementGroup) {
                    REBEL_LOG_ERROR("Element group " + surfacePressure->getGroupName() + " not found");
                    return false;
                }
                
                // Apply the pressure to each element in the group
                for (int elementId : elementGroup->elementIds) {
                    const auto* element = mesh->getElement(elementId);
                    if (!element) {
                        REBEL_LOG_ERROR("Element " + std::to_string(elementId) + " not found");
                        return false;
                    }
                    
                    // Get the nodes for the element
                    std::vector<Node> elementNodes;
                    for (int nodeId : element->nodeIds) {
                        const Node* node = mesh->getNode(nodeId);
                        if (!node) {
                            REBEL_LOG_ERROR("Node " + std::to_string(nodeId) + " not found");
                            return false;
                        }
                        elementNodes.push_back(*node);
                    }
                    
                    // Compute the element load vector
                    Eigen::VectorXd Fe = computeElementLoadVector(*element, *surfacePressure, elementNodes);
                    
                    // Get the DOFs for the element
                    std::vector<int> dofs;
                    for (int nodeId : element->nodeIds) {
                        const auto& nodeDofs = nodeIdToDofs[nodeId];
                        dofs.insert(dofs.end(), nodeDofs.begin(), nodeDofs.end());
                    }
                    
                    // Add the element load vector to the global load vector
                    for (size_t j = 0; j < dofs.size(); ++j) {
                        loadVector[dofs[j]] += Fe[j];
                    }
                }
                break;
            }
            case LoadType::BodyForce: {
                // Cast to body force load
                const auto* bodyForce = dynamic_cast<const BodyForceLoad*>(load.get());
                if (!bodyForce) {
                    REBEL_LOG_ERROR("Failed to cast load to BodyForceLoad");
                    return false;
                }
                
                // Get the element group
                const auto* elementGroup = mesh->getElementGroupByName(bodyForce->getGroupName());
                if (!elementGroup) {
                    REBEL_LOG_ERROR("Element group " + bodyForce->getGroupName() + " not found");
                    return false;
                }
                
                // Apply the body force to each element in the group
                for (int elementId : elementGroup->elementIds) {
                    const auto* element = mesh->getElement(elementId);
                    if (!element) {
                        REBEL_LOG_ERROR("Element " + std::to_string(elementId) + " not found");
                        return false;
                    }
                    
                    // Get the nodes for the element
                    std::vector<Node> elementNodes;
                    for (int nodeId : element->nodeIds) {
                        const Node* node = mesh->getNode(nodeId);
                        if (!node) {
                            REBEL_LOG_ERROR("Node " + std::to_string(nodeId) + " not found");
                            return false;
                        }
                        elementNodes.push_back(*node);
                    }
                    
                    // Compute the element load vector
                    Eigen::VectorXd Fe = computeElementLoadVector(*element, *bodyForce, elementNodes);
                    
                    // Get the DOFs for the element
                    std::vector<int> dofs;
                    for (int nodeId : element->nodeIds) {
                        const auto& nodeDofs = nodeIdToDofs[nodeId];
                        dofs.insert(dofs.end(), nodeDofs.begin(), nodeDofs.end());
                    }
                    
                    // Add the element load vector to the global load vector
                    for (size_t j = 0; j < dofs.size(); ++j) {
                        loadVector[dofs[j]] += Fe[j];
                    }
                }
                break;
            }
            default:
                REBEL_LOG_WARNING("Load type not implemented: " + std::to_string(static_cast<int>(load->getType())));
                break;
        }
        
        // Report progress
        if (progressCallback) {
            progressCallback(static_cast<float>(i + 1) / numLoads);
        }
    }
    
    REBEL_LOG_INFO("Load vector assembled");
    return true;
}

bool LinearStaticSolver::applyBoundaryConditions(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Applying boundary conditions...");
    
    // Process each boundary condition
    size_t numBCs = boundaryConditions.size();
    for (size_t i = 0; i < numBCs; ++i) {
        const auto& bc = boundaryConditions[i];
        
        // Check if the boundary condition is valid
        if (!bc->isValid(*mesh)) {
            REBEL_LOG_ERROR("Boundary condition " + bc->getId() + " is not valid for the mesh");
            return false;
        }
        
        // Apply the boundary condition based on its type
        switch (bc->getType()) {
            case BoundaryConditionType::Displacement: {
                // Cast to displacement boundary condition
                const auto* displacementBC = dynamic_cast<const DisplacementBC*>(bc.get());
                if (!displacementBC) {
                    REBEL_LOG_ERROR("Failed to cast boundary condition to DisplacementBC");
                    return false;
                }
                
                // Get the node group
                const auto* nodeGroup = mesh->getNodeGroupByName(displacementBC->getGroupName());
                if (!nodeGroup) {
                    REBEL_LOG_ERROR("Node group " + displacementBC->getGroupName() + " not found");
                    return false;
                }
                
                // Apply the displacement to each node in the group
                for (int nodeId : nodeGroup->nodeIds) {
                    const auto& dofs = nodeIdToDofs[nodeId];
                    
                    // Apply the displacement based on the direction
                    switch (displacementBC->getDirection()) {
                        case DisplacementDirection::X:
                            applyDisplacementBC(dofs[0], displacementBC->getValue());
                            break;
                        case DisplacementDirection::Y:
                            applyDisplacementBC(dofs[1], displacementBC->getValue());
                            break;
                        case DisplacementDirection::Z:
                            applyDisplacementBC(dofs[2], displacementBC->getValue());
                            break;
                        case DisplacementDirection::XY:
                            applyDisplacementBC(dofs[0], displacementBC->getValue());
                            applyDisplacementBC(dofs[1], displacementBC->getValue());
                            break;
                        case DisplacementDirection::YZ:
                            applyDisplacementBC(dofs[1], displacementBC->getValue());
                            applyDisplacementBC(dofs[2], displacementBC->getValue());
                            break;
                        case DisplacementDirection::XZ:
                            applyDisplacementBC(dofs[0], displacementBC->getValue());
                            applyDisplacementBC(dofs[2], displacementBC->getValue());
                            break;
                        case DisplacementDirection::XYZ:
                            applyDisplacementBC(dofs[0], displacementBC->getValue());
                            applyDisplacementBC(dofs[1], displacementBC->getValue());
                            applyDisplacementBC(dofs[2], displacementBC->getValue());
                            break;
                    }
                }
                break;
            }
            default:
                REBEL_LOG_WARNING("Boundary condition type not implemented: " + std::to_string(static_cast<int>(bc->getType())));
                break;
        }
        
        // Report progress
        if (progressCallback) {
            progressCallback(static_cast<float>(i + 1) / numBCs);
        }
    }
    
    REBEL_LOG_INFO("Boundary conditions applied");
    return true;
}

void LinearStaticSolver::applyDisplacementBC(int dof, double value) {
    // Set the displacement value
    displacementVector[dof] = value;
    
    // Modify the stiffness matrix and load vector
    // (Penalty method)
    double penalty = 1.0e10;
    stiffnessMatrix.coeffRef(dof, dof) = penalty;
    loadVector[dof] = penalty * value;
    
    // Zero out the row and column (except diagonal)
    for (int k = 0; k < stiffnessMatrix.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(stiffnessMatrix, k); it; ++it) {
            if (it.row() == dof && it.col() != dof) {
                it.valueRef() = 0.0;
            }
            if (it.col() == dof && it.row() != dof) {
                it.valueRef() = 0.0;
            }
        }
    }
}

bool LinearStaticSolver::solveSystem(
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Solving system of equations...");
    
    // Report progress
    if (progressCallback) {
        progressCallback(0.1f);
    }
    
    try {
        // Create solver
        Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
        
        // Report progress
        if (progressCallback) {
            progressCallback(0.2f);
        }
        
        // Analyze pattern
        solver.analyzePattern(stiffnessMatrix);
        
        // Report progress
        if (progressCallback) {
            progressCallback(0.4f);
        }
        
        // Factorize
        solver.factorize(stiffnessMatrix);
        
        // Report progress
        if (progressCallback) {
            progressCallback(0.7f);
        }
        
        // Check if factorization was successful
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Factorization failed");
            return false;
        }
        
        // Solve
        displacementVector = solver.solve(loadVector);
        
        // Check if solve was successful
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Solve failed");
            return false;
        }
        
        // Report progress
        if (progressCallback) {
            progressCallback(1.0f);
        }
        
        REBEL_LOG_INFO("System solved successfully");
        return true;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while solving system: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<FEAResult> LinearStaticSolver::computeResults(
    std::shared_ptr<Mesh> mesh,
    std::function<void(float)> progressCallback) {
    
    REBEL_LOG_INFO("Computing results...");
    
    // Create result object
    auto result = FEAResult::create(mesh, SolverType::Linear);
    
    // Get elements and nodes
    const auto& elements = mesh->getElements();
    const auto& nodes = mesh->getNodes();
    
    // Compute displacements
    std::vector<DisplacementResult> displacements;
    for (const auto& node : nodes) {
        const auto& dofs = nodeIdToDofs[node.id];
        double dx = displacementVector[dofs[0]];
        double dy = displacementVector[dofs[1]];
        double dz = displacementVector[dofs[2]];
        displacements.emplace_back(node.id, dx, dy, dz);
        result->addDisplacement(displacements.back());
    }
    
    // Report progress
    if (progressCallback) {
        progressCallback(0.3f);
    }
    
    // Compute stresses and strains
    size_t numElements = elements.size();
    for (size_t i = 0; i < numElements; ++i) {
        const auto& element = elements[i];
        
        // Get the material for the element
        auto material = element.material;
        if (!material) {
            REBEL_LOG_ERROR("Element " + std::to_string(element.id) + " has no material");
            return nullptr;
        }
        
        // Get the nodes for the element
        std::vector<Node> elementNodes;
        for (int nodeId : element.nodeIds) {
            const Node* node = mesh->getNode(nodeId);
            if (!node) {
                REBEL_LOG_ERROR("Node " + std::to_string(nodeId) + " not found");
                return nullptr;
            }
            elementNodes.push_back(*node);
        }
        
        // Compute stresses
        StressResult stress = computeElementStresses(element, *material, elementNodes, displacements);
        result->addStress(stress);
        
        // Compute strains
        StrainResult strain = computeElementStrains(element, *material, elementNodes, displacements);
        result->addStrain(strain);
        
        // Report progress
        if (progressCallback && i % 100 == 0) {
            progressCallback(0.3f + 0.6f * static_cast<float>(i) / numElements);
        }
    }
    
    // Compute reaction forces
    for (const auto& node : nodes) {
        // Check if the node has prescribed displacements
        bool hasBC = false;
        for (int dof : nodeIdToDofs[node.id]) {
            if (std::abs(stiffnessMatrix.coeff(dof, dof)) > 1.0e9) {
                hasBC = true;
                break;
            }
        }
        
        // Compute reaction forces for nodes with boundary conditions
        if (hasBC) {
            ReactionForceResult reactionForce = computeNodeReactionForce(node.id, mesh, displacements);
            result->addReactionForce(reactionForce);
        }
    }
    
    // Report progress
    if (progressCallback) {
        progressCallback(1.0f);
    }
    
    REBEL_LOG_INFO("Results computed successfully");
    return result;
}

Eigen::MatrixXd LinearStaticSolver::computeElementStiffnessMatrix(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes) {
    
    // Number of nodes in the element
    size_t numNodes = nodes.size();
    
    // Number of DOFs per node (3 for 3D)
    size_t dofsPerNode = 3;
    
    // Total number of DOFs for the element
    size_t numDofs = numNodes * dofsPerNode;
    
    // Initialize element stiffness matrix
    Eigen::MatrixXd Ke = Eigen::MatrixXd::Zero(numDofs, numDofs);
    
    // Get material properties
    double E = material.getYoungsModulus();
    double nu = material.getPoissonsRatio();
    
    // Compute D matrix (material property matrix)
    Eigen::MatrixXd D = computeDMatrix(material);
    
    // Gauss quadrature points and weights
    std::vector<double> gaussPoints;
    std::vector<double> gaussWeights;
    
    // Set up Gauss quadrature based on element type
    switch (element.type) {
        case ElementType::Tetra:
            // For tetrahedral elements, use 1-point quadrature
            gaussPoints = {0.25};
            gaussWeights = {1.0 / 6.0};
            break;
        case ElementType::Hexa:
            // For hexahedral elements, use 2x2x2 Gauss quadrature
            gaussPoints = {-0.57735026919, 0.57735026919};
            gaussWeights = {1.0, 1.0};
            break;
        default:
            // Default to 1-point quadrature
            gaussPoints = {0.0};
            gaussWeights = {2.0};
            break;
    }
    
    // Numerical integration
    for (size_t i = 0; i < gaussPoints.size(); ++i) {
        for (size_t j = 0; j < gaussPoints.size(); ++j) {
            for (size_t k = 0; k < gaussPoints.size(); ++k) {
                double xi = gaussPoints[i];
                double eta = gaussPoints[j];
                double zeta = gaussPoints[k];
                double weight = gaussWeights[i] * gaussWeights[j] * gaussWeights[k];
                
                // Compute shape functions and derivatives
                Eigen::VectorXd N;
                Eigen::MatrixXd dN;
                Eigen::Matrix3d J, invJ;
                double detJ;
                
                if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
                    // If computation fails, return zero matrix
                    return Eigen::MatrixXd::Zero(numDofs, numDofs);
                }
                
                // Compute B matrix
                Eigen::MatrixXd B = computeBMatrix(dN, invJ);
                
                // Compute element stiffness matrix contribution
                Ke += B.transpose() * D * B * detJ * weight;
            }
        }
    }
    
    return Ke;
}

Eigen::VectorXd LinearStaticSolver::computeElementLoadVector(
    const Element& element,
    const Load& load,
    const std::vector<Node>& nodes) {
    
    // Number of nodes in the element
    size_t numNodes = nodes.size();
    
    // Number of DOFs per node (3 for 3D)
    size_t dofsPerNode = 3;
    
    // Total number of DOFs for the element
    size_t numDofs = numNodes * dofsPerNode;
    
    // Initialize element load vector
    Eigen::VectorXd Fe = Eigen::VectorXd::Zero(numDofs);
    
    // Handle different load types
    switch (load.getType()) {
        case LoadType::SurfacePressure: {
            // Cast to surface pressure load
            const auto& pressureLoad = dynamic_cast<const SurfacePressureLoad&>(load);
            double pressure = pressureLoad.getPressure();
            
            // Gauss quadrature points and weights
            std::vector<double> gaussPoints;
            std::vector<double> gaussWeights;
            
            // Set up Gauss quadrature based on element type
            switch (element.type) {
                case ElementType::Triangle:
                    // For triangular elements, use 3-point quadrature
                    gaussPoints = {0.5, 0.0, 0.5};
                    gaussWeights = {1.0 / 6.0, 1.0 / 6.0, 1.0 / 6.0};
                    break;
                case ElementType::Quad:
                    // For quadrilateral elements, use 2x2 Gauss quadrature
                    gaussPoints = {-0.57735026919, 0.57735026919};
                    gaussWeights = {1.0, 1.0};
                    break;
                default:
                    // Default to 1-point quadrature
                    gaussPoints = {0.0};
                    gaussWeights = {2.0};
                    break;
            }
            
            // Numerical integration
            for (size_t i = 0; i < gaussPoints.size(); ++i) {
                for (size_t j = 0; j < gaussPoints.size(); ++j) {
                    double xi = gaussPoints[i];
                    double eta = gaussPoints[j];
                    double weight = gaussWeights[i] * gaussWeights[j];
                    
                    // Compute shape functions and derivatives
                    Eigen::VectorXd N;
                    Eigen::MatrixXd dN;
                    Eigen::Matrix3d J, invJ;
                    double detJ;
                    
                    if (!computeShapeFunctions(element, nodes, xi, eta, 0.0, N, dN, J, detJ, invJ)) {
                        // If computation fails, return zero vector
                        return Eigen::VectorXd::Zero(numDofs);
                    }
                    
                    // Compute normal vector
                    Eigen::Vector3d v1(dN(0, 0), dN(0, 1), dN(0, 2));
                    Eigen::Vector3d v2(dN(1, 0), dN(1, 1), dN(1, 2));
                    Eigen::Vector3d normal = v1.cross(v2).normalized();
                    
                    // Compute element load vector contribution
                    for (size_t k = 0; k < numNodes; ++k) {
                        Fe[k * dofsPerNode + 0] += N[k] * pressure * normal[0] * detJ * weight;
                        Fe[k * dofsPerNode + 1] += N[k] * pressure * normal[1] * detJ * weight;
                        Fe[k * dofsPerNode + 2] += N[k] * pressure * normal[2] * detJ * weight;
                    }
                }
            }
            break;
        }
        case LoadType::BodyForce: {
            // Cast to body force load
            const auto& bodyForceLoad = dynamic_cast<const BodyForceLoad&>(load);
            double forceX = bodyForceLoad.getForceX();
            double forceY = bodyForceLoad.getForceY();
            double forceZ = bodyForceLoad.getForceZ();
            
            // Gauss quadrature points and weights
            std::vector<double> gaussPoints;
            std::vector<double> gaussWeights;
            
            // Set up Gauss quadrature based on element type
            switch (element.type) {
                case ElementType::Tetra:
                    // For tetrahedral elements, use 1-point quadrature
                    gaussPoints = {0.25};
                    gaussWeights = {1.0 / 6.0};
                    break;
                case ElementType::Hexa:
                    // For hexahedral elements, use 2x2x2 Gauss quadrature
                    gaussPoints = {-0.57735026919, 0.57735026919};
                    gaussWeights = {1.0, 1.0};
                    break;
                default:
                    // Default to 1-point quadrature
                    gaussPoints = {0.0};
                    gaussWeights = {2.0};
                    break;
            }
            
            // Numerical integration
            for (size_t i = 0; i < gaussPoints.size(); ++i) {
                for (size_t j = 0; j < gaussPoints.size(); ++j) {
                    for (size_t k = 0; k < gaussPoints.size(); ++k) {
                        double xi = gaussPoints[i];
                        double eta = gaussPoints[j];
                        double zeta = gaussPoints[k];
                        double weight = gaussWeights[i] * gaussWeights[j] * gaussWeights[k];
                        
                        // Compute shape functions and derivatives
                        Eigen::VectorXd N;
                        Eigen::MatrixXd dN;
                        Eigen::Matrix3d J, invJ;
                        double detJ;
                        
                        if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
                            // If computation fails, return zero vector
                            return Eigen::VectorXd::Zero(numDofs);
                        }
                        
                        // Compute element load vector contribution
                        for (size_t l = 0; l < numNodes; ++l) {
                            Fe[l * dofsPerNode + 0] += N[l] * forceX * detJ * weight;
                            Fe[l * dofsPerNode + 1] += N[l] * forceY * detJ * weight;
                            Fe[l * dofsPerNode + 2] += N[l] * forceZ * detJ * weight;
                        }
                    }
                }
            }
            break;
        }
        default:
            // For other load types, return zero vector
            return Eigen::VectorXd::Zero(numDofs);
    }
    
    return Fe;
}

bool LinearStaticSolver::computeShapeFunctions(
    const Element& element,
    const std::vector<Node>& nodes,
    double xi, double eta, double zeta,
    Eigen::VectorXd& N,
    Eigen::MatrixXd& dN,
    Eigen::Matrix3d& J,
    double& detJ,
    Eigen::Matrix3d& invJ) {
    
    // Number of nodes in the element
    size_t numNodes = nodes.size();
    
    // Initialize shape functions and derivatives
    N = Eigen::VectorXd::Zero(numNodes);
    dN = Eigen::MatrixXd::Zero(numNodes, 3);
    
    // Compute shape functions and derivatives based on element type
    switch (element.type) {
        case ElementType::Tetra:
            // For tetrahedral elements with 4 nodes
            if (numNodes != 4) {
                REBEL_LOG_ERROR("Tetrahedral element must have 4 nodes");
                return false;
            }
            
            // Shape functions
            N[0] = 1.0 - xi - eta - zeta;
            N[1] = xi;
            N[2] = eta;
            N[3] = zeta;
            
            // Derivatives of shape functions
            dN(0, 0) = -1.0; dN(0, 1) = -1.0; dN(0, 2) = -1.0;
            dN(1, 0) =  1.0; dN(1, 1) =  0.0; dN(1, 2) =  0.0;
            dN(2, 0) =  0.0; dN(2, 1) =  1.0; dN(2, 2) =  0.0;
            dN(3, 0) =  0.0; dN(3, 1) =  0.0; dN(3, 2) =  1.0;
            break;
        case ElementType::Hexa:
            // For hexahedral elements with 8 nodes
            if (numNodes != 8) {
                REBEL_LOG_ERROR("Hexahedral element must have 8 nodes");
                return false;
            }
            
            // Shape functions
            N[0] = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 - zeta);
            N[1] = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 - zeta);
            N[2] = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 - zeta);
            N[3] = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 - zeta);
            N[4] = 0.125 * (1.0 - xi) * (1.0 - eta) * (1.0 + zeta);
            N[5] = 0.125 * (1.0 + xi) * (1.0 - eta) * (1.0 + zeta);
            N[6] = 0.125 * (1.0 + xi) * (1.0 + eta) * (1.0 + zeta);
            N[7] = 0.125 * (1.0 - xi) * (1.0 + eta) * (1.0 + zeta);
            
            // Derivatives of shape functions
            dN(0, 0) = -0.125 * (1.0 - eta) * (1.0 - zeta);
            dN(0, 1) = -0.125 * (1.0 - xi) * (1.0 - zeta);
            dN(0, 2) = -0.125 * (1.0 - xi) * (1.0 - eta);
            
            dN(1, 0) =  0.125 * (1.0 - eta) * (1.0 - zeta);
            dN(1, 1) = -0.125 * (1.0 + xi) * (1.0 - zeta);
            dN(1, 2) = -0.125 * (1.0 + xi) * (1.0 - eta);
            
            dN(2, 0) =  0.125 * (1.0 + eta) * (1.0 - zeta);
            dN(2, 1) =  0.125 * (1.0 + xi) * (1.0 - zeta);
            dN(2, 2) = -0.125 * (1.0 + xi) * (1.0 + eta);
            
            dN(3, 0) = -0.125 * (1.0 + eta) * (1.0 - zeta);
            dN(3, 1) =  0.125 * (1.0 - xi) * (1.0 - zeta);
            dN(3, 2) = -0.125 * (1.0 - xi) * (1.0 + eta);
            
            dN(4, 0) = -0.125 * (1.0 - eta) * (1.0 + zeta);
            dN(4, 1) = -0.125 * (1.0 - xi) * (1.0 + zeta);
            dN(4, 2) =  0.125 * (1.0 - xi) * (1.0 - eta);
            
            dN(5, 0) =  0.125 * (1.0 - eta) * (1.0 + zeta);
            dN(5, 1) = -0.125 * (1.0 + xi) * (1.0 + zeta);
            dN(5, 2) =  0.125 * (1.0 + xi) * (1.0 - eta);
            
            dN(6, 0) =  0.125 * (1.0 + eta) * (1.0 + zeta);
            dN(6, 1) =  0.125 * (1.0 + xi) * (1.0 + zeta);
            dN(6, 2) =  0.125 * (1.0 + xi) * (1.0 + eta);
            
            dN(7, 0) = -0.125 * (1.0 + eta) * (1.0 + zeta);
            dN(7, 1) =  0.125 * (1.0 - xi) * (1.0 + zeta);
            dN(7, 2) =  0.125 * (1.0 - xi) * (1.0 + eta);
            break;
        default:
            REBEL_LOG_ERROR("Element type not supported for shape function computation");
            return false;
    }
    
    // Compute Jacobian matrix
    J = Eigen::Matrix3d::Zero();
    for (size_t i = 0; i < numNodes; ++i) {
        J(0, 0) += dN(i, 0) * nodes[i].x;
        J(0, 1) += dN(i, 0) * nodes[i].y;
        J(0, 2) += dN(i, 0) * nodes[i].z;
        J(1, 0) += dN(i, 1) * nodes[i].x;
        J(1, 1) += dN(i, 1) * nodes[i].y;
        J(1, 2) += dN(i, 1) * nodes[i].z;
        J(2, 0) += dN(i, 2) * nodes[i].x;
        J(2, 1) += dN(i, 2) * nodes[i].y;
        J(2, 2) += dN(i, 2) * nodes[i].z;
    }
    
    // Compute determinant of Jacobian
    detJ = J.determinant();
    
    // Check if Jacobian is singular
    if (std::abs(detJ) < 1e-10) {
        REBEL_LOG_ERROR("Jacobian is singular");
        return false;
    }
    
    // Compute inverse of Jacobian
    invJ = J.inverse();
    
    return true;
}

Eigen::MatrixXd LinearStaticSolver::computeBMatrix(
    const Eigen::MatrixXd& dN,
    const Eigen::Matrix3d& invJ) {
    
    // Number of nodes
    size_t numNodes = dN.rows();
    
    // Initialize B matrix
    Eigen::MatrixXd B = Eigen::MatrixXd::Zero(6, numNodes * 3);
    
    // Compute B matrix
    for (size_t i = 0; i < numNodes; ++i) {
        // Compute derivatives of shape functions with respect to x, y, z
        double dNdx = dN(i, 0) * invJ(0, 0) + dN(i, 1) * invJ(1, 0) + dN(i, 2) * invJ(2, 0);
        double dNdy = dN(i, 0) * invJ(0, 1) + dN(i, 1) * invJ(1, 1) + dN(i, 2) * invJ(2, 1);
        double dNdz = dN(i, 0) * invJ(0, 2) + dN(i, 1) * invJ(1, 2) + dN(i, 2) * invJ(2, 2);
        
        // Fill B matrix
        B(0, i * 3 + 0) = dNdx;
        B(1, i * 3 + 1) = dNdy;
        B(2, i * 3 + 2) = dNdz;
        B(3, i * 3 + 0) = dNdy;
        B(3, i * 3 + 1) = dNdx;
        B(4, i * 3 + 1) = dNdz;
        B(4, i * 3 + 2) = dNdy;
        B(5, i * 3 + 0) = dNdz;
        B(5, i * 3 + 2) = dNdx;
    }
    
    return B;
}

Eigen::MatrixXd LinearStaticSolver::computeDMatrix(
    const Material& material) {
    
    // Get material properties
    double E = material.getYoungsModulus();
    double nu = material.getPoissonsRatio();
    
    // Compute material constants
    double c1 = E / ((1.0 + nu) * (1.0 - 2.0 * nu));
    double c2 = 1.0 - nu;
    double c3 = (1.0 - 2.0 * nu) / 2.0;
    
    // Initialize D matrix
    Eigen::MatrixXd D = Eigen::MatrixXd::Zero(6, 6);
    
    // Fill D matrix for isotropic material
    D(0, 0) = c1 * c2;
    D(0, 1) = c1 * nu;
    D(0, 2) = c1 * nu;
    D(1, 0) = c1 * nu;
    D(1, 1) = c1 * c2;
    D(1, 2) = c1 * nu;
    D(2, 0) = c1 * nu;
    D(2, 1) = c1 * nu;
    D(2, 2) = c1 * c2;
    D(3, 3) = c1 * c3;
    D(4, 4) = c1 * c3;
    D(5, 5) = c1 * c3;
    
    return D;
}

StressResult LinearStaticSolver::computeElementStresses(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes,
    const std::vector<DisplacementResult>& displacements) {
    
    // Number of nodes in the element
    size_t numNodes = nodes.size();
    
    // Get element center
    double xi = 0.0;
    double eta = 0.0;
    double zeta = 0.0;
    
    // Compute shape functions and derivatives at element center
    Eigen::VectorXd N;
    Eigen::MatrixXd dN;
    Eigen::Matrix3d J, invJ;
    double detJ;
    
    if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
        // If computation fails, return zero stress
        return StressResult(element.id, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
    
    // Compute B matrix
    Eigen::MatrixXd B = computeBMatrix(dN, invJ);
    
    // Compute D matrix
    Eigen::MatrixXd D = computeDMatrix(material);
    
    // Get displacements for the element nodes
    Eigen::VectorXd u = Eigen::VectorXd::Zero(numNodes * 3);
    for (size_t i = 0; i < numNodes; ++i) {
        int nodeId = element.nodeIds[i];
        
        // Find the displacement for this node
        auto it = std::find_if(displacements.begin(), displacements.end(),
                              [nodeId](const DisplacementResult& d) { return d.nodeId == nodeId; });
        
        if (it != displacements.end()) {
            u[i * 3 + 0] = it->x;
            u[i * 3 + 1] = it->y;
            u[i * 3 + 2] = it->z;
        }
    }
    
    // Compute strain
    Eigen::VectorXd strain = B * u;
    
    // Compute stress
    Eigen::VectorXd stress = D * strain;
    
    // Extract stress components
    double sigmaXX = stress[0];
    double sigmaYY = stress[1];
    double sigmaZZ = stress[2];
    double sigmaXY = stress[3];
    double sigmaYZ = stress[4];
    double sigmaXZ = stress[5];
    
    // Compute principal stresses
    Eigen::Matrix3d stressTensor;
    stressTensor << sigmaXX, sigmaXY, sigmaXZ,
                    sigmaXY, sigmaYY, sigmaYZ,
                    sigmaXZ, sigmaYZ, sigmaZZ;
    
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(stressTensor);
    Eigen::Vector3d principalStresses = solver.eigenvalues();
    
    // Sort principal stresses
    std::sort(principalStresses.data(), principalStresses.data() + 3, std::greater<double>());
    
    // Compute von Mises stress
    double vonMises = std::sqrt(0.5 * (
        std::pow(principalStresses[0] - principalStresses[1], 2) +
        std::pow(principalStresses[1] - principalStresses[2], 2) +
        std::pow(principalStresses[2] - principalStresses[0], 2)
    ));
    
    // Compute maximum shear stress
    double maxShear = (principalStresses[0] - principalStresses[2]) / 2.0;
    
    // Create stress result
    return StressResult(
        element.id,
        sigmaXX, sigmaYY, sigmaZZ,
        sigmaXY, sigmaYZ, sigmaXZ
    );
}

StrainResult LinearStaticSolver::computeElementStrains(
    const Element& element,
    const Material& material,
    const std::vector<Node>& nodes,
    const std::vector<DisplacementResult>& displacements) {
    
    // Number of nodes in the element
    size_t numNodes = nodes.size();
    
    // Get element center
    double xi = 0.0;
    double eta = 0.0;
    double zeta = 0.0;
    
    // Compute shape functions and derivatives at element center
    Eigen::VectorXd N;
    Eigen::MatrixXd dN;
    Eigen::Matrix3d J, invJ;
    double detJ;
    
    if (!computeShapeFunctions(element, nodes, xi, eta, zeta, N, dN, J, detJ, invJ)) {
        // If computation fails, return zero strain
        return StrainResult(element.id, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
    
    // Compute B matrix
    Eigen::MatrixXd B = computeBMatrix(dN, invJ);
    
    // Get displacements for the element nodes
    Eigen::VectorXd u = Eigen::VectorXd::Zero(numNodes * 3);
    for (size_t i = 0; i < numNodes; ++i) {
        int nodeId = element.nodeIds[i];
        
        // Find the displacement for this node
        auto it = std::find_if(displacements.begin(), displacements.end(),
                              [nodeId](const DisplacementResult& d) { return d.nodeId == nodeId; });
        
        if (it != displacements.end()) {
            u[i * 3 + 0] = it->x;
            u[i * 3 + 1] = it->y;
            u[i * 3 + 2] = it->z;
        }
    }
    
    // Compute strain
    Eigen::VectorXd strain = B * u;
    
    // Extract strain components
    double epsXX = strain[0];
    double epsYY = strain[1];
    double epsZZ = strain[2];
    double epsXY = strain[3];
    double epsYZ = strain[4];
    double epsXZ = strain[5];
    
    // Compute principal strains
    Eigen::Matrix3d strainTensor;
    strainTensor << epsXX, epsXY, epsXZ,
                    epsXY, epsYY, epsYZ,
                    epsXZ, epsYZ, epsZZ;
    
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(strainTensor);
    Eigen::Vector3d principalStrains = solver.eigenvalues();
    
    // Sort principal strains
    std::sort(principalStrains.data(), principalStrains.data() + 3, std::greater<double>());
    
    // Compute maximum shear strain
    double maxShear = (principalStrains[0] - principalStrains[2]) / 2.0;
    
    // Create strain result
    return StrainResult(
        element.id,
        epsXX, epsYY, epsZZ,
        epsXY, epsYZ, epsXZ
    );
}

ReactionForceResult LinearStaticSolver::computeNodeReactionForce(
    int nodeId,
    std::shared_ptr<Mesh> mesh,
    const std::vector<DisplacementResult>& displacements) {
    
    // Get the DOFs for the node
    const auto& dofs = nodeIdToDofs[nodeId];
    
    // Compute reaction forces
    double rx = 0.0;
    double ry = 0.0;
    double rz = 0.0;
    
    // For each DOF, compute the reaction force
    for (size_t i = 0; i < 3; ++i) {
        int dof = dofs[i];
        
        // Check if the DOF has a prescribed displacement
        if (std::abs(stiffnessMatrix.coeff(dof, dof)) > 1.0e9) {
            // Compute the reaction force
            double r = 0.0;
            for (int j = 0; j < numDofs; ++j) {
                r += stiffnessMatrix.coeff(dof, j) * displacementVector[j];
            }
            r -= loadVector[dof];
            
            // Add to the appropriate component
            if (i == 0) rx = r;
            else if (i == 1) ry = r;
            else if (i == 2) rz = r;
        }
    }
    
    // Create reaction force result
    return ReactionForceResult(nodeId, rx, ry, rz);
}

} // namespace rebel::simulation
