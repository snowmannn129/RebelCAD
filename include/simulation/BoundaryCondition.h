#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace rebel::simulation {

// Forward declarations
class Mesh;

/**
 * @enum BoundaryConditionType
 * @brief Types of boundary conditions.
 */
enum class BoundaryConditionType {
    Displacement,   ///< Prescribed displacement
    Force,          ///< Applied force
    Pressure,       ///< Applied pressure
    Temperature,    ///< Prescribed temperature
    HeatFlux,       ///< Applied heat flux
    Convection,     ///< Convection boundary
    Radiation,      ///< Radiation boundary
    Symmetry,       ///< Symmetry plane
    Contact         ///< Contact boundary
};

/**
 * @enum DisplacementDirection
 * @brief Directions for displacement boundary conditions.
 */
enum class DisplacementDirection {
    X,      ///< X direction
    Y,      ///< Y direction
    Z,      ///< Z direction
    XY,     ///< XY plane
    YZ,     ///< YZ plane
    XZ,     ///< XZ plane
    XYZ     ///< All directions (fixed)
};

/**
 * @class BoundaryCondition
 * @brief Base class for all boundary conditions.
 * 
 * The BoundaryCondition class defines the interface for all boundary conditions
 * that can be applied to a finite element model.
 */
class BoundaryCondition {
public:
    /**
     * @brief Creates a displacement boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param direction The direction of the displacement.
     * @param value The value of the displacement.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createDisplacement(
        const std::string& id,
        const std::string& nodeGroupName,
        DisplacementDirection direction,
        double value = 0.0);

    /**
     * @brief Creates a force boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param forceX The X component of the force.
     * @param forceY The Y component of the force.
     * @param forceZ The Z component of the force.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createForce(
        const std::string& id,
        const std::string& nodeGroupName,
        double forceX,
        double forceY,
        double forceZ);

    /**
     * @brief Creates a pressure boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param elementGroupName The name of the element group to apply to.
     * @param pressure The pressure value.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createPressure(
        const std::string& id,
        const std::string& elementGroupName,
        double pressure);

    /**
     * @brief Creates a temperature boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param temperature The temperature value.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createTemperature(
        const std::string& id,
        const std::string& nodeGroupName,
        double temperature);

    /**
     * @brief Creates a heat flux boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param elementGroupName The name of the element group to apply to.
     * @param heatFlux The heat flux value.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createHeatFlux(
        const std::string& id,
        const std::string& elementGroupName,
        double heatFlux);

    /**
     * @brief Creates a convection boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param elementGroupName The name of the element group to apply to.
     * @param filmCoefficient The film coefficient.
     * @param ambientTemperature The ambient temperature.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createConvection(
        const std::string& id,
        const std::string& elementGroupName,
        double filmCoefficient,
        double ambientTemperature);

    /**
     * @brief Creates a symmetry boundary condition.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param direction The direction of the symmetry plane.
     * @return Shared pointer to the created boundary condition.
     */
    static std::shared_ptr<BoundaryCondition> createSymmetry(
        const std::string& id,
        const std::string& nodeGroupName,
        DisplacementDirection direction);

    /**
     * @brief Gets the ID of the boundary condition.
     * @return The ID of the boundary condition.
     */
    const std::string& getId() const { return id; }

    /**
     * @brief Gets the type of the boundary condition.
     * @return The type of the boundary condition.
     */
    BoundaryConditionType getType() const { return type; }

    /**
     * @brief Gets the name of the node or element group.
     * @return The name of the node or element group.
     */
    const std::string& getGroupName() const { return groupName; }

    /**
     * @brief Checks if the boundary condition is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the boundary condition is valid, false otherwise.
     */
    virtual bool isValid(const Mesh& mesh) const = 0;

    /**
     * @brief Applies the boundary condition to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    virtual bool apply(Mesh& mesh) const = 0;

    /**
     * @brief Gets a string representation of the boundary condition.
     * @return String representation of the boundary condition.
     */
    virtual std::string toString() const = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~BoundaryCondition() = default;

protected:
    /**
     * @brief Constructs a new BoundaryCondition object.
     * 
     * @param id The ID of the boundary condition.
     * @param type The type of the boundary condition.
     * @param groupName The name of the node or element group.
     */
    BoundaryCondition(
        const std::string& id,
        BoundaryConditionType type,
        const std::string& groupName);

    std::string id;                      ///< Boundary condition ID
    BoundaryConditionType type;          ///< Boundary condition type
    std::string groupName;               ///< Node or element group name
};

/**
 * @class DisplacementBC
 * @brief Class for displacement boundary conditions.
 */
class DisplacementBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new DisplacementBC object.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param direction The direction of the displacement.
     * @param value The value of the displacement.
     */
    DisplacementBC(
        const std::string& id,
        const std::string& nodeGroupName,
        DisplacementDirection direction,
        double value);

    /**
     * @brief Gets the direction of the displacement.
     * @return The direction of the displacement.
     */
    DisplacementDirection getDirection() const { return direction; }

    /**
     * @brief Gets the value of the displacement.
     * @return The value of the displacement.
     */
    double getValue() const { return value; }

    /**
     * @brief Checks if the boundary condition is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the boundary condition is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the boundary condition to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the boundary condition.
     * @return String representation of the boundary condition.
     */
    std::string toString() const override;

private:
    DisplacementDirection direction;  ///< Direction of the displacement
    double value;                     ///< Value of the displacement
};

/**
 * @class ForceBC
 * @brief Class for force boundary conditions.
 */
class ForceBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new ForceBC object.
     * 
     * @param id The ID of the boundary condition.
     * @param nodeGroupName The name of the node group to apply to.
     * @param forceX The X component of the force.
     * @param forceY The Y component of the force.
     * @param forceZ The Z component of the force.
     */
    ForceBC(
        const std::string& id,
        const std::string& nodeGroupName,
        double forceX,
        double forceY,
        double forceZ);

    /**
     * @brief Gets the X component of the force.
     * @return The X component of the force.
     */
    double getForceX() const { return forceX; }

    /**
     * @brief Gets the Y component of the force.
     * @return The Y component of the force.
     */
    double getForceY() const { return forceY; }

    /**
     * @brief Gets the Z component of the force.
     * @return The Z component of the force.
     */
    double getForceZ() const { return forceZ; }

    /**
     * @brief Checks if the boundary condition is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the boundary condition is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the boundary condition to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the boundary condition.
     * @return String representation of the boundary condition.
     */
    std::string toString() const override;

private:
    double forceX;  ///< X component of the force
    double forceY;  ///< Y component of the force
    double forceZ;  ///< Z component of the force
};

/**
 * @class PressureBC
 * @brief Class for pressure boundary conditions.
 */
class PressureBC : public BoundaryCondition {
public:
    /**
     * @brief Constructs a new PressureBC object.
     * 
     * @param id The ID of the boundary condition.
     * @param elementGroupName The name of the element group to apply to.
     * @param pressure The pressure value.
     */
    PressureBC(
        const std::string& id,
        const std::string& elementGroupName,
        double pressure);

    /**
     * @brief Gets the pressure value.
     * @return The pressure value.
     */
    double getPressure() const { return pressure; }

    /**
     * @brief Checks if the boundary condition is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the boundary condition is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the boundary condition to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the boundary condition.
     * @return String representation of the boundary condition.
     */
    std::string toString() const override;

private:
    double pressure;  ///< Pressure value
};

} // namespace rebel::simulation
