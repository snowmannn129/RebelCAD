#include <gtest/gtest.h>
#include "sketching/Spline.h"
#include <cmath>

// [Keep all existing test code]

TEST_F(SplineTest, ControlPointInfluenceLinear) {
    Spline spline;
    spline.setDegree(1);  // Linear spline
    
    // Create a simple line
    spline.addControlPoint(0.0, 0.0);
    spline.addControlPoint(1.0, 1.0);
    
    auto influences = spline.calculateControlPointInfluences();
    ASSERT_EQ(influences.size(), 2);
    
    // For linear spline:
    // First point influence decreases from 1 to 0
    EXPECT_NEAR(influences[0].startParam, 0.0, 1e-6);
    EXPECT_NEAR(influences[0].endParam, 1.0, 1e-6);
    EXPECT_NEAR(influences[0].maxInfluence, 1.0, 1e-6);
    
    // Second point influence increases from 0 to 1
    EXPECT_NEAR(influences[1].startParam, 0.0, 1e-6);
    EXPECT_NEAR(influences[1].endParam, 1.0, 1e-6);
    EXPECT_NEAR(influences[1].maxInfluence, 1.0, 1e-6);
}

TEST_F(SplineTest, ControlPointInfluenceQuadratic) {
    Spline spline;
    spline.setDegree(2);  // Quadratic spline
    
    // Create a parabola
    spline.addControlPoint(0.0, 0.0);
    spline.addControlPoint(1.0, 1.0);
    spline.addControlPoint(2.0, 0.0);
    
    auto influences = spline.calculateControlPointInfluences();
    ASSERT_EQ(influences.size(), 3);
    
    // For quadratic B-spline:
    // Each control point should influence about 2/3 of the curve
    for (const auto& influence : influences) {
        EXPECT_GT(influence.maxInfluence, 0.0);
        EXPECT_LE(influence.maxInfluence, 1.0);
        
        // Check influence region size
        double regionSize = influence.endParam - influence.startParam;
        EXPECT_NEAR(regionSize, 0.67, 0.1);  // Approximately 2/3
    }
}

TEST_F(SplineTest, ControlPointInfluenceEmpty) {
    Spline spline;
    auto influences = spline.calculateControlPointInfluences();
    EXPECT_TRUE(influences.empty());
}

TEST_F(SplineTest, ControlPointInfluenceSampling) {
    Spline spline;
    spline.setDegree(3);  // Cubic spline
    
    // Create a curve
    spline.addControlPoint(0.0, 0.0);
    spline.addControlPoint(1.0, 1.0);
    spline.addControlPoint(2.0, -1.0);
    spline.addControlPoint(3.0, 0.0);
    
    // Test with different sampling rates
    auto influences1 = spline.calculateControlPointInfluences(50);
    auto influences2 = spline.calculateControlPointInfluences(200);
    
    ASSERT_EQ(influences1.size(), 4);
    ASSERT_EQ(influences2.size(), 4);
    
    // Higher sampling should give more precise results
    for (size_t i = 0; i < influences1.size(); ++i) {
        // Results should be similar but influences2 might be slightly more accurate
        EXPECT_NEAR(influences1[i].startParam, influences2[i].startParam, 0.1);
        EXPECT_NEAR(influences1[i].endParam, influences2[i].endParam, 0.1);
        EXPECT_NEAR(influences1[i].maxInfluence, influences2[i].maxInfluence, 0.1);
    }
}

TEST_F(SplineTest, ControlPointInfluenceNURBS) {
    Spline spline;
    spline.setDegree(2);
    
    // Create a weighted curve
    spline.addControlPoint(0.0, 0.0);
    spline.addControlPoint(1.0, 1.0);
    spline.addControlPoint(2.0, 0.0);
    
    std::vector<double> weights = {1.0, 2.0, 1.0};  // Middle point has more influence
    spline.setWeights(weights);
    
    auto influences = spline.calculateControlPointInfluences();
    ASSERT_EQ(influences.size(), 3);
    
    // Middle point should have higher maximum influence due to higher weight
    EXPECT_GT(influences[1].maxInfluence, influences[0].maxInfluence);
    EXPECT_GT(influences[1].maxInfluence, influences[2].maxInfluence);
}
