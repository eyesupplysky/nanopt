//! Integrator interface — computes incident radiance for a primary ray

#pragma once

#include "math/ray.hpp"
#include "spectrum/spectrum.hpp"
#include "spectrum/wavelength.hpp"

namespace nanopt {

class Scene;
class Sampler;

class Integrator {
public:
    virtual ~Integrator() = default;

    /// Compute incident radiance arriving along the given ray, sampled at the supplied wavelengths
    virtual Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler,
                        const SampledWavelengths& lambdas) const = 0;
};

}  // namespace nanopt
