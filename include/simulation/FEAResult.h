#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <utility>
#include "simulation/FEASystem.h"

namespace rebel::simulation {

// Forward declarations
class Mesh;

/**
 * @enum ResultType
 * @brief Types of FEA results.
 */
enum class ResultType {
    Displacement,   ///< Displacement
    Stress,         ///< Stress
    Strain,         ///< Strain
    Temperature,    ///< Temperature
    HeatFlux,       ///< Heat flux
    ReactionForce,  ///< Reaction force
    Velocity,       ///< Velocity
    Acceleration,   ///< Acceleration
    Energy,         ///< Energy
    Frequency,      ///< Natural frequency
    ModeShape       ///< Mode shape
};

/**
 * @enum StressComponent
 * @brief Components of stress tensor.
 */
enum class StressComponent {
    XX,             ///< Normal stress in X direction
    YY,             ///< Normal stress in Y direction
    ZZ,             ///< Normal stress in Z direction
    XY,             ///< Shear stress in XY plane
    YZ,             ///< Shear stress in YZ plane
    XZ,             ///< Shear stress in XZ plane
    VonMises,       ///< Von Mises equivalent stress
    MaxPrincipal,   ///< Maximum principal stress
    MidPrincipal,   ///< Middle principal stress
    MinPrincipal,   ///< Minimum principal stress
    MaxShear        ///< Maximum shear stress
};

/**
 * @enum StrainComponent
 * @brief Components of strain tensor.
 */
enum class StrainComponent {
    XX,             ///< Normal strain in X direction
    YY,             ///< Normal strain in Y direction
    ZZ,             ///< Normal strain in Z direction
    XY,             ///< Shear strain in XY plane
    YZ,             ///< Shear strain in YZ plane
    XZ,             ///< Shear strain in XZ plane
    MaxPrincipal,   ///< Maximum principal strain
    MidPrincipal,   ///< Middle principal strain
    MinPrincipal,   ///< Minimum principal strain
    MaxShear        ///< Maximum shear strain
};

/**
 * @struct DisplacementResult
 * @brief Displacement result for a node.
 */
struct DisplacementResult {
    int nodeId;             ///< Node ID
    double x, y, z;         ///< Displacement components
    double magnitude;       ///< Displacement magnitude
    
    /**
     * @brief Constructs a new DisplacementResult object.
     * 
     * @param nodeId Node ID
     * @param x X component of displacement
     * @param y Y component of displacement
     * @param z Z component of displacement
     */
    DisplacementResult(int nodeId, double x, double y, double z)
        : nodeId(nodeId), x(x), y(y), z(z), magnitude(std::sqrt(x*x + y*y + z*z)) {}
};

/**
 * @struct StressResult
 * @brief Stress result for an element.
 */
struct StressResult {
    int elementId;          ///< Element ID
    double xx, yy, zz;      ///< Normal stress components
    double xy, yz, xz;      ///< Shear stress components
    double vonMises;        ///< Von Mises equivalent stress
    double maxPrincipal;    ///< Maximum principal stress
    double midPrincipal;    ///< Middle principal stress
    double minPrincipal;    ///< Minimum principal stress
    double maxShear;        ///< Maximum shear stress
    
    /**
     * @brief Constructs a new StressResult object.
     * 
     * @param elementId Element ID
     * @param xx Normal stress in X direction
     * @param yy Normal stress in Y direction
     * @param zz Normal stress in Z direction
     * @param xy Shear stress in XY plane
     * @param yz Shear stress in YZ plane
     * @param xz Shear stress in XZ plane
     */
    StressResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz);
};

/**
 * @struct StrainResult
 * @brief Strain result for an element.
 */
struct StrainResult {
    int elementId;          ///< Element ID
    double xx, yy, zz;      ///< Normal strain components
    double xy, yz, xz;      ///< Shear strain components
    double maxPrincipal;    ///< Maximum principal strain
    double midPrincipal;    ///< Middle principal strain
    double minPrincipal;    ///< Minimum principal strain
    double maxShear;        ///< Maximum shear strain
    
