#include "ConstraintSolver.h"
#include <algorithm>
#include <cmath>
#include <set>

namespace RebelCAD {
namespace Assembly {
namespace Constraints {

ConstraintSolver::ConstraintSolver()
    : m_settings() {
}

ConstraintSolver::ConstraintSolver(const Settings& settings)
    : m_settings(settings) {
}

bool ConstraintSolver::addConstraint(std::shared_ptr<AssemblyConstraint> constraint) {
    if (!constraint) {
        return false;
    }

    // Check if the constraint is already in the solver
    auto it = std::find(m_constraints.begin(), m_constraints.end(), constraint);
    if (it != m_constraints.end()) {
        return false;
    }

    m_constraints.push_back(constraint);
    return true;
}

bool ConstraintSolver::removeConstraint(std::shared_ptr<AssemblyConstraint> constraint) {
    if (!constraint) {
        return false;
    }

    auto it = std::find(m_constraints.begin(), m_constraints.end(), constraint);
    if (it == m_constraints.end()) {
        return false;
    }

    m_constraints.erase(it);
    return true;
}

void ConstraintSolver::clearConstraints() {
    m_constraints.clear();
}

std::vector<std::shared_ptr<AssemblyConstraint>> ConstraintSolver::getConstraints() const {
    return m_constraints;
}

size_t ConstraintSolver::getConstraintCount() const {
    return m_constraints.size();
}

void ConstraintSolver::setSettings(const Settings& settings) {
    m_settings = settings;
}

const ConstraintSolver::Settings& ConstraintSolver::getSettings() const {
    return m_settings;
}

ConstraintSolver::Result ConstraintSolver::solve(std::function<void(float)> progressCallback) {
    // If there are no constraints, return success immediately
    if (m_constraints.empty()) {
        Result result;
        result.success = true;
        result.iterations = 0;
        result.error = 0.0;
        return result;
    }

    // Choose the solver algorithm based on the settings
    switch (m_settings.algorithm) {
        case Algorithm::Sequential:
            return solveSequential(progressCallback);
        case Algorithm::Relaxation:
            return solveRelaxation(progressCallback);
        default:
            // Default to Sequential
            return solveSequential(progressCallback);
    }
}

bool ConstraintSolver::isOverConstrained() const {
    return getConstrainedDegreesOfFreedom() > getTotalDegreesOfFreedom();
}

bool ConstraintSolver::isUnderConstrained() const {
    return getConstrainedDegreesOfFreedom() < getTotalDegreesOfFreedom();
}

int ConstraintSolver::getTotalDegreesOfFreedom() const {
    // Each component has 6 degrees of freedom (3 for position, 3 for orientation)
    return static_cast<int>(getAllComponents().size()) * 6;
}

int ConstraintSolver::getConstrainedDegreesOfFreedom() const {
    int dofs = 0;
    for (const auto& constraint : m_constraints) {
        dofs += constraint->getConstrainedDegreesOfFreedom();
    }
    return dofs;
}

int ConstraintSolver::getRemainingDegreesOfFreedom() const {
    return std::max(0, getTotalDegreesOfFreedom() - getConstrainedDegreesOfFreedom());
}

ConstraintSolver::Result ConstraintSolver::solveSequential(std::function<void(float)> progressCallback) {
    Result result;
    result.success = false;
    result.iterations = 0;
    result.error = 0.0;

    // If there are no constraints, return success immediately
    if (m_constraints.empty()) {
        result.success = true;
        return result;
    }

    // Sort constraints by priority
    std::vector<std::shared_ptr<AssemblyConstraint>> sortedConstraints = sortConstraintsByPriority();

    // Iterate until convergence or maximum iterations
    for (int iter = 0; iter < m_settings.maxIterations; ++iter) {
        result.iterations = iter + 1;

        // Report progress if callback is provided
        if (progressCallback) {
            float progress = static_cast<float>(iter) / static_cast<float>(m_settings.maxIterations);
            progressCallback(progress);
        }

        bool allSatisfied = true;
        double maxError = 0.0;

        // Enforce each constraint sequentially
        for (const auto& constraint : sortedConstraints) {
            if (!constraint->enforce()) {
                allSatisfied = false;
            }

            double error = constraint->getError();
            maxError = std::max(maxError, error);
        }

        result.error = maxError;

        // Check for convergence
        if (allSatisfied || maxError < m_settings.convergenceTolerance) {
            result.success = true;
            break;
        }
    }

    // Check which constraints are still unsatisfied
    for (const auto& constraint : m_constraints) {
        if (!constraint->isSatisfied()) {
            result.unsatisfiedConstraints.push_back(constraint->getName());
        }
    }

    return result;
}

ConstraintSolver::Result ConstraintSolver::solveRelaxation(std::function<void(float)> progressCallback) {
    Result result;
    result.success = false;
    result.iterations = 0;
    result.error = 0.0;

    // If there are no constraints, return success immediately
    if (m_constraints.empty()) {
        result.success = true;
        return result;
    }

    // Compute the initial error
    double initialError = computeTotalError();
    double currentError = initialError;
    result.error = currentError;

    // Check if the error is already below the tolerance
    if (currentError < m_settings.convergenceTolerance) {
        result.success = true;
        return result;
    }

    // Iterate until convergence or maximum iterations
    for (int iter = 0; iter < m_settings.maxIterations; ++iter) {
        result.iterations = iter + 1;

        // Report progress if callback is provided
        if (progressCallback) {
            float progress = static_cast<float>(iter) / static_cast<float>(m_settings.maxIterations);
            progressCallback(progress);
        }

        // Sort constraints by priority
        std::vector<std::shared_ptr<AssemblyConstraint>> sortedConstraints = sortConstraintsByPriority();

        // Enforce each constraint with relaxation
        for (const auto& constraint : sortedConstraints) {
            constraint->enforce();
        }

        // Compute the new error
        double newError = computeTotalError();
        result.error = newError;

        // Check for convergence
        if (std::abs(newError - currentError) < m_settings.convergenceTolerance) {
            result.success = true;
            break;
        }

        // Update the current error
        currentError = newError;
    }

    // Check which constraints are still unsatisfied
    for (const auto& constraint : m_constraints) {
        if (!constraint->isSatisfied()) {
            result.unsatisfiedConstraints.push_back(constraint->getName());
        }
    }

    return result;
}

std::vector<std::shared_ptr<Component>> ConstraintSolver::getAllComponents() const {
    std::set<std::shared_ptr<Component>> componentSet;

    // Collect all components from all constraints
    for (const auto& constraint : m_constraints) {
        std::vector<std::shared_ptr<Component>> components = constraint->getComponents();
        componentSet.insert(components.begin(), components.end());
    }

    // Convert the set to a vector
    std::vector<std::shared_ptr<Component>> components(componentSet.begin(), componentSet.end());
    return components;
}

double ConstraintSolver::computeTotalError() const {
    double totalError = 0.0;
    
    // Sum the squared errors of all constraints
    for (const auto& constraint : m_constraints) {
        double error = constraint->getError();
        totalError += error * error;
    }
    
    return totalError;
}

std::vector<std::shared_ptr<AssemblyConstraint>> ConstraintSolver::sortConstraintsByPriority() const {
    // Create a copy of the constraints
    std::vector<std::shared_ptr<AssemblyConstraint>> sortedConstraints = m_constraints;
    
    // Sort the constraints by priority (higher priority first)
    std::sort(sortedConstraints.begin(), sortedConstraints.end(), 
        [](const std::shared_ptr<AssemblyConstraint>& a, const std::shared_ptr<AssemblyConstraint>& b) {
            return a->getPriority() > b->getPriority();
        });
    
    return sortedConstraints;
}

} // namespace Constraints
} // namespace Assembly
} // namespace RebelCAD
