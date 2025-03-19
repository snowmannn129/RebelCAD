#include "../../include/modeling/NurbsSurface.hpp"
#include "../../include/modeling/SolidBody.hpp"
#include <algorithm>
#include <cmath>
#include <glm/gtc/epsilon.hpp>

namespace RebelCAD {
namespace Modeling {

std::shared_ptr<NurbsSurface> NurbsSurface::Create(
    const ControlPointGrid& controlPoints,
    size_t numU,
    size_t numV,
    size_t degreeU,
    size_t degreeV
) {
    // Validate input parameters
    if (controlPoints.size() != numU * numV) {
        throw Error::InvalidArgument(
            "Number of control points does not match grid dimensions"
        );
    }
    if (numU <= degreeU) {
        throw Error::InvalidArgument(
            "Number of control points in U direction must be greater than degree"
        );
    }
    if (numV <= degreeV) {
        throw Error::InvalidArgument(
            "Number of control points in V direction must be greater than degree"
        );
    }

    return std::shared_ptr<NurbsSurface>(
        new NurbsSurface(controlPoints, numU, numV, degreeU, degreeV)
    );
}

NurbsSurface::NurbsSurface(
    const ControlPointGrid& controlPoints,
    size_t numU,
    size_t numV,
    size_t degreeU,
    size_t degreeV
)
    : m_controlPoints(controlPoints)
    , m_weights(numU * numV, 1.0)  // Initialize all weights to 1.0
    , m_numU(numU)
    , m_numV(numV)
    , m_degreeU(degreeU)
    , m_degreeV(degreeV)
{
    // Initialize uniform knot vectors
    // For degree p, we need n + p + 1 knots where n is number of control points
    size_t numKnotsU = numU + degreeU + 1;
    size_t numKnotsV = numV + degreeV + 1;

    m_knotsU.resize(numKnotsU);
    m_knotsV.resize(numKnotsV);

    // Create uniform knot vectors with clamped ends
    // First p+1 knots are 0, last p+1 knots are 1
    for (size_t i = 0; i <= degreeU; ++i) {
        m_knotsU[i] = 0.0;
        m_knotsU[numKnotsU - 1 - i] = 1.0;
    }
    for (size_t i = degreeU + 1; i < numU; ++i) {
        m_knotsU[i] = static_cast<double>(i - degreeU) / (numU - degreeU);
    }

    for (size_t i = 0; i <= degreeV; ++i) {
        m_knotsV[i] = 0.0;
        m_knotsV[numKnotsV - 1 - i] = 1.0;
    }
    for (size_t i = degreeV + 1; i < numV; ++i) {
        m_knotsV[i] = static_cast<double>(i - degreeV) / (numV - degreeV);
    }
}

NurbsSurface::Point3D NurbsSurface::Evaluate(double u, double v) const {
    if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
        throw Error::OutOfRange("Parameter values must be in range [0,1]");
    }

    // Compute basis functions
    auto Nu = ComputeBasisFunctions(u, m_degreeU, m_knotsU);
    auto Nv = ComputeBasisFunctions(v, m_degreeV, m_knotsV);

    NurbsSurface::Point3D numerator(0.0);
    double denominator = 0.0;

    // Evaluate surface point using basis functions
    for (size_t i = 0; i < m_numU; ++i) {
        for (size_t j = 0; j < m_numV; ++j) {
            size_t idx = i * m_numV + j;
            double weight = m_weights[idx];
            double basis = Nu[i] * Nv[j] * weight;
            
            numerator += m_controlPoints[idx] * basis;
            denominator += basis;
        }
    }

    if (std::abs(denominator) < 1e-10) {
        throw Error::InternalError("Surface evaluation failed: zero denominator");
    }

    return numerator / denominator;
}

NurbsSurface::Point3D NurbsSurface::EvaluateDerivative(
    double u,
    double v,
    size_t du,
    size_t dv
) const {
    if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
        throw Error::OutOfRange("Parameter values must be in range [0,1]");
    }
    if (du > m_degreeU || dv > m_degreeV) {
        throw Error::InvalidArgument(
            "Derivative order cannot exceed surface degree"
        );
    }

