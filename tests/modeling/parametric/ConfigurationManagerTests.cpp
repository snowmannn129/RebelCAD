/**
 * @file ConfigurationManagerTests.cpp
 * @brief Unit tests for the ConfigurationManager class
 */

#include <gtest/gtest.h>
#include "../../../src/modeling/parametric/ConfigurationManager.h"
#include "../../../src/modeling/parametric/Configuration.h"
#include "../../../src/modeling/parametric/ParameterManager.h"
#include "../../../src/modeling/parametric/DependencyGraph.h"
#include <string>
#include <memory>

using namespace rebel_cad::modeling;

// Test fixture for ConfigurationManager tests
class ConfigurationManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create the parameter manager and dependency graph
        m_paramManager = std::make_unique<ParameterManager>();
        m_graph = std::make_unique<DependencyGraph>();
        
        // Create a test feature and add a parameter
        m_paramManager->createParameter("feature1", "param1", 10.0, Parameter::Type::Length);
        m_paramManager->createParameter("feature1", "param2", 20.0, Parameter::Type::Length);
        m_paramManager->createParameter("feature2", "param1", 30.0, Parameter::Type::Length);
        
        // Create the configuration manager
        m_configManager = std::make_unique<ConfigurationManager>(*m_paramManager, *m_graph);
    }

    void TearDown() override {
        m_configManager.reset();
        m_graph.reset();
        m_paramManager.reset();
    }

    std::unique_ptr<ParameterManager> m_paramManager;
    std::unique_ptr<DependencyGraph> m_graph;
    std::unique_ptr<ConfigurationManager> m_configManager;
};

// Test constructor and default configuration
TEST_F(ConfigurationManagerTest, ConstructorAndDefaultConfiguration) {
    // Check that a default configuration was created
    Configuration* defaultConfig = m_configManager->getConfiguration("Default");
    EXPECT_NE(nullptr, defaultConfig);
    EXPECT_EQ("Default", defaultConfig->getName());
    EXPECT_EQ("Default configuration", defaultConfig->getDescription());
    
    // Check that the default configuration is active
    Configuration* activeConfig = m_configManager->getActiveConfiguration();
    EXPECT_NE(nullptr, activeConfig);
    EXPECT_EQ("Default", activeConfig->getName());
}

// Test creating and retrieving configurations
TEST_F(ConfigurationManagerTest, CreateAndRetrieveConfigurations) {
    // Create a new configuration
    EXPECT_TRUE(m_configManager->createConfiguration("Config1", "Test configuration 1"));
    
    // Check that the configuration was created
    Configuration* config = m_configManager->getConfiguration("Config1");
    EXPECT_NE(nullptr, config);
    EXPECT_EQ("Config1", config->getName());
    EXPECT_EQ("Test configuration 1", config->getDescription());
    
    // Try to create a configuration with the same name
    EXPECT_FALSE(m_configManager->createConfiguration("Config1", "Duplicate configuration"));
    
    // Create another configuration
    EXPECT_TRUE(m_configManager->createConfiguration("Config2", "Test configuration 2"));
    
    // Get all configurations
    auto configs = m_configManager->getAllConfigurations();
    EXPECT_EQ(3, configs.size()); // Default, Config1, Config2
}

// Test removing configurations
TEST_F(ConfigurationManagerTest, RemoveConfigurations) {
    // Create configurations
    m_configManager->createConfiguration("Config1");
    m_configManager->createConfiguration("Config2");
    
    // Remove a configuration
    EXPECT_TRUE(m_configManager->removeConfiguration("Config1"));
    
    // Check that the configuration was removed
    EXPECT_EQ(nullptr, m_configManager->getConfiguration("Config1"));
    
    // Try to remove a non-existent configuration
    EXPECT_FALSE(m_configManager->removeConfiguration("NonExistent"));
    
    // Remove the active configuration
    m_configManager->setActiveConfiguration("Config2");
    EXPECT_TRUE(m_configManager->removeConfiguration("Config2"));
    
    // Check that a new active configuration was set
    EXPECT_EQ("Default", m_configManager->getActiveConfiguration()->getName());
}

// Test renaming configurations
TEST_F(ConfigurationManagerTest, RenameConfigurations) {
    // Create a configuration
    m_configManager->createConfiguration("Config1", "Test configuration");
    
    // Rename the configuration
    EXPECT_TRUE(m_configManager->renameConfiguration("Config1", "NewName"));
    
    // Check that the configuration was renamed
    EXPECT_EQ(nullptr, m_configManager->getConfiguration("Config1"));
    Configuration* config = m_configManager->getConfiguration("NewName");
    EXPECT_NE(nullptr, config);
    EXPECT_EQ("NewName", config->getName());
    EXPECT_EQ("Test configuration", config->getDescription());
    
    // Try to rename a non-existent configuration
    EXPECT_FALSE(m_configManager->renameConfiguration("NonExistent", "NewName2"));
    
    // Try to rename to an existing name
    m_configManager->createConfiguration("Config2");
    EXPECT_FALSE(m_configManager->renameConfiguration("Config2", "NewName"));
    
    // Rename the active configuration
    m_configManager->setActiveConfiguration("NewName");
    EXPECT_TRUE(m_configManager->renameConfiguration("NewName", "ActiveConfig"));
    
    // Check that the active configuration was updated
    EXPECT_EQ("ActiveConfig", m_configManager->getActiveConfiguration()->getName());
}

