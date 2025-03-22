#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace rebel::simulation {

// Forward declarations
class Material;
class Mesh;
class BoundaryCondition;
class Load;
class FEAResult;
class NonLinearSolver;
struct NonLinearSolverSettings;
class DynamicSolver;
struct DynamicSolverSettings;
class ThermalSolver;
struct ThermalSolverSettings;

/**
 * @enum SolverType
 * @brief Types of solvers available for FEA.
 */
enum class SolverType {
    Linear,             ///< Linear static analysis
    NonLinear,          ///< Non-linear static analysis
    Modal,              ///< Modal analysis (natural frequencies)
    Thermal,            ///< Thermal analysis
    ThermalStructural,  ///< Coupled thermal-structural analysis
    Fatigue,            ///< Fatigue analysis
    Buckling            ///< Buckling analysis
};

/**
 * @enum ElementType
 * @brief Types of finite elements.
 */
enum class ElementType {
    Beam,       ///< 1D beam element
    Triangle,   ///< 2D triangular element
    Quad,       ///< 2D quadrilateral element
    Tetra,      ///< 3D tetrahedral element
    Hexa,       ///< 3D hexahedral element
    Pyramid,    ///< 3D pyramid element
    Prism       ///< 3D prism element
};

/**
 * @struct FEASettings
 * @brief Settings for the FEA solver.
 */
struct FEASettings {
    SolverType solverType = SolverType::Linear;  ///< Type of solver to use
    ElementType elementType = ElementType::Tetra; ///< Type of elements to use
    int meshRefinementLevel = 1;                 ///< Mesh refinement level (1-5)
    double convergenceTolerance = 1e-6;          ///< Convergence tolerance for iterative solvers
    int maxIterations = 1000;                    ///< Maximum iterations for iterative solvers
    bool useAdaptiveMeshing = false;             ///< Whether to use adaptive meshing
    bool useGPUAcceleration = false;             ///< Whether to use GPU acceleration
    
    /**
     * @brief Constructs a new FEASettings object with default values.
     */
    FEASettings() = default;
};

/**
 * @class FEASystem
 * @brief Main class for the Finite Element Analysis system.
 * 
 * The FEASystem class manages the entire FEA workflow, including mesh generation,
 * material assignment, boundary condition and load application, solving, and
 * result visualization.
 */
class FEASystem {
public:
    /**
     * @brief Gets the singleton instance of the FEASystem.
     * @return Reference to the FEASystem instance.
     */
    static FEASystem& getInstance();

    /**
     * @brief Initializes the FEA system.
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize();
    
    /**
     * @brief Shuts down the FEA system.
     */
    void shutdown();

    /**
     * @brief Sets the settings for the FEA solver.
     * @param settings The settings to use.
     */
    void setSettings(const FEASettings& settings);
    
    /**
     * @brief Gets the current settings for the FEA solver.
     * @return The current settings.
     */
    const FEASettings& getSettings() const;

    /**
     * @brief Creates a mesh from a geometry model.
     * @param geometryId The ID of the geometry to mesh.
     * @param elementType The type of elements to use.
     * @param refinementLevel The mesh refinement level (1-5).
     * @return Shared pointer to the created mesh, or nullptr if failed.
     */
    std::shared_ptr<Mesh> createMesh(const std::string& geometryId, 
                                     ElementType elementType = ElementType::Tetra,
                                     int refinementLevel = 1);

    /**
     * @brief Registers a material for use in FEA.
     * @param material Shared pointer to the material to register.
     * @return True if registration was successful, false otherwise.
     */
    bool registerMaterial(std::shared_ptr<Material> material);
    
    /**
     * @brief Gets a registered material by name.
     * @param name The name of the material to get.
     * @return Shared pointer to the material, or nullptr if not found.
     */
    std::shared_ptr<Material> getMaterial(const std::string& name) const;
    
    /**
     * @brief Gets all registered materials.
     * @return Vector of shared pointers to all registered materials.
     */
    std::vector<std::shared_ptr<Material>> getAllMaterials() const;

