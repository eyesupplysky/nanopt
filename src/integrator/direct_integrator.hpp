//! Direct-lighting integrator — single-bounce shading for M1

#pragma once

#include "integrator/integrator.hpp"
#include "math/ray.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class Scene;
class Sampler;

class DirectIntegrator : public Integrator {
public:
    explicit DirectIntegrator(Spectrum background = Spectrum{0.0f});

    Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) const override;

private:
    Spectrum background_;
};

}  // namespace nanopt
