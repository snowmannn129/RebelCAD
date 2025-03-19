#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "modeling/NURBSBasis.h"

namespace RebelCAD {
namespace Modeling {

class NURBSBasis::Impl {
public:
    // Input data
    std::vector<float> knots;
    int degree = 0;
    bool is_initialized = false;

    // Cache for optimization
    struct BasisCache {
        float t;                                     // Parameter value
        int span;                                    // Knot span
        std::vector<float> values;                   // Basis function values
        std::vector<std::vector<float>> derivatives; // Basis function derivatives
    };
    mutable std::vector<BasisCache> cache;          // Cache entries
    mutable bool cache_valid = false;               // Whether cache is valid

    Impl() = default;

    /**
     * @brief Clear cached data
     */
    void Clear() {
        cache.clear();
        cache_valid = false;
    }

    /**
     * @brief Initialize basis calculator
     * @param knots Knot vector
     * @param degree Curve degree
     * @return true if initialization successful
     */
    bool Initialize(const std::vector<float>& knots, int degree) {
        if (knots.empty() || degree < 1) {
            return false;
        }

        this->knots = knots;
        this->degree = degree;
        Clear();
        is_initialized = true;
        return true;
    }

    /**
     * @brief Calculate basis functions at parameter value
     * @param t Parameter value
     * @param span Knot span containing t
     * @return Vector of basis function values
     */
    std::vector<float> Evaluate(float t, int span) const {
        // Check cache first
        for (const auto& entry : cache) {
            if (std::abs(entry.t - t) <= 1e-6f && entry.span == span) {
                return entry.values;
            }
        }

        // Calculate basis functions
        std::vector<float> values(degree + 1, 0.0f);
        std::vector<float> left(degree + 1);
        std::vector<float> right(degree + 1);

        // Initialize degree 0
        values[0] = 1.0f;

        // Calculate up to requested degree
        for (int j = 1; j <= degree; ++j) {
            left[j] = t - knots[span + 1 - j];
            right[j] = knots[span + j] - t;
            float saved = 0.0f;

            for (int r = 0; r < j; ++r) {
                float temp = values[r] / (right[r + 1] + left[j - r]);
                values[r] = saved + right[r + 1] * temp;
                saved = left[j - r] * temp;
            }

            values[j] = saved;
        }

        // Cache result
        cache.push_back({t, span, values, {}});
        if (cache.size() > 100) { // Limit cache size
            cache.erase(cache.begin());
        }

        return values;
    }

    /**
     * @brief Calculate basis function derivatives
     * @param t Parameter value
     * @param span Knot span containing t
     * @param order Maximum derivative order
     * @return Vector of basis function derivatives
     */
    std::vector<std::vector<float>> EvaluateDerivatives(
        float t, int span, int order) const {
        
        // Check cache first
        for (const auto& entry : cache) {
            if (std::abs(entry.t - t) <= 1e-6f && 
                entry.span == span &&
                !entry.derivatives.empty() &&
                entry.derivatives.size() > static_cast<size_t>(order)) {
                return entry.derivatives;
            }
        }

        // Calculate derivatives
        std::vector<std::vector<float>> derivatives(order + 1);
        for (int i = 0; i <= order; ++i) {
            derivatives[i].resize(degree + 1);
        }

        // Initialize with basis functions
        std::vector<std::vector<float>> ndu(degree + 1);
        for (auto& row : ndu) {
            row.resize(degree + 1);
        }
        ndu[0][0] = 1.0f;

        std::vector<float> left(degree + 1);
        std::vector<float> right(degree + 1);

        for (int j = 1; j <= degree; ++j) {
            left[j] = t - knots[span + 1 - j];
            right[j] = knots[span + j] - t;
            float saved = 0.0f;

            for (int r = 0; r < j; ++r) {
                ndu[j][r] = right[r + 1] + left[j - r];
                float temp = ndu[r][j - 1] / ndu[j][r];
                ndu[r][j] = saved + right[r + 1] * temp;
                saved = left[j - r] * temp;
            }

            ndu[j][j] = saved;
        }

        // Load basis functions
        for (int j = 0; j <= degree; ++j) {
            derivatives[0][j] = ndu[j][degree];
        }

        // Calculate derivatives
        std::vector<std::vector<float>> a(2);
        for (auto& row : a) {
            row.resize(degree + 1);
        }

        for (int r = 0; r <= degree; ++r) {
            int s1 = 0;
            int s2 = 1;
            a[0][0] = 1.0f;

            for (int k = 1; k <= order; ++k) {
                float d = 0.0f;
                int rk = r - k;
                int pk = degree - k;

                if (r >= k) {
                    a[s2][0] = a[s1][0] / ndu[pk + 1][rk];
                    d = a[s2][0] * ndu[rk][pk];
                }

                int j1 = rk >= -1 ? 1 : -rk;
                int j2 = r - 1 <= pk ? k - 1 : degree - r;

                for (int j = j1; j <= j2; ++j) {
                    a[s2][j] = (a[s1][j] - a[s1][j - 1]) / ndu[pk + 1][rk + j];
                    d += a[s2][j] * ndu[rk + j][pk];
                }

                if (r <= pk) {
                    a[s2][k] = -a[s1][k - 1] / ndu[pk + 1][r];
                    d += a[s2][k] * ndu[r][pk];
                }

                derivatives[k][r] = d;
                std::swap(s1, s2);
            }
        }

        // Multiply by correct factors
        float r = degree;
        for (int k = 1; k <= order; ++k) {
            for (int j = 0; j <= degree; ++j) {
                derivatives[k][j] *= r;
            }
            r *= (degree - k);
        }

        // Update or add to cache
        bool found = false;
        for (size_t i = 0; i < cache.size(); ++i) {
            if (std::abs(cache[i].t - t) <= 1e-6f && cache[i].span == span) {
                cache[i].derivatives = derivatives;
                found = true;
                break;
            }
        }

        if (!found) {
            cache.push_back({t, span, Evaluate(t, span), derivatives});
            if (cache.size() > 100) { // Limit cache size
                cache.erase(cache.begin());
            }
        }

        return derivatives;
    }

    /**
     * @brief Find knot span containing parameter value
     * @param t Parameter value
     * @return Index of knot span
     */
    int FindSpan(float t) const {
        if (knots.empty() || !is_initialized) {
            return -1;
        }

        // Handle boundary cases
        if (t <= knots.front()) {
            return degree;
        }
        if (t >= knots.back()) {
            return knots.size() - degree - 2;
        }

        // Binary search
        int low = degree;
        int high = knots.size() - degree - 1;
        int mid = (low + high) / 2;

        while (t < knots[mid] || t >= knots[mid + 1]) {
            if (t < knots[mid]) {
                high = mid;
            } else {
                low = mid;
            }
            mid = (low + high) / 2;
        }

        return mid;
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