    /**
     * @brief Constructs a new StrainResult object.
     * 
     * @param elementId Element ID
     * @param xx Normal strain in X direction
     * @param yy Normal strain in Y direction
     * @param zz Normal strain in Z direction
     * @param xy Shear strain in XY plane
     * @param yz Shear strain in YZ plane
     * @param xz Shear strain in XZ plane
     */
    StrainResult(int elementId, double xx, double yy, double zz, double xy, double yz, double xz);
};

/**
 * @struct ReactionForceResult
 * @brief Reaction force result for a node.
 */
struct ReactionForceResult {
    int nodeId;             ///< Node ID
    double x, y, z;         ///< Force components
    double magnitude;       ///< Force magnitude
    
    /**
     * @brief Constructs a new ReactionForceResult object.
     * 
     * @param nodeId Node ID
     * @param x X component of force
     * @param y Y component of force
     * @param z Z component of force
     */
    ReactionForceResult(int nodeId, double x, double y, double z)
        : nodeId(nodeId), x(x), y(y), z(z), magnitude(std::sqrt(x*x + y*y + z*z)) {}
};

/**
 * @struct FrequencyResult
 * @brief Natural frequency result.
 */
struct FrequencyResult {
    int modeNumber;         ///< Mode number
    double frequency;       ///< Frequency in Hz
    double period;          ///< Period in seconds
    double angularFrequency; ///< Angular frequency in rad/s
    
    /**
     * @brief Constructs a new FrequencyResult object.
     * 
     * @param modeNumber Mode number
     * @param frequency Frequency in Hz
     */
    FrequencyResult(int modeNumber, double frequency)
        : modeNumber(modeNumber), frequency(frequency), 
          period(1.0 / frequency), angularFrequency(2.0 * 3.14159265358979323846 * frequency) {}
};

/**
 * @struct PairHash
 * @brief Hash function for std::pair.
 */
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

/**
 * @struct ModeShapeResult
 * @brief Mode shape result for a node.
 */
struct ModeShapeResult {
    int nodeId;             ///< Node ID
    int modeNumber;         ///< Mode number
    double x, y, z;         ///< Displacement components
    double magnitude;       ///< Displacement magnitude
    
    /**
     * @brief Constructs a new ModeShapeResult object.
     * 
     * @param nodeId Node ID
     * @param modeNumber Mode number
     * @param x X component of displacement
     * @param y Y component of displacement
     * @param z Z component of displacement
     */
    ModeShapeResult(int nodeId, int modeNumber, double x, double y, double z)
        : nodeId(nodeId), modeNumber(modeNumber), x(x), y(y), z(z), 
          magnitude(std::sqrt(x*x + y*y + z*z)) {}
};

/**
 * @struct LoadStepResult
 * @brief Result for a load step in non-linear analysis.
 */
struct LoadStepResult {
    double loadFactor;                  ///< Load factor
    std::vector<double> displacements;  ///< Displacements
    
    /**
     * @brief Constructs a new LoadStepResult object.
     * 
     * @param loadFactor Load factor
     * @param displacements Displacements
     */
    LoadStepResult(double loadFactor, const std::vector<double>& displacements)
        : loadFactor(loadFactor), displacements(displacements) {}
};

/**
 * @class FEAResult
 * @brief Class representing the results of a finite element analysis.
 * 
 * The FEAResult class stores and provides access to the results of a
 * finite element analysis, including displacements, stresses, strains,
 * reaction forces, and more.
 */
class FEAResult {
public:
    /**
     * @brief Creates a new FEAResult object.
     * 
     * @param mesh Shared pointer to the mesh.
     * @param solverType The type of solver used.
     * @return Shared pointer to the created result.
     */
    static std::shared_ptr<FEAResult> create(
        std::shared_ptr<Mesh> mesh,
        SolverType solverType);

    /**
     * @brief Constructs a new FEAResult object with the specified number of DOFs.
     * 
     * @param numDofs The number of degrees of freedom.
     */
    FEAResult(int numDofs);

