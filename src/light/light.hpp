//! Light interface — provides direct-lighting samples toward a shading point

#pragma once

#include "math/point3.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

namespace nanopt {

struct LightSample {
    Vec3 wi;
    float distance = 0.0f;
    Spectrum radiance;
    float pdf = 0.0f;
};

class Light {
public:
    virtual ~Light() = default;

    /// Sample a direction toward the light from the shading point, returning radiance evaluated at the supplied wavelengths.
    /// u supplies the 2D random sample needed by area-style lights; delta lights ignore it.
    virtual LightSample sample(Point3 from, Sample2D u, const SampledWavelengths& lambdas) const = 0;

    /// Solid-angle pdf that sample() would assign to wi from `from`. Default 0 — correct for delta lights
    /// (probability zero of an arbitrary wi matching the delta direction). Non-delta lights override.
    virtual float pdfLi(Point3 from, Vec3 wi, const SampledWavelengths& lambdas) const {
        (void)from; (void)wi; (void)lambdas;
        return 0.0f;
    }

    /// True for delta-distribution lights (point, directional). Integrators skip pdf division for these.
    virtual bool isDelta() const { return false; }
};

}  // namespace nanopt
