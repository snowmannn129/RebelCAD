#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <set>
#include "../Component.h"
#include "AssemblyConstraint.h"

namespace RebelCAD {
namespace Assembly {
namespace Constraints {

/**
 * @brief Solver for assembly constraints
 * 
 * The ConstraintSolver class is responsible for solving a system of assembly constraints.
 * It can handle multiple constraints simultaneously, resolve conflicts, and find a valid
 * configuration that satisfies all constraints.
 */
class ConstraintSolver {
public:
    /**
     * @brief Solver algorithm enumeration
     */
    enum class Algorithm {
        Sequential,     ///< Sequential constraint satisfaction
        Relaxation      ///< Relaxation method
    };

    /**
     * @brief Solver settings structure
     */
    struct Settings {
        Algorithm algorithm = Algorithm::Sequential; ///< Solver algorithm
        double convergenceTolerance = 1e-6;          ///< Convergence tolerance
        int maxIterations = 100;                     ///< Maximum number of iterations
        double dampingFactor = 1.0;                  ///< Damping factor for relaxation method
    };

    /**
     * @brief Solver result structure
     */
    struct Result {
        bool success = false;                                ///< Whether the solve was successful
        int iterations = 0;                                  ///< Number of iterations performed
        double error = 0.0;                                  ///< Final error
        std::vector<std::string> unsatisfiedConstraints;     ///< Names of unsatisfied constraints
    };

    /**
     * @brief Constructs a constraint solver with default settings
     */
    ConstraintSolver();

    /**
     * @brief Constructs a constraint solver with specified settings
     * @param settings The solver settings
     */
    explicit ConstraintSolver(const Settings& settings);

    /**
     * @brief Destructor
     */
    ~ConstraintSolver() = default;

    /**
     * @brief Adds a constraint to the solver
     * @param constraint The constraint to add
     * @return true if the constraint was added successfully, false otherwise
     */
    bool addConstraint(std::shared_ptr<AssemblyConstraint> constraint);

    /**
     * @brief Removes a constraint from the solver
     * @param constraint The constraint to remove
     * @return true if the constraint was removed successfully, false otherwise
     */
    bool removeConstraint(std::shared_ptr<AssemblyConstraint> constraint);

    /**
     * @brief Removes all constraints from the solver
     */
    void clearConstraints();

    /**
     * @brief Gets all constraints in the solver
     * @return Vector of constraints
     */
    std::vector<std::shared_ptr<AssemblyConstraint>> getConstraints() const;

    /**
     * @brief Gets the number of constraints in the solver
     * @return Number of constraints
     */
    size_t getConstraintCount() const;

    /**
     * @brief Sets the solver settings
     * @param settings The new settings
     */
    void setSettings(const Settings& settings);

    /**
     * @brief Gets the solver settings
     * @return The current settings
     */
    const Settings& getSettings() const;

    /**
     * @brief Solves the constraint system
     * @param progressCallback Optional callback function for progress updates
     * @return The solver result
     */
    Result solve(std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Checks if the constraint system is over-constrained
     * @return true if the system is over-constrained, false otherwise
     */
    bool isOverConstrained() const;

    /**
     * @brief Checks if the constraint system is under-constrained
     * @return true if the system is under-constrained, false otherwise
     */
    bool isUnderConstrained() const;

    /**
     * @brief Gets the total degrees of freedom in the system
     * @return Total degrees of freedom
     */
    int getTotalDegreesOfFreedom() const;

    /**
     * @brief Gets the constrained degrees of freedom in the system
     * @return Constrained degrees of freedom
     */
    int getConstrainedDegreesOfFreedom() const;

    /**
     * @brief Gets the remaining degrees of freedom in the system
     * @return Remaining degrees of freedom
     */
    int getRemainingDegreesOfFreedom() const;

private:
    /**
     * @brief Solves the constraint system using the sequential method
     * @param progressCallback Optional callback function for progress updates
     * @return The solver result
     */
    Result solveSequential(std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Solves the constraint system using the relaxation method
     * @param progressCallback Optional callback function for progress updates
     * @return The solver result
     */
    Result solveRelaxation(std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Gets all components involved in the constraint system
     * @return Vector of components
     */
    std::vector<std::shared_ptr<Component>> getAllComponents() const;

    /**
     * @brief Computes the total error of the constraint system
     * @return Total error
     */
    double computeTotalError() const;

    /**
     * @brief Sorts constraints by priority
     * @return Sorted vector of constraints
     */
    std::vector<std::shared_ptr<AssemblyConstraint>> sortConstraintsByPriority() const;

    std::vector<std::shared_ptr<AssemblyConstraint>> m_constraints; ///< Constraints in the solver
    Settings m_settings; ///< Solver settings
};

} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
