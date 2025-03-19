#pragma once

#include <vector>
#include <memory>
#include "graphics/Color.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @brief Represents a B-spline curve in 2D space
 * 
 * This class implements a B-spline curve defined by control points.
 * B-splines provide smooth curve interpolation with local control,
 * making them ideal for CAD applications.
 */
class Spline {
public:
    /**
     * @brief Struct to represent a control point's influence region
     */
    struct ControlPointInfluence {
        double startParam;  // Start parameter where influence begins
        double endParam;    // End parameter where influence ends
        double maxInfluence; // Maximum basis function value
    };

    /**
     * @brief Construct a new Spline object
     */
    Spline();

    /**
     * @brief Construct a Spline with initial control points
     * @param controlPoints Vector of control points defining the spline
     */
    explicit Spline(const std::vector<std::pair<double, double>>& controlPoints);

    /**
     * @brief Add a control point to the spline
     * @param x X coordinate of the control point
     * @param y Y coordinate of the control point
     */
    void addControlPoint(double x, double y);

    /**
     * @brief Remove a control point at the specified index
     * @param index Index of the control point to remove
     * @return true if point was removed, false if index was invalid
     */
    bool removeControlPoint(size_t index);

    /**
     * @brief Move an existing control point to a new position
     * @param index Index of the control point to move
     * @param newX New X coordinate
     * @param newY New Y coordinate
     * @return true if point was moved, false if index was invalid
     */
    bool moveControlPoint(size_t index, double newX, double newY);

    /**
     * @brief Get the current control points
     * @return Vector of control point coordinates
     */
    const std::vector<std::pair<double, double>>& getControlPoints() const;

    /**
     * @brief Calculate points along the spline curve for rendering
     * @param minPoints Minimum number of points to generate (default: 50)
     * @param maxPoints Maximum number of points to generate (default: 500)
     * @param curvatureTolerance Tolerance for adaptive sampling (default: 0.1)
     * @return Vector of points defining the curve
     */
    std::vector<std::pair<double, double>> calculateCurvePoints(
        size_t minPoints = 50,
        size_t maxPoints = 500,
        double curvatureTolerance = 0.1) const;

    /**
     * @brief Calculate a point on the curve at parameter value u
     * @param u Parameter value between 0 and 1
     * @return Point coordinates
     */
    std::pair<double, double> evaluatePoint(double u) const;

    /**
     * @brief Calculate the curvature at a point on the curve
     * @param u Parameter value between 0 and 1
     * @return Curvature value at the point
     */
    double calculateCurvature(double u) const;

    /**
     * @brief Calculate the first derivative at parameter value u
     * @param u Parameter value between 0 and 1
     * @return Vector representing the first derivative (dx/du, dy/du)
     */
    std::pair<double, double> evaluateFirstDerivative(double u) const;

    /**
     * @brief Calculate the second derivative at parameter value u
     * @param u Parameter value between 0 and 1
     * @return Vector representing the second derivative (d²x/du², d²y/du²)
     */
    std::pair<double, double> evaluateSecondDerivative(double u) const;

    /**
     * @brief Set the degree of the B-spline
     * @param degree Degree of the B-spline (typically 2 or 3)
     */
    void setDegree(unsigned int degree);

    /**
     * @brief Get the current degree of the B-spline
     * @return Current degree value
     */
    unsigned int getDegree() const;

    /**
     * @brief Set the color for rendering the spline
     * @param color Color to use for rendering
     */
    void setColor(const Graphics::Color& color);

    /**
     * @brief Get the current rendering color
     * @return Current color
     */
    const Graphics::Color& getColor() const;

    /**
     * @brief Set a custom knot vector for non-uniform B-spline
     * @param knots Vector of knot values
     * @throw std::invalid_argument if knot vector is invalid
     */
    void setKnotVector(const std::vector<double>& knots);

    /**
     * @brief Get the current knot vector
     * @return Vector of knot values
     */
    std::vector<double> getKnotVector() const;

    /**
     * @brief Set weights for rational B-spline (NURBS)
     * @param weights Vector of weights for control points
     * @throw std::invalid_argument if weights size doesn't match control points
     */
    void setWeights(const std::vector<double>& weights);

    /**
     * @brief Get the current weights
     * @return Vector of weights (empty if not rational)
     */
    std::vector<double> getWeights() const;

    /**
     * @brief Convert to a clamped B-spline (interpolates endpoints)
     */
    void convertToClamped();

    /**
     * @brief Check if the spline is clamped
     * @return true if spline interpolates endpoints
     */
    bool isClamped() const;

