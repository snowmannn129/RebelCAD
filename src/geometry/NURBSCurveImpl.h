#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "modeling/NURBSCurve.h"
#include "modeling/NURBSBasis.h"
#include "modeling/KnotVector.h"

namespace RebelCAD {
namespace Modeling {

class NURBSCurve::Impl {
public:
    // Input data
    std::vector<glm::vec3> control_points;
    std::vector<float> weights;
    NURBSCurveParams params;
    bool is_configured = false;

    // Curve components
    std::unique_ptr<NURBSBasis> basis;
    std::unique_ptr<KnotVector> knot_vector;

    // Result data
    struct CurveGeometry {
        std::vector<float> vertices;        // Curve point coordinates
        std::vector<float> tangents;        // Tangent vectors
        std::vector<float> normals;         // Normal vectors (if available)
        std::vector<unsigned int> indices;  // Line segment indices
    };
    CurveGeometry result;

    // Preview cache
    struct PreviewCache {
        bool valid = false;
        std::vector<glm::vec3> cached_points;
        std::vector<float> cached_weights;
        NURBSCurveParams cached_params;
        CurveGeometry geometry;
    } preview_cache;

    Impl() : 
        basis(std::make_unique<NURBSBasis>()),
        knot_vector(std::make_unique<KnotVector>()) {}

    /**
     * @brief Clear all cached data
     */
    void Clear() {
        result = CurveGeometry();
        preview_cache.valid = false;
    }

    /**
     * @brief Evaluate curve point at parameter value
     * @param t Parameter value
     * @return Point on curve
     */
    glm::vec3 Evaluate(float t) const {
        if (!is_configured || control_points.empty()) {
            return glm::vec3(0.0f);
        }

        // Find knot span
        int span = basis->FindSpan(t);
        if (span < 0) {
            return glm::vec3(0.0f);
        }

        // Get basis functions
        std::vector<float> N = basis->Evaluate(t, span);
        if (N.empty()) {
            return glm::vec3(0.0f);
        }

        // Evaluate point
        glm::vec3 point(0.0f);
        float weight_sum = 0.0f;

        for (size_t i = 0; i < N.size(); ++i) {
            size_t idx = span - params.degree + i;
            if (idx >= control_points.size()) {
                continue;
            }

            float w = params.rational ? weights[idx] : 1.0f;
            point += control_points[idx] * (N[i] * w);
            weight_sum += N[i] * w;
        }

        if (params.rational) {
            if (std::abs(weight_sum) > params.tolerance) {
                point /= weight_sum;
            }
        }

        return point;
    }

    /**
     * @brief Evaluate curve derivative at parameter value
     * @param t Parameter value
     * @param order Derivative order
     * @return Derivative vectors
     */
    std::vector<glm::vec3> EvaluateDerivatives(float t, int order) const {
        if (!is_configured || control_points.empty() || order < 1) {
            return {};
        }

        // Find knot span
        int span = basis->FindSpan(t);
        if (span < 0) {
            return {};
        }

        // Get basis function derivatives
        auto ders = basis->EvaluateDerivatives(t, span, order);
        if (ders.empty()) {
            return {};
        }

        std::vector<glm::vec3> derivatives(order + 1);
        if (!params.rational) {
            // Non-rational case
            for (int k = 0; k <= order; ++k) {
                derivatives[k] = glm::vec3(0.0f);
                for (size_t i = 0; i < ders[k].size(); ++i) {
                    size_t idx = span - params.degree + i;
                    if (idx >= control_points.size()) {
                        continue;
                    }
                    derivatives[k] += control_points[idx] * ders[k][i];
                }
            }
        } else {
            // Rational case - use quotient rule
            std::vector<glm::vec3> A(order + 1, glm::vec3(0.0f));
            std::vector<float> w(order + 1, 0.0f);

            // Calculate weighted sum for each derivative
            for (int k = 0; k <= order; ++k) {
                for (size_t i = 0; i < ders[k].size(); ++i) {
                    size_t idx = span - params.degree + i;
                    if (idx >= control_points.size()) {
                        continue;
                    }
                    A[k] += control_points[idx] * weights[idx] * ders[k][i];
                    w[k] += weights[idx] * ders[k][i];
                }
            }

            // Apply quotient rule
            float w0 = w[0];
            if (std::abs(w0) > params.tolerance) {
                derivatives[0] = A[0] / w0;
                for (int k = 1; k <= order; ++k) {
                    glm::vec3 v = A[k];
                    for (int i = 1; i <= k; ++i) {
                        float bin = static_cast<float>(BinomialCoefficient(k, i));
                        v -= derivatives[k-i] * (w[i] * bin);
                    }
                    derivatives[k] = v / w0;
                }
            }
        }

        return derivatives;
    }

