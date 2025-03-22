/**
 * @file NonLinearSolver.h
 * @brief Non-linear solver for finite element analysis
 * 
 * This file defines the NonLinearSolver class, which implements
 * the non-linear solver for finite element analysis.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace rebel::simulation {

// Forward declarations
class Mesh;
class Material;
class BoundaryCondition;
class Load;
class FEAResult;
struct FEASettings;

/**
 * @enum NonLinearSolverMethod
 * @brief Methods for solving non-linear FEA problems.
 */
enum class NonLinearSolverMethod {
    NewtonRaphson,          ///< Newton-Raphson method
    ModifiedNewtonRaphson,  ///< Modified Newton-Raphson method
    QuasiNewton,            ///< Quasi-Newton method
    ArcLength               ///< Arc-length method
};

/**
 * @enum NonLinearityType
 * @brief Types of non-linearity in FEA problems.
 */
enum class NonLinearityType {
    Geometric,              ///< Geometric non-linearity
    Material,               ///< Material non-linearity
    Contact,                ///< Contact non-linearity
    Combined                ///< Combined non-linearity
};

/**
 * @struct NonLinearSolverSettings
 * @brief Settings for the non-linear solver.
 */
struct NonLinearSolverSettings {
    NonLinearSolverMethod method = NonLinearSolverMethod::NewtonRaphson; ///< Solver method
    NonLinearityType nonLinearityType = NonLinearityType::Combined;      ///< Non-linearity type
    double convergenceTolerance = 1e-6;                                  ///< Convergence tolerance
    int maxIterations = 100;                                             ///< Maximum iterations
    double loadIncrementFactor = 0.1;                                    ///< Load increment factor
    int numLoadSteps = 10;                                               ///< Number of load steps
    bool adaptiveLoadStepping = true;                                    ///< Use adaptive load stepping
    double minLoadStepSize = 0.01;                                       ///< Minimum load step size
    double maxLoadStepSize = 0.2;                                        ///< Maximum load step size
    double arcLengthParameter = 1.0;                                     ///< Arc-length parameter
    bool useLineSearch = true;                                           ///< Use line search
    double lineSearchTolerance = 0.8;                                    ///< Line search tolerance
    int maxLineSearchIterations = 10;                                    ///< Maximum line search iterations
};

/**
 * @class NonLinearSolver
 * @brief Solver for non-linear finite element analysis.
 * 
 * The NonLinearSolver class implements the finite element method
 * for non-linear analysis. It handles geometric non-linearity,
 * material non-linearity, and contact non-linearity.
 */
class NonLinearSolver {
public:
    /**
     * @brief Constructs a new NonLinearSolver object.
     * 
     * @param settings The FEA settings.
     * @param nonLinearSettings The non-linear solver settings.
     */
    NonLinearSolver(const FEASettings& settings, 
                    const NonLinearSolverSettings& nonLinearSettings = NonLinearSolverSettings());

    /**
     * @brief Destroys the NonLinearSolver object.
     */
    ~NonLinearSolver() = default;

    /**
     * @brief Solves the non-linear problem.
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
     * @brief Gets the non-linear solver settings.
     * @return The non-linear solver settings.
     */
    const NonLinearSolverSettings& getNonLinearSettings() const { return m_nonLinearSettings; }

    /**
     * @brief Sets the non-linear solver settings.
     * @param settings The new non-linear solver settings.
     */
    void setNonLinearSettings(const NonLinearSolverSettings& settings) { m_nonLinearSettings = settings; }

private:
    /**
     * @brief Solves the non-linear problem using the Newton-Raphson method.
     * 
     * @param mesh The mesh to solve on.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solveNewtonRaphson(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solves the non-linear problem using the modified Newton-Raphson method.
     * 
     * @param mesh The mesh to solve on.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solveModifiedNewtonRaphson(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solves the non-linear problem using the quasi-Newton method.
     * 
     * @param mesh The mesh to solve on.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solveQuasiNewton(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solves the non-linear problem using the arc-length method.
     * 
     * @param mesh The mesh to solve on.
     * @param boundaryConditions The boundary conditions to apply.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solveArcLength(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Applies boundary conditions to the system.
     * 
     * @param boundaryConditions The boundary conditions to apply.
     * @param tangentStiffness The tangent stiffness matrix to modify.
     * @param externalForces The external forces vector to modify.
     * @param displacements The displacements vector to modify.
     */
    void applyBoundaryConditions(
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        std::vector<std::vector<double>>& tangentStiffness,
        std::vector<double>& externalForces,
        std::vector<double>& displacements);