    /**
     * @brief Gets the mesh associated with the result.
     * @return Shared pointer to the mesh.
     */
    std::shared_ptr<Mesh> getMesh() const { return mesh; }

    /**
     * @brief Gets the solver type used for the analysis.
     * @return The solver type.
     */
    SolverType getSolverType() const { return solverType; }

    /**
     * @brief Gets the available result types.
     * @return Vector of available result types.
     */
    std::vector<ResultType> getAvailableResultTypes() const;

    /**
     * @brief Checks if a result type is available.
     * 
     * @param type The result type to check.
     * @return True if the result type is available, false otherwise.
     */
    bool hasResultType(ResultType type) const;

    /**
     * @brief Gets the displacement results.
     * @return Vector of displacement results.
     */
    const std::vector<DisplacementResult>& getDisplacements() const { return displacements; }

    /**
     * @brief Gets the stress results.
     * @return Vector of stress results.
     */
    const std::vector<StressResult>& getStresses() const { return stresses; }

    /**
     * @brief Gets the strain results.
     * @return Vector of strain results.
     */
    const std::vector<StrainResult>& getStrains() const { return strains; }

    /**
     * @brief Gets the reaction force results.
     * @return Vector of reaction force results.
     */
    const std::vector<ReactionForceResult>& getReactionForces() const { return reactionForces; }

    /**
     * @brief Gets the frequency results.
     * @return Vector of frequency results.
     */
    const std::vector<FrequencyResult>& getFrequencies() const { return frequencies; }

    /**
     * @brief Gets the mode shape results.
     * @return Vector of mode shape results.
     */
    const std::vector<ModeShapeResult>& getModeShapes() const { return modeShapes; }

    /**
     * @brief Gets the load step results.
     * @return Vector of load step results.
     */
    const std::vector<LoadStepResult>& getLoadSteps() const { return loadSteps; }

    /**
     * @brief Gets the displacement result for a node.
     * 
     * @param nodeId The ID of the node.
     * @return Pointer to the displacement result, or nullptr if not found.
     */
    const DisplacementResult* getDisplacement(int nodeId) const;

    /**
     * @brief Gets the stress result for an element.
     * 
     * @param elementId The ID of the element.
     * @return Pointer to the stress result, or nullptr if not found.
     */
    const StressResult* getStress(int elementId) const;

    /**
     * @brief Gets the strain result for an element.
     * 
     * @param elementId The ID of the element.
     * @return Pointer to the strain result, or nullptr if not found.
     */
    const StrainResult* getStrain(int elementId) const;

    /**
     * @brief Gets the reaction force result for a node.
     * 
     * @param nodeId The ID of the node.
     * @return Pointer to the reaction force result, or nullptr if not found.
     */
    const ReactionForceResult* getReactionForce(int nodeId) const;

    /**
     * @brief Gets the frequency result for a mode.
     * 
     * @param modeNumber The mode number.
     * @return Pointer to the frequency result, or nullptr if not found.
     */
    const FrequencyResult* getFrequency(int modeNumber) const;

    /**
     * @brief Gets the mode shape result for a node and mode.
     * 
     * @param nodeId The ID of the node.
     * @param modeNumber The mode number.
     * @return Pointer to the mode shape result, or nullptr if not found.
     */
    const ModeShapeResult* getModeShape(int nodeId, int modeNumber) const;

    /**
     * @brief Gets the maximum displacement magnitude.
     * @return The maximum displacement magnitude.
     */
    double getMaxDisplacementMagnitude() const;

    /**
     * @brief Gets the maximum stress value for a component.
     * 
     * @param component The stress component.
     * @return The maximum stress value.
     */
    double getMaxStress(StressComponent component) const;

    /**
     * @brief Gets the minimum stress value for a component.
     * 
     * @param component The stress component.
     * @return The minimum stress value.
     */
    double getMinStress(StressComponent component) const;

    /**
     * @brief Gets the maximum strain value for a component.
     * 
     * @param component The strain component.
     * @return The maximum strain value.
     */
    double getMaxStrain(StrainComponent component) const;

