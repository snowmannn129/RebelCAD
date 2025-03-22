#include "simulation/FEAResult.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace rebel::simulation {

// StressResult implementation
StressResult::StressResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz)
    : elementId(elementId), xx(xx), yy(yy), zz(zz), xy(xy), yz(yz), xz(xz) {
    
    // Calculate von Mises stress
    vonMises = std::sqrt(0.5 * ((xx - yy) * (xx - yy) + 
                                (yy - zz) * (yy - zz) + 
                                (zz - xx) * (zz - xx) + 
                                6.0 * (xy * xy + yz * yz + xz * xz)));
    
    // Calculate principal stresses
    double I1 = xx + yy + zz;
    double I2 = xx * yy + yy * zz + zz * xx - xy * xy - yz * yz - xz * xz;
    double I3 = xx * yy * zz + 2.0 * xy * yz * xz - xx * yz * yz - yy * xz * xz - zz * xy * xy;
    
    // Solve cubic equation for principal stresses
    double p = I1 * I1 - 3.0 * I2;
    double q = 2.0 * I1 * I1 * I1 - 9.0 * I1 * I2 + 27.0 * I3;
    double r = std::sqrt(std::abs(p));
    
    double phi = std::acos(q / (2.0 * r * r * r));
    
    maxPrincipal = I1 / 3.0 + 2.0 * r * std::cos(phi / 3.0) / 3.0;
    midPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 2.0 * M_PI) / 3.0) / 3.0;
    minPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 4.0 * M_PI) / 3.0) / 3.0;
    
    // Calculate maximum shear stress
    maxShear = (maxPrincipal - minPrincipal) / 2.0;
}

// StrainResult implementation
StrainResult::StrainResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz)
    : elementId(elementId), xx(xx), yy(yy), zz(zz), xy(xy), yz(yz), xz(xz) {
    
    // Calculate principal strains
    double I1 = xx + yy + zz;
    double I2 = xx * yy + yy * zz + zz * xx - xy * xy - yz * yz - xz * xz;
    double I3 = xx * yy * zz + 2.0 * xy * yz * xz - xx * yz * yz - yy * xz * xz - zz * xy * xy;
    
    // Solve cubic equation for principal strains
    double p = I1 * I1 - 3.0 * I2;
    double q = 2.0 * I1 * I1 * I1 - 9.0 * I1 * I2 + 27.0 * I3;
    double r = std::sqrt(std::abs(p));
    
    double phi = std::acos(q / (2.0 * r * r * r));
    
    maxPrincipal = I1 / 3.0 + 2.0 * r * std::cos(phi / 3.0) / 3.0;
    midPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 2.0 * M_PI) / 3.0) / 3.0;
    minPrincipal = I1 / 3.0 + 2.0 * r * std::cos((phi + 4.0 * M_PI) / 3.0) / 3.0;
    
    // Calculate maximum shear strain
    maxShear = (maxPrincipal - minPrincipal) / 2.0;
}

// FEAResult implementation
FEAResult::FEAResult(std::shared_ptr<Mesh> mesh, SolverType solverType)
    : mesh(mesh), solverType(solverType), numDofs(0) {
}

// Constructor with number of DOFs
FEAResult::FEAResult(int numDofs)
    : mesh(nullptr), solverType(SolverType::NonLinear), numDofs(numDofs) {
}

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

const DisplacementResult* FEAResult::getDisplacement(int nodeId) const {
    auto it = nodeIdToDisplacementIndex.find(nodeId);
    if (it != nodeIdToDisplacementIndex.end()) {
        return &displacements[it->second];
    }
    return nullptr;
}

const StressResult* FEAResult::getStress(int elementId) const {
    auto it = elementIdToStressIndex.find(elementId);
    if (it != elementIdToStressIndex.end()) {
        return &stresses[it->second];
    }
    return nullptr;
}

const StrainResult* FEAResult::getStrain(int elementId) const {
    auto it = elementIdToStrainIndex.find(elementId);
    if (it != elementIdToStrainIndex.end()) {
        return &strains[it->second];
    }
    return nullptr;
}