    /**
     * @brief Adds a boundary condition to the analysis.
     * @param boundaryCondition Shared pointer to the boundary condition to add.
     * @return True if addition was successful, false otherwise.
     */
    bool addBoundaryCondition(std::shared_ptr<BoundaryCondition> boundaryCondition);
    
    /**
     * @brief Removes a boundary condition from the analysis.
     * @param id The ID of the boundary condition to remove.
     * @return True if removal was successful, false otherwise.
     */
    bool removeBoundaryCondition(const std::string& id);
    
    /**
     * @brief Gets all boundary conditions in the analysis.
     * @return Vector of shared pointers to all boundary conditions.
     */
    std::vector<std::shared_ptr<BoundaryCondition>> getAllBoundaryConditions() const;

    /**
     * @brief Adds a load to the analysis.
     * @param load Shared pointer to the load to add.
     * @return True if addition was successful, false otherwise.
     */
    bool addLoad(std::shared_ptr<Load> load);
    
    /**
     * @brief Removes a load from the analysis.
     * @param id The ID of the load to remove.
     * @return True if removal was successful, false otherwise.
     */
    bool removeLoad(const std::string& id);
    
    /**
     * @brief Gets all loads in the analysis.
     * @return Vector of shared pointers to all loads.
     */
    std::vector<std::shared_ptr<Load>> getAllLoads() const;

    /**
     * @brief Runs the FEA solver.
     * @param progressCallback Optional callback function for progress updates.
     * @return Shared pointer to the result, or nullptr if failed.
     */
    std::shared_ptr<FEAResult> solve(
        std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Creates a non-linear solver with the specified settings.
     * @param nonLinearSettings The settings for the non-linear solver.
     * @return Shared pointer to the created solver, or nullptr if failed.
     */
    std::shared_ptr<NonLinearSolver> createNonLinearSolver(
        const NonLinearSolverSettings& nonLinearSettings);

    /**
     * @brief Creates a dynamic solver with the specified settings.
     * @param dynamicSettings The settings for the dynamic solver.
     * @return Shared pointer to the created solver, or nullptr if failed.
     */
    std::shared_ptr<DynamicSolver> createDynamicSolver(
        const DynamicSolverSettings& dynamicSettings);

    /**
     * @brief Creates a thermal solver with the specified settings.
     * @param thermalSettings The settings for the thermal solver.
     * @return Shared pointer to the created solver, or nullptr if failed.
     * 
     * Note: Temporarily commented out until ThermalSolver is fully implemented.
     */
    // std::shared_ptr<ThermalSolver> createThermalSolver(
    //     const ThermalSolverSettings& thermalSettings);

    /**
     * @brief Exports the FEA results to a file.
     * @param result The result to export.
     * @param filePath The path to export to.
     * @return True if export was successful, false otherwise.
     */
    bool exportResults(std::shared_ptr<FEAResult> result, const std::string& filePath);

    /**
     * @brief Checks if the FEA system is initialized.
     * @return True if the FEA system is initialized, false otherwise.
     */
    bool isInitialized() const { return initialized; }

private:
    /**
     * @brief Constructs a new FEASystem instance.
     * 
     * Private constructor to enforce the singleton pattern.
     */
    FEASystem();
    
    /**
     * @brief Destroys the FEASystem instance.
     */
    ~FEASystem();

    // Delete copy and move constructors/assignments
    FEASystem(const FEASystem&) = delete;
    FEASystem& operator=(const FEASystem&) = delete;
    FEASystem(FEASystem&&) = delete;
    FEASystem& operator=(FEASystem&&) = delete;

    /**
     * @brief Implementation class for the FEASystem.
     * 
     * This class implements the details of the FEA system.
     */
    class FEASystemImpl;
    std::unique_ptr<FEASystemImpl> impl;

    bool initialized = false;  ///< Flag indicating if the FEA system is initialized
    FEASettings settings;      ///< Current FEA settings
};

} // namespace rebel::simulation
