#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace rebel::simulation {

// Forward declarations
class Mesh;
class Material;
class BoundaryCondition;
class Load;
class FEAResult;
class ThermalResult;
struct FEASettings;
class Element;
class Node;

/**
 * @brief Settings for the thermal solver
 */
struct ThermalSolverSettings {
    enum class AnalysisType {
        SteadyState,    ///< Steady-state thermal analysis
        Transient       ///< Transient thermal analysis
    };

    AnalysisType analysisType = AnalysisType::SteadyState;  ///< Type of thermal analysis

    // Transient analysis settings
    double startTime = 0.0;                 ///< Start time for transient analysis (s)
    double endTime = 1.0;                   ///< End time for transient analysis (s)
    double timeStep = 0.01;                 ///< Time step for transient analysis (s)
    bool adaptiveTimeStep = false;          ///< Whether to use adaptive time stepping
    double minTimeStep = 0.001;             ///< Minimum time step for adaptive time stepping (s)
    double maxTimeStep = 0.1;               ///< Maximum time step for adaptive time stepping (s)
    double timeStepSafetyFactor = 0.9;      ///< Safety factor for adaptive time stepping

    // Solver settings
    double convergenceTolerance = 1e-6;     ///< Convergence tolerance for iterative solvers
    int maxIterations = 100;                ///< Maximum number of iterations for iterative solvers
    bool useDirectSolver = true;            ///< Whether to use a direct solver (true) or iterative solver (false)

    // Validate settings
    bool validate() const {
        // Check time step settings
        if (analysisType == AnalysisType::Transient) {
            if (startTime >= endTime) {
                return false;
            }
            if (timeStep <= 0.0) {
                return false;
            }
            if (adaptiveTimeStep) {
                if (minTimeStep <= 0.0 || maxTimeStep <= 0.0 || minTimeStep >= maxTimeStep) {
                    return false;
                }
                if (timeStepSafetyFactor <= 0.0 || timeStepSafetyFactor >= 1.0) {
                    return false;
                }
            }
        }

        // Check solver settings
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
 * @brief Result of a thermal analysis
 */
class ThermalResult {
public:
    ThermalResult() = default;
    explicit ThermalResult(int numTimePoints);
    ~ThermalResult() = default;

    // Time points for transient analysis
    std::vector<double> timePoints;

    // Temperature results
    std::vector<std::vector<double>> temperatures;

    // Heat flux results
    std::vector<std::vector<double>> heatFluxes;

    // Thermal gradient results
    std::vector<std::vector<double>> thermalGradients;

    // Export results to file
    bool exportToFile(const std::string& filePath) const;
};

/**
 * @brief Solver for thermal analysis problems
 */
class ThermalSolver {
public:
    /**
     * @brief Constructor
     * @param settings General FEA settings
     * @param thermalSettings Thermal solver specific settings
     */
    ThermalSolver(const FEASettings& settings, const ThermalSolverSettings& thermalSettings);

    /**
     * @brief Destructor
     */
    ~ThermalSolver() = default;

    /**
     * @brief Solve the thermal analysis problem
     * @param mesh The mesh to solve on
     * @param boundaryConditions The boundary conditions to apply
     * @param loads The loads to apply
     * @param progressCallback Callback function for progress reporting
     * @return The result of the analysis
     */
    std::shared_ptr<FEAResult> solve(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solve a steady-state thermal analysis problem
     * @param mesh The mesh to solve on
     * @param boundaryConditions The boundary conditions to apply
     * @param loads The loads to apply
     * @param progressCallback Callback function for progress reporting
     * @return The result of the analysis
     */
    std::shared_ptr<ThermalResult> solveSteadyState(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solve a transient thermal analysis problem
     * @param mesh The mesh to solve on
     * @param boundaryConditions The boundary conditions to apply
     * @param loads The loads to apply
     * @param initialTemperatures Initial temperatures (optional)
     * @param progressCallback Callback function for progress reporting
     * @return The result of the analysis
     */
    std::shared_ptr<ThermalResult> solveTransient(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        const std::vector<double>& initialTemperatures = {},
        std::function<void(float)> progressCallback = nullptr);

private:
    // Assemble conductivity matrix
    bool assembleConductivityMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    // Assemble capacity matrix
    bool assembleCapacityMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    // Assemble heat load vector
    bool assembleHeatLoadVector(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<Load>>& loads,
        double time,
        std::function<void(float)> progressCallback = nullptr);

    // Apply boundary conditions
    bool applyBoundaryConditions(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        double time,
        std::function<void(float)> progressCallback = nullptr);

    // Initialize time integration
    bool initializeTimeIntegration(
        std::shared_ptr<Mesh> mesh,
        const std::vector<double>& initialTemperatures,
        std::function<void(float)> progressCallback = nullptr);

    // Perform a time step for transient analysis
    bool timeStep(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    // Compute critical time step for transient analysis
    double computeCriticalTimeStep(std::shared_ptr<Mesh> mesh);

    // Compute element conductivity matrix
    Eigen::MatrixXd computeElementConductivityMatrix(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes);

    // Compute element capacity matrix
    Eigen::MatrixXd computeElementCapacityMatrix(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes);

    // Compute element heat load vector
    Eigen::VectorXd computeElementHeatLoadVector(
        const Element& element,
        const Load& load,
        const std::vector<Node>& nodes,
        double time);

    // Compute heat load value
    double computeHeatLoadValue(const Load& load, double time);

    // Compute shape functions
    bool computeShapeFunctions(
        const Element& element,
        const std::vector<Node>& nodes,
        double xi, double eta, double zeta,
        Eigen::VectorXd& N,
        Eigen::MatrixXd& dN,
        Eigen::Matrix3d& J,
        double& detJ,
        Eigen::Matrix3d& invJ);

    // Compute B matrix for thermal analysis
    Eigen::MatrixXd computeThermalBMatrix(
        const Eigen::MatrixXd& dN,
        const Eigen::Matrix3d& invJ);

    // Compute D matrix for thermal analysis
    Eigen::MatrixXd computeThermalDMatrix(
        const Material& material);

    // Compute element heat fluxes
    std::vector<double> computeElementHeatFluxes(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes,
        const std::vector<double>& temperatures);

    // Compute element thermal gradients
    std::vector<double> computeElementThermalGradients(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes,
        const std::vector<double>& temperatures);

    // Solve linear system
    bool solveLinearSystem(
        const Eigen::SparseMatrix<double>& A,
        const Eigen::VectorXd& b,
        Eigen::VectorXd& x);

    // Settings
    FEASettings m_settings;
    ThermalSolverSettings m_thermalSettings;

    // Mesh data
    int m_numDofs;

    // Matrices and vectors
    Eigen::SparseMatrix<double> m_conductivityMatrix;
    Eigen::SparseMatrix<double> m_capacityMatrix;
    Eigen::VectorXd m_heatLoadVector;

    // Solution data
    std::vector<double> m_temperatures;
    std::vector<double> m_heatFluxes;
    std::vector<double> m_thermalGradients;

    // Result
    std::shared_ptr<ThermalResult> m_result;
};

} // namespace rebel::simulation
