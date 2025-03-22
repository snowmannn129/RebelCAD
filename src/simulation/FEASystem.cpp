#include "simulation/FEASystem.h"
#include "simulation/Material.h"
#include "simulation/Mesh.h"
#include "simulation/BoundaryCondition.h"
#include "simulation/Load.h"
#include "simulation/FEAResult.h"
#include "simulation/LinearStaticSolver.h"
#include "simulation/NonLinearSolver.h"
#include "simulation/DynamicSolver.h"
// Temporarily comment out ThermalSolver until it's fully implemented
// #include "simulation/ThermalSolver.h"
#include "core/Log.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace rebel::simulation {

// Implementation class for FEASystem
class FEASystem::FEASystemImpl {
public:
    FEASystemImpl() = default;
    ~FEASystemImpl() = default;

    bool initialize() {
        REBEL_LOG_INFO("Initializing FEA system...");
        
        // Initialize material library
        initializeMaterialLibrary();
        
        // Initialize solvers
        initializeSolvers();
        
        REBEL_LOG_INFO("FEA system initialized successfully");
        return true;
    }

    void shutdown() {
        REBEL_LOG_INFO("Shutting down FEA system...");
        
        // Clear all data
        materials.clear();
        boundaryConditions.clear();
        loads.clear();
        
        REBEL_LOG_INFO("FEA system shut down successfully");
    }

    void setSettings(const FEASettings& newSettings) {
        settings = newSettings;
        REBEL_LOG_INFO("FEA settings updated");
    }

    const FEASettings& getSettings() const {
        return settings;
    }

    std::shared_ptr<Mesh> createMesh(const std::string& geometryId, 
                                     ElementType elementType,
                                     int refinementLevel) {
        REBEL_LOG_INFO("Creating mesh for geometry: " + geometryId);
        
        try {
            mesh = Mesh::createFromGeometry(geometryId, elementType, refinementLevel);
            if (mesh) {
                REBEL_LOG_INFO("Mesh created successfully with " + 
                              std::to_string(mesh->getNodeCount()) + " nodes and " +
                              std::to_string(mesh->getElementCount()) + " elements");
            } else {
                REBEL_LOG_ERROR("Failed to create mesh for geometry: " + geometryId);
            }
            return mesh;
        } catch (const std::exception& e) {
            REBEL_LOG_ERROR("Exception while creating mesh: " + std::string(e.what()));
            return nullptr;
        }
    }

    bool registerMaterial(std::shared_ptr<Material> material) {
        if (!material) {
            REBEL_LOG_ERROR("Cannot register null material");
            return false;
        }
        
        const std::string& name = material->getName();
        if (materials.find(name) != materials.end()) {
            REBEL_LOG_WARNING("Material with name '" + name + "' already exists, overwriting");
        }
        
        materials[name] = material;
        REBEL_LOG_INFO("Registered material: " + name);
        return true;
    }