// Test setting and getting the active configuration
TEST_F(ConfigurationManagerTest, ActiveConfiguration) {
    // Create configurations
    m_configManager->createConfiguration("Config1");
    m_configManager->createConfiguration("Config2");
    
    // Set the active configuration
    EXPECT_TRUE(m_configManager->setActiveConfiguration("Config1"));
    EXPECT_EQ("Config1", m_configManager->getActiveConfiguration()->getName());
    
    // Set another active configuration
    EXPECT_TRUE(m_configManager->setActiveConfiguration("Config2"));
    EXPECT_EQ("Config2", m_configManager->getActiveConfiguration()->getName());
    
    // Try to set a non-existent configuration as active
    EXPECT_FALSE(m_configManager->setActiveConfiguration("NonExistent"));
    EXPECT_EQ("Config2", m_configManager->getActiveConfiguration()->getName());
}

// Test applying configurations
TEST_F(ConfigurationManagerTest, ApplyConfiguration) {
    // Create a configuration
    m_configManager->createConfiguration("Config1");
    Configuration* config = m_configManager->getConfiguration("Config1");
    
    // Set parameter values in the configuration
    config->setParameterValue("feature1", "param1", 100.0);
    config->setParameterValue("feature1", "param2", 200.0);
    config->setParameterValue("feature2", "param1", 300.0);
    
    // Apply the configuration
    EXPECT_TRUE(m_configManager->applyConfiguration("Config1"));
    
    // Check that the parameter values were applied
    EXPECT_EQ(100.0, m_paramManager->getParameterValue("feature1", "param1"));
    EXPECT_EQ(200.0, m_paramManager->getParameterValue("feature1", "param2"));
    EXPECT_EQ(300.0, m_paramManager->getParameterValue("feature2", "param1"));
    
    // Try to apply a non-existent configuration
    EXPECT_FALSE(m_configManager->applyConfiguration("NonExistent"));
}

// Test updating configurations from the model
TEST_F(ConfigurationManagerTest, UpdateConfigurationFromModel) {
    // Create a configuration
    m_configManager->createConfiguration("Config1");
    
    // Set parameter values in the model
    m_paramManager->setParameterValue("feature1", "param1", 100.0);
    m_paramManager->setParameterValue("feature1", "param2", 200.0);
    m_paramManager->setParameterValue("feature2", "param1", 300.0);
    
    // Update the configuration from the model
    EXPECT_TRUE(m_configManager->updateConfigurationFromModel("Config1"));
    
    // Check that the configuration was updated
    Configuration* config = m_configManager->getConfiguration("Config1");
    EXPECT_EQ(100.0, config->getParameterValue("feature1", "param1"));
    EXPECT_EQ(200.0, config->getParameterValue("feature1", "param2"));
    EXPECT_EQ(300.0, config->getParameterValue("feature2", "param1"));
    
    // Update only specific parameters
    m_paramManager->setParameterValue("feature1", "param1", 1000.0);
    m_paramManager->setParameterValue("feature1", "param2", 2000.0);
    
    EXPECT_TRUE(m_configManager->updateConfigurationFromModel("Config1", {"feature1:param1"}));
    
    // Check that only the specified parameter was updated
    EXPECT_EQ(1000.0, config->getParameterValue("feature1", "param1"));
    EXPECT_EQ(200.0, config->getParameterValue("feature1", "param2"));
    EXPECT_EQ(300.0, config->getParameterValue("feature2", "param1"));
    
    // Try to update a non-existent configuration
    EXPECT_FALSE(m_configManager->updateConfigurationFromModel("NonExistent"));
}

// Test serialization and deserialization
TEST_F(ConfigurationManagerTest, SerializationDeserialization) {
    // Create configurations
    m_configManager->createConfiguration("Config1", "Test configuration 1");
    m_configManager->createConfiguration("Config2", "Test configuration 2");
    
    // Set parameter values
    Configuration* config1 = m_configManager->getConfiguration("Config1");
    config1->setParameterValue("feature1", "param1", 100.0);
    config1->setParameterValue("feature1", "param2", 200.0);
    
    Configuration* config2 = m_configManager->getConfiguration("Config2");
    config2->setParameterValue("feature2", "param1", 300.0);
    
    // Set the active configuration
    m_configManager->setActiveConfiguration("Config1");
    
    // Serialize
    std::string serialized = m_configManager->serialize();
    
    // Create a new configuration manager
    auto newParamManager = std::make_unique<ParameterManager>();
    auto newGraph = std::make_unique<DependencyGraph>();
    auto newConfigManager = std::make_unique<ConfigurationManager>(*newParamManager, *newGraph);
    
    // Deserialize
    EXPECT_TRUE(newConfigManager->deserialize(serialized));
    
    // Check that the configurations were deserialized
    EXPECT_NE(nullptr, newConfigManager->getConfiguration("Config1"));
    EXPECT_NE(nullptr, newConfigManager->getConfiguration("Config2"));
    
    // Check that the active configuration was deserialized
    EXPECT_EQ("Config1", newConfigManager->getActiveConfiguration()->getName());
    
    // Check that the parameter values were deserialized
    Configuration* newConfig1 = newConfigManager->getConfiguration("Config1");
    EXPECT_EQ(100.0, newConfig1->getParameterValue("feature1", "param1"));
    EXPECT_EQ(200.0, newConfig1->getParameterValue("feature1", "param2"));
    
    Configuration* newConfig2 = newConfigManager->getConfiguration("Config2");
    EXPECT_EQ(300.0, newConfig2->getParameterValue("feature2", "param1"));
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
