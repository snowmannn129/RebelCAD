#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

/**
 * @class NURBSBasis
 * @brief Class for computing NURBS basis functions and derivatives
 * 
 * The NURBSBasis class provides functionality for calculating
 * B-spline basis functions and their derivatives. It supports
 * both uniform and non-uniform knot vectors, and handles
 * caching of intermediate results for efficiency.
 */
class NURBSBasis {
public:
    NURBSBasis();
    ~NURBSBasis();

    /**
     * @brief Initialize basis calculator with knot vector and degree
     * @param knots Knot vector
     * @param degree Curve degree
     * @return true if initialization successful
     */
    bool Initialize(const std::vector<float>& knots, int degree);

    /**
     * @brief Calculate basis functions at parameter value
     * @param t Parameter value
     * @param span Knot span containing t
     * @return Vector of basis function values
     */
    std::vector<float> Evaluate(float t, int span) const;

    /**
     * @brief Calculate basis function derivatives at parameter value
     * @param t Parameter value
     * @param span Knot span containing t
     * @param order Maximum derivative order
     * @return Vector of basis function derivatives
     */
    std::vector<std::vector<float>> EvaluateDerivatives(
        float t, int span, int order) const;

    /**
     * @brief Find knot span containing parameter value
     * @param t Parameter value
     * @return Index of knot span
     */
    int FindSpan(float t) const;

    /**
     * @brief Get effective degree of basis functions
     * @return Curve degree
     */
    int GetDegree() const;

    /**
     * @brief Get number of basis functions
     * @return Number of functions
     */
    int GetCount() const;

protected:
    class Impl;
    std::unique_ptr<Impl> impl;

    /**
     * @brief Calculate basis function value
     * @param i Function index
     * @param p Degree
     * @param t Parameter value
     * @return Basis function value
     */
    float BasisFunction(int i, int p, float t) const;

    /**
     * @brief Calculate basis function derivative
     * @param i Function index
     * @param p Degree
     * @param t Parameter value
     * @param order Derivative order
     * @return Basis function derivative value
     */
    float BasisDerivative(int i, int p, float t, int order) const;

private:
    /**
     * @brief Calculate binomial coefficient
     * @param n Upper value
     * @param k Lower value
     * @return Binomial coefficient value
     */
    int BinomialCoefficient(int n, int k) const;

    /**
     * @brief Initialize coefficient cache
     * @param max_degree Maximum degree to cache
     */
    void InitializeCache(int max_degree);

    /**
     * @brief Clear cached values
     */
    void ClearCache();
};

} // namespace Modeling
} // namespace RebelCAD
