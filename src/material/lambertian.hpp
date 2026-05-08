//! Lambertian (perfect diffuse) BSDF

#pragma once

#include "material/bsdf.hpp"
#include "spectrum/rgb_spectrum.hpp"

namespace nanopt {

class LambertianBsdf : public Bsdf {
public:
    explicit LambertianBsdf(RgbSpectrum albedoRgb);

    Spectrum eval(Vec3 wi, Vec3 wo, Normal3 n, const SampledWavelengths& lambdas) const override;
    BsdfSample sample(Vec3 wo, Normal3 n, Sampler& sampler, const SampledWavelengths& lambdas) const override;

private:
    RgbSpectrum albedoRgb_;
};

}  // namespace nanopt