    // Compute basis functions and derivatives
    auto Nu = ComputeBasisFunctions(u, m_degreeU, m_knotsU, du);
    auto Nv = ComputeBasisFunctions(v, m_degreeV, m_knotsV, dv);

    NurbsSurface::Point3D derivative(0.0);
    double weight = 0.0;

    // Compute derivative using product rule
    for (size_t i = 0; i < m_numU; ++i) {
        for (size_t j = 0; j < m_numV; ++j) {
            size_t idx = i * m_numV + j;
            double w = m_weights[idx];
            double basis = Nu[i] * Nv[j] * w;
            
            derivative += m_controlPoints[idx] * basis;
            weight += basis;
        }
    }

    if (std::abs(weight) < 1e-10) {
        throw Error::InternalError(
            "Derivative evaluation failed: zero denominator"
        );
    }

    return derivative / weight;
}

NurbsSurface::Point3D NurbsSurface::GetControlPoint(size_t i, size_t j) const {
    if (i >= m_numU || j >= m_numV) {
        throw Error::OutOfRange("Control point indices out of range");
    }
    return m_controlPoints[i * m_numV + j];
}

void NurbsSurface::SetControlPoint(size_t i, size_t j, const NurbsSurface::Point3D& point) {
    if (i >= m_numU || j >= m_numV) {
        throw Error::OutOfRange("Control point indices out of range");
    }
    m_controlPoints[i * m_numV + j] = point;
}

double NurbsSurface::GetWeight(size_t i, size_t j) const {
    if (i >= m_numU || j >= m_numV) {
        throw Error::OutOfRange("Weight indices out of range");
    }
    return m_weights[i * m_numV + j];
}

void NurbsSurface::SetWeight(size_t i, size_t j, double weight) {
    if (i >= m_numU || j >= m_numV) {
        throw Error::OutOfRange("Weight indices out of range");
    }
    if (weight <= 0.0) {
        throw Error::InvalidArgument("Weight must be positive");
    }
    m_weights[i * m_numV + j] = weight;
}

std::vector<double> NurbsSurface::ComputeBasisFunctions(
    double u,
    size_t degree,
    const std::vector<double>& knots,
    size_t derivativeOrder
) const {
    size_t n = knots.size() - degree - 1;  // number of basis functions
    std::vector<double> basis(n, 0.0);

    // Special case for 0th derivative (standard basis functions)
    if (derivativeOrder == 0) {
        // Find knot span
        size_t span = degree;
        while (span < n && knots[span + 1] <= u) ++span;

        // Initialize zeroth degree basis functions
        std::vector<double> left(degree + 1);
        std::vector<double> right(degree + 1);
        basis[span] = 1.0;

        // Compute basis functions of increasing degree
        for (size_t d = 1; d <= degree; ++d) {
            left[d] = u - knots[span - d + 1];
            right[d] = knots[span + d] - u;
            double saved = 0.0;

            for (size_t r = 0; r < d; ++r) {
                double temp = basis[span - r] / (right[r + 1] + left[d - r]);
                basis[span - r] = saved + right[r + 1] * temp;
                saved = left[d - r] * temp;
            }
            basis[span - d] = saved;
        }
    }
    // Compute derivatives using recursive formula
    else {
        // Compute basis functions and their derivatives up to the requested order
        std::vector<std::vector<double>> ders(derivativeOrder + 1, std::vector<double>(n, 0.0));
        
        // Find knot span
        size_t span = degree;
        while (span < n && knots[span + 1] <= u) ++span;

        // Array of basis functions and knot differences
        std::vector<std::vector<double>> ndu(degree + 1, std::vector<double>(degree + 1));
        std::vector<double> left(degree + 1);
        std::vector<double> right(degree + 1);

        ndu[0][0] = 1.0;
        for (size_t j = 1; j <= degree; ++j) {
            left[j] = u - knots[span + 1 - j];
            right[j] = knots[span + j] - u;
            double saved = 0.0;

            for (size_t r = 0; r < j; ++r) {
                // Lower triangle
                ndu[j][r] = right[r + 1] + left[j - r];
                double temp = ndu[r][j - 1] / ndu[j][r];
                // Upper triangle
                ndu[r][j] = saved + right[r + 1] * temp;
                saved = left[j - r] * temp;
            }
            ndu[j][j] = saved;
        }

        // Load the basis functions
        for (size_t j = 0; j <= degree; ++j) {
            ders[0][span - degree + j] = ndu[j][degree];
        }

        // Compute derivatives
        std::vector<std::vector<double>> a(2, std::vector<double>(degree + 1));
        for (size_t r = 0; r <= degree; ++r) {
            size_t s1 = 0;
            size_t s2 = 1;
            a[0][0] = 1.0;

            // Compute kth derivative
            for (size_t k = 1; k <= derivativeOrder; ++k) {
                double d = 0.0;
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

                ders[k][span - degree + r] = d;

                // Switch rows
                std::swap(s1, s2);
            }
        }

        // Multiply through by the correct factors
        double fac = degree;
        for (size_t k = 1; k <= derivativeOrder; ++k) {
            for (size_t j = 0; j <= degree; ++j) {
                ders[k][span - degree + j] *= fac;
            }
            fac *= (degree - k);
        }

        return ders[derivativeOrder];
    }

    return basis;
}

