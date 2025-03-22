#pragma once

#include "simulation/FEASystem.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <Eigen/Sparse>

namespace rebel::simulation {

// Forward declarations
class Mesh;
class BoundaryCondition;
class Load;
class FEAResult;

/**
 * @struct ThermalSolverSettings
 * @brief Settings for the thermal solver.
 */
struct ThermalSolverSettings {
    /**
     * @enum AnalysisType
     * @brief Types of thermal analysis.
     */
    enum class AnalysisType {
        SteadyState,  ///< Steady-state thermal analysis
        Transient     ///< Transient thermal analysis
    };

    /**
     * @enum TimeIntegrationMethod
     * @brief Time integration methods for transient analysis.
     */
    enum class TimeIntegrationMethod {
        Explicit,     ///< Explicit time integration
        Implicit,     ///< Implicit time integration
        CrankNicolson ///< Crank-Nicolson time integration
    };

    AnalysisType analysisType = AnalysisType::SteadyState;  ///< Type of thermal analysis
    TimeIntegrationMethod timeIntegrationMethod = TimeIntegrationMethod::Implicit; ///< Time integration method
    double startTime = 0.0;                    ///< Start time for transient analysis
    double endTime = 1.0;                      ///< End time for transient analysis
    double timeStep = 0.1;                     ///< Time step for transient analysis
    bool adaptiveTimeStep = false;             ///< Whether to use adaptive time stepping
    double minTimeStep = 0.01;                 ///< Minimum time step for adaptive time stepping
    double maxTimeStep = 0.5;                  ///< Maximum time step for adaptive time stepping
    double convergenceTolerance = 1e-6;        ///< Convergence tolerance for iterative solvers
    int maxIterations = 1000;                  ///< Maximum iterations for iterative solvers
    
    /**
     * @brief Validates the settings.
     * @return True if settings are valid, false otherwise.
     */
    bool validate() const {
        // Basic validation
        if (startTime >= endTime) {
            return false;
        }
        
        if (timeStep <= 0.0) {
            return false;
        }
        
        if (adaptiveTimeStep && (minTimeStep <= 0.0 || maxTimeStep <= 0.0 || minTimeStep >= maxTimeStep)) {
            return false;
        }
        
        if (convergenceTolerance <= 0.0) {
            return false;
        }
        
        if (maxIterations <= 0) {
            return false;
        }
        
        return true;
    }
};

/**
 * @class ThermalResult
 * @brief Result of a thermal analysis.
 */
class ThermalResult {
public:
    /**
     * @brief Constructs a new ThermalResult object.
     * @param numTimePoints Number of time points in the result.
     */
    ThermalResult(int numTimePoints);
    
    /**
     * @brief Exports the result to a file.
     * @param filePath Path to the file.
     * @return True if export was successful, false otherwise.
     */
    bool exportToFile(const std::string& filePath) const;
    
std::vector<double> timePoints;                ///< Time points
std::vector<std::vector<double>> temperatures; ///< Temperatures at each time point
std::vector<std::vector<double>> heatFluxes;   ///< Heat fluxes at each time point
std::vector<std::vector<double>> thermalGradients; ///< Thermal gradients at each time point
};

/**
 * @class ThermalSolver
 * @brief Solver for thermal analysis.
 */
class ThermalSolver {
public:
    /**
     * @brief Constructs a new ThermalSolver object.
     * @param settings The FEA settings.
     * @param thermalSettings The thermal solver settings.
     */
    ThermalSolver(const FEASettings& settings, const ThermalSolverSettings& thermalSettings);
    
    /**
     * @brief Solves the thermal problem.
     * @param mesh The mesh.
     * @param boundaryConditions The boundary conditions.
     * @param loads The loads.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solve(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);
    
private:
    /**
     * @brief Solves a steady-state thermal problem.
     * @param mesh The mesh.
     * @param boundaryConditions The boundary conditions.
     * @param loads The loads.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<ThermalResult> solveSteadyState(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Solves a transient thermal problem.
     * @param mesh The mesh.
     * @param boundaryConditions The boundary conditions.
     * @param loads The loads.
     * @param initialTemperatures Initial temperatures.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<ThermalResult> solveTransient(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        const std::vector<double>& initialTemperatures,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Assembles the conductivity matrix.
     * @param mesh The mesh.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleConductivityMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Assembles the capacity matrix.
     * @param mesh The mesh.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleCapacityMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Assembles the heat load vector.
     * @param mesh The mesh.
     * @param loads The loads.
     * @param time The current time.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleHeatLoadVector(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<Load>>& loads,
        double time,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Applies boundary conditions.
     * @param mesh The mesh.
     * @param boundaryConditions The boundary conditions.
     * @param time The current time.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if application was successful, false otherwise.
     */
    bool applyBoundaryConditions(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        double time,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Initializes time integration.
     * @param mesh The mesh.
     * @param initialTemperatures Initial temperatures.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if initialization was successful, false otherwise.
     */
    bool initializeTimeIntegration(
        std::shared_ptr<Mesh> mesh,
        const std::vector<double>& initialTemperatures,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Performs a time step.
     * @param time The current time.
     * @param timeStep The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if time step was successful, false otherwise.
     */
    bool timeStep(
        double time,
        double timeStep,
        std::function<void(float)> progressCallback = nullptr);
    
    /**
     * @brief Solves a linear system.
     * @param A The system matrix.
     * @param b The right-hand side vector.
     * @param x The solution vector.
     * @return True if solution was successful, false otherwise.
     */
    bool solveLinearSystem(
        const std::unique_ptr<Eigen::SparseMatrix<double>>& A,
        const std::unique_ptr<Eigen::VectorXd>& b,
        std::unique_ptr<Eigen::VectorXd>& x);
    
    /**
     * @brief Creates a thermal solver with the specified settings.
     * @param thermalSettings The settings for the thermal solver.
     * @return Shared pointer to the created solver, or nullptr if failed.
     */
    std::shared_ptr<ThermalSolver> createThermalSolver(
        const ThermalSolverSettings& thermalSettings);
    
    FEASettings m_settings;                    ///< FEA settings
    ThermalSolverSettings m_thermalSettings;   ///< Thermal solver settings
    int m_numDofs;                             ///< Number of degrees of freedom
    
    // Use pointers for Eigen types to avoid incomplete type errors
    std::unique_ptr<Eigen::SparseMatrix<double>> m_conductivityMatrix; ///< Conductivity matrix
    std::unique_ptr<Eigen::SparseMatrix<double>> m_capacityMatrix;     ///< Capacity matrix
    std::unique_ptr<Eigen::VectorXd> m_heatLoadVector;                ///< Heat load vector
    
    std::vector<double> m_temperatures;        ///< Current temperatures
    std::shared_ptr<ThermalResult> m_result;   ///< Result of the analysis
};

} // namespace rebel::simulation
