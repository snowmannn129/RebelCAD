/**
 * @file ParametricModelingTest.cpp
 * @brief Test for the parametric modeling system
 * 
 * This file contains tests for the parametric modeling system, including
 * feature creation, parameter management, dependency tracking, and history
 * management.
 */

#include <gtest/gtest.h>
#include "../../../src/modeling/parametric/ParametricFeature.h"
#include "../../../src/modeling/parametric/ParameterManager.h"
#include "../../../src/modeling/parametric/DependencyGraph.h"
#include "../../../src/modeling/parametric/FeatureHistory.h"
#include "../../../src/modeling/parametric/ExtrusionFeature.h"
#include <memory>
#include <string>
#include <vector>

using namespace rebel_cad::modeling;

// Mock sketch feature for testing
class SketchFeature : public ParametricFeature {
public:
    SketchFeature(const FeatureId& id, const std::string& name)
        : ParametricFeature(id, name)
    {
        // Nothing to initialize
    }

    bool update() override {
        setStatus(FeatureStatus::Valid);
        return true;
    }

    std::vector<FeatureId> getDependencies() const override {
        return {};
    }

    void addDependency(const FeatureId& dependencyId) override {
        // Sketches don't have dependencies
    }

    void removeDependency(const FeatureId& dependencyId) override {
        // Sketches don't have dependencies
    }

    std::unordered_map<std::string, double> getParameters() const override {
        return {};
    }

    bool setParameter(const std::string& name, double value) override {
        return false;
    }

    std::string serialize() const override {
        return "id=" + getId() + "\nname=" + getName() + "\n";
    }

    bool deserialize(const std::string& serialized) override {
        return true;
    }
};

// Test fixture for parametric modeling tests
class ParametricModelingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create the parameter manager, dependency graph, and feature history
        paramManager = std::make_unique<ParameterManager>();
        graph = std::make_unique<DependencyGraph>();
        history = std::make_unique<FeatureHistory>(*graph, *paramManager);

        // Create a sketch feature
        sketch = std::make_unique<SketchFeature>("sketch1", "Sketch 1");
        graph->addFeature(sketch.get());

        // Create an extrusion feature
        extrusion = std::make_unique<ExtrusionFeature>("extrusion1", "Extrusion 1", "sketch1", *paramManager);
        graph->addFeature(extrusion.get());
    }

    void TearDown() override {
        // Clean up
        extrusion.reset();
        sketch.reset();
        history.reset();
        graph.reset();
        paramManager.reset();
    }

    std::unique_ptr<ParameterManager> paramManager;
    std::unique_ptr<DependencyGraph> graph;
    std::unique_ptr<FeatureHistory> history;
    std::unique_ptr<SketchFeature> sketch;
    std::unique_ptr<ExtrusionFeature> extrusion;
};

// Test creating a parametric feature
TEST_F(ParametricModelingTest, CreateFeature) {
    // Check that the features were created successfully
    EXPECT_EQ(sketch->getId(), "sketch1");
    EXPECT_EQ(sketch->getName(), "Sketch 1");
    EXPECT_EQ(extrusion->getId(), "extrusion1");
    EXPECT_EQ(extrusion->getName(), "Extrusion 1");
    EXPECT_EQ(extrusion->getSketchId(), "sketch1");
}

// Test parameter management
TEST_F(ParametricModelingTest, ParameterManagement) {
    // Check that the extrusion feature has the expected parameters
    auto params = extrusion->getParameters();
    EXPECT_EQ(params.size(), 6);
    EXPECT_EQ(params["depth"], 10.0);
    EXPECT_EQ(params["direction"], 0.0);
    EXPECT_EQ(params["endCondition"], 0.0);
    EXPECT_EQ(params["draft"], 0.0);
    EXPECT_EQ(params["thinWalled"], 0.0);
    EXPECT_EQ(params["wallThickness"], 1.0);

    // Test setting a parameter
    EXPECT_TRUE(extrusion->setParameter("depth", 20.0));
    params = extrusion->getParameters();
    EXPECT_EQ(params["depth"], 20.0);

    // Test setting an invalid parameter
    EXPECT_FALSE(extrusion->setParameter("invalid", 0.0));
}

