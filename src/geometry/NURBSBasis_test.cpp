#include "modeling/NURBSBasis.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

namespace RebelCAD {
namespace Modeling {

class NURBSBasisTest : public ::testing::Test {
protected:
    NURBSBasis basis;

    // Helper to verify basis function properties
    void VerifyBasisProperties(const std::vector<float>& values) {
        // Check partition of unity
        float sum = 0.0f;
        for (float value : values) {
            sum += value;
            // Values should be in [0,1]
            EXPECT_GE(value, 0.0f);
            EXPECT_LE(value, 1.0f);
        }
        EXPECT_NEAR(sum, 1.0f, 1e-6f);
    }

    // Helper to verify derivative properties
    void VerifyDerivativeProperties(
        const std::vector<std::vector<float>>& derivatives) {
        
        // First entry should be basis functions
        VerifyBasisProperties(derivatives[0]);

        // Sum of derivatives should be zero
        for (size_t i = 1; i < derivatives.size(); ++i) {
            float sum = 0.0f;
            for (float value : derivatives[i]) {
                sum += value;
            }
            EXPECT_NEAR(sum, 0.0f, 1e-6f);
        }
    }

    // Helper to create uniform knot vector
    std::vector<float> CreateUniformKnots(int degree, int control_points) {
        std::vector<float> knots;
        // Add degree + 1 knots at start
        for (int i = 0; i <= degree; ++i) {
            knots.push_back(0.0f);
        }
        // Add internal knots
        int internal_knots = control_points - degree - 1;
        for (int i = 1; i <= internal_knots; ++i) {
            knots.push_back(static_cast<float>(i) / (internal_knots + 1));
        }
        // Add degree + 1 knots at end
        for (int i = 0; i <= degree; ++i) {
            knots.push_back(1.0f);
        }
        return knots;
    }
};

TEST_F(NURBSBasisTest, Initialization) {
    // Test invalid inputs
    EXPECT_FALSE(basis.Initialize({}, 3));  // Empty knots
    EXPECT_FALSE(basis.Initialize({0.0f, 1.0f}, 3));  // Too few knots
    EXPECT_FALSE(basis.Initialize({1.0f, 0.0f}, 1));  // Decreasing knots
    EXPECT_FALSE(basis.Initialize({0.0f, INFINITY}, 1));  // Invalid knot
    EXPECT_FALSE(basis.Initialize({0.0f, 1.0f}, 0));  // Invalid degree

    // Test valid initialization
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 2.0f, 2.0f};
    EXPECT_TRUE(basis.Initialize(knots, 2));
    EXPECT_EQ(basis.GetDegree(), 2);
    EXPECT_EQ(basis.GetCount(), 4);
}

TEST_F(NURBSBasisTest, UniformQuadratic) {
    // Create uniform quadratic basis
    std::vector<float> knots = CreateUniformKnots(2, 5);  // degree 2, 5 control points
    ASSERT_TRUE(basis.Initialize(knots, 2));

    // Test basis functions at various parameters
    std::vector<float> values;
    
    // Test at start
    values = basis.Evaluate(0.0f, 2);
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);
    EXPECT_NEAR(values[0], 1.0f, 1e-6f);  // First basis function is 1 at start

    // Test at middle
    values = basis.Evaluate(0.5f, 3);
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);

    // Test at end
    values = basis.Evaluate(1.0f, 4);
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);
    EXPECT_NEAR(values[2], 1.0f, 1e-6f);  // Last basis function is 1 at end
}