    std::shared_ptr<Material> getMaterial(const std::string& name) const {
        auto it = materials.find(name);
        if (it != materials.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Material>> getAllMaterials() const {
        std::vector<std::shared_ptr<Material>> result;
        result.reserve(materials.size());
        for (const auto& pair : materials) {
            result.push_back(pair.second);
        }
        return result;
    }

    bool addBoundaryCondition(std::shared_ptr<BoundaryCondition> boundaryCondition) {
        if (!boundaryCondition) {
            REBEL_LOG_ERROR("Cannot add null boundary condition");
            return false;
        }
        
        const std::string& id = boundaryCondition->getId();
        if (boundaryConditionMap.find(id) != boundaryConditionMap.end()) {
            REBEL_LOG_WARNING("Boundary condition with ID '" + id + "' already exists, overwriting");
        }
        
        boundaryConditions.push_back(boundaryCondition);
        boundaryConditionMap[id] = boundaryConditions.size() - 1;
        REBEL_LOG_INFO("Added boundary condition: " + id);
        return true;
    }

    bool removeBoundaryCondition(const std::string& id) {
        auto it = boundaryConditionMap.find(id);
        if (it == boundaryConditionMap.end()) {
            REBEL_LOG_WARNING("Boundary condition with ID '" + id + "' not found");
            return false;
        }
        
        size_t index = it->second;
        boundaryConditions.erase(boundaryConditions.begin() + index);
        boundaryConditionMap.erase(it);
        
        // Update indices in the map
        for (auto& pair : boundaryConditionMap) {
            if (pair.second > index) {
                pair.second--;
            }
        }
        
        REBEL_LOG_INFO("Removed boundary condition: " + id);
        return true;
    }

    std::vector<std::shared_ptr<BoundaryCondition>> getAllBoundaryConditions() const {
        return boundaryConditions;
    }

    bool addLoad(std::shared_ptr<Load> load) {
        if (!load) {
            REBEL_LOG_ERROR("Cannot add null load");
            return false;
        }
        
        const std::string& id = load->getId();
        if (loadMap.find(id) != loadMap.end()) {
            REBEL_LOG_WARNING("Load with ID '" + id + "' already exists, overwriting");
        }
        
        loads.push_back(load);
        loadMap[id] = loads.size() - 1;
        REBEL_LOG_INFO("Added load: " + id);
        return true;
    }

    bool removeLoad(const std::string& id) {
        auto it = loadMap.find(id);
        if (it == loadMap.end()) {
            REBEL_LOG_WARNING("Load with ID '" + id + "' not found");
            return false;
        }
        
        size_t index = it->second;
        loads.erase(loads.begin() + index);
        loadMap.erase(it);
        
        // Update indices in the map
        for (auto& pair : loadMap) {
            if (pair.second > index) {
                pair.second--;
            }
        }
        
        REBEL_LOG_INFO("Removed load: " + id);
        return true;
    }

    std::vector<std::shared_ptr<Load>> getAllLoads() const {
        return loads;
    }

    std::shared_ptr<FEAResult> solve(std::function<void(float)> progressCallback) {
        if (boundaryConditions.empty()) {
            REBEL_LOG_ERROR("No boundary conditions defined");
            return nullptr;
        }
        
        if (loads.empty()) {
            REBEL_LOG_ERROR("No loads defined");
            return nullptr;
        }
        
        REBEL_LOG_INFO("Starting FEA solver...");
        REBEL_LOG_INFO("Solver type: " + solverTypeToString(settings.solverType));
        REBEL_LOG_INFO("Element type: " + elementTypeToString(settings.elementType));
        REBEL_LOG_INFO("Mesh refinement level: " + std::to_string(settings.meshRefinementLevel));
        
        // Create a solver based on the solver type
        switch (settings.solverType) {
            case SolverType::Linear: {
                // Create a linear static solver
                LinearStaticSolver solver(settings);
                
                // Solve the problem
                return solver.solve(mesh, boundaryConditions, loads, progressCallback);
            }
            case SolverType::NonLinear: {
                // Create a non-linear solver with default settings
                NonLinearSolverSettings nonLinearSettings;
                NonLinearSolver solver(settings, nonLinearSettings);
                
                // Solve the problem
                return solver.solve(mesh, boundaryConditions, loads, progressCallback);
            }
            case SolverType::Modal: {
                // Create a dynamic solver with default settings
                DynamicSolverSettings dynamicSettings;
                dynamicSettings.numModes = 10;
                DynamicSolver solver(settings, dynamicSettings);
                
                // Solve the problem
                return solver.solve(mesh, boundaryConditions, loads, progressCallback);
            }
            case SolverType::Thermal: {
                // Thermal solver not implemented yet
                REBEL_LOG_ERROR("Thermal solver not implemented yet");
                return nullptr;
            }
            case SolverType::ThermalStructural:
                REBEL_LOG_ERROR("Thermal-structural solver not implemented yet");
                return nullptr;
            case SolverType::Fatigue:
                REBEL_LOG_ERROR("Fatigue solver not implemented yet");
                return nullptr;
            case SolverType::Buckling:
                REBEL_LOG_ERROR("Buckling solver not implemented yet");
                return nullptr;
            default:
                REBEL_LOG_ERROR("Unknown solver type");
                return nullptr;
        }
    }

    std::shared_ptr<NonLinearSolver> createNonLinearSolver(const NonLinearSolverSettings& nonLinearSettings) {
        REBEL_LOG_INFO("Creating non-linear solver...");
        
        try {
            auto solver = std::make_shared<NonLinearSolver>(settings, nonLinearSettings);
            REBEL_LOG_INFO("Non-linear solver created successfully");
            return solver;
        } catch (const std::exception& e) {
            REBEL_LOG_ERROR("Exception while creating non-linear solver: " + std::string(e.what()));
            return nullptr;
        }
    }

    std::shared_ptr<DynamicSolver> createDynamicSolver(const DynamicSolverSettings& dynamicSettings) {
        REBEL_LOG_INFO("Creating dynamic solver...");
        
        try {
            auto solver = std::make_shared<DynamicSolver>(settings, dynamicSettings);
            REBEL_LOG_INFO("Dynamic solver created successfully");
            return solver;
        } catch (const std::exception& e) {
            REBEL_LOG_ERROR("Exception while creating dynamic solver: " + std::string(e.what()));
            return nullptr;
        }
    }
    
    // Temporarily comment out createThermalSolver until ThermalSolver is fully implemented
    /*
    std::shared_ptr<ThermalSolver> createThermalSolver(const ThermalSolverSettings& thermalSettings) {
        REBEL_LOG_INFO("Creating thermal solver...");
        
        try {
            auto solver = std::make_shared<ThermalSolver>(settings, thermalSettings);
            REBEL_LOG_INFO("Thermal solver created successfully");
            return solver;
        } catch (const std::exception& e) {
            REBEL_LOG_ERROR("Exception while creating thermal solver: " + std::string(e.what()));
            return nullptr;
        }
    }
    */

    bool exportResults(std::shared_ptr<FEAResult> result, const std::string& filePath) {
        if (!result) {
            REBEL_LOG_ERROR("Cannot export null result");
            return false;
        }
        
        REBEL_LOG_INFO("Exporting FEA results to: " + filePath);
        
        try {
            return result->exportToFile(filePath);
        } catch (const std::exception& e) {
            REBEL_LOG_ERROR("Exception while exporting results: " + std::string(e.what()));
            return false;
        }
    }

private:
    void initializeMaterialLibrary() {
        REBEL_LOG_INFO("Initializing material library...");
        
        // Add some common materials
        auto commonMaterials = Material::createCommonMaterialLibrary();
        for (const auto& material : commonMaterials) {
            registerMaterial(material);
        }
        
        REBEL_LOG_INFO("Material library initialized with " + 
                      std::to_string(materials.size()) + " materials");
    }

    void initializeSolvers() {
        REBEL_LOG_INFO("Initializing FEA solvers...");
        
        // Initialize solvers based on available types
        // This is a placeholder for actual solver initialization
        
        REBEL_LOG_INFO("FEA solvers initialized");
    }

    std::string solverTypeToString(SolverType type) const {
        switch (type) {
            case SolverType::Linear: return "Linear";
            case SolverType::NonLinear: return "NonLinear";
            case SolverType::Modal: return "Modal";
            case SolverType::Thermal: return "Thermal";
            case SolverType::ThermalStructural: return "ThermalStructural";
            case SolverType::Fatigue: return "Fatigue";
            case SolverType::Buckling: return "Buckling";
            default: return "Unknown";
        }
    }

    std::string elementTypeToString(ElementType type) const {
        switch (type) {
            case ElementType::Beam: return "Beam";
            case ElementType::Triangle: return "Triangle";
            case ElementType::Quad: return "Quad";
            case ElementType::Tetra: return "Tetra";
            case ElementType::Hexa: return "Hexa";
            case ElementType::Pyramid: return "Pyramid";
            case ElementType::Prism: return "Prism";
            default: return "Unknown";
        }
    }

    FEASettings settings;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<BoundaryCondition>> boundaryConditions;
    std::unordered_map<std::string, size_t> boundaryConditionMap;
    std::vector<std::shared_ptr<Load>> loads;
    std::unordered_map<std::string, size_t> loadMap;
    std::shared_ptr<Mesh> mesh;
};

// Singleton instance
FEASystem& FEASystem::getInstance() {
    static FEASystem instance;
    return instance;
}

FEASystem::FEASystem()
    : impl(std::make_unique<FEASystemImpl>()),
      initialized(false) {
}

FEASystem::~FEASystem() {
    if (initialized) {
        shutdown();
    }
}

bool FEASystem::initialize() {
    if (initialized) {
        REBEL_LOG_WARNING("FEA system already initialized");
        return true;
    }
    
    bool success = impl->initialize();
    if (success) {
        initialized = true;
    }
    return success;
}

void FEASystem::shutdown() {
    if (!initialized) {
        REBEL_LOG_WARNING("FEA system not initialized");
        return;
    }
    
    impl->shutdown();
    initialized = false;
}

void FEASystem::setSettings(const FEASettings& settings) {
    impl->setSettings(settings);
}

const FEASettings& FEASystem::getSettings() const {
    return impl->getSettings();
}

std::shared_ptr<Mesh> FEASystem::createMesh(const std::string& geometryId, 
                                           ElementType elementType,
                                           int refinementLevel) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->createMesh(geometryId, elementType, refinementLevel);
}

bool FEASystem::registerMaterial(std::shared_ptr<Material> material) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->registerMaterial(material);
}

