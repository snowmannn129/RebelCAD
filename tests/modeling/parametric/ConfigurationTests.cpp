/**
 * @file ConfigurationTests.cpp
 * @brief Unit tests for the Configuration class
 */

#include <gtest/gtest.h>
#include "../../../src/modeling/parametric/Configuration.h"
#include <string>

using namespace rebel_cad::modeling;

// Test fixture for Configuration tests
class ConfigurationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test configuration
        m_config = std::make_unique<Configuration>("TestConfig", "Test configuration");
    }

    void TearDown() override {
        m_config.reset();
    }

    std::unique_ptr<Configuration> m_config;
};

// Test constructor and basic getters
TEST_F(ConfigurationTest, ConstructorAndGetters) {
    EXPECT_EQ("TestConfig", m_config->getName());
    EXPECT_EQ("Test configuration", m_config->getDescription());
}

// Test setters
TEST_F(ConfigurationTest, Setters) {
    m_config->setName("NewName");
    m_config->setDescription("New description");
    
    EXPECT_EQ("NewName", m_config->getName());
    EXPECT_EQ("New description", m_config->getDescription());
}

// Test parameter value management
TEST_F(ConfigurationTest, ParameterValueManagement) {
    // Set parameter values
    EXPECT_TRUE(m_config->setParameterValue("feature1", "param1", 10.0));
    EXPECT_TRUE(m_config->setParameterValue("feature1", "param2", 20.0));
    EXPECT_TRUE(m_config->setParameterValue("feature2", "param1", 30.0));
    
    // Check parameter values
    EXPECT_EQ(10.0, m_config->getParameterValue("feature1", "param1"));
    EXPECT_EQ(20.0, m_config->getParameterValue("feature1", "param2"));
    EXPECT_EQ(30.0, m_config->getParameterValue("feature2", "param1"));
    
    // Check default value for non-existent parameter
    EXPECT_EQ(0.0, m_config->getParameterValue("feature3", "param1"));
    EXPECT_EQ(5.0, m_config->getParameterValue("feature3", "param1", 5.0));
    
    // Check hasParameterValue
    EXPECT_TRUE(m_config->hasParameterValue("feature1", "param1"));
    EXPECT_FALSE(m_config->hasParameterValue("feature3", "param1"));
    
    // Remove parameter value
    EXPECT_TRUE(m_config->removeParameterValue("feature1", "param1"));
    EXPECT_FALSE(m_config->hasParameterValue("feature1", "param1"));
    EXPECT_FALSE(m_config->removeParameterValue("feature3", "param1"));
    
    // Get all parameter values
    auto allValues = m_config->getAllParameterValues();
    EXPECT_EQ(2, allValues.size());
    
    // Clear parameter values
    m_config->clearParameterValues();
    EXPECT_EQ(0, m_config->getAllParameterValues().size());
}

// Test serialization and deserialization
TEST_F(ConfigurationTest, SerializationDeserialization) {
    // Set parameter values
    m_config->setParameterValue("feature1", "param1", 10.0);
    m_config->setParameterValue("feature1", "param2", 20.0);
    m_config->setParameterValue("feature2", "param1", 30.0);
    
    // Serialize
    std::string serialized = m_config->serialize();
    
    // Create a new configuration
    auto newConfig = std::make_unique<Configuration>("NewConfig");
    
    // Deserialize
    EXPECT_TRUE(newConfig->deserialize(serialized));
    
    // Check values
    EXPECT_EQ("TestConfig", newConfig->getName());
    EXPECT_EQ("Test configuration", newConfig->getDescription());
    EXPECT_EQ(10.0, newConfig->getParameterValue("feature1", "param1"));
    EXPECT_EQ(20.0, newConfig->getParameterValue("feature1", "param2"));
    EXPECT_EQ(30.0, newConfig->getParameterValue("feature2", "param1"));
}

// Test invalid deserialization
TEST_F(ConfigurationTest, InvalidDeserialization) {
    auto newConfig = std::make_unique<Configuration>("NewConfig");
    
    // Invalid format
    EXPECT_FALSE(newConfig->deserialize("Invalid format"));
    
    // Missing sections
    EXPECT_FALSE(newConfig->deserialize("Configuration:\nName: TestConfig"));
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
