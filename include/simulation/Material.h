#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace rebel::simulation {

/**
 * @enum MaterialType
 * @brief Types of materials supported by the FEA system.
 */
enum class MaterialType {
    Isotropic,          ///< Isotropic material (same properties in all directions)
    Orthotropic,        ///< Orthotropic material (different properties in 3 perpendicular directions)
    Anisotropic,        ///< Anisotropic material (different properties in all directions)
    Hyperelastic,       ///< Hyperelastic material (non-linear elastic behavior)
    ElastoPlastic,      ///< Elasto-plastic material (with yield criteria)
    Viscoelastic,       ///< Viscoelastic material (time-dependent behavior)
    Composite           ///< Composite material (layered structure)
};

/**
 * @class Material
 * @brief Class representing a material for FEA.
 * 
 * The Material class defines the properties of a material used in FEA,
 * such as Young's modulus, Poisson's ratio, density, etc.
 */
class Material {
public:
    /**
     * @brief Creates a new isotropic material.
     * 
     * @param name The name of the material.
     * @param youngsModulus Young's modulus (E) in Pa.
     * @param poissonsRatio Poisson's ratio (ν).
     * @param density Density (ρ) in kg/m³.
     * @param thermalExpansionCoeff Thermal expansion coefficient (α) in 1/K.
     * @param thermalConductivity Thermal conductivity (k) in W/(m·K).
     * @param specificHeat Specific heat capacity (c) in J/(kg·K).
     * @return Shared pointer to the created material.
     */
    static std::shared_ptr<Material> createIsotropic(
        const std::string& name,
        double youngsModulus,
        double poissonsRatio,
        double density,
        double thermalExpansionCoeff = 0.0,
        double thermalConductivity = 0.0,
        double specificHeat = 0.0);

    /**
     * @brief Creates a new orthotropic material.
     * 
     * @param name The name of the material.
     * @param youngsModulusX Young's modulus in X direction (Ex) in Pa.
     * @param youngsModulusY Young's modulus in Y direction (Ey) in Pa.
     * @param youngsModulusZ Young's modulus in Z direction (Ez) in Pa.
     * @param poissonsRatioXY Poisson's ratio in XY plane (νxy).
     * @param poissonsRatioYZ Poisson's ratio in YZ plane (νyz).
     * @param poissonsRatioXZ Poisson's ratio in XZ plane (νxz).
     * @param shearModulusXY Shear modulus in XY plane (Gxy) in Pa.
     * @param shearModulusYZ Shear modulus in YZ plane (Gyz) in Pa.
     * @param shearModulusXZ Shear modulus in XZ plane (Gxz) in Pa.
     * @param density Density (ρ) in kg/m³.
     * @return Shared pointer to the created material.
     */
    static std::shared_ptr<Material> createOrthotropic(
        const std::string& name,
        double youngsModulusX,
        double youngsModulusY,
        double youngsModulusZ,
        double poissonsRatioXY,
        double poissonsRatioYZ,
        double poissonsRatioXZ,
        double shearModulusXY,
        double shearModulusYZ,
        double shearModulusXZ,
        double density);

    /**
     * @brief Creates a new elasto-plastic material.
     * 
     * @param name The name of the material.
     * @param youngsModulus Young's modulus (E) in Pa.
     * @param poissonsRatio Poisson's ratio (ν).
     * @param density Density (ρ) in kg/m³.
     * @param yieldStrength Yield strength (σy) in Pa.
     * @param ultimateStrength Ultimate strength (σu) in Pa.
     * @param hardeningModulus Hardening modulus (H) in Pa.
     * @return Shared pointer to the created material.
     */
    static std::shared_ptr<Material> createElastoPlastic(
        const std::string& name,
        double youngsModulus,
        double poissonsRatio,
        double density,
        double yieldStrength,
        double ultimateStrength,
        double hardeningModulus);

    /**
     * @brief Gets the name of the material.
     * @return The name of the material.
     */
    const std::string& getName() const { return name; }

    /**
     * @brief Gets the type of the material.
     * @return The type of the material.
     */
    MaterialType getType() const { return type; }

    /**
     * @brief Gets a property value by name.
     * 
     * @param propertyName The name of the property to get.
     * @return The value of the property, or 0.0 if not found.
     */
    double getProperty(const std::string& propertyName) const;

    /**
     * @brief Sets a property value by name.
     * 
     * @param propertyName The name of the property to set.
     * @param value The value to set.
     * @return True if the property was set, false if the property doesn't exist.
     */
    bool setProperty(const std::string& propertyName, double value);

    /**
     * @brief Gets all property names for this material.
     * @return Vector of property names.
     */
    std::vector<std::string> getPropertyNames() const;

    /**
     * @brief Checks if the material has a specific property.
     * 
     * @param propertyName The name of the property to check.
     * @return True if the material has the property, false otherwise.
     */
    bool hasProperty(const std::string& propertyName) const;

    /**
     * @brief Gets the density of the material.
     * @return The density in kg/m³.
     */
    double getDensity() const;

    /**
     * @brief Sets the density of the material.
     * @param density The density in kg/m³.
     */
    void setDensity(double density);

    /**
     * @brief Gets Young's modulus for isotropic materials.
     * @return Young's modulus in Pa.
     * @throws Error if the material is not isotropic.
     */
    double getYoungsModulus() const;

    /**
     * @brief Gets Poisson's ratio for isotropic materials.
     * @return Poisson's ratio.
     * @throws Error if the material is not isotropic.
     */
    double getPoissonsRatio() const;

    /**
     * @brief Gets the shear modulus for isotropic materials.
     * @return The shear modulus in Pa.
     * @throws Error if the material is not isotropic.
     */
    double getShearModulus() const;

    /**
     * @brief Gets the bulk modulus for isotropic materials.
     * @return The bulk modulus in Pa.
     * @throws Error if the material is not isotropic.
     */
    double getBulkModulus() const;

    /**
     * @brief Gets the thermal expansion coefficient for isotropic materials.
     * @return The thermal expansion coefficient in 1/K.
     * @throws Error if the material is not isotropic.
     */
    double getThermalExpansionCoefficient() const;

    /**
     * @brief Gets the thermal conductivity for isotropic materials.
     * @return The thermal conductivity in W/(m·K).
     * @throws Error if the material is not isotropic.
     */
    double getThermalConductivity() const;

    /**
     * @brief Gets the specific heat capacity for isotropic materials.
     * @return The specific heat capacity in J/(kg·K).
     * @throws Error if the material is not isotropic.
     */
    double getSpecificHeat() const;

    /**
     * @brief Creates a copy of this material.
     * @return A new shared pointer to a copy of this material.
     */
    std::shared_ptr<Material> clone() const;

    /**
     * @brief Creates a material library with common materials.
     * @return Vector of shared pointers to common materials.
     */
    static std::vector<std::shared_ptr<Material>> createCommonMaterialLibrary();

protected:
    /**
     * @brief Constructs a new Material object.
     * 
     * @param name The name of the material.
     * @param type The type of the material.
     */
    Material(const std::string& name, MaterialType type);

private:
    std::string name;                                  ///< Name of the material
    MaterialType type;                                 ///< Type of the material
    std::unordered_map<std::string, double> properties; ///< Material properties
};

} // namespace rebel::simulation
