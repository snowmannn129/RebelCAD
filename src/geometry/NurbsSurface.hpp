#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../core/Error.hpp"

namespace RebelCAD {
namespace Modeling {

class NurbsSurface {
public:
    using Point3D = glm::dvec3;
    using ControlPointGrid = std::vector<Point3D>;
    using WeightGrid = std::vector<double>;

    /**
     * Creates a NURBS surface with the given control points and degrees.
     * 
     * @param controlPoints Control points in row-major order
     * @param numU Number of control points in U direction
     * @param numV Number of control points in V direction
     * @param degreeU Degree in U direction
     * @param degreeV Degree in V direction
     * @return std::shared_ptr<NurbsSurface>
     * @throws RebelCAD::Error if invalid parameters
     */
    static std::shared_ptr<NurbsSurface> Create(
        const ControlPointGrid& controlPoints,
        size_t numU,
        size_t numV,
        size_t degreeU,
        size_t degreeV
    );

    /**
     * Evaluates the surface at the given parameter values.
     * 
     * @param u Parameter in U direction [0,1]
     * @param v Parameter in V direction [0,1]
     * @return Point3D
     * @throws RebelCAD::Error if parameters out of range
     */
    Point3D Evaluate(double u, double v) const;

    /**
     * Evaluates surface derivative at given parameter values.
     * 
     * @param u Parameter in U direction [0,1]
     * @param v Parameter in V direction [0,1]
     * @param du Order of derivative in U direction
     * @param dv Order of derivative in V direction
     * @return Point3D
     * @throws RebelCAD::Error if parameters invalid
     */
    Point3D EvaluateDerivative(double u, double v, size_t du, size_t dv) const;

    /**
     * Gets control point at given indices.
     * 
     * @param i Index in U direction
     * @param j Index in V direction
     * @return Point3D
     * @throws RebelCAD::Error if indices out of range
     */
    Point3D GetControlPoint(size_t i, size_t j) const;

    /**
     * Sets control point at given indices.
     * 
     * @param i Index in U direction
     * @param j Index in V direction
     * @param point New control point position
     * @throws RebelCAD::Error if indices out of range
     */
    void SetControlPoint(size_t i, size_t j, const Point3D& point);

    /**
     * Gets weight at given indices.
     * 
     * @param i Index in U direction
     * @param j Index in V direction
     * @return double
     * @throws RebelCAD::Error if indices out of range
     */
    double GetWeight(size_t i, size_t j) const;

    /**
     * Sets weight at given indices.
     * 
     * @param i Index in U direction
     * @param j Index in V direction
     * @param weight New weight value
     * @throws RebelCAD::Error if indices out of range or weight <= 0
     */
    void SetWeight(size_t i, size_t j, double weight);

    /**
     * Converts surface to tessellated solid body.
     * 
     * @param uDiv Number of divisions in U direction
     * @param vDiv Number of divisions in V direction
     * @return std::shared_ptr<SolidBody>
     * @throws RebelCAD::Error if divisions < 1
     */
    std::shared_ptr<class SolidBody> ToSolidBody(size_t uDiv, size_t vDiv) const;

private:
    NurbsSurface(
        const ControlPointGrid& controlPoints,
        size_t numU,
        size_t numV,
        size_t degreeU,
        size_t degreeV
    );

    /**
     * Validates surface parameters.
     * 
     * @throws RebelCAD::Error if parameters invalid
     */
    void ValidateParameters() const;

    /**
     * Computes basis functions and their derivatives.
     * 
     * @param u Parameter value
     * @param degree Curve degree
     * @param knots Knot vector
     * @param derivativeOrder Order of derivative to compute (0 for basis functions)
     * @return std::vector<double> Basis function values or derivatives
     */
    std::vector<double> ComputeBasisFunctions(
        double u,
        size_t degree,
        const std::vector<double>& knots,
        size_t derivativeOrder = 0
    ) const;

    ControlPointGrid m_controlPoints;
    WeightGrid m_weights;
    size_t m_numU;
    size_t m_numV;
    size_t m_degreeU;
    size_t m_degreeV;
    std::vector<double> m_knotsU;
    std::vector<double> m_knotsV;
};

} // namespace Modeling
} // namespace RebelCAD
