#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace rebel::simulation {

// Forward declarations
class Mesh;

/**
 * @enum LoadType
 * @brief Types of loads.
 */
enum class LoadType {
    PointForce,     ///< Point force
    LineForce,      ///< Line force
    SurfacePressure, ///< Surface pressure
    BodyForce,      ///< Body force (e.g., gravity)
    Moment,         ///< Moment
    Acceleration,   ///< Acceleration
    Centrifugal,    ///< Centrifugal force
    Thermal,        ///< Thermal load
    Pretension      ///< Pretension load
};

/**
 * @enum LoadVariation
 * @brief Variation of load with time.
 */
enum class LoadVariation {
    Static,         ///< Static load (constant)
    Transient,      ///< Transient load (time-dependent)
    Harmonic,       ///< Harmonic load (sinusoidal)
    Random          ///< Random load
};

/**
 * @class Load
 * @brief Base class for all loads.
 * 
 * The Load class defines the interface for all loads
 * that can be applied to a finite element model.
 */
class Load {
public:
    /**
     * @brief Creates a point force load.
     * 
     * @param id The ID of the load.
     * @param nodeGroupName The name of the node group to apply to.
     * @param forceX The X component of the force.
     * @param forceY The Y component of the force.
     * @param forceZ The Z component of the force.
     * @param variation The variation of the load with time.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createPointForce(
        const std::string& id,
        const std::string& nodeGroupName,
        double forceX,
        double forceY,
        double forceZ,
        LoadVariation variation = LoadVariation::Static);

    /**
     * @brief Creates a surface pressure load.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param pressure The pressure value.
     * @param variation The variation of the load with time.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createSurfacePressure(
        const std::string& id,
        const std::string& elementGroupName,
        double pressure,
        LoadVariation variation = LoadVariation::Static);

    /**
     * @brief Creates a body force load.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param forceX The X component of the force per unit volume.
     * @param forceY The Y component of the force per unit volume.
     * @param forceZ The Z component of the force per unit volume.
     * @param variation The variation of the load with time.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createBodyForce(
        const std::string& id,
        const std::string& elementGroupName,
        double forceX,
        double forceY,
        double forceZ,
        LoadVariation variation = LoadVariation::Static);

    /**
     * @brief Creates a gravity load.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param accelerationX The X component of the acceleration.
     * @param accelerationY The Y component of the acceleration.
     * @param accelerationZ The Z component of the acceleration.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createGravity(
        const std::string& id,
        const std::string& elementGroupName,
        double accelerationX,
        double accelerationY,
        double accelerationZ);

    /**
     * @brief Creates a centrifugal load.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param centerX The X coordinate of the center of rotation.
     * @param centerY The Y coordinate of the center of rotation.
     * @param centerZ The Z coordinate of the center of rotation.
     * @param axisX The X component of the rotation axis.
     * @param axisY The Y component of the rotation axis.
     * @param axisZ The Z component of the rotation axis.
     * @param angularVelocity The angular velocity in rad/s.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createCentrifugal(
        const std::string& id,
        const std::string& elementGroupName,
        double centerX,
        double centerY,
        double centerZ,
        double axisX,
        double axisY,
        double axisZ,
        double angularVelocity);

    /**
     * @brief Creates a thermal load.
     * 
     * @param id The ID of the load.
     * @param nodeGroupName The name of the node group to apply to.
     * @param temperature The temperature value.
     * @param referenceTemperature The reference temperature.
     * @param variation The variation of the load with time.
     * @return Shared pointer to the created load.
     */
    static std::shared_ptr<Load> createThermal(
        const std::string& id,
        const std::string& nodeGroupName,
        double temperature,
        double referenceTemperature,
        LoadVariation variation = LoadVariation::Static);

    /**
     * @brief Gets the ID of the load.
     * @return The ID of the load.
     */
    const std::string& getId() const { return id; }

    /**
     * @brief Gets the type of the load.
     * @return The type of the load.
     */
    LoadType getType() const { return type; }

    /**
     * @brief Gets the variation of the load with time.
     * @return The variation of the load with time.
     */
    LoadVariation getVariation() const { return variation; }

    /**
     * @brief Gets the name of the node or element group.
     * @return The name of the node or element group.
     */
    const std::string& getGroupName() const { return groupName; }

