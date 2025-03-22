/**
 * @file ParametricFeature.cpp
 * @brief Implementation of the ParametricFeature class
 */

#include "modeling/parametric/ParametricFeature.h"

namespace rebel_cad {
namespace modeling {

ParametricFeature::ParametricFeature(const FeatureId& id, const std::string& name)
    : m_id(id)
    , m_name(name)
    , m_status(FeatureStatus::Invalid)
{
    // Initialize the feature with an invalid status
    // It will be updated when the feature is first computed
}

} // namespace modeling
} // namespace rebel_cad
