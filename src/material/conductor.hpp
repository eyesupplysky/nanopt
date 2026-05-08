//! Rough-conductor BSDF — GGX microfacet with measured complex IOR

#pragma once

#include "material/bsdf.hpp"
#include "material/conductor_data.hpp"

namespace nanopt {

class ConductorBsdf : public Bsdf {
public:
    ConductorBsdf(MetalKind metal, float roughness);

    Spectrum eval(Vec3 wi, Vec3 wo, const Frame& frame, const SampledWavelengths& lambdas) const override;
    BsdfSample sample(Vec3 wo, const Frame& frame, Sampler& sampler, const SampledWavelengths& lambdas) const override;

private:
    MetalKind metal_;
    float alpha_;
};

}  // namespace nanopt