const ReactionForceResult* FEAResult::getReactionForce(int nodeId) const {
    auto it = nodeIdToReactionForceIndex.find(nodeId);
    if (it != nodeIdToReactionForceIndex.end()) {
        return &reactionForces[it->second];
    }
    return nullptr;
}

const FrequencyResult* FEAResult::getFrequency(int modeNumber) const {
    auto it = modeNumberToFrequencyIndex.find(modeNumber);
    if (it != modeNumberToFrequencyIndex.end()) {
        return &frequencies[it->second];
    }
    return nullptr;
}

const ModeShapeResult* FEAResult::getModeShape(int nodeId, int modeNumber) const {
    auto it = nodeModeToModeShapeIndex.find(std::make_pair(nodeId, modeNumber));
    if (it != nodeModeToModeShapeIndex.end()) {
        return &modeShapes[it->second];
    }
    return nullptr;
}

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

bool FEAResult::exportToFile(const std::string& filePath) const {
    REBEL_LOG_INFO("Exporting FEA results to file: " + filePath);
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            REBEL_LOG_ERROR("Failed to open file for writing: " + filePath);
            return false;
        }
        
        // Write header
        file << "# FEA Results" << std::endl;
        if (mesh) {
            file << "# Mesh: " << mesh->getId() << std::endl;
        }
        file << "# Solver Type: " << static_cast<int>(solverType) << std::endl;
        
        // Write displacements
        if (!displacements.empty()) {
            file << "# Displacements: " << displacements.size() << std::endl;
            file << "Displacements" << std::endl;
            for (const auto& displacement : displacements) {
                file << displacement.nodeId << " "
                     << displacement.x << " "
                     << displacement.y << " "
                     << displacement.z << " "
                     << displacement.magnitude << std::endl;
            }
        }
        
        // Write stresses
        if (!stresses.empty()) {
            file << "# Stresses: " << stresses.size() << std::endl;
            file << "Stresses" << std::endl;
            for (const auto& stress : stresses) {
                file << stress.elementId << " "
                     << stress.xx << " "
                     << stress.yy << " "
                     << stress.zz << " "
                     << stress.xy << " "
                     << stress.yz << " "
                     << stress.xz << " "
                     << stress.vonMises << " "
                     << stress.maxPrincipal << " "
                     << stress.midPrincipal << " "
                     << stress.minPrincipal << " "
                     << stress.maxShear << std::endl;
            }
        }
        
        // Write strains
        if (!strains.empty()) {
            file << "# Strains: " << strains.size() << std::endl;
            file << "Strains" << std::endl;
            for (const auto& strain : strains) {
                file << strain.elementId << " "
                     << strain.xx << " "
                     << strain.yy << " "
                     << strain.zz << " "
                     << strain.xy << " "
                     << strain.yz << " "
                     << strain.xz << " "
                     << strain.maxPrincipal << " "
                     << strain.midPrincipal << " "
                     << strain.minPrincipal << " "
                     << strain.maxShear << std::endl;
            }
        }
        
        // Write reaction forces
        if (!reactionForces.empty()) {
            file << "# ReactionForces: " << reactionForces.size() << std::endl;
            file << "ReactionForces" << std::endl;
            for (const auto& force : reactionForces) {
                file << force.nodeId << " "
                     << force.x << " "
                     << force.y << " "
                     << force.z << " "
                     << force.magnitude << std::endl;
            }
        }
        
        // Write frequencies
        if (!frequencies.empty()) {
            file << "# Frequencies: " << frequencies.size() << std::endl;
            file << "Frequencies" << std::endl;
            for (const auto& freq : frequencies) {
                file << freq.modeNumber << " "
                     << freq.frequency << " "
                     << freq.period << " "
                     << freq.angularFrequency << std::endl;
            }
        }
        
        // Write mode shapes
        if (!modeShapes.empty()) {
            file << "# ModeShapes: " << modeShapes.size() << std::endl;
            file << "ModeShapes" << std::endl;
            for (const auto& shape : modeShapes) {
                file << shape.nodeId << " "
                     << shape.modeNumber << " "
                     << shape.x << " "
                     << shape.y << " "
                     << shape.z << " "
                     << shape.magnitude << std::endl;
            }
        }
        
        // Write load steps
        if (!loadSteps.empty()) {
            file << "# LoadSteps: " << loadSteps.size() << std::endl;
            file << "LoadSteps" << std::endl;
            for (const auto& step : loadSteps) {
                file << step.loadFactor << " ";
                for (const auto& disp : step.displacements) {
                    file << disp << " ";
                }
                file << std::endl;
            }
        }
        
        file.close();
        REBEL_LOG_INFO("FEA results exported successfully");
        return true;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while exporting FEA results: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<FEAResult> FEAResult::importFromFile(const std::string& filePath) {
    REBEL_LOG_INFO("Importing FEA results from file: " + filePath);
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            REBEL_LOG_ERROR("Failed to open file for reading: " + filePath);
            return nullptr;
        }
        
        std::string line;
        std::string meshId;
        SolverType solverType = SolverType::Linear;
        
        // Read header
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != '#') {
                continue;
            }
            
            if (line.substr(0, 7) == "# Mesh:") {
                meshId = line.substr(8);
            } else if (line.substr(0, 14) == "# Solver Type:") {
                int type;
                std::istringstream(line.substr(15)) >> type;
                solverType = static_cast<SolverType>(type);
            }
            
            if (!meshId.empty()) {
                break;
            }
        }
        
        if (meshId.empty()) {
            REBEL_LOG_ERROR("Mesh ID not found in file");
            return nullptr;
        }
        
        // Create a dummy mesh for now
        // In a real implementation, we would load the mesh from a file or database
        auto mesh = Mesh::importFromFile(meshId);
        if (!mesh) {
            REBEL_LOG_ERROR("Failed to create mesh from ID: " + meshId);
            return nullptr;
        }
        auto result = std::shared_ptr<FEAResult>(new FEAResult(mesh, solverType));
        
        // Read results
        std::string section;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            if (line == "Displacements" || line == "Stresses" || line == "Strains" ||
                line == "ReactionForces" || line == "Frequencies" || line == "ModeShapes" ||
                line == "LoadSteps") {
                section = line;
                continue;
            }
            
            std::istringstream iss(line);
            
            if (section == "Displacements") {
                int nodeId;
                double x, y, z, magnitude;
                iss >> nodeId >> x >> y >> z >> magnitude;
                result->addDisplacement(DisplacementResult(nodeId, x, y, z));
            } else if (section == "Stresses") {
                int elementId;
                double xx, yy, zz, xy, yz, xz, vonMises, maxPrincipal, midPrincipal, minPrincipal, maxShear;
                iss >> elementId >> xx >> yy >> zz >> xy >> yz >> xz >> vonMises >> maxPrincipal >> midPrincipal >> minPrincipal >> maxShear;
                result->addStress(StressResult(elementId, xx, yy, zz, xy, yz, xz));
            } else if (section == "Strains") {
                int elementId;
                double xx, yy, zz, xy, yz, xz, maxPrincipal, midPrincipal, minPrincipal, maxShear;
                iss >> elementId >> xx >> yy >> zz >> xy >> yz >> xz >> maxPrincipal >> midPrincipal >> minPrincipal >> maxShear;
                result->addStrain(StrainResult(elementId, xx, yy, zz, xy, yz, xz));
            } else if (section == "ReactionForces") {
                int nodeId;
                double x, y, z, magnitude;
                iss >> nodeId >> x >> y >> z >> magnitude;
                result->addReactionForce(ReactionForceResult(nodeId, x, y, z));
            } else if (section == "Frequencies") {
                int modeNumber;
                double frequency, period, angularFrequency;
                iss >> modeNumber >> frequency >> period >> angularFrequency;
                result->addFrequency(FrequencyResult(modeNumber, frequency));
            } else if (section == "ModeShapes") {
                int nodeId, modeNumber;
                double x, y, z, magnitude;
                iss >> nodeId >> modeNumber >> x >> y >> z >> magnitude;
                result->addModeShape(ModeShapeResult(nodeId, modeNumber, x, y, z));
            } else if (section == "LoadSteps") {
                double loadFactor;
                iss >> loadFactor;
                
                std::vector<double> displacements;
                double disp;
                while (iss >> disp) {
                    displacements.push_back(disp);
                }
                
                result->addLoadStep(loadFactor, displacements);
            }
        }
        
        file.close();
        REBEL_LOG_INFO("FEA results imported successfully");
        return result;
    } catch (const std::exception& e) {
        REBEL_LOG_ERROR("Exception while importing FEA results: " + std::string(e.what()));
        return nullptr;
    }
}

