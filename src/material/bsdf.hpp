//! BSDF interface — bidirectional scattering distribution function

#pragma once

#include "math/frame.hpp"
#include "math/vec3.hpp"
#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

namespace nanopt {

class Sampler;

struct BsdfSample {
    Vec3 wi;
    Spectrum value;
    float pdf = 0.0f;
    bool dispersive = false;
};

class Bsdf {
public:
    virtual ~Bsdf() = default;

    /// Evaluate the BSDF for incoming wi and outgoing wo at a surface with shading frame, returning radiance at the supplied wavelengths
    virtual Spectrum eval(Vec3 wi, Vec3 wo, const Frame& frame, const SampledWavelengths& lambdas) const = 0;

    /// Sample an incoming direction wi given the outgoing direction wo at a surface with shading frame
    virtual BsdfSample sample(Vec3 wo, const Frame& frame, Sampler& sampler, const SampledWavelengths& lambdas) const = 0;

    /// Solid-angle pdf that sample() would assign to this wi given wo at the surface.
    /// Default returns 0 — the correct value for delta lobes (probability zero of matching an arbitrary wi).
    /// Non-delta BSDFs override and must return a value consistent with what sample() reports for the same wi.
    virtual float pdf(Vec3 wi, Vec3 wo, const Frame& frame, const SampledWavelengths& lambdas) const {
        (void)wi; (void)wo; (void)frame; (void)lambdas;
        return 0.0f;
    }

    /// True for delta (perfect-specular) BSDFs — integrators skip next-event estimation against these surfaces
    /// because the chance of a light-sampled direction matching the delta lobe is zero
    virtual bool isDelta() const { return false; }
};

}  // namespace nanopt