void NurbsSurface::ValidateParameters() const {
    if (m_controlPoints.size() != m_numU * m_numV) {
        throw Error::InternalError("Invalid control point count");
    }
    if (m_weights.size() != m_numU * m_numV) {
        throw Error::InternalError("Invalid weight count");
    }
    if (m_knotsU.size() != m_numU + m_degreeU + 1) {
        throw Error::InternalError("Invalid U knot vector size");
    }
    if (m_knotsV.size() != m_numV + m_degreeV + 1) {
        throw Error::InternalError("Invalid V knot vector size");
    }
}

std::shared_ptr<SolidBody> NurbsSurface::ToSolidBody(
    size_t uDiv,
    size_t vDiv
) const {
    if (uDiv < 1 || vDiv < 1) {
        throw Error::InvalidArgument(
            "Tessellation divisions must be greater than zero"
        );
    }

    // Create vertices by evaluating surface points
    std::vector<SolidBody::Vertex> vertices;
    vertices.reserve((uDiv + 1) * (vDiv + 1));

    // Step size for parameter space
    double du = 1.0 / uDiv;
    double dv = 1.0 / vDiv;

    // Generate vertices
    for (size_t j = 0; j <= vDiv; ++j) {
        double v = j * dv;
        for (size_t i = 0; i <= uDiv; ++i) {
            double u = i * du;

            // Evaluate position
            auto position = Evaluate(u, v);

            // Evaluate derivatives for normal calculation
            auto du_vec = EvaluateDerivative(u, v, 1, 0);
            auto dv_vec = EvaluateDerivative(u, v, 0, 1);
            auto normal = glm::normalize(glm::cross(du_vec, dv_vec));

            // Create vertex with position, normal, and UV coordinates
            vertices.push_back(SolidBody::Vertex(
                position,
                normal,
                SolidBody::UV(u, v)
            ));
        }
    }

    // Create triangles
    std::vector<SolidBody::Triangle> triangles;
    triangles.reserve(uDiv * vDiv * 2);  // 2 triangles per grid cell

    // Generate triangles
    for (size_t j = 0; j < vDiv; ++j) {
        for (size_t i = 0; i < uDiv; ++i) {
            // Calculate vertex indices for current grid cell
            size_t v00 = j * (uDiv + 1) + i;
            size_t v10 = v00 + 1;
            size_t v01 = (j + 1) * (uDiv + 1) + i;
            size_t v11 = v01 + 1;

            // Add two triangles for the grid cell
            triangles.push_back(SolidBody::Triangle(v00, v10, v11));
            triangles.push_back(SolidBody::Triangle(v00, v11, v01));
        }
    }

    return SolidBody::Create(vertices, triangles);
}

} // namespace Modeling
} // namespace RebelCAD
