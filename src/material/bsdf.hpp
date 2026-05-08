//! BSDF interface — bidirectional scattering distribution function

#pragma once

#include "math/normal3.hpp"
#include "math/vec3.hpp"
#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

namespace nanopt {

class Sampler;

struct BsdfSample {
    Vec3 wi;
    Spectrum value;
    float pdf = 0.0f;
};

class Bsdf {
public:
    virtual ~Bsdf() = default;

    /// Evaluate the BSDF for incoming wi and outgoing wo at a surface with normal n, returning radiance at the supplied wavelengths
    virtual Spectrum eval(Vec3 wi, Vec3 wo, Normal3 n, const SampledWavelengths& lambdas) const = 0;

    /// Sample an incoming direction wi given the outgoing direction wo at a surface with normal n
    virtual BsdfSample sample(Vec3 wo, Normal3 n, Sampler& sampler, const SampledWavelengths& lambdas) const = 0;
};

}  // namespace nanopt
