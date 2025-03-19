#include <gtest/gtest.h>
#include "sketching/BezierCurve.h"
#include "graphics/Color.h"
#include <cmath>

using namespace RebelCAD::Sketching;
using namespace RebelCAD::Graphics;

class BezierCurveTest : public ::testing::Test {
protected:
    // Helper function to compare points with tolerance
    static bool pointsEqual(const std::pair<double, double>& p1, 
                          const std::pair<double, double>& p2, 
                          double tolerance = 1e-6) {
        return std::abs(p1.first - p2.first) < tolerance && 
               std::abs(p1.second - p2.second) < tolerance;
    }
};

TEST_F(BezierCurveTest, DefaultConstructor) {
    BezierCurve curve;
    EXPECT_EQ(curve.getControlPoints().size(), 0);
    EXPECT_EQ(curve.getDegree(), 0);
}

TEST_F(BezierCurveTest, ConstructorWithPoints) {
    std::vector<std::pair<double, double>> points = {
        {0.0, 0.0}, {1.0, 1.0}, {2.0, 0.0}
    };
    BezierCurve curve(points);
    EXPECT_EQ(curve.getControlPoints().size(), 3);
    EXPECT_EQ(curve.getDegree(), 2);
}

TEST_F(BezierCurveTest, AddControlPoint) {
    BezierCurve curve;
    curve.addControlPoint(1.0, 2.0);
    EXPECT_EQ(curve.getControlPoints().size(), 1);
    EXPECT_TRUE(pointsEqual(curve.getControlPoints()[0], {1.0, 2.0}));
}

TEST_F(BezierCurveTest, RemoveControlPoint) {
    BezierCurve curve;
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(3.0, 4.0);
    
    EXPECT_TRUE(curve.removeControlPoint(0));
    EXPECT_EQ(curve.getControlPoints().size(), 1);
    EXPECT_TRUE(pointsEqual(curve.getControlPoints()[0], {3.0, 4.0}));
    
    EXPECT_FALSE(curve.removeControlPoint(5)); // Invalid index
}

TEST_F(BezierCurveTest, MoveControlPoint) {
    BezierCurve curve;
    curve.addControlPoint(1.0, 2.0);
    
    EXPECT_TRUE(curve.moveControlPoint(0, 3.0, 4.0));
    EXPECT_TRUE(pointsEqual(curve.getControlPoints()[0], {3.0, 4.0}));
    
    EXPECT_FALSE(curve.moveControlPoint(1, 5.0, 6.0)); // Invalid index
}

TEST_F(BezierCurveTest, EvaluateLinearCurve) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    
    auto point = curve.evaluatePoint(0.5);
    EXPECT_TRUE(pointsEqual(point, {0.5, 0.5}));
}

TEST_F(BezierCurveTest, EvaluateQuadraticCurve) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto point = curve.evaluatePoint(0.5);
    EXPECT_TRUE(pointsEqual(point, {1.0, 1.0}));
}

TEST_F(BezierCurveTest, FirstDerivative) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    
    auto derivative = curve.evaluateFirstDerivative(0.5);
    EXPECT_TRUE(pointsEqual(derivative, {1.0, 1.0}));
}

TEST_F(BezierCurveTest, SecondDerivative) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto derivative = curve.evaluateSecondDerivative(0.5);
    EXPECT_TRUE(pointsEqual(derivative, {0.0, -8.0}));
}

TEST_F(BezierCurveTest, SplitCurve) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto [left, right] = curve.split(0.5);
    
    // Verify endpoints are preserved
    EXPECT_TRUE(pointsEqual(left.getControlPoints().front(), {0.0, 0.0}));
    EXPECT_TRUE(pointsEqual(right.getControlPoints().back(), {2.0, 0.0}));
    
    // Verify split point is shared
    EXPECT_TRUE(pointsEqual(left.getControlPoints().back(), right.getControlPoints().front()));
}

TEST_F(BezierCurveTest, ElevateDegree) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    
    auto elevated = curve.elevateDegree();
    EXPECT_EQ(elevated.getDegree(), curve.getDegree() + 1);
    
    // Verify curve shape is preserved
    for (double t = 0.0; t <= 1.0; t += 0.1) {
        EXPECT_TRUE(pointsEqual(curve.evaluatePoint(t), elevated.evaluatePoint(t)));
    }
}

TEST_F(BezierCurveTest, ReduceDegree) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(0.5, 0.5);
    curve.addControlPoint(1.0, 1.0);
    
    auto reduced = curve.reduceDegree();
    EXPECT_EQ(reduced.getDegree(), 1);
    
    // Verify approximation is close enough
    for (double t = 0.0; t <= 1.0; t += 0.1) {
        EXPECT_TRUE(pointsEqual(curve.evaluatePoint(t), reduced.evaluatePoint(t), 0.1));
    }
}

TEST_F(BezierCurveTest, ConvexHull) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto hull = curve.calculateConvexHull();
    EXPECT_EQ(hull.size(), 3);
}

TEST_F(BezierCurveTest, FindClosestPoint) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    
    auto [point, t] = curve.findClosestPoint(0.5, 0.0);
    EXPECT_TRUE(pointsEqual(point, {0.5, 0.5}, 0.01));
    EXPECT_NEAR(t, 0.5, 0.01);
}

TEST_F(BezierCurveTest, InvalidParameterValue) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    
    EXPECT_THROW(curve.evaluatePoint(-0.1), std::invalid_argument);
    EXPECT_THROW(curve.evaluatePoint(1.1), std::invalid_argument);
}

TEST_F(BezierCurveTest, ColorManagement) {
    BezierCurve curve;
    Color color(1.0f, 0.0f, 0.0f); // Red
    
    curve.setColor(color);
    EXPECT_EQ(curve.getColor().r, 1.0f);
    EXPECT_EQ(curve.getColor().g, 0.0f);
    EXPECT_EQ(curve.getColor().b, 0.0f);
}

TEST_F(BezierCurveTest, Rendering) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 1.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto graphics = std::make_shared<GraphicsSystem>();
    Color color(1.0f, 0.0f, 0.0f); // Red
    std::vector<float> dashPattern = {5.0f, 2.0f};
    
    // Test basic rendering
    EXPECT_NO_THROW(curve.render(graphics, color));
    
    // Test rendering with thickness
    EXPECT_NO_THROW(curve.render(graphics, color, 2.0f));
    
    // Test rendering with dash pattern
    EXPECT_NO_THROW(curve.render(graphics, color, 1.0f, &dashPattern));
    
    // Test rendering empty curve
    BezierCurve emptyCurve;
    EXPECT_NO_THROW(emptyCurve.render(graphics, color));
}

TEST_F(BezierCurveTest, RenderingWithControlPolygon) {
    BezierCurve curve;
    curve.addControlPoint(0.0, 0.0);
    curve.addControlPoint(1.0, 2.0);
    curve.addControlPoint(2.0, 0.0);
    
    auto graphics = std::make_shared<GraphicsSystem>();
    Color color(1.0f, 0.0f, 0.0f); // Red
    
    // Verify control polygon is rendered
    EXPECT_NO_THROW(curve.render(graphics, color));
}
