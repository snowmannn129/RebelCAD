#pragma once

#include <vector>
#include <array>
#include <memory>
#include "Geometry.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Represents a NURBS (Non-Uniform Rational B-Spline) surface
 * 
 * A NURBS surface is defined by:
 * - A bidirectional grid of control points
 * - Two knot vectors (U and V directions)
 * - Weights for each control point
 * - Degrees in U and V directions
 */
class NurbsSurface {
public:
    /**
     * @brief Creates a NURBS surface from a grid of control points
     * @param controlPoints Grid of control points (row-major order)
     * @param numU Number of control points in U direction
     * @param numV Number of control points in V direction
     * @param degreeU Degree in U direction
     * @param degreeV Degree in V direction
     * @param weights Optional weights for control points (default all 1.0)
     * @return Shared pointer to new NURBS surface
     */
    static std::shared_ptr<NurbsSurface> Create(
        const std::vector<Point3D>& controlPoints,
        size_t numU,
        size_t numV,
        size_t degreeU,
        size_t degreeV,
        const std::vector<double>& weights = std::vector<double>());

    /**
     * @brief Evaluates the surface at given parameter values
     * @param u Parameter in U direction [0,1]
     * @param v Parameter in V direction [0,1]
     * @return 3D point on surface
     */
    Point3D Evaluate(double u, double v) const;

    /**
     * @brief Computes surface derivatives at given parameter values
     * @param u Parameter in U direction [0,1]
     * @param v Parameter in V direction [0,1]
     * @param derivU Order of U derivative
     * @param derivV Order of V derivative
     * @return Derivative vector at (u,v)
     */
    Point3D EvaluateDerivative(double u, double v, size_t derivU, size_t derivV) const;

    /**
     * @brief Converts surface to a tessellated solid body
     * @param uDivisions Number of divisions in U direction
     * @param vDivisions Number of divisions in V direction
     * @return Solid body representation
     */
    std::shared_ptr<SolidBody> ToSolidBody(size_t uDivisions = 32, size_t vDivisions = 32) const;

    /**
     * @brief Gets the degree in U direction
     */
    size_t GetDegreeU() const { return m_degreeU; }

    /**
     * @brief Gets the degree in V direction
     */
    size_t GetDegreeV() const { return m_degreeV; }

    /**
     * @brief Gets the number of control points in U direction
     */
    size_t GetNumU() const { return m_numU; }

    /**
     * @brief Gets the number of control points in V direction
     */
    size_t GetNumV() const { return m_numV; }

    /**
     * @brief Gets the control point at given indices
     * @param i Index in U direction
     * @param j Index in V direction
     * @return Control point at (i,j)
     */
    Point3D GetControlPoint(size_t i, size_t j) const;

    /**
     * @brief Sets the control point at given indices
     * @param i Index in U direction
     * @param j Index in V direction
     * @param point New control point position
     */
    void SetControlPoint(size_t i, size_t j, const Point3D& point);

    /**
     * @brief Gets the weight for control point at given indices
     * @param i Index in U direction
     * @param j Index in V direction
     * @return Weight value
     */
    double GetWeight(size_t i, size_t j) const;

    /**
     * @brief Sets the weight for control point at given indices
     * @param i Index in U direction
     * @param j Index in V direction
     * @param weight New weight value
     */
    void SetWeight(size_t i, size_t j, double weight);

private:
    NurbsSurface(size_t numU, size_t numV, size_t degreeU, size_t degreeV);

    /**
     * @brief Computes basis functions and their derivatives
     * @param u Parameter value
     * @param span Knot span index
     * @param degree Curve degree
     * @param knots Knot vector
     * @param derivatives Number of derivatives to compute (0 = just basis functions)
     * @return Vector of basis functions and their derivatives
     */
    std::vector<std::vector<double>> ComputeBasisFunctions(
        double u, size_t span, size_t degree,
        const std::vector<double>& knots, size_t derivatives) const;

    /**
     * @brief Finds the knot span index for parameter value
     * @param u Parameter value
     * @param degree Curve degree
     * @param knots Knot vector
     * @return Span index
     */
    size_t FindSpan(double u, size_t degree, const std::vector<double>& knots) const;

    size_t m_numU;                    // Number of control points in U direction
    size_t m_numV;                    // Number of control points in V direction
    size_t m_degreeU;                 // Degree in U direction
    size_t m_degreeV;                 // Degree in V direction
    std::vector<Point3D> m_points;    // Control points (row-major order)
    std::vector<double> m_weights;    // Weights for control points
    std::vector<double> m_knotsU;     // Knot vector in U direction
    std::vector<double> m_knotsV;     // Knot vector in V direction
};

} // namespace Modeling
} // namespace RebelCAD