// Test dependency tracking
TEST_F(ParametricModelingTest, DependencyTracking) {
    // Check that the extrusion feature depends on the sketch
    auto dependencies = extrusion->getDependencies();
    EXPECT_EQ(dependencies.size(), 1);
    EXPECT_EQ(dependencies[0], "sketch1");

    // Check that the dependency graph correctly tracks dependencies
    auto dependents = graph->getDependents("sketch1");
    EXPECT_EQ(dependents.size(), 1);
    EXPECT_EQ(dependents[0], "extrusion1");

    // Test adding a dependency
    extrusion->addDependency("newDependency");
    dependencies = extrusion->getDependencies();
    EXPECT_EQ(dependencies.size(), 2);
    EXPECT_EQ(dependencies[1], "newDependency");

    // Test removing a dependency
    extrusion->removeDependency("newDependency");
    dependencies = extrusion->getDependencies();
    EXPECT_EQ(dependencies.size(), 1);
    EXPECT_EQ(dependencies[0], "sketch1");
}

// Test feature updating
TEST_F(ParametricModelingTest, FeatureUpdating) {
    // Check that the features are initially invalid
    EXPECT_FALSE(sketch->isValid());
    EXPECT_FALSE(extrusion->isValid());

    // Update the sketch
    EXPECT_TRUE(graph->updateFeature("sketch1"));
    EXPECT_TRUE(sketch->isValid());
    EXPECT_FALSE(extrusion->isValid());

    // Update the extrusion
    EXPECT_TRUE(graph->updateFeature("extrusion1"));
    EXPECT_TRUE(sketch->isValid());
    EXPECT_TRUE(extrusion->isValid());
}

// Test history management
TEST_F(ParametricModelingTest, HistoryManagement) {
    // Check that the history is initially empty
    EXPECT_EQ(history->getOperationCount(), 0);
    EXPECT_FALSE(history->canUndo());
    EXPECT_FALSE(history->canRedo());

    // Create a parameter change operation
    auto operation = std::make_unique<SetParameterOperation>("extrusion1", "depth", 10.0, 20.0);
    history->addOperation(std::move(operation));

    // Check that the operation was added to the history
    EXPECT_EQ(history->getOperationCount(), 1);
    EXPECT_TRUE(history->canUndo());
    EXPECT_FALSE(history->canRedo());

    // Test undoing the operation
    EXPECT_TRUE(history->undo());
    EXPECT_EQ(paramManager->getParameterValue("extrusion1", "depth"), 10.0);
    EXPECT_FALSE(history->canUndo());
    EXPECT_TRUE(history->canRedo());

    // Test redoing the operation
    EXPECT_TRUE(history->redo());
    EXPECT_EQ(paramManager->getParameterValue("extrusion1", "depth"), 20.0);
    EXPECT_TRUE(history->canUndo());
    EXPECT_FALSE(history->canRedo());
}

// Test serialization and deserialization
TEST_F(ParametricModelingTest, Serialization) {
    // Serialize the extrusion feature
    std::string serialized = extrusion->serialize();
    EXPECT_FALSE(serialized.empty());

    // Create a new extrusion feature
    auto newExtrusion = std::make_unique<ExtrusionFeature>("extrusion2", "Extrusion 2", "sketch1", *paramManager);

    // Deserialize the serialized data into the new feature
    EXPECT_TRUE(newExtrusion->deserialize(serialized));

    // Check that the new feature has the same parameters
    auto params = newExtrusion->getParameters();
    EXPECT_EQ(params["depth"], 10.0);
    EXPECT_EQ(params["direction"], 0.0);
    EXPECT_EQ(params["endCondition"], 0.0);
    EXPECT_EQ(params["draft"], 0.0);
    EXPECT_EQ(params["thinWalled"], 0.0);
    EXPECT_EQ(params["wallThickness"], 1.0);
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
