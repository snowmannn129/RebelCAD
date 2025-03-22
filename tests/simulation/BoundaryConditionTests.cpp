/**
 * @file BoundaryConditionTests.cpp
 * @brief Unit tests for the BoundaryCondition class and its derived classes
 */

#include "simulation/BoundaryCondition.h"
#include "simulation/Mesh.h"
#include "simulation/Material.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

using namespace rebel::simulation;

/**
 * @brief Test fixture for BoundaryCondition tests
 */
class BoundaryConditionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple mesh for testing
        mesh = Mesh::createFromGeometry("test_geometry", ElementType::Tetra, 1);
        ASSERT_NE(mesh, nullptr);
        
        // Create a material for element groups
        steel = Material::createIsotropic("Steel", 210.0e9, 0.3, 7850.0);
        ASSERT_NE(steel, nullptr);
        
        // Add node groups
        leftNodesId = mesh->createNodeGroup("LeftNodes");
        rightNodesId = mesh->createNodeGroup("RightNodes");
        topNodesId = mesh->createNodeGroup("TopNodes");
        
        // Add element groups
        leftElementsId = mesh->createElementGroup("LeftElements", steel);
        rightElementsId = mesh->createElementGroup("RightElements", steel);
        topElementsId = mesh->createElementGroup("TopElements", steel);
        
        // Ensure groups were created successfully
        ASSERT_GE(leftNodesId, 0);
        ASSERT_GE(rightNodesId, 0);
        ASSERT_GE(topNodesId, 0);
        ASSERT_GE(leftElementsId, 0);
        ASSERT_GE(rightElementsId, 0);
        ASSERT_GE(topElementsId, 0);
    }

    void TearDown() override {
        // Clean up
        mesh.reset();
        steel.reset();
    }

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> steel;
    int leftNodesId, rightNodesId, topNodesId;
    int leftElementsId, rightElementsId, topElementsId;
};

/**
 * @brief Test creating a displacement boundary condition
 */
TEST_F(BoundaryConditionTest, CreateDisplacement) {
    // Create a displacement boundary condition
    auto bc = BoundaryCondition::createDisplacement(
        "fixed_left",
        "LeftNodes",
        DisplacementDirection::XYZ,
        0.0
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "fixed_left");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Displacement);
    EXPECT_EQ(bc->getGroupName(), "LeftNodes");
    
    // Cast to DisplacementBC to access specific methods
    auto displacementBC = std::dynamic_pointer_cast<DisplacementBC>(bc);
    ASSERT_NE(displacementBC, nullptr);
    EXPECT_EQ(displacementBC->getDirection(), DisplacementDirection::XYZ);
    EXPECT_DOUBLE_EQ(displacementBC->getValue(), 0.0);
    
    // Verify string representation
    EXPECT_FALSE(displacementBC->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(displacementBC->isValid(*mesh));
}

/**
 * @brief Test creating a force boundary condition
 */
TEST_F(BoundaryConditionTest, CreateForce) {
    // Create a force boundary condition
    auto bc = BoundaryCondition::createForce(
        "force_right",
        "RightNodes",
        100.0,  // X component
        0.0,    // Y component
        0.0     // Z component
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "force_right");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Force);
    EXPECT_EQ(bc->getGroupName(), "RightNodes");
    
    // Cast to ForceBC to access specific methods
    auto forceBC = std::dynamic_pointer_cast<ForceBC>(bc);
    ASSERT_NE(forceBC, nullptr);
    EXPECT_DOUBLE_EQ(forceBC->getForceX(), 100.0);
    EXPECT_DOUBLE_EQ(forceBC->getForceY(), 0.0);
    EXPECT_DOUBLE_EQ(forceBC->getForceZ(), 0.0);
    
    // Verify string representation
    EXPECT_FALSE(forceBC->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(forceBC->isValid(*mesh));
}

/**
 * @brief Test creating a pressure boundary condition
 */