TEST_F(NURBSBasisTest, Derivatives) {
    // Create cubic basis
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    ASSERT_TRUE(basis.Initialize(knots, 3));

    // Test derivatives at middle
    float t = 0.5f;
    int span = basis.FindSpan(t);
    ASSERT_GE(span, 0);

    auto derivatives = basis.EvaluateDerivatives(t, span, 3);
    ASSERT_FALSE(derivatives.empty());
    ASSERT_EQ(derivatives.size(), 4);  // 0th to 3rd derivatives
    VerifyDerivativeProperties(derivatives);

    // Test invalid inputs
    EXPECT_TRUE(basis.EvaluateDerivatives(t, span, -1).empty());
    EXPECT_TRUE(basis.EvaluateDerivatives(t, span, 4).empty());  // > degree
    EXPECT_TRUE(basis.EvaluateDerivatives(INFINITY, span, 1).empty());
    EXPECT_TRUE(basis.EvaluateDerivatives(t, -1, 1).empty());
}

TEST_F(NURBSBasisTest, SpanFinding) {
    // Create uniform cubic basis
    std::vector<float> knots = CreateUniformKnots(3, 6);  // degree 3, 6 control points
    ASSERT_TRUE(basis.Initialize(knots, 3));

    // Test span finding
    EXPECT_EQ(basis.FindSpan(0.0f), 3);  // First valid span
    EXPECT_EQ(basis.FindSpan(0.5f), 4);  // Middle span
    EXPECT_EQ(basis.FindSpan(1.0f), 5);  // Last valid span

    // Test invalid inputs
    EXPECT_EQ(basis.FindSpan(INFINITY), -1);
    EXPECT_EQ(basis.FindSpan(NAN), -1);
}

TEST_F(NURBSBasisTest, NonUniformBasis) {
    // Create non-uniform knot vector
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.7f, 1.0f, 1.0f, 1.0f};
    ASSERT_TRUE(basis.Initialize(knots, 2));

    // Test basis functions at internal knots
    std::vector<float> values;
    
    values = basis.Evaluate(0.2f, basis.FindSpan(0.2f));
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);

    values = basis.Evaluate(0.5f, basis.FindSpan(0.5f));
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);

    values = basis.Evaluate(0.7f, basis.FindSpan(0.7f));
    ASSERT_FALSE(values.empty());
    VerifyBasisProperties(values);
}

TEST_F(NURBSBasisTest, MultipleKnots) {
    // Create knot vector with multiple internal knot
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f};
    ASSERT_TRUE(basis.Initialize(knots, 2));

    // Test continuity at multiple knot
    float epsilon = 0.001f;
    auto values_before = basis.Evaluate(0.5f - epsilon, basis.FindSpan(0.5f - epsilon));
    auto values_at = basis.Evaluate(0.5f, basis.FindSpan(0.5f));
    auto values_after = basis.Evaluate(0.5f + epsilon, basis.FindSpan(0.5f + epsilon));

    ASSERT_FALSE(values_before.empty());
    ASSERT_FALSE(values_at.empty());
    ASSERT_FALSE(values_after.empty());

    VerifyBasisProperties(values_before);
    VerifyBasisProperties(values_at);
    VerifyBasisProperties(values_after);
}

TEST_F(NURBSBasisTest, ErrorHandling) {
    // Test uninitialized state
    EXPECT_TRUE(basis.Evaluate(0.5f, 2).empty());
    EXPECT_TRUE(basis.EvaluateDerivatives(0.5f, 2, 1).empty());
    EXPECT_EQ(basis.FindSpan(0.5f), -1);
    EXPECT_EQ(basis.GetCount(), 0);

    // Initialize with valid data
    std::vector<float> knots = CreateUniformKnots(2, 4);
    ASSERT_TRUE(basis.Initialize(knots, 2));

    // Test invalid parameter values
    EXPECT_TRUE(basis.Evaluate(INFINITY, 2).empty());
    EXPECT_TRUE(basis.Evaluate(NAN, 2).empty());

    // Test invalid span indices
    EXPECT_TRUE(basis.Evaluate(0.5f, -1).empty());
    EXPECT_TRUE(basis.Evaluate(0.5f, knots.size()).empty());
}

} // namespace Modeling
} // namespace RebelCAD