    /**
     * @brief Generate discretized curve geometry
     * @param segments Number of segments
     * @return true if generation successful
     */
    bool GenerateGeometry(int segments) {
        if (!is_configured || control_points.empty() || segments < 1) {
            return false;
        }

        result = CurveGeometry();
        result.vertices.reserve(segments * 3);
        result.tangents.reserve(segments * 3);
        result.indices.reserve((segments - 1) * 2);

        // Generate points along curve
        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / segments;
            
            // Evaluate point
            glm::vec3 point = Evaluate(t);
            result.vertices.push_back(point.x);
            result.vertices.push_back(point.y);
            result.vertices.push_back(point.z);

            // Evaluate tangent
            auto derivatives = EvaluateDerivatives(t, 1);
            if (!derivatives.empty()) {
                glm::vec3 tangent = glm::normalize(derivatives[1]);
                result.tangents.push_back(tangent.x);
                result.tangents.push_back(tangent.y);
                result.tangents.push_back(tangent.z);
            }

            // Generate line segment indices
            if (i > 0) {
                result.indices.push_back(i - 1);
                result.indices.push_back(i);
            }
        }

        return true;
    }

    /**
     * @brief Validate curve configuration
     * @return true if configuration is valid
     */
    /**
     * @brief Elevate curve degree
     * @param target_degree Target degree for elevation
     * @return true if elevation successful
     */
    bool ElevateDegree(int target_degree) {
        if (!ValidateConfiguration() || target_degree <= params.degree) {
            return false;
        }

        // Store original degree
        int original_degree = params.degree;
        int elevation = target_degree - original_degree;

        // Allocate new control points and weights
        std::vector<glm::vec3> new_points(control_points.size() + elevation);
        std::vector<float> new_weights;
        if (params.rational) {
            new_weights.resize(weights.size() + elevation);
        }

        // Calculate binomial coefficients
        std::vector<float> coeff(original_degree + 2);
        for (int i = 0; i <= original_degree + 1; ++i) {
            coeff[i] = static_cast<float>(BinomialCoefficient(original_degree + 1, i));
        }

        // Compute new control points
        for (size_t i = 0; i < new_points.size(); ++i) {
            float t = static_cast<float>(i) / (new_points.size() - 1);
            glm::vec3 point(0.0f);
            float weight_sum = 0.0f;

            for (size_t j = 0; j <= original_degree; ++j) {
                float blend = coeff[j] * std::pow(1.0f - t, original_degree - j) * std::pow(t, j);
                float w = params.rational ? weights[j] : 1.0f;
                point += control_points[j] * (blend * w);
                weight_sum += blend * w;
            }

            if (params.rational) {
                new_weights[i] = weight_sum;
                point /= weight_sum;
            }
            new_points[i] = point;
        }

        // Update curve data
        control_points = std::move(new_points);
        if (params.rational) {
            weights = std::move(new_weights);
        }
        params.degree = target_degree;
        Clear();

        return true;
    }