std::shared_ptr<Material> FEASystem::getMaterial(const std::string& name) const {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->getMaterial(name);
}

std::vector<std::shared_ptr<Material>> FEASystem::getAllMaterials() const {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return {};
    }
    
    return impl->getAllMaterials();
}

bool FEASystem::addBoundaryCondition(std::shared_ptr<BoundaryCondition> boundaryCondition) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->addBoundaryCondition(boundaryCondition);
}

bool FEASystem::removeBoundaryCondition(const std::string& id) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->removeBoundaryCondition(id);
}

std::vector<std::shared_ptr<BoundaryCondition>> FEASystem::getAllBoundaryConditions() const {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return {};
    }
    
    return impl->getAllBoundaryConditions();
}

bool FEASystem::addLoad(std::shared_ptr<Load> load) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->addLoad(load);
}

bool FEASystem::removeLoad(const std::string& id) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->removeLoad(id);
}

std::vector<std::shared_ptr<Load>> FEASystem::getAllLoads() const {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return {};
    }
    
    return impl->getAllLoads();
}

std::shared_ptr<FEAResult> FEASystem::solve(std::function<void(float)> progressCallback) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->solve(progressCallback);
}

std::shared_ptr<NonLinearSolver> FEASystem::createNonLinearSolver(const NonLinearSolverSettings& nonLinearSettings) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->createNonLinearSolver(nonLinearSettings);
}

std::shared_ptr<DynamicSolver> FEASystem::createDynamicSolver(const DynamicSolverSettings& dynamicSettings) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->createDynamicSolver(dynamicSettings);
}

// Temporarily comment out createThermalSolver until ThermalSolver is fully implemented
/*
std::shared_ptr<ThermalSolver> FEASystem::createThermalSolver(const ThermalSolverSettings& thermalSettings) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return nullptr;
    }
    
    return impl->createThermalSolver(thermalSettings);
}
*/

bool FEASystem::exportResults(std::shared_ptr<FEAResult> result, const std::string& filePath) {
    if (!initialized) {
        REBEL_LOG_ERROR("FEA system not initialized");
        return false;
    }
    
    return impl->exportResults(result, filePath);
}

} // namespace rebel::simulation
