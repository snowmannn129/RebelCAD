#include "simulation/ThermalSolver.h"
#include "core/Log.h"
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/SparseCholesky>

namespace rebel::simulation {

// Solve linear system
bool ThermalSolver::solveLinearSystem(
    const Eigen::SparseMatrix<double>& A,
    const Eigen::VectorXd& b,
    Eigen::VectorXd& x) {
    
    REBEL_LOG_INFO("Solving linear system...");
    
    // Check matrix and vector sizes
    if (A.rows() != A.cols() || A.rows() != b.size()) {
        REBEL_LOG_ERROR("Matrix and vector sizes do not match");
        return false;
    }
    
    // Initialize solution vector
    x.resize(b.size());
    
    // Choose solver based on settings
    if (m_thermalSettings.useDirectSolver) {
        // Use direct solver (Cholesky decomposition)
        REBEL_LOG_INFO("Using direct solver (Cholesky decomposition)");
        
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
        solver.compute(A);
        
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Failed to compute Cholesky decomposition");
            return false;
        }
        
        x = solver.solve(b);
        
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Failed to solve linear system using Cholesky decomposition");
            return false;
        }
    } else {
        // Use iterative solver (Conjugate Gradient)
        REBEL_LOG_INFO("Using iterative solver (Conjugate Gradient)");
        
        Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper> solver;
        solver.setTolerance(m_thermalSettings.convergenceTolerance);
        solver.setMaxIterations(m_thermalSettings.maxIterations);
        
        solver.compute(A);
        
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Failed to initialize Conjugate Gradient solver");
            return false;
        }
        
        x = solver.solve(b);
        
        if (solver.info() != Eigen::Success) {
            REBEL_LOG_ERROR("Failed to solve linear system using Conjugate Gradient");
            return false;
        }
        
        REBEL_LOG_INFO("Conjugate Gradient solver converged in " + std::to_string(solver.iterations()) + 
                       " iterations with error " + std::to_string(solver.error()));
    }
    
    REBEL_LOG_INFO("Linear system solved successfully");
    return true;
}

} // namespace rebel::simulation
