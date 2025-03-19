#include "modeling/NURBSBasis.h"
#include "modeling/NURBSBasisImpl.h"
#include <algorithm>

namespace RebelCAD {
namespace Modeling {

NURBSBasis::NURBSBasis() : impl(std::make_unique<Impl>()) {}
NURBSBasis::~NURBSBasis() = default;

bool NURBSBasis::Initialize(const std::vector<float>& knots, int degree) {
    if (knots.empty() || degree < 1) {
        return false;
    }

    // Validate knot vector size
    if (knots.size() < static_cast<size_t>(degree + 2)) {
        return false;  // Need at least degree + 2 knots
    }

    // Check for non-decreasing knot sequence
    if (!std::is_sorted(knots.begin(), knots.end())) {
        return false;
    }

    // Check for valid knot values
    if (std::any_of(knots.begin(), knots.end(),
        [](float k) { return !std::isfinite(k); })) {
        return false;
    }

    return impl->Initialize(knots, degree);
}

std::vector<float> NURBSBasis::Evaluate(float t, int span) const {
    if (!impl->is_initialized) {
        return {};
    }

    // Validate span index
    if (span < impl->degree || 
        span >= static_cast<int>(impl->knots.size() - impl->degree - 1)) {
        return {};
    }

    // Validate parameter value
    if (!std::isfinite(t)) {
        return {};
    }

    return impl->Evaluate(t, span);
}

std::vector<std::vector<float>> NURBSBasis::EvaluateDerivatives(
    float t, int span, int order) const {
    
    if (!impl->is_initialized) {
        return {};
    }

    // Validate span index
    if (span < impl->degree || 
        span >= static_cast<int>(impl->knots.size() - impl->degree - 1)) {
        return {};
    }

    // Validate parameter value and order
    if (!std::isfinite(t) || order < 0 || order > impl->degree) {
        return {};
    }

    return impl->EvaluateDerivatives(t, span, order);
}

int NURBSBasis::FindSpan(float t) const {
    if (!impl->is_initialized) {
        return -1;
    }

    if (!std::isfinite(t)) {
        return -1;
    }

    return impl->FindSpan(t);
}

int NURBSBasis::GetDegree() const {
    return impl->degree;
}

int NURBSBasis::GetCount() const {
    if (!impl->is_initialized) {
        return 0;
    }
    return impl->knots.size() - impl->degree - 1;
}

} // namespace Modeling
} // namespace RebelCAD
