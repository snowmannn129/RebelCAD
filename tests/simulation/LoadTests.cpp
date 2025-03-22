/**
 * @file LoadTests.cpp
 * @brief Unit tests for the Load class and its derived classes
 */

#include "simulation/Load.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace rebel::simulation;

/**
 * @brief Test fixture for Load tests
 */
class LoadTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple mesh for testing
        mesh = Mesh::createFromGeometry("test_geometry", ElementType::Tetra, 1);
        ASSERT_NE(mesh, nullptr);
        
        // Create a material for element groups
        material = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
        ASSERT_NE(material, nullptr);
        
        // Add node groups
        nodeGroupId = mesh->createNodeGroup("TestNodes");
        
        // Add element groups
        elementGroupId = mesh->createElementGroup("TestElements", material);
        
        // Ensure groups were created successfully
        ASSERT_GE(nodeGroupId, 0);
        ASSERT_GE(elementGroupId, 0);
    }

    void TearDown() override {
        // No cleanup needed
    }

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    int nodeGroupId;
    int elementGroupId;
};

/**
 * @brief Test creating a surface pressure load
 */
TEST_F(LoadTest, CreateSurfacePressure) {
    // Create a surface pressure load
    auto load = Load::createSurfacePressure(
        "pressure_test",
        "TestElements",
        1.0e6  // 1 MPa
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "pressure_test");
    EXPECT_EQ(load->getType(), LoadType::SurfacePressure);
    EXPECT_EQ(load->getGroupName(), "TestElements");
    
    // Cast to SurfacePressureLoad to access specific methods
    auto pressureLoad = std::dynamic_pointer_cast<SurfacePressureLoad>(load);
    ASSERT_NE(pressureLoad, nullptr);
    EXPECT_DOUBLE_EQ(pressureLoad->getPressure(), 1.0e6);
    
    // Verify string representation
    EXPECT_FALSE(pressureLoad->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(pressureLoad->isValid(*mesh));
}

/**
 * @brief Test creating a point force load
 */
TEST_F(LoadTest, CreatePointForce) {
    // Create a point force load
    auto load = Load::createPointForce(
        "force_test",
        "TestNodes",
        100.0,  // X component
        200.0,  // Y component
        300.0   // Z component
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "force_test");
    EXPECT_EQ(load->getType(), LoadType::PointForce);
    EXPECT_EQ(load->getGroupName(), "TestNodes");
    
    // Cast to PointForceLoad to access specific methods
    auto forceLoad = std::dynamic_pointer_cast<PointForceLoad>(load);
    ASSERT_NE(forceLoad, nullptr);
    EXPECT_DOUBLE_EQ(forceLoad->getForceX(), 100.0);
    EXPECT_DOUBLE_EQ(forceLoad->getForceY(), 200.0);
    EXPECT_DOUBLE_EQ(forceLoad->getForceZ(), 300.0);
    
    // Verify string representation
    EXPECT_FALSE(forceLoad->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(forceLoad->isValid(*mesh));
}

/**
 * @brief Test creating a body force load
 */
TEST_F(LoadTest, CreateBodyForce) {
    // Create a body force load
    auto load = Load::createBodyForce(
        "body_force_test",
        "TestElements",
        0.0,    // X component
        -9.81,  // Y component
        0.0     // Z component
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "body_force_test");
    EXPECT_EQ(load->getType(), LoadType::BodyForce);
    EXPECT_EQ(load->getGroupName(), "TestElements");
    
    // Cast to BodyForceLoad to access specific methods
    auto bodyForceLoad = std::dynamic_pointer_cast<BodyForceLoad>(load);
    ASSERT_NE(bodyForceLoad, nullptr);
    EXPECT_DOUBLE_EQ(bodyForceLoad->getForceX(), 0.0);
    EXPECT_DOUBLE_EQ(bodyForceLoad->getForceY(), -9.81);
    EXPECT_DOUBLE_EQ(bodyForceLoad->getForceZ(), 0.0);
    
    // Verify string representation
    EXPECT_FALSE(bodyForceLoad->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(bodyForceLoad->isValid(*mesh));
}

/**
 * @brief Test creating a gravity load
 */
TEST_F(LoadTest, CreateGravity) {
    // Create a gravity load
    auto load = Load::createGravity(
        "gravity_test",
        "TestElements",
        0.0,    // X component
        -9.81,  // Y component
        0.0     // Z component
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "gravity_test");
    EXPECT_EQ(load->getType(), LoadType::BodyForce);
    EXPECT_EQ(load->getGroupName(), "TestElements");
    
    // Verify string representation
    EXPECT_FALSE(load->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(load->isValid(*mesh));
}

/**
 * @brief Test creating a centrifugal load
 */
TEST_F(LoadTest, CreateCentrifugal) {
    // Create a centrifugal load
    auto load = Load::createCentrifugal(
        "centrifugal_test",
        "TestElements",
        0.0, 0.0, 0.0,  // Center point
        0.0, 0.0, 1.0,  // Axis direction
        100.0           // Angular velocity (rad/s)
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "centrifugal_test");
    EXPECT_EQ(load->getType(), LoadType::Centrifugal);
    EXPECT_EQ(load->getGroupName(), "TestElements");
    
    // Verify string representation
    EXPECT_FALSE(load->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(load->isValid(*mesh));
}

/**
 * @brief Test creating a thermal load
 */
TEST_F(LoadTest, CreateThermal) {
    // Create a thermal load
    auto load = Load::createThermal(
        "thermal_test",
        "TestNodes",
        100.0,  // Temperature (°C)
        20.0    // Reference temperature (°C)
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "thermal_test");
    EXPECT_EQ(load->getType(), LoadType::Thermal);
    EXPECT_EQ(load->getGroupName(), "TestNodes");
    
    // Verify string representation
    EXPECT_FALSE(load->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(load->isValid(*mesh));
}

/**
 * @brief Test load variation
 */
TEST_F(LoadTest, LoadVariation) {
    // Create a point force load with static variation
    auto staticLoad = Load::createPointForce(
        "static_force",
        "TestNodes",
        100.0, 0.0, 0.0,
        LoadVariation::Static
    );
    
    // Create a point force load with transient variation
    auto transientLoad = Load::createPointForce(
        "transient_force",
        "TestNodes",
        100.0, 0.0, 0.0,
        LoadVariation::Transient
    );
    
    // Verify the load variations
    ASSERT_NE(staticLoad, nullptr);
    ASSERT_NE(transientLoad, nullptr);
    EXPECT_EQ(staticLoad->getVariation(), LoadVariation::Static);
    EXPECT_EQ(transientLoad->getVariation(), LoadVariation::Transient);
}

/**
 * @brief Test invalid load
 */
TEST_F(LoadTest, InvalidLoad) {
    // Create a load with an invalid node group
    auto load = Load::createPointForce(
        "invalid_load",
        "NonExistentGroup",
        100.0, 0.0, 0.0
    );
    
    // Verify the load properties
    ASSERT_NE(load, nullptr);
    EXPECT_EQ(load->getId(), "invalid_load");
    EXPECT_EQ(load->getType(), LoadType::PointForce);
    EXPECT_EQ(load->getGroupName(), "NonExistentGroup");
    
    // Verify validity
    EXPECT_FALSE(load->isValid(*mesh));
}