void FEAResult::addDisplacement(const DisplacementResult& result) {
    displacements.push_back(result);
    nodeIdToDisplacementIndex[result.nodeId] = displacements.size() - 1;
}

void FEAResult::addStress(const StressResult& result) {
    stresses.push_back(result);
    elementIdToStressIndex[result.elementId] = stresses.size() - 1;
}

void FEAResult::addStrain(const StrainResult& result) {
    strains.push_back(result);
    elementIdToStrainIndex[result.elementId] = strains.size() - 1;
}

void FEAResult::addReactionForce(const ReactionForceResult& result) {
    reactionForces.push_back(result);
    nodeIdToReactionForceIndex[result.nodeId] = reactionForces.size() - 1;
}

void FEAResult::addFrequency(const FrequencyResult& result) {
    frequencies.push_back(result);
    modeNumberToFrequencyIndex[result.modeNumber] = frequencies.size() - 1;
}

void FEAResult::addModeShape(const ModeShapeResult& result) {
    modeShapes.push_back(result);
    nodeModeToModeShapeIndex[std::make_pair(result.nodeId, result.modeNumber)] = modeShapes.size() - 1;
}

void FEAResult::addLoadStep(double loadFactor, const std::vector<double>& displacements) {
    loadSteps.push_back(LoadStepResult(loadFactor, displacements));
}

