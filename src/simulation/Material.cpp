#include "simulation/Material.h"
#include "core/Log.h"
#include <cmath>
#include <stdexcept>

namespace rebel::simulation {

Material::Material(const std::string& name, MaterialType type)
    : name(name), type(type) {
}

std::shared_ptr<Material> Material::createIsotropic(
    const std::string& name,
    double youngsModulus,
    double poissonsRatio,
    double density,
    double thermalExpansionCoeff,
    double thermalConductivity,
    double specificHeat) {
    
    if (youngsModulus <= 0.0) {
        REBEL_LOG_ERROR("Invalid Young's modulus: " + std::to_string(youngsModulus));
        return nullptr;
    }
    
    if (poissonsRatio < -1.0 || poissonsRatio > 0.5) {
        REBEL_LOG_ERROR("Invalid Poisson's ratio: " + std::to_string(poissonsRatio));
        return nullptr;
    }
    
    if (density <= 0.0) {
        REBEL_LOG_ERROR("Invalid density: " + std::to_string(density));
        return nullptr;
    }
    
    auto material = std::shared_ptr<Material>(new Material(name, MaterialType::Isotropic));
    
    material->properties["youngsModulus"] = youngsModulus;
    material->properties["poissonsRatio"] = poissonsRatio;
    material->properties["density"] = density;
    material->properties["thermalExpansionCoeff"] = thermalExpansionCoeff;
    material->properties["thermalConductivity"] = thermalConductivity;
    material->properties["specificHeat"] = specificHeat;
    
    // Derived properties
    double shearModulus = youngsModulus / (2.0 * (1.0 + poissonsRatio));
    double bulkModulus = youngsModulus / (3.0 * (1.0 - 2.0 * poissonsRatio));
    
    material->properties["shearModulus"] = shearModulus;
    material->properties["bulkModulus"] = bulkModulus;
    
    REBEL_LOG_INFO("Created isotropic material: " + name);
    return material;
}

std::shared_ptr<Material> Material::createOrthotropic(
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
    double density) {
    
    if (youngsModulusX <= 0.0 || youngsModulusY <= 0.0 || youngsModulusZ <= 0.0) {
        REBEL_LOG_ERROR("Invalid Young's modulus values");
        return nullptr;
    }
    
    if (poissonsRatioXY < -1.0 || poissonsRatioXY > 0.5 ||
        poissonsRatioYZ < -1.0 || poissonsRatioYZ > 0.5 ||
        poissonsRatioXZ < -1.0 || poissonsRatioXZ > 0.5) {
        REBEL_LOG_ERROR("Invalid Poisson's ratio values");
        return nullptr;
    }
    
    if (shearModulusXY <= 0.0 || shearModulusYZ <= 0.0 || shearModulusXZ <= 0.0) {
        REBEL_LOG_ERROR("Invalid shear modulus values");
        return nullptr;
    }
    
    if (density <= 0.0) {
        REBEL_LOG_ERROR("Invalid density: " + std::to_string(density));
        return nullptr;
    }
    
    auto material = std::shared_ptr<Material>(new Material(name, MaterialType::Orthotropic));
    
    material->properties["youngsModulusX"] = youngsModulusX;
    material->properties["youngsModulusY"] = youngsModulusY;
    material->properties["youngsModulusZ"] = youngsModulusZ;
    material->properties["poissonsRatioXY"] = poissonsRatioXY;
    material->properties["poissonsRatioYZ"] = poissonsRatioYZ;
    material->properties["poissonsRatioXZ"] = poissonsRatioXZ;
    material->properties["shearModulusXY"] = shearModulusXY;
    material->properties["shearModulusYZ"] = shearModulusYZ;
    material->properties["shearModulusXZ"] = shearModulusXZ;
    material->properties["density"] = density;
    
    REBEL_LOG_INFO("Created orthotropic material: " + name);
    return material;
}

std::shared_ptr<Material> Material::createElastoPlastic(
    const std::string& name,
    double youngsModulus,
    double poissonsRatio,
    double density,
    double yieldStrength,
    double ultimateStrength,
    double hardeningModulus) {
    
    if (youngsModulus <= 0.0) {
        REBEL_LOG_ERROR("Invalid Young's modulus: " + std::to_string(youngsModulus));
        return nullptr;
    }
    
    if (poissonsRatio < -1.0 || poissonsRatio > 0.5) {
        REBEL_LOG_ERROR("Invalid Poisson's ratio: " + std::to_string(poissonsRatio));
        return nullptr;
    }
    
    if (density <= 0.0) {
        REBEL_LOG_ERROR("Invalid density: " + std::to_string(density));
        return nullptr;
    }
    
    if (yieldStrength <= 0.0) {
        REBEL_LOG_ERROR("Invalid yield strength: " + std::to_string(yieldStrength));
        return nullptr;
    }
    
    if (ultimateStrength <= yieldStrength) {
        REBEL_LOG_ERROR("Ultimate strength must be greater than yield strength");
        return nullptr;
    }
    
    if (hardeningModulus <= 0.0) {
        REBEL_LOG_ERROR("Invalid hardening modulus: " + std::to_string(hardeningModulus));
        return nullptr;
    }
    
    auto material = std::shared_ptr<Material>(new Material(name, MaterialType::ElastoPlastic));
    
    material->properties["youngsModulus"] = youngsModulus;
    material->properties["poissonsRatio"] = poissonsRatio;
    material->properties["density"] = density;
    material->properties["yieldStrength"] = yieldStrength;
    material->properties["ultimateStrength"] = ultimateStrength;
    material->properties["hardeningModulus"] = hardeningModulus;
    
    // Derived properties
    double shearModulus = youngsModulus / (2.0 * (1.0 + poissonsRatio));
    double bulkModulus = youngsModulus / (3.0 * (1.0 - 2.0 * poissonsRatio));
    
    material->properties["shearModulus"] = shearModulus;
    material->properties["bulkModulus"] = bulkModulus;
    
    REBEL_LOG_INFO("Created elasto-plastic material: " + name);
    return material;
}

double Material::getProperty(const std::string& propertyName) const {
    auto it = properties.find(propertyName);
    if (it != properties.end()) {
        return it->second;
    }
    
    REBEL_LOG_WARNING("Property '" + propertyName + "' not found in material '" + name + "'");
    return 0.0;
}

bool Material::setProperty(const std::string& propertyName, double value) {
    auto it = properties.find(propertyName);
    if (it != properties.end()) {
        it->second = value;
        REBEL_LOG_INFO("Updated property '" + propertyName + "' in material '" + name + "'");
        return true;
    }
    
    REBEL_LOG_WARNING("Property '" + propertyName + "' not found in material '" + name + "'");
    return false;
}

std::vector<std::string> Material::getPropertyNames() const {
    std::vector<std::string> names;
    names.reserve(properties.size());
    
    for (const auto& pair : properties) {
        names.push_back(pair.first);
    }
    
    return names;
}

bool Material::hasProperty(const std::string& propertyName) const {
    return properties.find(propertyName) != properties.end();
}

double Material::getDensity() const {
    return getProperty("density");
}

void Material::setDensity(double density) {
    if (density <= 0.0) {
        REBEL_LOG_ERROR("Invalid density: " + std::to_string(density));
        return;
    }
    
    setProperty("density", density);
}

double Material::getYoungsModulus() const {
    if (type != MaterialType::Isotropic && type != MaterialType::ElastoPlastic) {
        REBEL_LOG_ERROR("Young's modulus is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("youngsModulus");
}

double Material::getPoissonsRatio() const {
    if (type != MaterialType::Isotropic && type != MaterialType::ElastoPlastic) {
        REBEL_LOG_ERROR("Poisson's ratio is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("poissonsRatio");
}

double Material::getShearModulus() const {
    if (type != MaterialType::Isotropic && type != MaterialType::ElastoPlastic) {
        REBEL_LOG_ERROR("Shear modulus is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("shearModulus");
}

double Material::getBulkModulus() const {
    if (type != MaterialType::Isotropic && type != MaterialType::ElastoPlastic) {
        REBEL_LOG_ERROR("Bulk modulus is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("bulkModulus");
}

double Material::getThermalExpansionCoefficient() const {
    if (type != MaterialType::Isotropic) {
        REBEL_LOG_ERROR("Thermal expansion coefficient is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("thermalExpansionCoeff");
}

double Material::getThermalConductivity() const {
    if (type != MaterialType::Isotropic) {
        REBEL_LOG_ERROR("Thermal conductivity is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("thermalConductivity");
}

double Material::getSpecificHeat() const {
    if (type != MaterialType::Isotropic) {
        REBEL_LOG_ERROR("Specific heat is only available for isotropic materials");
        return 0.0;
    }
    
    return getProperty("specificHeat");
}

std::shared_ptr<Material> Material::clone() const {
    auto material = std::shared_ptr<Material>(new Material(name, type));
    material->properties = properties;
    return material;
}

std::vector<std::shared_ptr<Material>> Material::createCommonMaterialLibrary() {
    std::vector<std::shared_ptr<Material>> materials;
    
    // Steel (structural)
    materials.push_back(createIsotropic(
        "Steel (Structural)",
        210.0e9,    // Young's modulus (Pa)
        0.3,        // Poisson's ratio
        7850.0,     // Density (kg/m³)
        1.2e-5,     // Thermal expansion coefficient (1/K)
        50.0,       // Thermal conductivity (W/(m·K))
        490.0       // Specific heat (J/(kg·K))
    ));
    
    // Aluminum (6061-T6)
    materials.push_back(createIsotropic(
        "Aluminum (6061-T6)",
        68.9e9,     // Young's modulus (Pa)
        0.33,       // Poisson's ratio
        2700.0,     // Density (kg/m³)
        2.3e-5,     // Thermal expansion coefficient (1/K)
        167.0,      // Thermal conductivity (W/(m·K))
        896.0       // Specific heat (J/(kg·K))
    ));
    
    // Titanium (Ti-6Al-4V)
    materials.push_back(createIsotropic(
        "Titanium (Ti-6Al-4V)",
        113.8e9,    // Young's modulus (Pa)
        0.342,      // Poisson's ratio
        4430.0,     // Density (kg/m³)
        8.6e-6,     // Thermal expansion coefficient (1/K)
        6.7,        // Thermal conductivity (W/(m·K))
        526.3       // Specific heat (J/(kg·K))
    ));
    
    // Concrete
    materials.push_back(createIsotropic(
        "Concrete",
        30.0e9,     // Young's modulus (Pa)
        0.2,        // Poisson's ratio
        2400.0,     // Density (kg/m³)
        1.0e-5,     // Thermal expansion coefficient (1/K)
        1.7,        // Thermal conductivity (W/(m·K))
        880.0       // Specific heat (J/(kg·K))
    ));
    
    // Plastic (ABS)
    materials.push_back(createIsotropic(
        "Plastic (ABS)",
        2.3e9,      // Young's modulus (Pa)
        0.35,       // Poisson's ratio
        1050.0,     // Density (kg/m³)
        7.4e-5,     // Thermal expansion coefficient (1/K)
        0.17,       // Thermal conductivity (W/(m·K))
        1470.0      // Specific heat (J/(kg·K))
    ));
    
    // Glass
    materials.push_back(createIsotropic(
        "Glass",
        70.0e9,     // Young's modulus (Pa)
        0.22,       // Poisson's ratio
        2500.0,     // Density (kg/m³)
        9.0e-6,     // Thermal expansion coefficient (1/K)
        1.05,       // Thermal conductivity (W/(m·K))
        840.0       // Specific heat (J/(kg·K))
    ));
    
    // Rubber
    materials.push_back(createIsotropic(
        "Rubber",
        0.01e9,     // Young's modulus (Pa)
        0.49,       // Poisson's ratio
        1100.0,     // Density (kg/m³)
        2.0e-4,     // Thermal expansion coefficient (1/K)
        0.16,       // Thermal conductivity (W/(m·K))
        1800.0      // Specific heat (J/(kg·K))
    ));
    
    // Wood (Oak) - Orthotropic
    materials.push_back(createOrthotropic(
        "Wood (Oak)",
        11.0e9,     // Young's modulus X (Pa)
        1.0e9,      // Young's modulus Y (Pa)
        0.67e9,     // Young's modulus Z (Pa)
        0.37,       // Poisson's ratio XY
        0.48,       // Poisson's ratio YZ
        0.43,       // Poisson's ratio XZ
        0.93e9,     // Shear modulus XY (Pa)
        0.21e9,     // Shear modulus YZ (Pa)
        0.25e9,     // Shear modulus XZ (Pa)
        750.0       // Density (kg/m³)
    ));
    
    // Steel (Structural) - Elasto-plastic
    materials.push_back(createElastoPlastic(
        "Steel (Structural) - Elasto-plastic",
        210.0e9,    // Young's modulus (Pa)
        0.3,        // Poisson's ratio
        7850.0,     // Density (kg/m³)
        250.0e6,    // Yield strength (Pa)
        400.0e6,    // Ultimate strength (Pa)
        21.0e9      // Hardening modulus (Pa)
    ));
    
    // Aluminum (6061-T6) - Elasto-plastic
    materials.push_back(createElastoPlastic(
        "Aluminum (6061-T6) - Elasto-plastic",
        68.9e9,     // Young's modulus (Pa)
        0.33,       // Poisson's ratio
        2700.0,     // Density (kg/m³)
        276.0e6,    // Yield strength (Pa)
        310.0e6,    // Ultimate strength (Pa)
        6.89e9      // Hardening modulus (Pa)
    ));
    
    return materials;
}

} // namespace rebel::simulation
