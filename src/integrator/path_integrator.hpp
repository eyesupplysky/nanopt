//! Path-tracing integrator — Russian-roulette-terminated multi-bounce GI with next-event estimation

#pragma once

#include "integrator/integrator.hpp"
#include "math/ray.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class Scene;
class Sampler;

class PathIntegrator : public Integrator {
public:
    explicit PathIntegrator(int maxDepth = 8, int rrStartDepth = 3,
                            Spectrum background = Spectrum{0.0f});

    Spectrum Li(const Ray& ray, const Scene& scene, Sampler& sampler) const override;

private:
    int maxDepth_;
    int rrStartDepth_;
    Spectrum background_;
};

}  // namespace nanopt
