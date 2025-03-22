#include "simulation/FEAResult.h"
#include "simulation/Mesh.h"
#include "core/Log.h"
#include <fstream>
#include <sstream>

namespace rebel::simulation {

// FEAResult implementation - File I/O operations

// Export results to a file
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

// Import results from a file
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

} // namespace rebel::simulation
