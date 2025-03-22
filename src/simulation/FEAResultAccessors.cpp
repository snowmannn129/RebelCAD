#include "simulation/FEAResult.h"

namespace rebel::simulation {

// FEAResult implementation - Accessor methods

// Get displacement result for a node
const DisplacementResult* FEAResult::getDisplacement(int nodeId) const {
    auto it = nodeIdToDisplacementIndex.find(nodeId);
    if (it != nodeIdToDisplacementIndex.end()) {
        return &displacements[it->second];
    }
    return nullptr;
}

// Get stress result for an element
const StressResult* FEAResult::getStress(int elementId) const {
    auto it = elementIdToStressIndex.find(elementId);
    if (it != elementIdToStressIndex.end()) {
        return &stresses[it->second];
    }
    return nullptr;
}

// Get strain result for an element
const StrainResult* FEAResult::getStrain(int elementId) const {
    auto it = elementIdToStrainIndex.find(elementId);
    if (it != elementIdToStrainIndex.end()) {
        return &strains[it->second];
    }
    return nullptr;
}

// Get reaction force result for a node
const ReactionForceResult* FEAResult::getReactionForce(int nodeId) const {
    auto it = nodeIdToReactionForceIndex.find(nodeId);
    if (it != nodeIdToReactionForceIndex.end()) {
        return &reactionForces[it->second];
    }
    return nullptr;
}

// Get frequency result for a mode
const FrequencyResult* FEAResult::getFrequency(int modeNumber) const {
    auto it = modeNumberToFrequencyIndex.find(modeNumber);
    if (it != modeNumberToFrequencyIndex.end()) {
        return &frequencies[it->second];
    }
    return nullptr;
}

// Get mode shape result for a node and mode
const ModeShapeResult* FEAResult::getModeShape(int nodeId, int modeNumber) const {
    auto it = nodeModeToModeShapeIndex.find(std::make_pair(nodeId, modeNumber));
    if (it != nodeModeToModeShapeIndex.end()) {
        return &modeShapes[it->second];
    }
    return nullptr;
}

} // namespace rebel::simulation