    /**
     * @brief Applies boundary conditions to the system during iteration.
     * 
     * @param boundaryConditions The boundary conditions to apply.
     * @param tangentStiffness The tangent stiffness matrix to modify.
     * @param residualForces The residual forces vector to modify.
     */
    void applyBoundaryConditionsToSystem(
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        std::vector<std::vector<double>>& tangentStiffness,
        std::vector<double>& residualForces);

    /**
     * @brief Updates external forces based on loads and load factor.
     * 
     * @param loads The loads to apply.
     * @param externalForces The external forces vector to modify.
     * @param loadFactor The load factor to apply.
     */
    void updateExternalForces(
        const std::vector<std::shared_ptr<Load>>& loads,
        std::vector<double>& externalForces,
        double loadFactor);

    /**
     * @brief Computes internal forces based on current displacements.
     * 
     * @param mesh The mesh to compute on.
     * @param displacements The current displacements.
     * @param internalForces The internal forces vector to fill.
     */
    void computeInternalForces(
        std::shared_ptr<Mesh> mesh,
        const std::vector<double>& displacements,
        std::vector<double>& internalForces);

    /**
     * @brief Computes internal forces for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @param elementDisplacements The element displacements.
     * @return The element internal forces.
     */
    std::vector<double> computeElementInternalForces(
        std::shared_ptr<Mesh> mesh,
        int elemIdx,
        const std::vector<double>& elementDisplacements);

    /**
     * @brief Computes strains for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @param elementDisplacements The element displacements.
     * @return The element strains.
     */
    std::vector<double> computeElementStrains(
        std::shared_ptr<Mesh> mesh,
        int elemIdx,
        const std::vector<double>& elementDisplacements);

    /**
     * @brief Computes the strain-displacement matrix for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @return The strain-displacement matrix.
     */
    std::vector<std::vector<double>> computeStrainDisplacementMatrix(
        std::shared_ptr<Mesh> mesh,
        int elemIdx);

    /**
     * @brief Computes the tangent stiffness matrix.
     * 
     * @param mesh The mesh to compute on.
     * @param displacements The current displacements.
     * @param tangentStiffness The tangent stiffness matrix to fill.
     */
    void computeTangentStiffness(
        std::shared_ptr<Mesh> mesh,
        const std::vector<double>& displacements,
        std::vector<std::vector<double>>& tangentStiffness);

    /**
     * @brief Computes the tangent stiffness matrix for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @param elementDisplacements The element displacements.
     * @return The element tangent stiffness matrix.
     */
    std::vector<std::vector<double>> computeElementTangentStiffness(
        std::shared_ptr<Mesh> mesh,
        int elemIdx,
        const std::vector<double>& elementDisplacements);

    /**
     * @brief Computes the geometric stiffness matrix for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @param elementStresses The element stresses.
     * @return The element geometric stiffness matrix.
     */
    std::vector<std::vector<double>> computeElementGeometricStiffness(
        std::shared_ptr<Mesh> mesh,
        int elemIdx,
        const std::vector<double>& elementStresses);

    /**
     * @brief Computes the stiffness matrix for an element.
     * 
     * @param mesh The mesh to compute on.
     * @param elemIdx The element index.
     * @param elementDisplacements The element displacements.
     * @return The element stiffness matrix.
     */
    std::vector<std::vector<double>> computeElementStiffness(
        std::shared_ptr<Mesh> mesh,
        int elemIdx,
        const std::vector<double>& elementDisplacements);

    /**
     * @brief Performs line search to improve convergence.
     * 
     * @param mesh The mesh to compute on.
     * @param displacements The current displacements.
     * @param du The displacement increment.
     * @param internalForces The current internal forces.
     * @param externalForces The current external forces.
     * @return The line search parameter.
     */
    double performLineSearch(
        std::shared_ptr<Mesh> mesh,
        const std::vector<double>& displacements,
        const std::vector<double>& du,
        const std::vector<double>& internalForces,
        const std::vector<double>& externalForces);

    /**
     * @brief Solves a linear system of equations.
     * 
     * @param A The system matrix.
     * @param b The right-hand side vector.
     * @return The solution vector.
     */
    std::vector<double> solveLinearSystem(
        const std::vector<std::vector<double>>& A,
        const std::vector<double>& b);

    /**
     * @brief Computes the Euclidean norm of a vector.
     * 
     * @param v The vector to compute the norm of.
     * @return The Euclidean norm.
     */
    double computeNorm(const std::vector<double>& v);

    const FEASettings& m_settings;                     ///< FEA settings
    NonLinearSolverSettings m_nonLinearSettings;       ///< Non-linear solver settings
    int m_numDofs;                                     ///< Number of degrees of freedom
};

} // namespace rebel::simulation
