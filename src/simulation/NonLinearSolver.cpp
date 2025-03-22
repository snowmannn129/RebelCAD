#include "simulation/NonLinearSolver.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include <string>
#include <cmath>
#include <algorithm>

namespace rebel::simulation {

NonLinearSolver::NonLinearSolver(const FEASettings& settings, 
                                 const NonLinearSolverSettings& nonLinearSettings)
    : m_settings(settings),
      m_nonLinearSettings(nonLinearSettings),
      m_numDofs(0) {
    // Log creation of solver
}

std::shared_ptr<FEAResult> NonLinearSolver::solve(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    const std::vector<std::shared_ptr<Load>>& loads,
    std::function<void(float)> progressCallback) {
    
    if (!mesh) {
        // Log error: Mesh is null
        return nullptr;
    }
    
    if (boundaryConditions.empty()) {
        // Log error: No boundary conditions provided
        return nullptr;
    }
    
    if (loads.empty()) {
        // Log error: No loads provided
        return nullptr;
    }
    
    // Choose the appropriate solver method
    switch (m_nonLinearSettings.method) {
        case NonLinearSolverMethod::NewtonRaphson:
            return solveNewtonRaphson(mesh, boundaryConditions, loads, progressCallback);
        case NonLinearSolverMethod::ModifiedNewtonRaphson:
            return solveModifiedNewtonRaphson(mesh, boundaryConditions, loads, progressCallback);
        case NonLinearSolverMethod::QuasiNewton:
            return solveQuasiNewton(mesh, boundaryConditions, loads, progressCallback);
        case NonLinearSolverMethod::ArcLength:
            return solveArcLength(mesh, boundaryConditions, loads, progressCallback);
        default:
            // Log error: Unknown solver method
            return nullptr;
    }
}

std::shared_ptr<FEAResult> NonLinearSolver::solveNewtonRaphson(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    const std::vector<std::shared_ptr<Load>>& loads,
    std::function<void(float)> progressCallback) {
    
    // Log start of Newton-Raphson solution
    // REBEL_LOG_INFO("Starting Newton-Raphson non-linear solution");
    
    // 1. Set up the system (DOFs, matrices, etc.)
    // Determine the number of degrees of freedom (DOFs)
    m_numDofs = mesh->getNodeCount() * mesh->getDofsPerNode();
    
    // Create vectors for displacements, velocities, and accelerations
    std::vector<double> displacements(m_numDofs, 0.0);
    std::vector<double> incrementalDisplacements(m_numDofs, 0.0);
    std::vector<double> residualForces(m_numDofs, 0.0);
    std::vector<double> internalForces(m_numDofs, 0.0);
    std::vector<double> externalForces(m_numDofs, 0.0);
    
    // Create matrix for tangent stiffness
    // In a real implementation, this would be a sparse matrix
    // For simplicity, we'll use a vector of vectors here
    std::vector<std::vector<double>> tangentStiffness(m_numDofs, std::vector<double>(m_numDofs, 0.0));
    
    // 2. Apply initial boundary conditions
    applyBoundaryConditions(boundaryConditions, tangentStiffness, externalForces, displacements);
    
    // 3. Loop through load steps
    int numLoadSteps = m_nonLinearSettings.numLoadSteps;
    double loadFactor = 0.0;
    double loadIncrement = 1.0 / numLoadSteps;
    
    if (m_nonLinearSettings.adaptiveLoadStepping) {
        loadIncrement = m_nonLinearSettings.loadIncrementFactor;
    }
    
    // Create a result object to store the solution
    auto result = std::make_shared<FEAResult>(m_numDofs);
    
    // Initialize progress
    float progress = 0.0f;
    if (progressCallback) {
        progressCallback(progress);
    }
    
    // Loop through load steps
    for (int loadStep = 0; loadStep < numLoadSteps; ++loadStep) {
        // Update load factor
        double previousLoadFactor = loadFactor;
        loadFactor += loadIncrement;
        if (loadFactor > 1.0) {
            loadFactor = 1.0;
        }
        
        // Log load step
        // REBEL_LOG_INFO("Load step " + std::to_string(loadStep + 1) + " of " + std::to_string(numLoadSteps) + 
        //               ", load factor = " + std::to_string(loadFactor));
        
        // Update external forces for current load factor
        updateExternalForces(loads, externalForces, loadFactor);
        
        // 4. For each load step, iterate until convergence
        bool converged = false;
        int iteration = 0;
        
        // Reset incremental displacements for this load step
        std::fill(incrementalDisplacements.begin(), incrementalDisplacements.end(), 0.0);
        
        while (!converged && iteration < m_nonLinearSettings.maxIterations) {
            // Compute internal forces based on current displacements
            computeInternalForces(mesh, displacements, internalForces);
            
            // Compute residual forces: R = Fext - Fint
            for (int i = 0; i < m_numDofs; ++i) {
                residualForces[i] = externalForces[i] - internalForces[i];
            }
            
            // Check convergence
            double residualNorm = computeNorm(residualForces);
            double externalNorm = computeNorm(externalForces);
            double relativeTolerance = residualNorm / (externalNorm > 1e-10 ? externalNorm : 1.0);
            
            // Log iteration
            // REBEL_LOG_INFO("  Iteration " + std::to_string(iteration + 1) + 
            //               ", residual norm = " + std::to_string(residualNorm) +
            //               ", relative tolerance = " + std::to_string(relativeTolerance));
            
            if (relativeTolerance < m_nonLinearSettings.convergenceTolerance) {
                converged = true;
                // REBEL_LOG_INFO("  Converged after " + std::to_string(iteration + 1) + " iterations");
                break;
            }
            
            // Compute tangent stiffness matrix
            computeTangentStiffness(mesh, displacements, tangentStiffness);
            
            // Apply boundary conditions to tangent stiffness and residual
            applyBoundaryConditionsToSystem(boundaryConditions, tangentStiffness, residualForces);
            
            // Solve the system: K * du = R
            std::vector<double> du = solveLinearSystem(tangentStiffness, residualForces);
            
            // Update incremental displacements
            for (int i = 0; i < m_numDofs; ++i) {
                incrementalDisplacements[i] += du[i];
            }
            
            // Line search if enabled
            double alpha = 1.0;
            if (m_nonLinearSettings.useLineSearch) {
                alpha = performLineSearch(mesh, displacements, du, internalForces, externalForces);
            }
            
            // Update displacements
            for (int i = 0; i < m_numDofs; ++i) {
                displacements[i] += alpha * du[i];
            }
            
            // Increment iteration counter
            ++iteration;
        }
        
        // Check if the load step converged
        if (!converged) {
            // REBEL_LOG_WARNING("Load step " + std::to_string(loadStep + 1) + " did not converge after " + 
            //                  std::to_string(m_nonLinearSettings.maxIterations) + " iterations");
            
            // If adaptive load stepping is enabled, reduce the load increment and retry
            if (m_nonLinearSettings.adaptiveLoadStepping) {
                loadFactor = previousLoadFactor;
                loadIncrement *= 0.5;
                if (loadIncrement < m_nonLinearSettings.minLoadStepSize) {
                    // REBEL_LOG_ERROR("Load increment too small, aborting solution");
                    return nullptr;
                }
                // REBEL_LOG_INFO("Reducing load increment to " + std::to_string(loadIncrement) + " and retrying");
                --loadStep;
                continue;
            } else {
                // REBEL_LOG_ERROR("Non-linear solution failed to converge");
                return nullptr;
            }
        }
        
        // If adaptive load stepping is enabled and convergence was fast, increase the load increment
        if (m_nonLinearSettings.adaptiveLoadStepping && converged && iteration < m_nonLinearSettings.maxIterations / 4) {
            loadIncrement = std::min(loadIncrement * 1.5, m_nonLinearSettings.maxLoadStepSize);
            // REBEL_LOG_INFO("Increasing load increment to " + std::to_string(loadIncrement));
        }
        
        // Store results for this load step
        result->addLoadStep(loadFactor, displacements);
        
        // Update progress
        progress = static_cast<float>(loadStep + 1) / numLoadSteps;
        if (progressCallback) {
            progressCallback(progress);
        }
    }
    
    // 5. Compute and return results
    result->computeStressesAndStrains(mesh, displacements);
    
    // REBEL_LOG_INFO("Newton-Raphson non-linear solution completed successfully");
    return result;
}

// Helper methods for the Newton-Raphson solver

void NonLinearSolver::applyBoundaryConditions(
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    std::vector<std::vector<double>>& tangentStiffness,
    std::vector<double>& externalForces,
    std::vector<double>& displacements) {
    
    // Apply boundary conditions to the system
    for (const auto& bc : boundaryConditions) {
        // Get the DOFs affected by this boundary condition
        std::vector<int> dofs = bc->getAffectedDofs();
        
        // Get the prescribed values
        std::vector<double> values = bc->getPrescribedValues();
        
        // Apply the boundary condition
        for (size_t i = 0; i < dofs.size(); ++i) {
            int dof = dofs[i];
            double value = values[i];
            
            // Set the displacement
            displacements[dof] = value;
            
            // Modify the stiffness matrix and force vector
            for (int j = 0; j < m_numDofs; ++j) {
                tangentStiffness[dof][j] = 0.0;
                tangentStiffness[j][dof] = 0.0;
            }
            tangentStiffness[dof][dof] = 1.0;
            
            externalForces[dof] = value;
        }
    }
}

void NonLinearSolver::applyBoundaryConditionsToSystem(
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    std::vector<std::vector<double>>& tangentStiffness,
    std::vector<double>& residualForces) {
    
    // Apply boundary conditions to the system
    for (const auto& bc : boundaryConditions) {
        // Get the DOFs affected by this boundary condition
        std::vector<int> dofs = bc->getAffectedDofs();
        
        // Apply the boundary condition
        for (int dof : dofs) {
            // Modify the stiffness matrix and residual vector
            for (int j = 0; j < m_numDofs; ++j) {
                tangentStiffness[dof][j] = 0.0;
                tangentStiffness[j][dof] = 0.0;
            }
            tangentStiffness[dof][dof] = 1.0;
            
            residualForces[dof] = 0.0;
        }
    }
}

void NonLinearSolver::updateExternalForces(
    const std::vector<std::shared_ptr<Load>>& loads,
    std::vector<double>& externalForces,
    double loadFactor) {
    
    // Reset external forces
    std::fill(externalForces.begin(), externalForces.end(), 0.0);
    
    // Apply loads
    for (const auto& load : loads) {
        // Get the DOFs affected by this load
        std::vector<int> dofs = load->getAffectedDofs();
        
        // Get the load values
        std::vector<double> values = load->getLoadValues();
        
        // Apply the load
        for (size_t i = 0; i < dofs.size(); ++i) {
            int dof = dofs[i];
            double value = values[i];
            
            // Scale by load factor
            externalForces[dof] += value * loadFactor;
        }
    }
}

void NonLinearSolver::computeInternalForces(
    std::shared_ptr<Mesh> mesh,
    const std::vector<double>& displacements,
    std::vector<double>& internalForces) {
    
    // Reset internal forces
    std::fill(internalForces.begin(), internalForces.end(), 0.0);
    
    // Loop over elements
    for (int elemIdx = 0; elemIdx < mesh->getElementCount(); ++elemIdx) {
        // Get element data
        std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
        
        // Get element displacements
        std::vector<double> elementDisplacements;
        elementDisplacements.reserve(elementDofs.size());
        for (int dof : elementDofs) {
            elementDisplacements.push_back(displacements[dof]);
        }
        
        // Compute element internal forces
        std::vector<double> elementInternalForces = computeElementInternalForces(mesh, elemIdx, elementDisplacements);
        
        // Assemble into global internal forces
        for (size_t i = 0; i < elementDofs.size(); ++i) {
            int dof = elementDofs[i];
            internalForces[dof] += elementInternalForces[i];
        }
    }
}

std::vector<double> NonLinearSolver::computeElementInternalForces(
    std::shared_ptr<Mesh> mesh,
    int elemIdx,
    const std::vector<double>& elementDisplacements) {
    
    // Get element data
    std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
    
    // Get element material
    std::shared_ptr<Material> material = mesh->getElementMaterial(elemIdx);
    
    // Compute element strains
    std::vector<double> elementStrains = computeElementStrains(mesh, elemIdx, elementDisplacements);
    
    // Compute element stresses
    std::vector<double> elementStresses = material->computeStresses(elementStrains);
    
    // Compute element internal forces
    std::vector<double> elementInternalForces(elementDofs.size(), 0.0);
    
    // In a real implementation, this would use the element shape functions
    // and numerical integration to compute the internal forces
    // For simplicity, we'll use a placeholder implementation
    
    // Get the element stiffness matrix
    std::vector<std::vector<double>> elementStiffness = computeElementStiffness(mesh, elemIdx, elementDisplacements);
    
    // Compute internal forces as K * u
    for (size_t i = 0; i < elementDofs.size(); ++i) {
        for (size_t j = 0; j < elementDofs.size(); ++j) {
            elementInternalForces[i] += elementStiffness[i][j] * elementDisplacements[j];
        }
    }
    
    return elementInternalForces;
}

std::vector<double> NonLinearSolver::computeElementStrains(
    std::shared_ptr<Mesh> mesh,
    int elemIdx,
    const std::vector<double>& elementDisplacements) {
    
    // Get element data
    std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
    
    // In a real implementation, this would use the element shape functions
    // and their derivatives to compute the strains
    // For simplicity, we'll use a placeholder implementation
    
    // For a linear elastic problem, strain = B * u
    // where B is the strain-displacement matrix
    
    // Get the B matrix
    std::vector<std::vector<double>> B = computeStrainDisplacementMatrix(mesh, elemIdx);
    
    // Compute strains
    std::vector<double> elementStrains(B.size(), 0.0);
    for (size_t i = 0; i < B.size(); ++i) {
        for (size_t j = 0; j < elementDofs.size(); ++j) {
            elementStrains[i] += B[i][j] * elementDisplacements[j];
        }
    }
    
    // For geometric non-linearity, add non-linear terms
    if (m_nonLinearSettings.nonLinearityType == NonLinearityType::Geometric ||
        m_nonLinearSettings.nonLinearityType == NonLinearityType::Combined) {
        // Add non-linear strain terms
        // This is a placeholder for the actual implementation
    }
    
    return elementStrains;
}

std::vector<std::vector<double>> NonLinearSolver::computeStrainDisplacementMatrix(
    std::shared_ptr<Mesh> mesh,
    int elemIdx) {
    
    // Get element data
    std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
    
    // In a real implementation, this would compute the strain-displacement matrix
    // based on the element type and shape functions
    // For simplicity, we'll use a placeholder implementation
    
    // Create a dummy B matrix
    std::vector<std::vector<double>> B(6, std::vector<double>(elementDofs.size(), 0.0));
    
    // Fill with some values
    for (size_t i = 0; i < B.size(); ++i) {
        for (size_t j = 0; j < elementDofs.size(); ++j) {
            B[i][j] = 0.1 * (i + 1) * (j + 1);
        }
    }
    
    return B;
}

void NonLinearSolver::computeTangentStiffness(
    std::shared_ptr<Mesh> mesh,
    const std::vector<double>& displacements,
    std::vector<std::vector<double>>& tangentStiffness) {
    
    // Reset tangent stiffness
    for (auto& row : tangentStiffness) {
        std::fill(row.begin(), row.end(), 0.0);
    }
    
    // Loop over elements
    for (int elemIdx = 0; elemIdx < mesh->getElementCount(); ++elemIdx) {
        // Get element data
        std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
        
        // Get element displacements
        std::vector<double> elementDisplacements;
        elementDisplacements.reserve(elementDofs.size());
        for (int dof : elementDofs) {
            elementDisplacements.push_back(displacements[dof]);
        }
        
        // Compute element tangent stiffness
        std::vector<std::vector<double>> elementTangentStiffness = 
            computeElementTangentStiffness(mesh, elemIdx, elementDisplacements);
        
        // Assemble into global tangent stiffness
        for (size_t i = 0; i < elementDofs.size(); ++i) {
            int rowDof = elementDofs[i];
            for (size_t j = 0; j < elementDofs.size(); ++j) {
                int colDof = elementDofs[j];
                tangentStiffness[rowDof][colDof] += elementTangentStiffness[i][j];
            }
        }
    }
}

std::vector<std::vector<double>> NonLinearSolver::computeElementTangentStiffness(
    std::shared_ptr<Mesh> mesh,
    int elemIdx,
    const std::vector<double>& elementDisplacements) {
    
    // Get element data
    std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
    
    // Get element material
    std::shared_ptr<Material> material = mesh->getElementMaterial(elemIdx);
    
    // In a real implementation, this would compute the element tangent stiffness
    // based on the current displacements, strains, and stresses
    // For simplicity, we'll use a placeholder implementation
    
    // For a linear elastic problem, K = B^T * D * B
    // where B is the strain-displacement matrix and D is the material stiffness matrix
    
    // Get the B matrix
    std::vector<std::vector<double>> B = computeStrainDisplacementMatrix(mesh, elemIdx);
    
    // Get the D matrix
    std::vector<std::vector<double>> D = material->getStiffnessMatrix();
    
    // Compute B^T * D
    std::vector<std::vector<double>> BTD(elementDofs.size(), std::vector<double>(D.size(), 0.0));
    for (size_t i = 0; i < elementDofs.size(); ++i) {
        for (size_t j = 0; j < D.size(); ++j) {
            for (size_t k = 0; k < B.size(); ++k) {
                BTD[i][j] += B[k][i] * D[k][j];
            }
        }
    }
    
    // Compute B^T * D * B
    std::vector<std::vector<double>> elementStiffness(elementDofs.size(), std::vector<double>(elementDofs.size(), 0.0));
    for (size_t i = 0; i < elementDofs.size(); ++i) {
        for (size_t j = 0; j < elementDofs.size(); ++j) {
            for (size_t k = 0; k < D.size(); ++k) {
                elementStiffness[i][j] += BTD[i][k] * B[k][j];
            }
        }
    }
    
    // For geometric non-linearity, add geometric stiffness
    if (m_nonLinearSettings.nonLinearityType == NonLinearityType::Geometric ||
        m_nonLinearSettings.nonLinearityType == NonLinearityType::Combined) {
        // Compute element strains
        std::vector<double> elementStrains = computeElementStrains(mesh, elemIdx, elementDisplacements);
        
        // Compute element stresses
        std::vector<double> elementStresses = material->computeStresses(elementStrains);
        
        // Compute geometric stiffness
        std::vector<std::vector<double>> geometricStiffness = 
            computeElementGeometricStiffness(mesh, elemIdx, elementStresses);
        
        // Add to element stiffness
        for (size_t i = 0; i < elementDofs.size(); ++i) {
            for (size_t j = 0; j < elementDofs.size(); ++j) {
                elementStiffness[i][j] += geometricStiffness[i][j];
            }
        }
    }
    
    return elementStiffness;
}

std::vector<std::vector<double>> NonLinearSolver::computeElementGeometricStiffness(
    std::shared_ptr<Mesh> mesh,
    int elemIdx,
    const std::vector<double>& elementStresses) {
    
    // Get element data
    std::vector<int> elementDofs = mesh->getElementDofs(elemIdx);
    
    // In a real implementation, this would compute the element geometric stiffness
    // based on the current stresses
    // For simplicity, we'll use a placeholder implementation
    
    // Create a dummy geometric stiffness matrix
    std::vector<std::vector<double>> geometricStiffness(elementDofs.size(), std::vector<double>(elementDofs.size(), 0.0));
    
    // Fill with some values
    for (size_t i = 0; i < elementDofs.size(); ++i) {
        for (size_t j = 0; j < elementDofs.size(); ++j) {
            geometricStiffness[i][j] = 0.01 * (i + 1) * (j + 1);
        }
    }
    
    return geometricStiffness;
}

std::vector<std::vector<double>> NonLinearSolver::computeElementStiffness(
    std::shared_ptr<Mesh> mesh,
    int elemIdx,
    const std::vector<double>& elementDisplacements) {
    
    // This is a simplified version that just returns the tangent stiffness
    return computeElementTangentStiffness(mesh, elemIdx, elementDisplacements);
}

double NonLinearSolver::performLineSearch(
    std::shared_ptr<Mesh> mesh,
    const std::vector<double>& displacements,
    const std::vector<double>& du,
    const std::vector<double>& internalForces,
    const std::vector<double>& externalForces) {
    
    // Compute initial residual dot product
    double initialProduct = 0.0;
    for (int i = 0; i < m_numDofs; ++i) {
        initialProduct += du[i] * (externalForces[i] - internalForces[i]);
    }
    
    // If initial product is close to zero, return default alpha
    if (std::abs(initialProduct) < 1e-10) {
        return 1.0;
    }
    
    // Try different alpha values
    double alpha = 1.0;
    double bestAlpha = 1.0;
    double bestProduct = std::abs(initialProduct);
    
    for (int i = 0; i < m_nonLinearSettings.maxLineSearchIterations; ++i) {
        // Compute trial displacements
        std::vector<double> trialDisplacements = displacements;
        for (int j = 0; j < m_numDofs; ++j) {
            trialDisplacements[j] += alpha * du[j];
        }
        
        // Compute trial internal forces
        std::vector<double> trialInternalForces(m_numDofs, 0.0);
        computeInternalForces(mesh, trialDisplacements, trialInternalForces);
        
        // Compute residual dot product
        double product = 0.0;
        for (int j = 0; j < m_numDofs; ++j) {
            product += du[j] * (externalForces[j] - trialInternalForces[j]);
        }
        
        // Check if this is better
        if (std::abs(product) < bestProduct) {
            bestAlpha = alpha;
            bestProduct = std::abs(product);
        }
        
        // Check convergence
        if (std::abs(product) < m_nonLinearSettings.lineSearchTolerance * std::abs(initialProduct)) {
            return alpha;
        }
        
        // Update alpha
        alpha *= 0.5;
    }
    
    // Return best alpha found
    return bestAlpha;
}

std::vector<double> NonLinearSolver::solveLinearSystem(
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b) {
    
    // In a real implementation, this would use a sparse linear solver
    // For simplicity, we'll use a placeholder implementation
    
    // Create the solution vector
    std::vector<double> x(b.size(), 0.0);
    
    // For this placeholder, we'll just return a scaled version of b
    for (size_t i = 0; i < b.size(); ++i) {
        x[i] = b[i] * 0.01;
    }
    
    return x;
}

double NonLinearSolver::computeNorm(const std::vector<double>& v) {
    double sum = 0.0;
    for (double value : v) {
        sum += value * value;
    }
    return std::sqrt(sum);
}

std::shared_ptr<FEAResult> NonLinearSolver::solveModifiedNewtonRaphson(
    std::shared_ptr<Mesh> mesh,
    const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
    const std::vector<std::shared_ptr<Load>>& loads,
    std::function<void(float)> progressCallback) {
    
    // Log start of Modified Newton-Raphson solution
    // REBEL_LOG_INFO("Starting Modified Newton-Raphson non-linear solution");
    
    // 1. Set up the system (DOFs, matrices, etc.)
    // Determine the number of degrees of freedom (DOFs)
    m_numDofs = mesh->getNodeCount() * mesh->getDofsPerNode();
    
    // Create vectors for displacements, velocities, and accelerations
    std::vector<double> displacements(m_numDofs, 0.0);
    std::vector<double> incrementalDisplacements(m_numDofs, 0.0);
    std::vector<double> residualForces(m_numDofs, 0.0);
    std::vector<double> internalForces(m_numDofs, 0.0);
    std::vector<double> externalForces(m_numDofs, 0.0);
    
    // Create matrix for tangent stiffness
    // In a real implementation, this would be a sparse matrix
    // For simplicity, we'll use a vector of vectors here
    std::vector<std::vector<double>> tangentStiffness(m_numDofs, std::vector<double>(m_numDofs, 0.0));
    
    // 2. Apply initial boundary conditions
    applyBoundaryConditions(boundaryConditions, tangentStiffness, externalForces, displacements);
    
    // 3. Loop through load steps
    int numLoadSteps = m_nonLinearSettings.numLoadSteps;
    double loadFactor = 0.0;
    double loadIncrement = 1.0 / numLoadSteps;
    
    if (m_nonLinearSettings.adaptiveLoadStepping) {
        loadIncrement = m_nonLinearSettings.loadIncrementFactor;
    }
    
    // Create a result object to store the solution
    auto result = std::
