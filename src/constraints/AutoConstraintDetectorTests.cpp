#include "gtest/gtest.h"
#include "constraints/AutoConstraintDetector.h"
#include "sketching/Line.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"

namespace RebelCAD {
namespace Tests {

class AutoConstraintDetectorTests : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_unique<AutoConstraintDetector>();
    }

    std::unique_ptr<AutoConstraintDetector> detector;
};

TEST_F(AutoConstraintDetectorTests, DetectsParallelLines) {
    // Create two parallel horizontal lines
    auto line1 = std::make_shared<Line>(Point(0, 0), Point(10, 0));
    auto line2 = std::make_shared<Line>(Point(0, 5), Point(10, 5));
    
    std::vector<std::shared_ptr<SketchElement>> elements = {line1, line2};
    auto constraints = detector->detectConstraints(elements);
    
    ASSERT_EQ(constraints.size(), 1);
    EXPECT_NE(dynamic_cast<ParallelConstraint*>(constraints[0].get()), nullptr);
}

TEST_F(AutoConstraintDetectorTests, DetectsPerpendicularLines) {
    // Create two perpendicular lines
    auto line1 = std::make_shared<Line>(Point(0, 0), Point(10, 0));
    auto line2 = std::make_shared<Line>(Point(5, -5), Point(5, 5));
    
    std::vector<std::shared_ptr<SketchElement>> elements = {line1, line2};
    auto constraints = detector->detectConstraints(elements);
    
    ASSERT_EQ(constraints.size(), 1);
    EXPECT_NE(dynamic_cast<PerpendicularConstraint*>(constraints[0].get()), nullptr);
}

TEST_F(AutoConstraintDetectorTests, DetectsConcentricCircles) {
    // Create two concentric circles
    auto circle1 = std::make_shared<Circle>(Point(5, 5), 2.0);
    auto circle2 = std::make_shared<Circle>(Point(5, 5), 4.0);
    
    std::vector<std::shared_ptr<SketchElement>> elements = {circle1, circle2};
    auto constraints = detector->detectConstraints(elements);
    
    ASSERT_EQ(constraints.size(), 1);
    EXPECT_NE(dynamic_cast<ConcentricConstraint*>(constraints[0].get()), nullptr);
}

TEST_F(AutoConstraintDetectorTests, DetectsTangentCircles) {
    // Create two tangent circles
    auto circle1 = std::make_shared<Circle>(Point(0, 0), 2.0);
    auto circle2 = std::make_shared<Circle>(Point(4, 0), 2.0);
    
    std::vector<std::shared_ptr<SketchElement>> elements = {circle1, circle2};
    auto constraints = detector->detectConstraints(elements);
    
    ASSERT_EQ(constraints.size(), 2); // Should detect both tangency and equal radius
    bool hasTangent = false;
    bool hasEqualRadius = false;
    
    for (const auto& constraint : constraints) {
        if (dynamic_cast<TangentConstraint*>(constraint.get())) hasTangent = true;
        if (dynamic_cast<RadiusConstraint*>(constraint.get())) hasEqualRadius = true;
    }
    
    EXPECT_TRUE(hasTangent);
    EXPECT_TRUE(hasEqualRadius);
}

TEST_F(AutoConstraintDetectorTests, DetectsEqualLength) {
    // Create two equal length lines
    auto line1 = std::make_shared<Line>(Point(0, 0), Point(5, 0));
    auto line2 = std::make_shared<Line>(Point(0, 5), Point(5, 5));
    
    std::vector<std::shared_ptr<SketchElement>> elements = {line1, line2};
    auto constraints = detector->detectConstraints(elements);
    
    ASSERT_EQ(constraints.size(), 2); // Should detect both parallel and equal length
    bool hasParallel = false;
    bool hasEqualLength = false;
    
    for (const auto& constraint : constraints) {
        if (dynamic_cast<ParallelConstraint*>(constraint.get())) hasParallel = true;
        if (dynamic_cast<LengthConstraint*>(constraint.get())) hasEqualLength = true;
    }
    
    EXPECT_TRUE(hasParallel);
    EXPECT_TRUE(hasEqualLength);
}

TEST_F(AutoConstraintDetectorTests, HandlesEmptyInput) {
    std::vector<std::shared_ptr<SketchElement>> elements;
    auto constraints = detector->detectConstraints(elements);
    
    EXPECT_TRUE(constraints.empty());
}

TEST_F(AutoConstraintDetectorTests, HandlesNullElements) {
    std::vector<std::shared_ptr<SketchElement>> elements;
    elements.push_back(nullptr);
    elements.push_back(std::make_shared<Line>(Point(0, 0), Point(5, 0)));
    
    auto constraints = detector->detectConstraints(elements);
    
    EXPECT_TRUE(constraints.empty());
}

TEST_F(AutoConstraintDetectorTests, CustomTolerances) {
    // Create almost parallel lines (1.5 degrees off)
    auto line1 = std::make_shared<Line>(Point(0, 0), Point(10, 0));
    auto line2 = std::make_shared<Line>(Point(0, 5), Point(10, 5.262)); // ~1.5 degrees
    
    // Default tolerance should not detect as parallel
    auto constraints = detector->detectConstraints({line1, line2});
    EXPECT_TRUE(constraints.empty());
    
    // Custom tolerance should detect as parallel
    AutoConstraintDetector::Config config;
    config.parallelTolerance = 0.03; // ~1.7 degrees
    auto lenientDetector = std::make_unique<AutoConstraintDetector>(config);
    
    constraints = lenientDetector->detectConstraints({line1, line2});
    ASSERT_EQ(constraints.size(), 1);
    EXPECT_NE(dynamic_cast<ParallelConstraint*>(constraints[0].get()), nullptr);
}

} // namespace Tests
} // namespace RebelCAD
