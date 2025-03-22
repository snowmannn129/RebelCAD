#include "simulation/FEAResult.h"
#include <algorithm>
#include <limits>

namespace rebel::simulation {

// FEAResult implementation - Statistical methods

// Get maximum displacement magnitude
double FEAResult::getMaxDisplacementMagnitude() const {
    if (displacements.empty()) {
        return 0.0;
    }
    
    double maxMagnitude = 0.0;
    for (const auto& displacement : displacements) {
        maxMagnitude = std::max(maxMagnitude, displacement.magnitude);
    }
    
    return maxMagnitude;
}

// Get maximum stress value for a component
double FEAResult::getMaxStress(StressComponent component) const {
    if (stresses.empty()) {
        return 0.0;
    }
    
    double maxValue = -std::numeric_limits<double>::max();
    
    for (const auto& stress : stresses) {
        double value = 0.0;
        
        switch (component) {
            case StressComponent::XX:
                value = stress.xx;
                break;
            case StressComponent::YY:
                value = stress.yy;
                break;
            case StressComponent::ZZ:
                value = stress.zz;
                break;
            case StressComponent::XY:
                value = stress.xy;
                break;
            case StressComponent::YZ:
                value = stress.yz;
                break;
            case StressComponent::XZ:
                value = stress.xz;
                break;
            case StressComponent::VonMises:
                value = stress.vonMises;
                break;
            case StressComponent::MaxPrincipal:
                value = stress.maxPrincipal;
                break;
            case StressComponent::MidPrincipal:
                value = stress.midPrincipal;
                break;
            case StressComponent::MinPrincipal:
                value = stress.minPrincipal;
                break;
            case StressComponent::MaxShear:
                value = stress.maxShear;
                break;
            default:
                break;
        }
        
        maxValue = std::max(maxValue, value);
    }
    
    return maxValue;
}

// Get minimum stress value for a component
double FEAResult::getMinStress(StressComponent component) const {
    if (stresses.empty()) {
        return 0.0;
    }
    
    double minValue = std::numeric_limits<double>::max();
    
    for (const auto& stress : stresses) {
        double value = 0.0;
        
        switch (component) {
            case StressComponent::XX:
                value = stress.xx;
                break;
            case StressComponent::YY:
                value = stress.yy;
                break;
            case StressComponent::ZZ:
                value = stress.zz;
                break;
            case StressComponent::XY:
                value = stress.xy;
                break;
            case StressComponent::YZ:
                value = stress.yz;
                break;
            case StressComponent::XZ:
                value = stress.xz;
                break;
            case StressComponent::VonMises:
                value = stress.vonMises;
                break;
            case StressComponent::MaxPrincipal:
                value = stress.maxPrincipal;
                break;
            case StressComponent::MidPrincipal:
                value = stress.midPrincipal;
                break;
            case StressComponent::MinPrincipal:
                value = stress.minPrincipal;
                break;
            case StressComponent::MaxShear:
                value = stress.maxShear;
                break;
            default:
                break;
        }
        
        minValue = std::min(minValue, value);
    }
    
    return minValue;
}

// Get maximum strain value for a component
double FEAResult::getMaxStrain(StrainComponent component) const {
    if (strains.empty()) {
        return 0.0;
    }
    
    double maxValue = -std::numeric_limits<double>::max();
    
    for (const auto& strain : strains) {
        double value = 0.0;
        
        switch (component) {
            case StrainComponent::XX:
                value = strain.xx;
                break;
            case StrainComponent::YY:
                value = strain.yy;
                break;
            case StrainComponent::ZZ:
                value = strain.zz;
                break;
            case StrainComponent::XY:
                value = strain.xy;
                break;
            case StrainComponent::YZ:
                value = strain.yz;
                break;
            case StrainComponent::XZ:
                value = strain.xz;
                break;
            case StrainComponent::MaxPrincipal:
                value = strain.maxPrincipal;
                break;
            case StrainComponent::MidPrincipal:
                value = strain.midPrincipal;
                break;
            case StrainComponent::MinPrincipal:
                value = strain.minPrincipal;
                break;
            case StrainComponent::MaxShear:
                value = strain.maxShear;
                break;
            default:
                break;
        }
        
        maxValue = std::max(maxValue, value);
    }
    
    return maxValue;
}

// Get minimum strain value for a component
double FEAResult::getMinStrain(StrainComponent component) const {
    if (strains.empty()) {
        return 0.0;
    }
    
    double minValue = std::numeric_limits<double>::max();
    
    for (const auto& strain : strains) {
        double value = 0.0;
        
        switch (component) {
            case StrainComponent::XX:
                value = strain.xx;
                break;
            case StrainComponent::YY:
                value = strain.yy;
                break;
            case StrainComponent::ZZ:
                value = strain.zz;
                break;
            case StrainComponent::XY:
                value = strain.xy;
                break;
            case StrainComponent::YZ:
                value = strain.yz;
                break;
            case StrainComponent::XZ:
                value = strain.xz;
                break;
            case StrainComponent::MaxPrincipal:
                value = strain.maxPrincipal;
                break;
            case StrainComponent::MidPrincipal:
                value = strain.midPrincipal;
                break;
            case StrainComponent::MinPrincipal:
                value = strain.minPrincipal;
                break;
            case StrainComponent::MaxShear:
                value = strain.maxShear;
                break;
            default:
                break;
        }
        
        minValue = std::min(minValue, value);
    }
    
    return minValue;
}

// Get maximum reaction force magnitude
double FEAResult::getMaxReactionForceMagnitude() const {
    if (reactionForces.empty()) {
        return 0.0;
    }
    
    double maxMagnitude = 0.0;
    for (const auto& force : reactionForces) {
        maxMagnitude = std::max(maxMagnitude, force.magnitude);
    }
    
    return maxMagnitude;
}

// Get total reaction force in a direction
double FEAResult::getTotalReactionForce(int direction) const {
    if (reactionForces.empty()) {
        return 0.0;
    }
    
    double total = 0.0;
    
    for (const auto& force : reactionForces) {
        switch (direction) {
            case 0: // X
                total += force.x;
                break;
            case 1: // Y
                total += force.y;
                break;
            case 2: // Z
                total += force.z;
                break;
            default:
                break;
        }
    }
    
    return total;
}

} // namespace rebel::simulation