void FEAResult::computeStressesAndStrains(std::shared_ptr<Mesh> mesh, const std::vector<double>& displacements) {
    if (!mesh) {
        REBEL_LOG_ERROR("Cannot compute stresses and strains with null mesh");
        return;
    }
    
    REBEL_LOG_INFO("Computing stresses and strains from displacements");
    
    // Clear existing stresses and strains
    stresses.clear();
    strains.clear();
    elementIdToStressIndex.clear();
    elementIdToStrainIndex.clear();
    
    // Loop over elements
    for (int elemIdx = 0; elemIdx < mesh->getElementCount(); ++elemIdx) {
        // Since we don't have access to the Mesh class implementation,
        // we'll use a placeholder approach that doesn't rely on specific Mesh methods
        
        // Assume element IDs are sequential starting from 1
        int elementId = elemIdx + 1;
        
        // Placeholder: Assume 3 DOFs per node (x, y, z) and 4 nodes per element (tetrahedron)
        std::vector<int> elementDofs;
        for (int i = 0; i < 4; ++i) {
            int nodeIdx = elemIdx * 4 + i; // Placeholder: Assume nodes are ordered by element
            elementDofs.push_back(nodeIdx * 3);     // x
            elementDofs.push_back(nodeIdx * 3 + 1); // y
            elementDofs.push_back(nodeIdx * 3 + 2); // z
        }
        
        // Get element displacements
        std::vector<double> elementDisplacements;
        elementDisplacements.reserve(elementDofs.size());
        for (int dof : elementDofs) {
            if (dof < static_cast<int>(displacements.size())) {
                elementDisplacements.push_back(displacements[dof]);
            } else {
                elementDisplacements.push