    /**
     * @brief Insert a new knot into the knot vector
     * @param u Parameter value where to insert knot
     * @param r Multiplicity of insertion (default: 1)
     * @throw std::invalid_argument if parameter value is invalid
     */
    void insertKnot(double u, unsigned int r = 1);

    /**
     * @brief Remove a knot from the knot vector
     * @param u Parameter value of knot to remove
     * @param r Number of times to remove (default: 1)
     * @param tolerance Error tolerance for control point adjustment
     * @return Number of knots actually removed
     */
    unsigned int removeKnot(double u, unsigned int r = 1, double tolerance = 1e-6);

    /**
     * @brief Calculate the arc length of the spline curve
     * @param tolerance Tolerance for numerical integration (default: 1e-6)
     * @return Total length of the curve
     */
    double calculateArcLength(double tolerance = 1e-6) const;

    /**
     * @brief Find the parameter value at a given arc length
     * @param distance Distance along the curve
     * @param tolerance Tolerance for numerical integration (default: 1e-6)
     * @return Parameter value u corresponding to the distance
     */
    double parameterAtLength(double distance, double tolerance = 1e-6) const;

    /**
     * @brief Calculate point at a specific arc length distance
     * @param distance Distance along the curve
     * @param tolerance Tolerance for numerical integration (default: 1e-6)
     * @return Point coordinates at the specified distance
     */
    std::pair<double, double> pointAtLength(double distance, double tolerance = 1e-6) const;

    /**
     * @brief Find the closest point on the spline to a given point
     * @param x X coordinate of the target point
     * @param y Y coordinate of the target point
     * @param tolerance Convergence tolerance for Newton iteration (default: 1e-6)
     * @param maxIterations Maximum number of Newton iterations (default: 20)
     * @return Pair containing:
     *         - Point coordinates on the spline
     *         - Parameter value u where the closest point occurs
     */
    std::pair<std::pair<double, double>, double> findClosestPoint(
        double x, double y,
        double tolerance = 1e-6,
        size_t maxIterations = 20) const;

    /**
     * @brief Calculate the distance between a point and the spline at parameter u
     * @param x X coordinate of the point
     * @param y Y coordinate of the point
     * @param u Parameter value on the spline
     * @return Distance between the point and spline at u
     */
    double distanceToPoint(double x, double y, double u) const;

    /**
     * @brief Calculate the influence region for each control point
     * @param numSamples Number of samples for basis function evaluation (default: 100)
     * @return Vector of ControlPointInfluence structs, one for each control point
     */
    std::vector<ControlPointInfluence> calculateControlPointInfluences(size_t numSamples = 100) const;

private:
    /**
     * @brief Helper function for numerical integration using Gaussian quadrature
     * @param start Start parameter value
     * @param end End parameter value
     * @param tolerance Integration tolerance
     * @return Definite integral value
     */
    double integrateGaussian(double start, double end, double tolerance) const;

    /**
     * @brief Split the spline into two curves at a parameter value
     * @param u Parameter value where to split (between 0 and 1)
     * @return Pair of splines representing the two parts of the original curve
     * @throw std::invalid_argument if u is not between 0 and 1
     */
    std::pair<Spline, Spline> splitAtParameter(double u) const;

    /**
     * @brief Calculate the basis function value for B-spline
     * @param i Knot span index
     * @param degree Degree of the basis function
     * @param u Parameter value
     * @param knots Knot vector
     * @return Basis function value
     */
    double basisFunction(int i, int degree, double u, const std::vector<double>& knots) const;

    /**
     * @brief Generate a uniform knot vector for the B-spline
     * @return Vector of knot values
     */
    std::vector<double> generateKnotVector() const;

    std::vector<std::pair<double, double>> controlPoints_; ///< Control points defining the spline
    std::vector<double> weights_;                         ///< Weights for rational B-spline (NURBS)
    std::vector<double> customKnots_;                     ///< Custom knot vector for non-uniform B-spline
    unsigned int degree_;                                 ///< Degree of the B-spline
    Graphics::Color color_;                               ///< Color for rendering
    bool useCustomKnots_;                                 ///< Flag indicating use of custom knot vector
    bool isRational_;                                     ///< Flag indicating if curve is rational (NURBS)
    
    // Cache for performance optimization
    mutable std::vector<double> knotVectorCache_;        ///< Cached knot vector
    mutable bool knotVectorDirty_;                       ///< Flag indicating if knot vector needs regeneration
    
    /**
     * @brief Ensure the knot vector cache is up to date
     */
    void updateKnotVectorCache() const;
    
    /**
     * @brief Validate control points for the current degree
     * @throw std::invalid_argument if insufficient control points
     */
    void validateControlPoints() const;
};

} // namespace Sketching
} // namespace RebelCAD