    /**
     * @brief Reduce curve degree
     * @param target_degree Target degree for reduction
     * @return true if reduction successful
     */
    bool ReduceDegree(int target_degree) {
        if (!ValidateConfiguration() || 
            target_degree >= params.degree || 
            target_degree < 1) {
            return false;
        }

        // Store original degree
        int original_degree = params.degree;
        int reduction = original_degree - target_degree;

        // Allocate new control points and weights
        std::vector<glm::vec3> new_points(control_points.size() - reduction);
        std::vector<float> new_weights;
        if (params.rational) {
            new_weights.resize(weights.size() - reduction);
        }

        // Calculate least squares approximation
        std::vector<float> parameters;
        parameters.reserve(100);  // Sample points
        for (int i = 0; i <= 100; ++i) {
            parameters.push_back(static_cast<float>(i) / 100.0f);
        }

        // Build system of equations
        std::vector<std::vector<float>> A(new_points.size(), std::vector<float>(new_points.size()));
        std::vector<glm::vec3> b(new_points.size(), glm::vec3(0.0f));

        for (float t : parameters) {
            // Evaluate original curve
            glm::vec3 point = Evaluate(t);

            // Calculate basis functions for reduced degree
            int span = basis->FindSpan(t);
            std::vector<float> N = basis->Evaluate(t, span);

            // Build normal equations
            for (size_t i = 0; i < new_points.size(); ++i) {
                float Ni = N[i];
                b[i] += point * Ni;
                for (size_t j = 0; j < new_points.size(); ++j) {
                    float Nj = N[j];
                    A[i][j] += Ni * Nj;
                }
            }
        }

        // Solve system using Gaussian elimination
        for (size_t i = 0; i < new_points.size(); ++i) {
            // Find pivot
            float max_val = std::abs(A[i][i]);
            size_t max_row = i;
            for (size_t k = i + 1; k < new_points.size(); ++k) {
                if (std::abs(A[k][i]) > max_val) {
                    max_val = std::abs(A[k][i]);
                    max_row = k;
                }
            }

            // Swap rows if needed
            if (max_row != i) {
                std::swap(A[i], A[max_row]);
                std::swap(b[i], b[max_row]);
            }

            // Eliminate column i
            for (size_t j = i + 1; j < new_points.size(); ++j) {
                float coef = A[j][i] / A[i][i];
                b[j] -= b[i] * coef;
                for (size_t k = i; k < new_points.size(); ++k) {
                    A[j][k] -= A[i][k] * coef;
                }
            }
        }

        // Back substitution
        for (int i = new_points.size() - 1; i >= 0; --i) {
            glm::vec3 sum(0.0f);
            for (size_t j = i + 1; j < new_points.size(); ++j) {
                sum += new_points[j] * A[i][j];
            }
            new_points[i] = (b[i] - sum) / A[i][i];
        }

        // Update curve data
        control_points = std::move(new_points);
        if (params.rational) {
            // Approximate weights using linear interpolation
            for (size_t i = 0; i < new_weights.size(); ++i) {
                float t = static_cast<float>(i) / (new_weights.size() - 1);
                size_t j = static_cast<size_t>(t * (weights.size() - 1));
                float alpha = t * (weights.size() - 1) - j;
                new_weights[i] = weights[j] * (1.0f - alpha) + weights[j + 1] * alpha;
            }
            weights = std::move(new_weights);
        }
        params.degree = target_degree;
        Clear();

        return true;
    }

    /**
     * @brief Insert knot into curve
     * @param t Parameter value for new knot
     * @param multiplicity Number of times to insert knot
     * @return true if insertion successful
     */
    bool InsertKnot(float t, int multiplicity) {
        if (!ValidateConfiguration() || 
            !std::isfinite(t) || 
            multiplicity < 1) {
            return false;
        }

        // Find knot span
        int span = basis->FindSpan(t);
        if (span < 0) {
            return false;
        }

        // Calculate new control points
        std::vector<glm::vec3> new_points;
        std::vector<float> new_weights;
        new_points.reserve(control_points.size() + multiplicity);
        if (params.rational) {
            new_weights.reserve(weights.size() + multiplicity);
        }

        // Copy points before affected region
        for (int i = 0; i <= span - params.degree; ++i) {
            new_points.push_back(control_points[i]);
            if (params.rational) {
                new_weights.push_back(weights[i]);
            }
        }

        // Calculate affected control points
        std::vector<glm::vec3> temp(params.degree + 1);
        std::vector<float> temp_weights;
        if (params.rational) {
            temp_weights.resize(params.degree + 1);
        }

        for (int i = 0; i <= params.degree; ++i) {
            temp[i] = control_points[span - params.degree + i];
            if (params.rational) {
                temp_weights[i] = weights[span - params.degree + i];
            }
        }

        // Insert knot
        for (int j = 1; j <= multiplicity; ++j) {
            int L = span - params.degree + j;
            for (int i = 0; i <= params.degree - j; ++i) {
                float alpha = (t - knot_vector->GetKnots()[L + i]) / 
                            (knot_vector->GetKnots()[i + span + 1] - knot_vector->GetKnots()[L + i]);
                temp[i] = temp[i] * alpha + temp[i + 1] * (1.0f - alpha);
                if (params.rational) {
                    temp_weights[i] = temp_weights[i] * alpha + temp_weights[i + 1] * (1.0f - alpha);
                }
            }
            new_points.push_back(temp[0]);
            if (params.rational) {
                new_weights.push_back(temp_weights[0]);
            }
        }

        // Copy remaining points
        for (size_t i = span - params.degree + 1; i < control_points.size(); ++i) {
            new_points.push_back(control_points[i]);
            if (params.rational) {
                new_weights.push_back(weights[i]);
            }
        }

        // Update knot vector
        if (knot_vector->InsertKnot(t, multiplicity) != KnotVector::ErrorCode::None) {
            return false;
        }

        // Update curve data
        control_points = std::move(new_points);
        if (params.rational) {
            weights = std::move(new_weights);
        }
        Clear();

        return true;
    }

