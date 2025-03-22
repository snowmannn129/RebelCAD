/**
 * @file DynamicSolver.h
 * @brief Dynamic solver for finite element analysis
 * 
 * This file defines the DynamicSolver class, which implements
 * the dynamic solver for finite element analysis, including
 * time integration methods and modal analysis.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Eigenvalues>

namespace rebel::simulation {

// Forward declarations for result types
struct DisplacementResult;
struct VelocityResult;
struct AccelerationResult;
struct StressResult;
struct StrainResult;
struct ReactionForceResult;

// Forward declarations
class Mesh;
class Material;
class BoundaryCondition;
class Load;
class FEAResult;
struct FEASettings;
struct Node;
struct Element;

/**
 * @enum TimeIntegrationMethod
 * @brief Methods for time integration in dynamic analysis.
 */
enum class TimeIntegrationMethod {
    Newmark,        ///< Newmark method
    HHT,            ///< Hilber-Hughes-Taylor (HHT-α) method
    CentralDiff,    ///< Central difference method
    Wilson,         ///< Wilson-θ method
    Bathe           ///< Bathe method
};

/**
 * @enum ModalAnalysisMethod
 * @brief Methods for modal analysis.
 */
enum class ModalAnalysisMethod {
    Lanczos,        ///< Lanczos method
    Subspace,       ///< Subspace iteration method
    PowerIteration  ///< Power iteration method
};

/**
 * @struct DynamicSolverSettings
 * @brief Settings for the dynamic solver.
 */
struct DynamicSolverSettings {
    // Time integration settings
    TimeIntegrationMethod timeIntegrationMethod = TimeIntegrationMethod::Newmark; ///< Time integration method
    double startTime = 0.0;                     ///< Start time
    double endTime = 1.0;                       ///< End time
    double timeStep = 0.01;                     ///< Time step
    int numTimeSteps = 100;                     ///< Number of time steps
    bool adaptiveTimeStep = false;              ///< Use adaptive time stepping
    double minTimeStep = 0.001;                 ///< Minimum time step
    double maxTimeStep = 0.1;                   ///< Maximum time step
    double timeStepSafetyFactor = 0.9;          ///< Safety factor for adaptive time stepping
    
    // Newmark method parameters
    double newmarkBeta = 0.25;                  ///< Newmark β parameter (0.25 for average acceleration)
    double newmarkGamma = 0.5;                  ///< Newmark γ parameter (0.5 for average acceleration)
    
    // HHT method parameters
    double hhtAlpha = -0.1;                     ///< HHT α parameter (-1/3 ≤ α ≤ 0 for unconditional stability)
    
    // Wilson method parameters
    double wilsonTheta = 1.4;                   ///< Wilson θ parameter (θ ≥ 1.37 for unconditional stability)
    
    // Damping parameters
    bool useDamping = false;                    ///< Use damping
    double rayleighAlpha = 0.0;                 ///< Rayleigh damping mass coefficient
    double rayleighBeta = 0.0;                  ///< Rayleigh damping stiffness coefficient
    
    // Modal analysis settings
    ModalAnalysisMethod modalMethod = ModalAnalysisMethod::Lanczos; ///< Modal analysis method
    int numModes = 10;                          ///< Number of modes to compute
    double minFrequency = 0.0;                  ///< Minimum frequency of interest
    double maxFrequency = 1000.0;               ///< Maximum frequency of interest
    double convergenceTolerance = 1e-6;         ///< Convergence tolerance for modal analysis
    int maxIterations = 100;                    ///< Maximum iterations for modal analysis
    
    // Output settings
    bool saveDisplacements = true;              ///< Save displacements
    bool saveVelocities = true;                 ///< Save velocities
    bool saveAccelerations = true;              ///< Save accelerations
    bool saveStresses = true;                   ///< Save stresses
    bool saveStrains = true;                    ///< Save strains
    bool saveReactionForces = true;             ///< Save reaction forces
    int saveInterval = 1;                       ///< Save results every N time steps
    
    /**
     * @brief Constructs a new DynamicSolverSettings object with default values.
     */
    DynamicSolverSettings() = default;
    
