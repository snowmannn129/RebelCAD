/**
 * @file MaterialTests.cpp
 * @brief Unit tests for the Material class
 */

#include "simulation/Material.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace rebel::simulation;

/**
 * @brief Test fixture for Material tests
 */
class MaterialTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }

    void TearDown() override {
        // Common cleanup for all tests
    }
};

/**
 * @brief Test creating an isotropic material
 */
TEST_F(MaterialTest, CreateIsotropic) {
    // Create an isotropic material
    auto steel = Material::createIsotropic(
        "Steel",
        210.0e9,    // Young's modulus (Pa)
        0.3,        // Poisson's ratio
        7850.0      // Density (kg/m³)
    );
    
    // Verify the material properties
    ASSERT_NE(steel, nullptr);
    EXPECT_EQ(steel->getName(), "Steel");
    EXPECT_EQ(steel->getType(), MaterialType::Isotropic);
    EXPECT_DOUBLE_EQ(steel->getYoungsModulus(), 210.0e9);
    EXPECT_DOUBLE_EQ(steel->getPoissonsRatio(), 0.3);
    EXPECT_DOUBLE_EQ(steel->getDensity(), 7850.0);
}

/**
 * @brief Test creating an orthotropic material
 */
TEST_F(MaterialTest, CreateOrthotropic) {
    // Create an orthotropic material
    auto wood = Material::createOrthotropic(
        "Wood",
        11.0e9,     // Young's modulus X (Pa)
        0.7e9,      // Young's modulus Y (Pa)
        0.5e9,      // Young's modulus Z (Pa)
        0.35,       // Poisson's ratio XY
        0.4,        // Poisson's ratio YZ
        0.3,        // Poisson's ratio XZ
        0.6e9,      // Shear modulus XY (Pa)
        0.7e9,      // Shear modulus YZ (Pa)
        0.8e9,      // Shear modulus XZ (Pa)
        600.0       // Density (kg/m³)
    );
    
    // Verify the material properties
    ASSERT_NE(wood, nullptr);
    EXPECT_EQ(wood->getName(), "Wood");
    EXPECT_EQ(wood->getType(), MaterialType::Orthotropic);
    
    EXPECT_DOUBLE_EQ(wood->getProperty("YoungModulusX"), 11.0e9);
    EXPECT_DOUBLE_EQ(wood->getProperty("YoungModulusY"), 0.7e9);
    EXPECT_DOUBLE_EQ(wood->getProperty("YoungModulusZ"), 0.5e9);
    
    EXPECT_DOUBLE_EQ(wood->getProperty("PoissonRatioXY"), 0.35);
    EXPECT_DOUBLE_EQ(wood->getProperty("PoissonRatioYZ"), 0.4);
    EXPECT_DOUBLE_EQ(wood->getProperty("PoissonRatioXZ"), 0.3);
    
    EXPECT_DOUBLE_EQ(wood->getProperty("ShearModulusXY"), 0.6e9);
    EXPECT_DOUBLE_EQ(wood->getProperty("ShearModulusYZ"), 0.7e9);
    EXPECT_DOUBLE_EQ(wood->getProperty("ShearModulusXZ"), 0.8e9);
    
    EXPECT_DOUBLE_EQ(wood->getDensity(), 600.0);
}

/**
 * @brief Test creating an elasto-plastic material
 */
TEST_F(MaterialTest, CreateElastoPlastic) {
    // Create an elasto-plastic material
    auto aluminum = Material::createElastoPlastic(
        "Aluminum",
        70.0e9,     // Young's modulus (Pa)
        0.33,       // Poisson's ratio
        2700.0,     // Density (kg/m³)
        240.0e6,    // Yield strength (Pa)
        300.0e6,    // Ultimate strength (Pa)
        50.0e9      // Hardening modulus (Pa)
    );
    
    // Verify the material properties
    ASSERT_NE(aluminum, nullptr);
    EXPECT_EQ(aluminum->getName(), "Aluminum");
    EXPECT_EQ(aluminum->getType(), MaterialType::ElastoPlastic);
    EXPECT_DOUBLE_EQ(aluminum->getYoungsModulus(), 70.0e9);
    EXPECT_DOUBLE_EQ(aluminum->getPoissonsRatio(), 0.33);
    EXPECT_DOUBLE_EQ(aluminum->getDensity(), 2700.0);
    EXPECT_DOUBLE_EQ(aluminum->getProperty("YieldStrength"), 240.0e6);
    EXPECT_DOUBLE_EQ(aluminum->getProperty("UltimateStrength"), 300.0e6);
    EXPECT_DOUBLE_EQ(aluminum->getProperty("HardeningModulus"), 50.0e9);
}