    /**
     * @brief Remove knot from curve
     * @param t Parameter value of knot to remove
     * @param multiplicity Number of times to remove knot
     * @return true if removal successful
     */
    bool RemoveKnot(float t, int multiplicity) {
        if (!ValidateConfiguration() || 
            !std::isfinite(t) || 
            multiplicity < 1) {
            return false;
        }

        // Find knot span
        int span = basis->FindSpan(t);
        if (span < 0) {
            return false;
        }

        // Calculate error tolerance based on curve size
        float curve_size = 0.0f;
        for (size_t i = 1; i < control_points.size(); ++i) {
            curve_size = std::max(curve_size, 
                glm::length(control_points[i] - control_points[i-1]));
        }
        float error_tolerance = curve_size * params.tolerance;

        // Try removing knots
        std::vector<glm::vec3> temp_points = control_points;
        std::vector<float> temp_weights = weights;
        std::vector<float> temp_knots = knot_vector->GetKnots();

        for (int r = 0; r < multiplicity; ++r) {
            // Find knot index
            auto it = std::find_if(temp_knots.begin(), temp_knots.end(),
                [t](float k) { return std::abs(k - t) <= 1e-6f; });
            if (it == temp_knots.end()) {
                break;
            }
            int k = std::distance(temp_knots.begin(), it);

            // Calculate affected control points
            int first = k - params.degree;
            int last = k;
            std::vector<glm::vec3> new_points(temp_points.size() - 1);
            std::vector<float> new_weights;
            if (params.rational) {
                new_weights.resize(temp_weights.size() - 1);
            }

            // Copy unaffected points
            for (int i = 0; i < first; ++i) {
                new_points[i] = temp_points[i];
                if (params.rational) {
                    new_weights[i] = temp_weights[i];
                }
            }
            for (size_t i = last + 1; i < temp_points.size(); ++i) {
                new_points[i-1] = temp_points[i];
                if (params.rational) {
                    new_weights[i-1] = temp_weights[i];
                }
            }

            // Calculate new control points
            for (int i = first; i <= last - 1; ++i) {
                float alpha = (t - temp_knots[i]) / 
                            (temp_knots[i + params.degree + 1] - temp_knots[i]);
                glm::vec3 point;
                if (std::abs(alpha) <= params.tolerance) {
                    point = temp_points[i+1];
                } else if (std::abs(1.0f - alpha) <= params.tolerance) {
                    point = temp_points[i];
                } else {
                    point = (temp_points[i] - temp_points[i+1] * (1.0f - alpha)) / alpha;
                }

                // Check if removal maintains curve shape
                glm::vec3 original = Evaluate(temp_knots[i]);
                if (glm::length(point - original) > error_tolerance) {
                    return false;  // Cannot remove knot without exceeding error tolerance
                }

                new_points[i] = point;
                if (params.rational) {
                    float w;
                    if (std::abs(alpha) <= params.tolerance) {
                        w = temp_weights[i+1];
                    } else if (std::abs(1.0f - alpha) <= params.tolerance) {
                        w = temp_weights[i];
                    } else {
                        w = (temp_weights[i] - temp_weights[i+1] * (1.0f - alpha)) / alpha;
                    }
                    new_weights[i] = w;
                }
            }

            // Update temporary data
            temp_points = std::move(new_points);
            if (params.rational) {
                temp_weights = std::move(new_weights);
            }
            temp_knots.erase(it);
        }

        // Update knot vector
        if (knot_vector->RemoveKnot(t, multiplicity) != KnotVector::ErrorCode::None) {
            return false;
        }

        // Update curve data
        control_points = std::move(temp_points);
        if (params.rational) {
            weights = std::move(temp_weights);
        }
        Clear();

        return true;
    }

