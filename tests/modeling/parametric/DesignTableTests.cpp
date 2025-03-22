/**
 * @file DesignTableTests.cpp
 * @brief Unit tests for the DesignTable class
 */

#include <gtest/gtest.h>
#include "../../../src/modeling/parametric/DesignTable.h"
#include "../../../src/modeling/parametric/ConfigurationManager.h"
#include "../../../src/modeling/parametric/Configuration.h"
#include "../../../src/modeling/parametric/ParameterManager.h"
#include "../../../src/modeling/parametric/DependencyGraph.h"
#include <string>
#include <memory>
#include <fstream>

using namespace rebel_cad::modeling;

// Test fixture for DesignTable tests
class DesignTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create the parameter manager and dependency graph
        m_paramManager = std::make_unique<ParameterManager>();
        m_graph = std::make_unique<DependencyGraph>();
        
        // Create a test feature and add parameters
        m_paramManager->createParameter("feature1", "param1", 10.0, Parameter::Type::Length);
        m_paramManager->createParameter("feature1", "param2", 20.0, Parameter::Type::Length);
        m_paramManager->createParameter("feature2", "param1", 30.0, Parameter::Type::Length);
        
        // Create the configuration manager
        m_configManager = std::make_unique<ConfigurationManager>(*m_paramManager, *m_graph);
        
        // Create additional configurations
        m_configManager->createConfiguration("Config1", "Test configuration 1");
        m_configManager->createConfiguration("Config2", "Test configuration 2");
        
        // Set parameter values in the configurations
        Configuration* config1 = m_configManager->getConfiguration("Config1");
        config1->setParameterValue("feature1", "param1", 100.0);
        config1->setParameterValue("feature1", "param2", 200.0);
        
        Configuration* config2 = m_configManager->getConfiguration("Config2");
        config2->setParameterValue("feature1", "param1", 1000.0);
        config2->setParameterValue("feature2", "param1", 3000.0);
        
        // Create the design table
        m_designTable = std::make_unique<DesignTable>(*m_configManager, *m_paramManager);
    }

    void TearDown() override {
        m_designTable.reset();
        m_configManager.reset();
        m_graph.reset();
        m_paramManager.reset();
    }

    std::unique_ptr<ParameterManager> m_paramManager;
    std::unique_ptr<DependencyGraph> m_graph;
    std::unique_ptr<ConfigurationManager> m_configManager;
    std::unique_ptr<DesignTable> m_designTable;
};

// Test adding and removing parameters
TEST_F(DesignTableTest, AddRemoveParameters) {
    // Add parameters to the design table
    EXPECT_TRUE(m_designTable->addParameter("feature1", "param1"));
    EXPECT_TRUE(m_designTable->addParameter("feature1", "param2"));
    EXPECT_TRUE(m_designTable->addParameter("feature2", "param1"));
    
    // Check that the parameters were added
    auto params = m_designTable->getParameters();
    EXPECT_EQ(3, params.size());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param1") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param2") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature2:param1") != params.end());
    
    // Try to add a parameter that already exists
    EXPECT_FALSE(m_designTable->addParameter("feature1", "param1"));
    
    // Remove a parameter
    EXPECT_TRUE(m_designTable->removeParameter("feature1", "param1"));
    
    // Check that the parameter was removed
    params = m_designTable->getParameters();
    EXPECT_EQ(2, params.size());
    EXPECT_FALSE(std::find(params.begin(), params.end(), "feature1:param1") != params.end());
    
    // Try to remove a parameter that doesn't exist
    EXPECT_FALSE(m_designTable->removeParameter("feature3", "param1"));
}

