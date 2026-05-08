//! Smooth-dielectric BSDF — perfect specular reflect/refract with wavelength-dependent IOR

#pragma once

#include "material/bsdf.hpp"
#include "material/sellmeier.hpp"

namespace nanopt {

class DielectricBsdf : public Bsdf {
public:
    explicit DielectricBsdf(SellmeierGlass glass);

    Spectrum eval(Vec3 wi, Vec3 wo, const Frame& frame, const SampledWavelengths& lambdas) const override;
    BsdfSample sample(Vec3 wo, const Frame& frame, Sampler& sampler, const SampledWavelengths& lambdas) const override;
    bool isDelta() const override { return true; }

private:
    SellmeierGlass glass_;
};

}  // namespace nanopt