    /**
     * @brief Split curve at parameter value
     * @param t Parameter value to split at
     * @param left Left portion of split curve
     * @param right Right portion of split curve
     * @return true if split successful
     */
    /**
     * @brief Split curve at parameter value
     * @param t Parameter value to split at
     * @param left Left portion of split curve
     * @param right Right portion of split curve
     * @return true if split successful
     */
    bool Split(float t, NURBSCurve& left, NURBSCurve& right) const {
        if (!ValidateConfiguration() || !std::isfinite(t) || t <= 0.0f || t >= 1.0f) {
            return false;
        }

        // Insert knot at split point to maximum multiplicity
        NURBSCurve temp;
        temp.impl->control_points = control_points;
        temp.impl->weights = weights;
        temp.impl->params = params;
        temp.impl->is_configured = is_configured;
        temp.impl->basis = std::make_unique<NURBSBasis>(*basis);
        temp.impl->knot_vector = std::make_unique<KnotVector>(*knot_vector);

        if (!temp.impl->InsertKnot(t, params.degree)) {
            return false;
        }

        // Find split index in knot vector
        const auto& knots = temp.impl->knot_vector->GetKnots();
        const float tolerance = params.tolerance;
        auto it = std::find_if(knots.begin(), knots.end(),
            [t, tolerance](float k) { return std::abs(k - t) <= tolerance; });
        if (it == knots.end()) {
            return false;
        }
        size_t split_idx = std::distance(knots.begin(), it);

        // Create left curve
        std::vector<glm::vec3> left_points(temp.impl->control_points.begin(),
            temp.impl->control_points.begin() + split_idx + 1);
        std::vector<float> left_weights;
        if (params.rational) {
            left_weights.assign(temp.impl->weights.begin(),
                temp.impl->weights.begin() + split_idx + 1);
        }

        // Create right curve
        std::vector<glm::vec3> right_points(temp.impl->control_points.begin() + split_idx,
            temp.impl->control_points.end());
        std::vector<float> right_weights;
        if (params.rational) {
            right_weights.assign(temp.impl->weights.begin() + split_idx,
                temp.impl->weights.end());
        }

        // Configure curves
        left.impl->control_points = std::move(left_points);
        right.impl->control_points = std::move(right_points);
        if (params.rational) {
            left.impl->weights = std::move(left_weights);
            right.impl->weights = std::move(right_weights);
        }

        left.impl->params = params;
        right.impl->params = params;
        left.impl->is_configured = true;
        right.impl->is_configured = true;

        return true;
    }

    /**
     * @brief Join two curves at their endpoints
     * @param other Curve to join with
     * @return true if join successful
     */
    bool Join(const NURBSCurve& other) {
        if (!ValidateConfiguration() || !other.impl->ValidateConfiguration()) {
            return false;
        }

        // Check curves can be joined
        if (params.degree != other.impl->params.degree ||
            params.rational != other.impl->params.rational) {
            return false;
        }

        // Check endpoints match
        glm::vec3 this_end = Evaluate(1.0f);
        glm::vec3 other_start = other.impl->Evaluate(0.0f);
        if (glm::length(this_end - other_start) > params.tolerance) {
            return false;
        }

        // Combine control points
        std::vector<glm::vec3> new_points = control_points;
        new_points.insert(new_points.end(),
            other.impl->control_points.begin() + 1,  // Skip first point (duplicate)
            other.impl->control_points.end());

        // Combine weights for rational curves
        if (params.rational) {
            std::vector<float> new_weights = weights;
            new_weights.insert(new_weights.end(),
                other.impl->weights.begin() + 1,  // Skip first weight
                other.impl->weights.end());
            weights = std::move(new_weights);
        }

        control_points = std::move(new_points);
        Clear();

        return true;
    }

    bool ValidateConfiguration() const {
        if (control_points.empty()) {
            return false;
        }

        if (params.rational) {
            if (weights.size() != control_points.size()) {
                return false;
            }
            // Check for valid weights
            for (float w : weights) {
                if (w <= 0.0f || !std::isfinite(w)) {
                    return false;
                }
            }
        }

        if (params.degree < 1) {
            return false;
        }

        if (params.tolerance <= 0.0f) {
            return false;
        }

        return true;
    }

private:
    /**
     * @brief Calculate binomial coefficient
     * @param n Upper value
     * @param k Lower value
     * @return Binomial coefficient value
     */
    int BinomialCoefficient(int n, int k) const {
        if (k > n - k) {
            k = n - k;
        }

        int result = 1;
        for (int i = 0; i < k; ++i) {
            result *= (n - i);
            result /= (i + 1);
        }

        return result;
    }
};

} // namespace Modeling
} // namespace RebelCAD
