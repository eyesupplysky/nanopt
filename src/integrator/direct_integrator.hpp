//! Direct-lighting integrator — single-bounce shading; kept as a baseline alongside PathIntegrator

#pragma once

#include "integrator/integrator.hpp"
#include "math/ray.hpp"
#include "spectrum/rgb_spectrum.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class Scene;
class Sampler;

class DirectIntegrator : public Integrator {
public:
    explicit DirectIntegrator(RgbSpectrum backgroundRgb = RgbSpectrum{});

    Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler,
                const SampledWavelengths& lambdas) const override;

private:
    RgbSpectrum backgroundRgb_;
};

}  // namespace nanopt