    /**
     * @brief Checks if the load is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    virtual bool isValid(const Mesh& mesh) const = 0;

    /**
     * @brief Applies the load to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    virtual bool apply(Mesh& mesh) const = 0;

    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    virtual std::string toString() const = 0;

    /**
     * @brief Virtual destructor.
     */
    virtual ~Load() = default;

protected:
    /**
     * @brief Constructs a new Load object.
     * 
     * @param id The ID of the load.
     * @param type The type of the load.
     * @param groupName The name of the node or element group.
     * @param variation The variation of the load with time.
     */
    Load(
        const std::string& id,
        LoadType type,
        const std::string& groupName,
        LoadVariation variation);

    std::string id;          ///< Load ID
    LoadType type;           ///< Load type
    std::string groupName;   ///< Node or element group name
    LoadVariation variation; ///< Variation of the load with time
};

/**
 * @class PointForceLoad
 * @brief Class for point force loads.
 */
class PointForceLoad : public Load {
public:
    /**
     * @brief Constructs a new PointForceLoad object.
     * 
     * @param id The ID of the load.
     * @param nodeGroupName The name of the node group to apply to.
     * @param forceX The X component of the force.
     * @param forceY The Y component of the force.
     * @param forceZ The Z component of the force.
     * @param variation The variation of the load with time.
     */
    PointForceLoad(
        const std::string& id,
        const std::string& nodeGroupName,
        double forceX,
        double forceY,
        double forceZ,
        LoadVariation variation);

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
     * @brief Checks if the load is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the load to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;

private:
    double forceX;  ///< X component of the force
    double forceY;  ///< Y component of the force
    double forceZ;  ///< Z component of the force
};

/**
 * @class SurfacePressureLoad
 * @brief Class for surface pressure loads.
 */
class SurfacePressureLoad : public Load {
public:
    /**
     * @brief Constructs a new SurfacePressureLoad object.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param pressure The pressure value.
     * @param variation The variation of the load with time.
     */
    SurfacePressureLoad(
        const std::string& id,
        const std::string& elementGroupName,
        double pressure,
        LoadVariation variation);

    /**
     * @brief Gets the pressure value.
     * @return The pressure value.
     */
    double getPressure() const { return pressure; }

    /**
     * @brief Checks if the load is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the load to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;

private:
    double pressure;  ///< Pressure value
};

/**
 * @class BodyForceLoad
 * @brief Class for body force loads.
 */
class BodyForceLoad : public Load {
public:
    /**
     * @brief Constructs a new BodyForceLoad object.
     * 
     * @param id The ID of the load.
     * @param elementGroupName The name of the element group to apply to.
     * @param forceX The X component of the force per unit volume.
     * @param forceY The Y component of the force per unit volume.
     * @param forceZ The Z component of the force per unit volume.
     * @param variation The variation of the load with time.
     */
    BodyForceLoad(
        const std::string& id,
        const std::string& elementGroupName,
        double forceX,
        double forceY,
        double forceZ,
        LoadVariation variation);

    /**
     * @brief Gets the X component of the force per unit volume.
     * @return The X component of the force per unit volume.
     */
    double getForceX() const { return forceX; }

    /**
     * @brief Gets the Y component of the force per unit volume.
     * @return The Y component of the force per unit volume.
     */
    double getForceY() const { return forceY; }

    /**
     * @brief Gets the Z component of the force per unit volume.
     * @return The Z component of the force per unit volume.
     */
    double getForceZ() const { return forceZ; }

    /**
     * @brief Checks if the load is valid for a given mesh.
     * 
     * @param mesh The mesh to check against.
     * @return True if the load is valid, false otherwise.
     */
    bool isValid(const Mesh& mesh) const override;

    /**
     * @brief Applies the load to a mesh.
     * 
     * @param mesh The mesh to apply to.
     * @return True if application was successful, false otherwise.
     */
    bool apply(Mesh& mesh) const override;

    /**
     * @brief Gets a string representation of the load.
     * @return String representation of the load.
     */
    std::string toString() const override;

private:
    double forceX;  ///< X component of the force per unit volume
    double forceY;  ///< Y component of the force per unit volume
    double forceZ;  ///< Z component of the force per unit volume
};

} // namespace rebel::simulation