    /**
     * @brief Sets up the time integration parameters based on the selected method.
     */
    void setupTimeIntegrationParameters() {
        switch (timeIntegrationMethod) {
            case TimeIntegrationMethod::Newmark:
                // Average acceleration method (unconditionally stable)
                newmarkBeta = 0.25;
                newmarkGamma = 0.5;
                break;
            case TimeIntegrationMethod::HHT:
                // HHT-α method with α = -0.1 (unconditionally stable)
                hhtAlpha = -0.1;
                newmarkBeta = (1 - hhtAlpha) * (1 - hhtAlpha) / 4;
                newmarkGamma = 0.5 - hhtAlpha;
                break;
            case TimeIntegrationMethod::CentralDiff:
                // Central difference method (conditionally stable)
                newmarkBeta = 0.0;
                newmarkGamma = 0.5;
                break;
            case TimeIntegrationMethod::Wilson:
                // Wilson-θ method with θ = 1.4 (unconditionally stable)
                wilsonTheta = 1.4;
                break;
            case TimeIntegrationMethod::Bathe:
                // Bathe method (unconditionally stable)
                break;
        }
    }
    
    /**
     * @brief Validates the settings.
     * @return True if settings are valid, false otherwise.
     */
    bool validate() const {
        // Check time integration parameters
        if (startTime >= endTime) {
            return false;
        }
        
        if (timeStep <= 0.0) {
            return false;
        }
        
        if (adaptiveTimeStep && (minTimeStep <= 0.0 || maxTimeStep <= minTimeStep)) {
            return false;
        }
        
        // Check Newmark parameters
        if (timeIntegrationMethod == TimeIntegrationMethod::Newmark) {
            // For unconditional stability: 2β ≥ γ ≥ 0.5
            if (newmarkBeta < 0.0 || newmarkGamma < 0.5 || 2.0 * newmarkBeta < newmarkGamma) {
                return false;
            }
        }
        
        // Check HHT parameters
        if (timeIntegrationMethod == TimeIntegrationMethod::HHT) {
            // For unconditional stability: -1/3 ≤ α ≤ 0
            if (hhtAlpha < -1.0/3.0 || hhtAlpha > 0.0) {
                return false;
            }
        }
        
        // Check Wilson parameters
        if (timeIntegrationMethod == TimeIntegrationMethod::Wilson) {
            // For unconditional stability: θ ≥ 1.37
            if (wilsonTheta < 1.37) {
                return false;
            }
        }
        
        // Check modal analysis parameters
        if (numModes <= 0) {
            return false;
        }
        
        if (minFrequency < 0.0 || maxFrequency <= minFrequency) {
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
 * @struct ModalResult
 * @brief Result of modal analysis.
 */
struct ModalResult {
    std::vector<double> frequencies;                ///< Natural frequencies (Hz)
    std::vector<double> periods;                    ///< Natural periods (s)
    std::vector<double> angularFrequencies;         ///< Angular frequencies (rad/s)
    std::vector<double> eigenvalues;                ///< Eigenvalues
    std::vector<Eigen::VectorXd> eigenvectors;      ///< Eigenvectors (mode shapes)
    std::vector<double> modalMasses;                ///< Modal masses
    std::vector<double> modalStiffnesses;           ///< Modal stiffnesses
    std::vector<double> participationFactors;       ///< Participation factors
    std::vector<double> effectiveMasses;            ///< Effective masses
    double totalMass;                               ///< Total mass
    std::vector<double> effectiveMassRatios;        ///< Effective mass ratios
    
    /**
     * @brief Constructs a new ModalResult object.
     * @param numModes Number of modes.
     */
    ModalResult(int numModes)
        : frequencies(numModes, 0.0),
          periods(numModes, 0.0),
          angularFrequencies(numModes, 0.0),
          eigenvalues(numModes, 0.0),
          eigenvectors(numModes),
          modalMasses(numModes, 0.0),
          modalStiffnesses(numModes, 0.0),
          participationFactors(numModes, 0.0),
          effectiveMasses(numModes, 0.0),
          totalMass(0.0),
          effectiveMassRatios(numModes, 0.0) {
    }
};

/**
 * @struct TimeHistoryResult
 * @brief Result of time history analysis.
 */
struct TimeHistoryResult {
    std::vector<double> timePoints;                 ///< Time points
    std::vector<std::vector<DisplacementResult>> displacements;    ///< Displacements at each time point
    std::vector<std::vector<VelocityResult>> velocities;          ///< Velocities at each time point
    std::vector<std::vector<AccelerationResult>> accelerations;    ///< Accelerations at each time point
    std::vector<std::vector<StressResult>> stresses;              ///< Stresses at each time point
    std::vector<std::vector<StrainResult>> strains;               ///< Strains at each time point
    std::vector<std::vector<ReactionForceResult>> reactionForces; ///< Reaction forces at each time point
    
    /**
     * @brief Constructs a new TimeHistoryResult object.
     * @param numTimePoints Number of time points.
     */
    TimeHistoryResult(int numTimePoints)
        : timePoints(numTimePoints, 0.0),
          displacements(numTimePoints),
          velocities(numTimePoints),
          accelerations(numTimePoints),
          stresses(numTimePoints),
          strains(numTimePoints),
          reactionForces(numTimePoints) {
    }
};

/**
 * @class DynamicSolver
 * @brief Solver for dynamic finite element analysis.
 * 
 * The DynamicSolver class implements the finite element method
 * for dynamic analysis, including time integration methods and
 * modal analysis.
 */
class DynamicSolver {
public:
    /**
     * @brief Constructs a new DynamicSolver object.
     * 
     * @param settings The FEA settings.
     * @param dynamicSettings The dynamic solver settings.
     */
    DynamicSolver(const FEASettings& settings, 
                  const DynamicSolverSettings& dynamicSettings = DynamicSolverSettings());

    /**
     * @brief Destroys the DynamicSolver object.
     */
    ~DynamicSolver() = default;

    /**
     * @brief Solves the dynamic problem.
     * 
     * @param mesh The mesh to solve on.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solve(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs modal analysis.
     * 
     * @param mesh The mesh to analyze.
     * @param boundaryConditions The boundary conditions to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the modal result, or nullptr if failed.
     */
    std::shared_ptr<ModalResult> solveModal(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs time history analysis.
     * 
     * @param mesh The mesh to analyze.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param initialDisplacements Optional initial displacements.
     * @param initialVelocities Optional initial velocities.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the time history result, or nullptr if failed.
     */
    std::shared_ptr<TimeHistoryResult> solveTimeHistory(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        const std::vector<DisplacementResult>& initialDisplacements = {},
        const std::vector<VelocityResult>& initialVelocities = {},
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Gets the dynamic solver settings.
     * @return The dynamic solver settings.
     */
    const DynamicSolverSettings& getDynamicSettings() const { return m_dynamicSettings; }

    /**
     * @brief Sets the dynamic solver settings.
     * @param settings The new dynamic solver settings.
     */
    void setDynamicSettings(const DynamicSolverSettings& settings) { m_dynamicSettings = settings; }

private:
    /**
     * @brief Assembles the mass matrix.
     * 
     * @param mesh The mesh to assemble for.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleMassMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Assembles the damping matrix.
     * 
     * @param mesh The mesh to assemble for.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleDampingMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Assembles the stiffness matrix.
     * 
     * @param mesh The mesh to assemble for.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleStiffnessMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Assembles the load vector at a specific time.
     * 
     * @param mesh The mesh to assemble for.
     * @param loads The loads to apply.
     * @param time The time point.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleLoadVector(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<Load>>& loads,
        double time,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Applies boundary conditions to the system.
     * 
     * @param mesh The mesh to apply to.
     * @param boundaryConditions The boundary conditions to apply.
     * @param time The time point.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if application was successful, false otherwise.
     */
    bool applyBoundaryConditions(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        double time,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Initializes the time integration.
     * 
     * @param mesh The mesh to initialize for.
     * @param initialDisplacements Optional initial displacements.
     * @param initialVelocities Optional initial velocities.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if initialization was successful, false otherwise.
     */
    bool initializeTimeIntegration(
        std::shared_ptr<Mesh> mesh,
        const std::vector<DisplacementResult>& initialDisplacements,
        const std::vector<VelocityResult>& initialVelocities,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs a time step using the Newmark method.
     * 
     * @param time The current time.
     * @param dt The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if the time step was successful, false otherwise.
     */
    bool timeStepNewmark(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs a time step using the HHT method.
     * 
     * @param time The current time.
     * @param dt The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if the time step was successful, false otherwise.
     */
    bool timeStepHHT(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs a time step using the central difference method.
     * 
     * @param time The current time.
     * @param dt The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if the time step was successful, false otherwise.
     */
    bool timeStepCentralDiff(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs a time step using the Wilson method.
     * 
     * @param time The current time.
     * @param dt The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if the time step was successful, false otherwise.
     */
    bool timeStepWilson(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Performs a time step using the Bathe method.
     * 
     * @param time The current time.
     * @param dt The time step.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if the time step was successful, false otherwise.
     */
    bool timeStepBathe(
        double time,
        double dt,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Computes the critical time step for explicit methods.
     * 
     * @param mesh The mesh to compute for.
     * @return The critical time step.
     */
    double computeCriticalTimeStep(std::shared_ptr<Mesh> mesh);

    /**
     * @brief Computes the element mass matrix.
     * 
     * @param element The element to compute for.
     * @param material The material of the element.
     * @param nodes The nodes of the element.
     * @return The element mass matrix.
     */
    Eigen::MatrixXd computeElementMassMatrix(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes);

    /**
     * @brief Computes the element damping matrix.
     * 
     * @param element The element to compute for.
     * @param material The material of the element.
     * @param nodes The nodes of the element.
     * @param elementMassMatrix The element mass matrix.
     * @param elementStiffnessMatrix The element stiffness matrix.
     * @return The element damping matrix.
     */
    Eigen::MatrixXd computeElementDampingMatrix(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes,
        const Eigen::MatrixXd& elementMassMatrix,
        const Eigen::MatrixXd& elementStiffnessMatrix);

    /**
     * @brief Computes the element stiffness matrix.
     * 
     * @param element The element to compute for.
     * @param material The material of the element.
     * @param nodes The nodes of the element.
     * @return The element stiffness matrix.
     */
    Eigen::MatrixXd computeElementStiffnessMatrix(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes);

    /**
     * @brief Computes the element load vector at a specific time.
     * 
     * @param element The element to compute for.
     * @param load The load to apply.
     * @param nodes The nodes of the element.
     * @param time The time point.
     * @return The element load vector.
     */
    Eigen::VectorXd computeElementLoadVector(
        const Element& element,
        const Load& load,
        const std::vector<Node>& nodes,
        double time);

    /**
     * @brief Computes the load value at a specific time.
     * 
     * @param load The load to compute for.
     * @param time The time point.
     * @return The load value.
     */
    double computeLoadValue(const Load& load, double time);

    /**
     * @brief Computes the shape functions and their derivatives for a point.
     * 
     * @param element The element to compute for.
     * @param nodes The nodes of the element.
     * @param xi The natural coordinate in the first direction.
     * @param eta The natural coordinate in the second direction.
     * @param zeta The natural coordinate in the third direction.
     * @param N Output parameter for the shape functions.
     * @param dN Output parameter for the derivatives of the shape functions.
     * @param J Output parameter for the Jacobian matrix.
     * @param detJ Output parameter for the determinant of the Jacobian.
     * @param invJ Output parameter for the inverse of the Jacobian.
     * @return True if computation was successful, false otherwise.
     */
    bool computeShapeFunctions(
        const Element& element,
        const std::vector<Node>& nodes,
        double xi, double eta, double zeta,
        Eigen::VectorXd& N,
        Eigen::MatrixXd& dN,
        Eigen::Matrix3d& J,
        double& detJ,
        Eigen::Matrix3d& invJ);

    /**
     * @brief Computes the B matrix for a point.
     * 
     * @param dN The derivatives of the shape functions.
     * @param invJ The inverse of the Jacobian.
     * @return The B matrix.
     */
    Eigen::MatrixXd computeBMatrix(
        const Eigen::MatrixXd& dN,
        const Eigen::Matrix3d& invJ);

    /**
     * @brief Computes the D matrix for a material.
     * 
     * @param material The material to compute for.
     * @return The D matrix.
     */
    Eigen::MatrixXd computeDMatrix(
        const Material& material);

    /**
     * @brief Computes the stresses for an element.
     * 
     * @param element The element to compute for.
     * @param material The material of the element.
     * @param nodes The nodes of the element.
     * @param displacements The displacements of the nodes.
     * @return The stress result for the element.
     */
    StressResult computeElementStresses(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes,
        const std::vector<DisplacementResult>& displacements);

    /**
     * @brief Computes the strains for an element.
     * 
     * @param element The element to compute for.
     * @param material The material of the element.
     * @param nodes The nodes of the element.
     * @param displacements The displacements of the nodes.
     * @return The strain result for the element.
     */
    StrainResult computeElementStrains(
        const Element& element,
        const Material& material,
        const std::vector<Node>& nodes,
        const std::vector<DisplacementResult>& displacements);

    /**
     * @brief Computes the reaction forces for a node.
     * 
     * @param nodeId The ID of the node.
     * @param mesh The mesh to compute for.
     * @param displacements The displacements of the nodes.
     * @param velocities The velocities of the nodes.
     * @param accelerations The accelerations of the nodes.
     * @return The reaction force result for the node.
     */
    ReactionForceResult computeNodeReactionForce(
        int nodeId,
        std::shared_ptr<Mesh> mesh,
        const std::vector<DisplacementResult>& displacements,
        const std::vector<VelocityResult>& velocities,
        const std::vector<AccelerationResult>& accelerations);

    /**
     * @brief Applies a displacement boundary condition.
     * 
     * @param dof The degree of freedom to apply to.
     * @param value The displacement value.
     */
    void applyDisplacementBC(int dof, double value);

    /**
     * @brief Solves a linear system of equations.
     * 
     * @param A The system matrix.
     * @param b The right-hand side vector.
     * @param x The solution vector.
     * @return True if solving was successful, false otherwise.
     */
    bool solveLinearSystem(
        const Eigen::SparseMatrix<double>& A,
        const Eigen::VectorXd& b,
        Eigen::VectorXd& x);

    /**
     * @brief Solves the generalized eigenvalue problem.
     * 
     * @param K The stiffness matrix.
     * @param M The mass matrix.
     * @param numModes The number of modes to compute.
     * @param eigenvalues Output parameter for the eigenvalues.
     * @param eigenvectors Output parameter for the eigenvectors.
     * @return True if solving was successful, false otherwise.
     */
    bool solveEigenvalueProblem(
        const Eigen::SparseMatrix<double>& K,
        const Eigen::SparseMatrix<double>& M,
        int numModes,
        Eigen::VectorXd& eigenvalues,
        Eigen::MatrixXd& eigenvectors);

    /**
     * @brief Computes the participation factors and effective masses.
     * 
     * @param M The mass matrix.
     * @param eigenvectors The eigenvectors.
     * @param numModes The number of modes.
     * @param participationFactors Output parameter for the participation factors.
     * @param effectiveMasses Output parameter for the effective masses.
     * @param totalMass Output parameter for the total mass.
     * @return True if computation was successful, false otherwise.
     */
    bool computeParticipationFactors(
        const Eigen::SparseMatrix<double>& M,
        const Eigen::MatrixXd& eigenvectors,
        int numModes,
        Eigen::VectorXd& participationFactors,
        Eigen::VectorXd& effectiveMasses,
        double& totalMass);

    const FEASettings& m_settings;                     ///< FEA settings
    DynamicSolverSettings m_dynamicSettings;           ///< Dynamic solver settings
    Eigen::SparseMatrix<double> m_massMatrix;          ///< Global mass matrix
    Eigen::SparseMatrix<double> m_dampingMatrix;       ///< Global damping matrix
    Eigen::SparseMatrix<double> m_stiffnessMatrix;     ///< Global stiffness matrix
    Eigen::VectorXd m_loadVector;                      ///< Global load vector
    Eigen::VectorXd m_displacementVector;              ///< Global displacement vector
    Eigen::VectorXd m_velocityVector;                  ///< Global velocity vector
    Eigen::VectorXd m_accelerationVector;              ///< Global acceleration vector
    std::unordered_map<int, int> m_nodeIdToDofIndex;   ///< Map from node ID to DOF index
    std::unordered_map<int, std::vector<int>> m_nodeIdToDofs; ///< Map from node ID to DOFs
    int m_numDofs;                                     ///< Number of degrees of freedom
    std::shared_ptr<TimeHistoryResult> m_timeHistory;  ///< Time history result
};

} // namespace rebel::simulation