    /**
     * @brief Gets the minimum strain value for a component.
     * 
     * @param component The strain component.
     * @return The minimum strain value.
     */
    double getMinStrain(StrainComponent component) const;

    /**
     * @brief Gets the maximum reaction force magnitude.
     * @return The maximum reaction force magnitude.
     */
    double getMaxReactionForceMagnitude() const;

    /**
     * @brief Gets the total reaction force.
     * 
     * @param direction The direction (0 for X, 1 for Y, 2 for Z).
     * @return The total reaction force in the specified direction.
     */
    double getTotalReactionForce(int direction) const;

    /**
     * @brief Exports the results to a file.
     * 
     * @param filePath The path to export to.
     * @return True if export was successful, false otherwise.
     */
    bool exportToFile(const std::string& filePath) const;

    /**
     * @brief Imports results from a file.
     * 
     * @param filePath The path to import from.
     * @return Shared pointer to the imported result, or nullptr if failed.
     */
    static std::shared_ptr<FEAResult> importFromFile(const std::string& filePath);

    /**
     * @brief Adds a displacement result.
     * 
     * @param result The displacement result to add.
     */
    void addDisplacement(const DisplacementResult& result);

    /**
     * @brief Adds a stress result.
     * 
     * @param result The stress result to add.
     */
    void addStress(const StressResult& result);

    /**
     * @brief Adds a strain result.
     * 
     * @param result The strain result to add.
     */
    void addStrain(const StrainResult& result);

    /**
     * @brief Adds a reaction force result.
     * 
     * @param result The reaction force result to add.
     */
    void addReactionForce(const ReactionForceResult& result);

    /**
     * @brief Adds a frequency result.
     * 
     * @param result The frequency result to add.
     */
    void addFrequency(const FrequencyResult& result);

    /**
     * @brief Adds a mode shape result.
     * 
     * @param result The mode shape result to add.
     */
    void addModeShape(const ModeShapeResult& result);

    /**
     * @brief Adds a load step result.
     * 
     * @param loadFactor The load factor.
     * @param displacements The displacements.
     */
    void addLoadStep(double loadFactor, const std::vector<double>& displacements);

    /**
     * @brief Computes stresses and strains from displacements.
     * 
     * @param mesh The mesh.
     * @param displacements The displacements.
     */
    void computeStressesAndStrains(std::shared_ptr<Mesh> mesh, const std::vector<double>& displacements);

private:
    /**
     * @brief Constructs a new FEAResult object.
     * 
     * @param mesh Shared pointer to the mesh.
     * @param solverType The type of solver used.
     */
    FEAResult(std::shared_ptr<Mesh> mesh, SolverType solverType);

    std::shared_ptr<Mesh> mesh;                      ///< Mesh
    SolverType solverType;                           ///< Solver type
    int numDofs;                                     ///< Number of degrees of freedom
    std::vector<DisplacementResult> displacements;   ///< Displacement results
    std::vector<StressResult> stresses;              ///< Stress results
    std::vector<StrainResult> strains;               ///< Strain results
    std::vector<ReactionForceResult> reactionForces; ///< Reaction force results
    std::vector<FrequencyResult> frequencies;        ///< Frequency results
    std::vector<ModeShapeResult> modeShapes;         ///< Mode shape results
    std::vector<LoadStepResult> loadSteps;           ///< Load step results
    std::unordered_map<int, size_t> nodeIdToDisplacementIndex; ///< Map from node ID to displacement index
    std::unordered_map<int, size_t> elementIdToStressIndex;    ///< Map from element ID to stress index
    std::unordered_map<int, size_t> elementIdToStrainIndex;    ///< Map from element ID to strain index
    std::unordered_map<int, size_t> nodeIdToReactionForceIndex; ///< Map from node ID to reaction force index
    std::unordered_map<int, size_t> modeNumberToFrequencyIndex; ///< Map from mode number to frequency index
    std::unordered_map<std::pair<int, int>, size_t, PairHash> nodeModeToModeShapeIndex; ///< Map from (node ID, mode number) to mode shape index
};

} // namespace rebel::simulation