TEST_F(BoundaryConditionTest, CreatePressure) {
    // Create a pressure boundary condition
    auto bc = BoundaryCondition::createPressure(
        "pressure_top",
        "TopElements",
        1.0e6  // 1 MPa
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "pressure_top");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Pressure);
    EXPECT_EQ(bc->getGroupName(), "TopElements");
    
    // Cast to PressureBC to access specific methods
    auto pressureBC = std::dynamic_pointer_cast<PressureBC>(bc);
    ASSERT_NE(pressureBC, nullptr);
    EXPECT_DOUBLE_EQ(pressureBC->getPressure(), 1.0e6);
    
    // Verify string representation
    EXPECT_FALSE(pressureBC->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(pressureBC->isValid(*mesh));
}

/**
 * @brief Test creating a temperature boundary condition
 */
TEST_F(BoundaryConditionTest, CreateTemperature) {
    // Create a temperature boundary condition
    auto bc = BoundaryCondition::createTemperature(
        "temp_left",
        "LeftNodes",
        100.0  // 100°C
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "temp_left");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Temperature);
    EXPECT_EQ(bc->getGroupName(), "LeftNodes");
    
    // Verify string representation
    EXPECT_FALSE(bc->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(bc->isValid(*mesh));
}

/**
 * @brief Test creating a heat flux boundary condition
 */
TEST_F(BoundaryConditionTest, CreateHeatFlux) {
    // Create a heat flux boundary condition
    auto bc = BoundaryCondition::createHeatFlux(
        "flux_right",
        "RightElements",
        1000.0  // 1000 W/m²
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "flux_right");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::HeatFlux);
    EXPECT_EQ(bc->getGroupName(), "RightElements");
    
    // Verify string representation
    EXPECT_FALSE(bc->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(bc->isValid(*mesh));
}

/**
 * @brief Test creating a convection boundary condition
 */
TEST_F(BoundaryConditionTest, CreateConvection) {
    // Create a convection boundary condition
    auto bc = BoundaryCondition::createConvection(
        "conv_top",
        "TopElements",
        25.0,   // Film coefficient (W/m²·K)
        20.0    // Ambient temperature (°C)
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "conv_top");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Convection);
    EXPECT_EQ(bc->getGroupName(), "TopElements");
    
    // Verify string representation
    EXPECT_FALSE(bc->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(bc->isValid(*mesh));
}

/**
 * @brief Test creating a symmetry boundary condition
 */
TEST_F(BoundaryConditionTest, CreateSymmetry) {
    // Create a symmetry boundary condition
    auto bc = BoundaryCondition::createSymmetry(
        "sym_left",
        "LeftNodes",
        DisplacementDirection::X
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "sym_left");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Symmetry);
    EXPECT_EQ(bc->getGroupName(), "LeftNodes");
    
    // Verify string representation
    EXPECT_FALSE(bc->toString().empty());
    
    // Verify validity
    EXPECT_TRUE(bc->isValid(*mesh));
}

/**
 * @brief Test invalid boundary condition
 */
TEST_F(BoundaryConditionTest, InvalidBoundaryCondition) {
    // Create a boundary condition with an invalid node group
    auto bc = BoundaryCondition::createDisplacement(
        "invalid",
        "NonExistentGroup",
        DisplacementDirection::XYZ,
        0.0
    );
    
    // Verify the boundary condition properties
    ASSERT_NE(bc, nullptr);
    EXPECT_EQ(bc->getId(), "invalid");
    EXPECT_EQ(bc->getType(), BoundaryConditionType::Displacement);
    EXPECT_EQ(bc->getGroupName(), "NonExistentGroup");
    
    // Verify validity
    EXPECT_FALSE(bc->isValid(*mesh));
}

/**
 * @brief Test applying boundary conditions
 */
TEST_F(BoundaryConditionTest, ApplyBoundaryCondition) {
    // Create a displacement boundary condition
    auto bc = BoundaryCondition::createDisplacement(
        "fixed_left",
        "LeftNodes",
        DisplacementDirection::XYZ,
        0.0
    );
    
    // Apply the boundary condition
    EXPECT_TRUE(bc->apply(*mesh));
    
    // Create a boundary condition with an invalid node group
    auto invalidBC = BoundaryCondition::createDisplacement(
        "invalid",
        "NonExistentGroup",
        DisplacementDirection::XYZ,
        0.0
    );
    
    // Apply the invalid boundary condition
    EXPECT_FALSE(invalidBC->apply(*mesh));
}