// Test adding and removing configurations
TEST_F(DesignTableTest, AddRemoveConfigurations) {
    // Add parameters to the design table
    m_designTable->addParameter("feature1", "param1");
    m_designTable->addParameter("feature1", "param2");
    m_designTable->addParameter("feature2", "param1");
    
    // Add configurations to the design table
    EXPECT_TRUE(m_designTable->addConfiguration("Default"));
    EXPECT_TRUE(m_designTable->addConfiguration("Config1"));
    EXPECT_TRUE(m_designTable->addConfiguration("Config2"));
    
    // Check that the configurations were added
    auto configs = m_designTable->getConfigurations();
    EXPECT_EQ(3, configs.size());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Default") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config1") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config2") != configs.end());
    
    // Try to add a configuration that already exists
    EXPECT_FALSE(m_designTable->addConfiguration("Config1"));
    
    // Remove a configuration
    EXPECT_TRUE(m_designTable->removeConfiguration("Config1"));
    
    // Check that the configuration was removed
    configs = m_designTable->getConfigurations();
    EXPECT_EQ(2, configs.size());
    EXPECT_FALSE(std::find(configs.begin(), configs.end(), "Config1") != configs.end());
    
    // Try to remove a configuration that doesn't exist
    EXPECT_FALSE(m_designTable->removeConfiguration("NonExistent"));
}

// Test setting and getting parameter values
TEST_F(DesignTableTest, ParameterValues) {
    // Add parameters to the design table
    m_designTable->addParameter("feature1", "param1");
    m_designTable->addParameter("feature1", "param2");
    m_designTable->addParameter("feature2", "param1");
    
    // Add configurations to the design table
    m_designTable->addConfiguration("Default");
    m_designTable->addConfiguration("Config1");
    m_designTable->addConfiguration("Config2");
    
    // Set parameter values
    EXPECT_TRUE(m_designTable->setParameterValue("Config1", "feature1", "param1", 100.0));
    EXPECT_TRUE(m_designTable->setParameterValue("Config1", "feature1", "param2", 200.0));
    EXPECT_TRUE(m_designTable->setParameterValue("Config2", "feature1", "param1", 1000.0));
    EXPECT_TRUE(m_designTable->setParameterValue("Config2", "feature2", "param1", 3000.0));
    
    // Check parameter values
    EXPECT_EQ(100.0, m_designTable->getParameterValue("Config1", "feature1", "param1"));
    EXPECT_EQ(200.0, m_designTable->getParameterValue("Config1", "feature1", "param2"));
    EXPECT_EQ(1000.0, m_designTable->getParameterValue("Config2", "feature1", "param1"));
    EXPECT_EQ(3000.0, m_designTable->getParameterValue("Config2", "feature2", "param1"));
    
    // Check default values for non-existent parameters
    EXPECT_EQ(0.0, m_designTable->getParameterValue("Config1", "feature2", "param1"));
    EXPECT_EQ(5.0, m_designTable->getParameterValue("Config1", "feature3", "param1", 5.0));
    
    // Try to set a parameter value for a non-existent configuration
    EXPECT_FALSE(m_designTable->setParameterValue("NonExistent", "feature1", "param1", 100.0));
    
    // Set a parameter value for a non-existent parameter (should add the parameter)
    EXPECT_TRUE(m_designTable->setParameterValue("Config1", "feature3", "param1", 300.0));
    EXPECT_EQ(300.0, m_designTable->getParameterValue("Config1", "feature3", "param1"));
}

// Test updating the design table from configurations
TEST_F(DesignTableTest, UpdateFromConfigurations) {
    // Update the design table from configurations
    EXPECT_TRUE(m_designTable->updateFromConfigurations());
    
    // Check that the parameters were added
    auto params = m_designTable->getParameters();
    EXPECT_EQ(3, params.size());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param1") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param2") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature2:param1") != params.end());
    
    // Check that the configurations were added
    auto configs = m_designTable->getConfigurations();
    EXPECT_EQ(3, configs.size());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Default") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config1") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config2") != configs.end());
    
    // Check that the parameter values were added
    EXPECT_EQ(100.0, m_designTable->getParameterValue("Config1", "feature1", "param1"));
    EXPECT_EQ(200.0, m_designTable->getParameterValue("Config1", "feature1", "param2"));
    EXPECT_EQ(1000.0, m_designTable->getParameterValue("Config2", "feature1", "param1"));
    EXPECT_EQ(3000.0, m_designTable->getParameterValue("Config2", "feature2", "param1"));
}