/**
 * @brief Test creating the common material library
 */
TEST_F(MaterialTest, CreateCommonMaterialLibrary) {
    // Create the common material library
    auto materials = Material::createCommonMaterialLibrary();
    
    // Verify that the library contains materials
    EXPECT_FALSE(materials.empty());
    
    // Verify that the library contains common materials
    bool hasSteel = false;
    bool hasAluminum = false;
    bool hasTitanium = false;
    
    for (const auto& material : materials) {
        if (material->getName() == "Steel") {
            hasSteel = true;
        } else if (material->getName() == "Aluminum") {
            hasAluminum = true;
        } else if (material->getName() == "Titanium") {
            hasTitanium = true;
        }
    }
    
    EXPECT_TRUE(hasSteel);
    EXPECT_TRUE(hasAluminum);
    EXPECT_TRUE(hasTitanium);
}

/**
 * @brief Test invalid material creation
 */
TEST_F(MaterialTest, InvalidMaterialCreation) {
    // Test creating an isotropic material with invalid parameters
    auto invalidMaterial = Material::createIsotropic(
        "",         // Empty name
        -1.0,       // Negative Young's modulus
        2.0,        // Poisson's ratio > 0.5
        -1.0        // Negative density
    );
    
    // Verify that the material is null or has default values
    if (invalidMaterial) {
        EXPECT_NE(invalidMaterial->getName(), "");
        EXPECT_GT(invalidMaterial->getYoungsModulus(), 0.0);
        EXPECT_LE(invalidMaterial->getPoissonsRatio(), 0.5);
        EXPECT_GT(invalidMaterial->getDensity(), 0.0);
    }
}

/**
 * @brief Test material equality
 */
TEST_F(MaterialTest, MaterialEquality) {
    // Create two identical materials
    auto steel1 = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
    auto steel2 = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
    
    // Create a different material
    auto aluminum = Material::createIsotropic("Aluminum", 70.0e9, 0.33, 2700.0);
    
    // Verify equality by comparing properties
    EXPECT_EQ(steel1->getName(), steel2->getName());
    EXPECT_EQ(steel1->getType(), steel2->getType());
    EXPECT_DOUBLE_EQ(steel1->getYoungsModulus(), steel2->getYoungsModulus());
    EXPECT_DOUBLE_EQ(steel1->getPoissonsRatio(), steel2->getPoissonsRatio());
    EXPECT_DOUBLE_EQ(steel1->getDensity(), steel2->getDensity());
    
    // Verify inequality
    EXPECT_NE(steel1->getName(), aluminum->getName());
    EXPECT_EQ(steel1->getType(), aluminum->getType()); // Both are isotropic
    EXPECT_NE(steel1->getYoungsModulus(), aluminum->getYoungsModulus());
    EXPECT_NE(steel1->getPoissonsRatio(), aluminum->getPoissonsRatio());
    EXPECT_NE(steel1->getDensity(), aluminum->getDensity());
}

/**
 * @brief Test material copy
 */
TEST_F(MaterialTest, MaterialCopy) {
    // Create a material
    auto steel = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
    
    // Copy the material
    auto steelCopy = std::make_shared<Material>(*steel);
    
    // Verify that the copy has the same properties
    EXPECT_EQ(steelCopy->getName(), steel->getName());
    EXPECT_EQ(steelCopy->getType(), steel->getType());
    EXPECT_DOUBLE_EQ(steelCopy->getYoungsModulus(), steel->getYoungsModulus());
    EXPECT_DOUBLE_EQ(steelCopy->getPoissonsRatio(), steel->getPoissonsRatio());
    EXPECT_DOUBLE_EQ(steelCopy->getDensity(), steel->getDensity());
}

/**
 * @brief Test material cloning
 */
TEST_F(MaterialTest, MaterialCloning) {
    // Create a material
    auto steel = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
    
    // Clone the material
    auto steelClone = steel->clone();
    
    // Verify that the clone has the same properties
    ASSERT_NE(steelClone, nullptr);
    EXPECT_EQ(steelClone->getName(), steel->getName());
    EXPECT_EQ(steelClone->getType(), steel->getType());
    EXPECT_DOUBLE_EQ(steelClone->getYoungsModulus(), steel->getYoungsModulus());
    EXPECT_DOUBLE_EQ(steelClone->getPoissonsRatio(), steel->getPoissonsRatio());
    EXPECT_DOUBLE_EQ(steelClone->getDensity(), steel->getDensity());
}
