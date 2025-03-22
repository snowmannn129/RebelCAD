#include "simulation/FEAResult.h"

namespace rebel::simulation {

// FEAResult implementation - Modifier methods

// Add a displacement result
void FEAResult::addDisplacement(const DisplacementResult& result) {
    displacements.push_back(result);
    nodeIdToDisplacementIndex[result.nodeId] = displacements.size() - 1;
}

// Add a stress result
void FEAResult::addStress(const StressResult& result) {
    stresses.push_back(result);
    elementIdToStressIndex[result.elementId] = stresses.size() - 1;
}

// Add a strain result
void FEAResult::addStrain(const StrainResult& result) {
    strains.push_back(result);
    elementIdToStrainIndex[result.elementId] = strains.size() - 1;
}

// Add a reaction force result
void FEAResult::addReactionForce(const ReactionForceResult& result) {
    reactionForces.push_back(result);
    nodeIdToReactionForceIndex[result.nodeId] = reactionForces.size() - 1;
}

// Add a frequency result
void FEAResult::addFrequency(const FrequencyResult& result) {
    frequencies.push_back(result);
    modeNumberToFrequencyIndex[result.modeNumber] = frequencies.size() - 1;
}

// Add a mode shape result
void FEAResult::addModeShape(const ModeShapeResult& result) {
    modeShapes.push_back(result);
    nodeModeToModeShapeIndex[std::make_pair(result.nodeId, result.modeNumber)] = modeShapes.size() - 1;
}

// Add a load step result
void FEAResult::addLoadStep(double loadFactor, const std::vector<double>& displacements) {
    loadSteps.push_back(LoadStepResult(loadFactor, displacements));
}

} // namespace rebel::simulation
