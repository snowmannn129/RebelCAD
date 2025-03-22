/**
 * @file LinearStaticSolver.h
 * @brief Linear static solver for finite element analysis
 * 
 * This file defines the LinearStaticSolver class, which implements
 * the linear static solver for finite element analysis.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace rebel::simulation {

// Forward declarations
class Mesh;
class Material;
class BoundaryCondition;
class Load;
class FEAResult;
struct FEASettings;
struct Node;
struct Element;
struct DisplacementResult;
struct StressResult;
struct StrainResult;
struct ReactionForceResult;

/**
 * @class LinearStaticSolver
 * @brief Solver for linear static finite element analysis.
 * 
 * The LinearStaticSolver class implements the finite element method
 * for linear static analysis. It assembles the global stiffness matrix
 * and load vector, applies boundary conditions, solves the system of
 * equations, and computes the results.
 */
class LinearStaticSolver {
public:
    /**
     * @brief Constructs a new LinearStaticSolver object.
     * 
     * @param settings The FEA settings.
     */
    LinearStaticSolver(const FEASettings& settings);

    /**
     * @brief Destroys the LinearStaticSolver object.
     */
    ~LinearStaticSolver() = default;

    /**
     * @brief Solves the linear static problem.
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

private:
    /**
     * @brief Assembles the global stiffness matrix.
     * 
     * @param mesh The mesh to assemble for.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleStiffnessMatrix(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Assembles the global load vector.
     * 
     * @param mesh The mesh to assemble for.
     * @param loads The loads to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if assembly was successful, false otherwise.
     */
    bool assembleLoadVector(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<Load>>& loads,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Applies boundary conditions to the system.
     * 
     * @param mesh The mesh to apply to.
     * @param boundaryConditions The boundary conditions to apply.
     * @param progressCallback Optional callback function for progress updates.
     * @return True if application was successful, false otherwise.
     */
    bool applyBoundaryConditions(
        std::shared_ptr<Mesh> mesh,
        const std::vector<std::shared_ptr<BoundaryCondition>>& boundaryConditions,
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solves the system of equations.
     * 
     * @param progressCallback Optional callback function for progress updates.
     * @return True if solving was successful, false otherwise.
     */
    bool solveSystem(
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Computes the results from the solution.
     * 
     * @param mesh The mesh to compute results for.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> computeResults(
        std::shared_ptr<Mesh> mesh,
        std::function<void(float)> progressCallback = nullptr);

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
     * @brief Computes the element load vector.
     * 
     * @param element The element to compute for.
     * @param load The load to apply.
     * @param nodes The nodes of the element.
     * @return The element load vector.
     */
    Eigen::VectorXd computeElementLoadVector(
        const Element& element,
        const Load& load,
        const std::vector<Node>& nodes);

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
     * @return The reaction force result for the node.
     */
    ReactionForceResult computeNodeReactionForce(
        int nodeId,
        std::shared_ptr<Mesh> mesh,
        const std::vector<DisplacementResult>& displacements);
        
    /**
     * @brief Applies a displacement boundary condition.
     * 
     * @param dof The degree of freedom to apply to.
     * @param value The displacement value.
     */
    void applyDisplacementBC(int dof, double value);

    const FEASettings& settings;                     ///< FEA settings
    Eigen::SparseMatrix<double> stiffnessMatrix;     ///< Global stiffness matrix
    Eigen::VectorXd loadVector;                      ///< Global load vector
    Eigen::VectorXd displacementVector;              ///< Global displacement vector
    std::unordered_map<int, int> nodeIdToDofIndex;   ///< Map from node ID to DOF index
    std::unordered_map<int, std::vector<int>> nodeIdToDofs; ///< Map from node ID to DOFs
    int numDofs;                                     ///< Number of degrees of freedom
};

} // namespace rebel::simulation