// Test updating configurations from the design table
TEST_F(DesignTableTest, UpdateConfigurations) {
    // Update the design table from configurations
    m_designTable->updateFromConfigurations();
    
    // Modify parameter values in the design table
    m_designTable->setParameterValue("Config1", "feature1", "param1", 150.0);
    m_designTable->setParameterValue("Config2", "feature2", "param1", 3500.0);
    
    // Update configurations from the design table
    EXPECT_TRUE(m_designTable->updateConfigurations());
    
    // Check that the configurations were updated
    Configuration* config1 = m_configManager->getConfiguration("Config1");
    EXPECT_EQ(150.0, config1->getParameterValue("feature1", "param1"));
    
    Configuration* config2 = m_configManager->getConfiguration("Config2");
    EXPECT_EQ(3500.0, config2->getParameterValue("feature2", "param1"));
}

// Test serialization and deserialization
TEST_F(DesignTableTest, SerializationDeserialization) {
    // Update the design table from configurations
    m_designTable->updateFromConfigurations();
    
    // Serialize the design table
    std::string serialized = m_designTable->serialize();
    
    // Create a new design table
    auto newParamManager = std::make_unique<ParameterManager>();
    auto newGraph = std::make_unique<DependencyGraph>();
    auto newConfigManager = std::make_unique<ConfigurationManager>(*newParamManager, *newGraph);
    auto newDesignTable = std::make_unique<DesignTable>(*newConfigManager, *newParamManager);
    
    // Deserialize the design table
    EXPECT_TRUE(newDesignTable->deserialize(serialized));
    
    // Check that the parameters were deserialized
    auto params = newDesignTable->getParameters();
    EXPECT_EQ(3, params.size());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param1") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param2") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature2:param1") != params.end());
    
    // Check that the configurations were deserialized
    auto configs = newDesignTable->getConfigurations();
    EXPECT_EQ(3, configs.size());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Default") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config1") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config2") != configs.end());
    
    // Check that the parameter values were deserialized
    EXPECT_EQ(100.0, newDesignTable->getParameterValue("Config1", "feature1", "param1"));
    EXPECT_EQ(200.0, newDesignTable->getParameterValue("Config1", "feature1", "param2"));
    EXPECT_EQ(1000.0, newDesignTable->getParameterValue("Config2", "feature1", "param1"));
    EXPECT_EQ(3000.0, newDesignTable->getParameterValue("Config2", "feature2", "param1"));
}

// Test CSV import and export
TEST_F(DesignTableTest, CsvImportExport) {
    // Update the design table from configurations
    m_designTable->updateFromConfigurations();
    
    // Export the design table to a CSV file
    std::string csvPath = "design_table_test.csv";
    EXPECT_TRUE(m_designTable->exportToCsv(csvPath));
    
    // Create a new design table
    auto newParamManager = std::make_unique<ParameterManager>();
    auto newGraph = std::make_unique<DependencyGraph>();
    auto newConfigManager = std::make_unique<ConfigurationManager>(*newParamManager, *newGraph);
    auto newDesignTable = std::make_unique<DesignTable>(*newConfigManager, *newParamManager);
    
    // Import the design table from the CSV file
    EXPECT_TRUE(newDesignTable->importFromCsv(csvPath));
    
    // Check that the parameters were imported
    auto params = newDesignTable->getParameters();
    EXPECT_EQ(3, params.size());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param1") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature1:param2") != params.end());
    EXPECT_TRUE(std::find(params.begin(), params.end(), "feature2:param1") != params.end());
    
    // Check that the configurations were imported
    auto configs = newDesignTable->getConfigurations();
    EXPECT_EQ(3, configs.size());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Default") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config1") != configs.end());
    EXPECT_TRUE(std::find(configs.begin(), configs.end(), "Config2") != configs.end());
    
    // Check that the parameter values were imported
    EXPECT_EQ(100.0, newDesignTable->getParameterValue("Config1", "feature1", "param1"));
    EXPECT_EQ(200.0, newDesignTable->getParameterValue("Config1", "feature1", "param2"));
    EXPECT_EQ(1000.0, newDesignTable->getParameterValue("Config2", "feature1", "param1"));
    EXPECT_EQ(3000.0, newDesignTable->getParameterValue("Config2", "feature2", "param1"));
    
    // Clean up the CSV file
    std::remove(csvPath.c_str());
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
