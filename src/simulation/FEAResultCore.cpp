#include "simulation/FEAResult.h"
#include "simulation/Mesh.h"
#include "core/Log.h"

namespace rebel::simulation {

// FEAResult implementation - Core functionality

// Constructor with mesh and solver type
FEAResult::FEAResult(std::shared_ptr<Mesh> mesh, SolverType solverType)
    : mesh(mesh), solverType(solverType), numDofs(0) {
}

// Constructor with number of DOFs
FEAResult::FEAResult(int numDofs)
    : mesh(nullptr), solverType(SolverType::NonLinear), numDofs(numDofs) {
}

// Factory method to create a new FEAResult
std::shared_ptr<FEAResult> FEAResult::create(
    std::shared_ptr<Mesh> mesh,
    SolverType solverType) {
    
    if (!mesh) {
        REBEL_LOG_ERROR("Cannot create FEA result with null mesh");
        return nullptr;
    }
    
    REBEL_LOG_INFO("Creating FEA result for mesh: " + mesh->getId());
    return std::shared_ptr<FEAResult>(new FEAResult(mesh, solverType));
}

// Get available result types
std::vector<ResultType> FEAResult::getAvailableResultTypes() const {
    std::vector<ResultType> types;
    
    if (!displacements.empty()) {
        types.push_back(ResultType::Displacement);
    }
    
    if (!stresses.empty()) {
        types.push_back(ResultType::Stress);
    }
    
    if (!strains.empty()) {
        types.push_back(ResultType::Strain);
    }
    
    if (!reactionForces.empty()) {
        types.push_back(ResultType::ReactionForce);
    }
    
    if (!frequencies.empty()) {
        types.push_back(ResultType::Frequency);
    }
    
    if (!modeShapes.empty()) {
        types.push_back(ResultType::ModeShape);
    }
    
    return types;
}

// Check if a result type is available
bool FEAResult::hasResultType(ResultType type) const {
    switch (type) {
        case ResultType::Displacement:
            return !displacements.empty();
        case ResultType::Stress:
            return !stresses.empty();
        case ResultType::Strain:
            return !strains.empty();
        case ResultType::ReactionForce:
            return !reactionForces.empty();
        case ResultType::Frequency:
            return !frequencies.empty();
        case ResultType::ModeShape:
            return !modeShapes.empty();
        default:
            return false;
    }
}

} // namespace rebel::simulation
