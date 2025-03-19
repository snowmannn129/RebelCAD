#include "sketching/Spline.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace RebelCAD {
namespace Sketching {

Spline::Spline()
    : degree_(3)  // Default to cubic spline
    , useCustomKnots_(false)
    , isRational_(false)
    , knotVectorDirty_(true)
    , color_(Graphics::Color(0.0, 0.0, 1.0))  // Default to blue
{
}

Spline::Spline(const std::vector<std::pair<double, double>>& controlPoints)
    : Spline()  // Call default constructor first
{
    controlPoints_ = controlPoints;
    validateControlPoints();
}

void Spline::addControlPoint(double x, double y) {
    controlPoints_.push_back(std::make_pair(x, y));
    knotVectorDirty_ = true;
}

bool Spline::removeControlPoint(size_t index) {
    if (index >= controlPoints_.size()) {
        return false;
    }
    
    controlPoints_.erase(controlPoints_.begin() + index);
    if (isRational_ && !weights_.empty()) {
        weights_.erase(weights_.begin() + index);
    }
    knotVectorDirty_ = true;
    return true;
}

bool Spline::moveControlPoint(size_t index, double newX, double newY) {
    if (index >= controlPoints_.size()) {
        return false;
    }
    
    controlPoints_[index] = std::make_pair(newX, newY);
    return true;
}

const std::vector<std::pair<double, double>>& Spline::getControlPoints() const {
    return controlPoints_;
}

void Spline::setDegree(unsigned int degree) {
    if (degree < 1) {
        throw std::invalid_argument("Spline degree must be at least 1");
    }
    degree_ = degree;
    knotVectorDirty_ = true;
    validateControlPoints();
}

unsigned int Spline::getDegree() const {
    return degree_;
}

void Spline::setColor(const Graphics::Color& color) {
    color_ = color;
}

const Graphics::Color& Spline::getColor() const {
    return color_;
}

void Spline::setKnotVector(const std::vector<double>& knots) {
    // Validate knot vector
    if (knots.size() < controlPoints_.size() + degree_ + 1) {
        throw std::invalid_argument("Invalid knot vector size");
    }
    
    // Check knot vector is non-decreasing
    for (size_t i = 1; i < knots.size(); ++i) {
        if (knots[i] < knots[i-1]) {
            throw std::invalid_argument("Knot vector must be non-decreasing");
        }
    }
    
    customKnots_ = knots;
    useCustomKnots_ = true;
    knotVectorDirty_ = false;
    knotVectorCache_ = knots;
}

std::vector<double> Spline::getKnotVector() const {
    updateKnotVectorCache();
    return useCustomKnots_ ? customKnots_ : knotVectorCache_;
}

void Spline::setWeights(const std::vector<double>& weights) {
    if (weights.size() != controlPoints_.size()) {
        throw std::invalid_argument("Weights size must match number of control points");
    }
    
    // Validate all weights are positive
    if (std::any_of(weights.begin(), weights.end(), 
                    [](double w) { return w <= 0.0; })) {
        throw std::invalid_argument("All weights must be positive");
    }
    
    weights_ = weights;
    isRational_ = true;
}

std::vector<double> Spline::getWeights() const {
    return weights_;
}

void Spline::convertToClamped() {
    if (controlPoints_.empty()) return;
    
    std::vector<double> clampedKnots;
    clampedKnots.reserve(controlPoints_.size() + degree_ + 1);
    
    // Add degree+1 knots at start
    for (unsigned int i = 0; i <= degree_; ++i) {
        clampedKnots.push_back(0.0);
    }
    
    // Add internal knots
    size_t numInternalKnots = controlPoints_.size() - degree_ - 1;
    for (size_t i = 1; i <= numInternalKnots; ++i) {
        clampedKnots.push_back(static_cast<double>(i) / (numInternalKnots + 1));
    }
    
    // Add degree+1 knots at end
    for (unsigned int i = 0; i <= degree_; ++i) {
        clampedKnots.push_back(1.0);
    }
    
    setKnotVector(clampedKnots);
}

bool Spline::isClamped() const {
    const auto& knots = getKnotVector();
    if (knots.size() < 2 * (degree_ + 1)) return false;
    
    // Check start clamp
    for (unsigned int i = 1; i <= degree_; ++i) {
        if (knots[i] != knots[0]) return false;
    }
    
    // Check end clamp
    size_t last = knots.size() - 1;
    for (unsigned int i = 1; i <= degree_; ++i) {
        if (knots[last - i] != knots[last]) return false;
    }
    
    return true;
}

double Spline::basisFunction(int i, int degree, double u, const std::vector<double>& knots) const {
    if (degree == 0) {
        return (u >= knots[i] && u < knots[i + 1]) ? 1.0 : 0.0;
    }
    
    double denom1 = knots[i + degree] - knots[i];
    double denom2 = knots[i + degree + 1] - knots[i + 1];
    
    double c1 = (denom1 > 1e-10) ? 
                ((u - knots[i]) / denom1 * basisFunction(i, degree - 1, u, knots)) : 0.0;
    
    double c2 = (denom2 > 1e-10) ? 
                ((knots[i + degree + 1] - u) / denom2 * basisFunction(i + 1, degree - 1, u, knots)) : 0.0;
    
    return c1 + c2;
}

std::pair<double, double> Spline::evaluatePoint(double u) const {
    if (controlPoints_.empty()) {
        throw std::runtime_error("Cannot evaluate empty spline");
    }
    
    validateControlPoints();
    updateKnotVectorCache();
    
    const auto& knots = useCustomKnots_ ? customKnots_ : knotVectorCache_;
    
    // Handle endpoint cases
    if (u <= knots.front()) return controlPoints_.front();
    if (u >= knots.back()) return controlPoints_.back();
    
    double x = 0.0, y = 0.0;
    double denominator = 0.0;
    
    for (size_t i = 0; i < controlPoints_.size(); ++i) {
        double basis = basisFunction(i, degree_, u, knots);
        double weight = isRational_ ? weights_[i] : 1.0;
        
        x += controlPoints_[i].first * basis * weight;
        y += controlPoints_[i].second * basis * weight;
        denominator += basis * weight;
    }
    
    if (isRational_) {
        x /= denominator;
        y /= denominator;
    }
    
    return std::make_pair(x, y);
}

std::vector<std::pair<double, double>> Spline::calculateCurvePoints(
    size_t minPoints, size_t maxPoints, double curvatureTolerance) const {
    if (controlPoints_.empty()) {
        return std::vector<std::pair<double, double>>();
    }
    
    validateControlPoints();
    updateKnotVectorCache();
    
    std::vector<std::pair<double, double>> points;
    points.reserve(minPoints);
    
    // Start with uniform sampling
    for (size_t i = 0; i <= minPoints; ++i) {
        double u = static_cast<double>(i) / minPoints;
        points.push_back(evaluatePoint(u));
    }
    
    // Adaptive refinement based on curvature
    size_t currentSize = points.size();
    while (currentSize < maxPoints) {
        bool refined = false;
        
        for (size_t i = 0; i < points.size() - 1; ++i) {
            double u = static_cast<double>(i) / (points.size() - 1);
            double curvature = std::abs(calculateCurvature(u));
            
            if (curvature > curvatureTolerance) {
                double uMid = (i + 0.5) / (points.size() - 1);
                points.insert(points.begin() + i + 1, evaluatePoint(uMid));
                refined = true;
            }
        }
        
        if (!refined) break;
        currentSize = points.size();
    }
    
    return points;
}

double Spline::calculateCurvature(double u) const {
    auto d1 = evaluateFirstDerivative(u);
    auto d2 = evaluateSecondDerivative(u);
    
    double num = d1.first * d2.second - d1.second * d2.first;
    double den = std::pow(d1.first * d1.first + d1.second * d1.second, 1.5);
    
    return std::abs(den) < 1e-10 ? 0.0 : num / den;
}

std::pair<double, double> Spline::evaluateFirstDerivative(double u) const {
    validateControlPoints();
    updateKnotVectorCache();
    
    const auto& knots = useCustomKnots_ ? customKnots_ : knotVectorCache_;
    
    double x = 0.0, y = 0.0;
    double denominator = 0.0;
    
    for (size_t i = 0; i < controlPoints_.size() - 1; ++i) {
        double coef = degree_ / (knots[i + degree_ + 1] - knots[i + 1]);
        double basis = basisFunction(i, degree_ - 1, u, knots);
        double weight = isRational_ ? weights_[i] : 1.0;
        
        x += coef * (controlPoints_[i + 1].first - controlPoints_[i].first) * basis * weight;
        y += coef * (controlPoints_[i + 1].second - controlPoints_[i].second) * basis * weight;
        denominator += basis * weight;
    }
    
    if (isRational_) {
        x /= denominator;
        y /= denominator;
    }
    
    return std::make_pair(x, y);
}

std::pair<double, double> Spline::evaluateSecondDerivative(double u) const {
    validateControlPoints();
    updateKnotVectorCache();
    
    const auto& knots = useCustomKnots_ ? customKnots_ : knotVectorCache_;
    
    double x = 0.0, y = 0.0;
    double denominator = 0.0;
    
    for (size_t i = 0; i < controlPoints_.size() - 2; ++i) {
        double coef = degree_ * (degree_ - 1) / 
                     ((knots[i + degree_ + 1] - knots[i + 1]) * 
                      (knots[i + degree_] - knots[i]));
        double basis = basisFunction(i, degree_ - 2, u, knots);
        double weight = isRational_ ? weights_[i] : 1.0;
        
        x += coef * (controlPoints_[i + 2].first - 2 * controlPoints_[i + 1].first + 
                     controlPoints_[i].first) * basis * weight;
        y += coef * (controlPoints_[i + 2].second - 2 * controlPoints_[i + 1].second + 
                     controlPoints_[i].second) * basis * weight;
        denominator += basis * weight;
    }
    
    if (isRational_) {
        x /= denominator;
        y /= denominator;
    }
    
    return std::make_pair(x, y);
}

void Spline::insertKnot(double u, unsigned int r) {
    if (u < 0.0 || u > 1.0) {
        throw std::invalid_argument("Knot value must be between 0 and 1");
    }
    
    updateKnotVectorCache();
    const auto& knots = useCustomKnots_ ? customKnots_ : knotVectorCache_;
    
    // Find knot span
    size_t k = 0;
    while (k < knots.size() - 1 && knots[k + 1] <= u) ++k;
    
    // Create new control points and knots
    std::vector<std::pair<double, double>> newControlPoints;
    std::vector<double> newKnots;
    std::vector<double> newWeights;
    
    // TODO: Implement knot insertion algorithm
    // This requires careful handling of control points and weights
    // For now, throw not implemented
    throw std::runtime_error("Knot insertion not yet implemented");
}

unsigned int Spline::removeKnot(double u, unsigned int r, double tolerance) {
    if (u < 0.0 || u > 1.0) {
        throw std::invalid_argument("Knot value must be between 0 and 1");
    }
    
    // TODO: Implement knot removal algorithm
    // This requires careful handling of control points and weights
    // For now, throw not implemented
    throw std::runtime_error("Knot removal not yet implemented");
}

double Spline::calculateArcLength(double tolerance) const {
    return integrateGaussian(0.0, 1.0, tolerance);
}

double Spline::parameterAtLength(double distance, double tolerance) const {
    if (distance <= 0.0) return 0.0;
    
    double totalLength = calculateArcLength(tolerance);
    if (distance >= totalLength) return 1.0;
    
    // Binary search for parameter value
    double left = 0.0, right = 1.0;
    while (right - left > tolerance) {
        double mid = (left + right) / 2.0;
        double length = integrateGaussian(0.0, mid, tolerance);
        
        if (std::abs(length - distance) < tolerance) {
            return mid;
        }
        
        if (length < distance) {
            left = mid;
        } else {
            right = mid;
        }
    }
    
    return (left + right) / 2.0;
}

std::pair<double, double> Spline::pointAtLength(double distance, double tolerance) const {
    double u = parameterAtLength(distance, tolerance);
    return evaluatePoint(u);
}

std::pair<std::pair<double, double>, double> Spline::findClosestPoint(
    double x, double y, double tolerance, size_t maxIterations) const {
    
    // Initial guess using uniform sampling
    double minDist = std::numeric_limits<double>::max();
    double bestU = 0.0;
    
    const size_t numSamples = 50;
    for (size_t i = 0; i <= numSamples; ++i) {
        double u = static_cast<double>(i) / numSamples;
        auto point = evaluatePoint(u);
        double dist = std::pow(point.first - x, 2) + std::pow(point.second - y, 2);
        
        if (dist < minDist) {
            minDist = dist;
            bestU = u;
        }
    }
    
    // Newton iteration for refinement
    for (size_t iter = 0; iter < maxIterations; ++iter) {
        auto point = evaluatePoint(bestU);
        auto deriv = evaluateFirstDerivative(bestU);
        
        double f = (point.first - x) * deriv.first + (point.second - y) * deriv.second;
        auto deriv2 = evaluateSecondDerivative(bestU);
        double fPrime = deriv.first * deriv.first + deriv.second * deriv.second +
                       (point.first - x) * deriv2.first + (point.second - y) * deriv2.second;
        
        if (std::abs(fPrime) < tolerance) break;
        
        double delta = f / fPrime;
        double newU = bestU - delta;
        
        // Clamp to valid parameter range
        newU = std::max(0.0, std::min(1.0, newU));
        
        if (std::abs(newU - bestU) < tolerance) break;
        bestU = newU;
    }
    
    auto closestPoint = evaluatePoint(bestU);
    return std::make_pair(closestPoint, bestU);
}

double Spline::distanceToPoint(double x, double y, double u) const {
    auto point = evaluatePoint(u);
    double dx = point.first - x;
    double dy = point.second - y;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<Spline::ControlPointInfluence> Spline::calculateControlPointInfluences(size_t numSamples) const {
    try {
        validateControlPoints();
    } catch (const std::invalid_argument&) {
        return std::vector<ControlPointInfluence>();
    }

    updateKnotVectorCache();
    std::vector<ControlPointInfluence> influences;
    influences.reserve(controlPoints_.size());

    // Get knot vector (either custom or generated)
    const std::vector<double>& knots = useCustomKnots_ ? customKnots_ : knotVectorCache_;

    // For each control point
    for (size_t i = 0; i < controlPoints_.size(); ++i) {
        ControlPointInfluence influence;
        influence.startParam = 1.0;  // Will be reduced to actual start
        influence.endParam = 0.0;    // Will be increased to actual end
        influence.maxInfluence = 0.0;

        // Sample the basis function across the parameter range
        for (size_t j = 0; j <= numSamples; ++j) {
            double u = static_cast<double>(j) / numSamples;
            double basis = basisFunction(i, degree_, u, knots);

            // Update influence region
            if (basis > 1e-6) {  // Consider non-zero influence
                influence.startParam = std::min(influence.startParam, u);
                influence.endParam = std::max(influence.endParam, u);
                influence.maxInfluence = std::max(influence.maxInfluence, basis);
            }
        }

        influences.push_back(influence);
    }

    return influences;
}

std::pair<Spline, Spline> Spline::splitAtParameter(double u) const {
    if (u <= 0.0 || u >= 1.0) {
        throw std::invalid_argument("Split parameter must be between 0 and 1");
    }
    
    // TODO: Implement curve splitting algorithm
    // This requires careful handling of control points, weights, and knot vectors
    // For now, throw not implemented
    throw std::runtime_error("Curve splitting not yet implemented");
}

double Spline::integrateGaussian(double start, double end, double tolerance) const {
    // 5-point Gauss-Legendre quadrature
    static const double weights[] = {
        0.2369268850561891,
        0.4786286704993665,
        0.5688888888888889,
        0.4786286704993665,
        0.2369268850561891
    };
    
    static const double abscissae[] = {
        -0.9061798459386640,
        -0.5384693101056831,
        0.0000000000000000,
        0.5384693101056831,
        0.9061798459386640
    };
    
    double sum = 0.0;
    double halfLength = (end - start) / 2.0;
    double midPoint = (start + end) / 2.0;
    
    for (int i = 0; i < 5; ++i) {
        double u = midPoint + halfLength * abscissae[i];
        auto deriv = evaluateFirstDerivative(u);
        double speed = std::sqrt(deriv.first * deriv.first + deriv.second * deriv.second);
        sum += weights[i] * speed;
    }
    
    return halfLength * sum;
}

void Spline::updateKnotVectorCache() const {
    if (!knotVectorDirty_ && !knotVectorCache_.empty()) return;
    
    if (useCustomKnots_) {
        knotVectorCache_ = customKnots_;
    } else {
        knotVectorCache_ = generateKnotVector();
    }
    
    knotVectorDirty_ = false;
}

std::vector<double> Spline::generateKnotVector() const {
    size_t n = controlPoints_.size() - 1;  // n + 1 control points
    size_t m = n + degree_ + 1;            // m + 1 knots needed
    
    std::vector<double> knots;
    knots.reserve(m + 1);
    
    // Add degree + 1 knots at start
    for (unsigned int i = 0; i <= degree_; ++i) {
        knots.push_back(0.0);
    }
    
    // Add internal knots
    size_t numInternal = m - 2 * (degree_ + 1);
    for (size_t i = 1; i <= numInternal + 1; ++i) {
        knots.push_back(static_cast<double>(i) / (numInternal + 2));
    }
    
    // Add degree + 1 knots at end
    for (unsigned int i = 0; i <= degree_; ++i) {
        knots.push_back(1.0);
    }
    
    return knots;
}

void Spline::validateControlPoints() const {
    if (controlPoints_.size() < degree_ + 1) {
        throw std::invalid_argument(
            "Insufficient control points for degree " + std::to_string(degree_) +
            ". Need at least " + std::to_string(degree_ + 1) + " points.");
    }
}

} // namespace Sketching
} // namespace RebelCAD
