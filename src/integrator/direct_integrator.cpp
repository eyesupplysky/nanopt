//! Direct-lighting integrator implementation

#include "integrator/direct_integrator.hpp"

#include <algorithm>
#include <optional>

#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "light/light.hpp"
#include "material/bsdf.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/rgb_upsample.hpp"

namespace nanopt {

DirectIntegrator::DirectIntegrator(RgbSpectrum backgroundRgb) : backgroundRgb_(backgroundRgb) {}

Spectrum DirectIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& sampler,
                              const SampledWavelengths& lambdas) const {
    const std::optional<Hit> hit = scene.world().intersect(ray);
    if (!hit.has_value()) {
        return sampledIlluminantFromRgb(backgroundRgb_, lambdas);
    }

    Spectrum total{0.0f};
    constexpr float kShadowEpsilon = 1e-4f;

    for (const auto& light : scene.lights()) {
        const LightSample ls = light->sample(hit->position, sampler.get2D(), lambdas);
        if (ls.pdf <= 0.0f || ls.radiance.isBlack()) {
            continue;
        }

        const Point3 shadowOrigin = hit->position + ls.wi * kShadowEpsilon;
        const Ray shadow{shadowOrigin, ls.wi, ls.distance - kShadowEpsilon};
        if (scene.world().intersect(shadow).has_value()) {
            continue;
        }

        const float cosTheta = std::max(0.0f, dot(ls.wi, hit->normal));
        if (cosTheta <= 0.0f) {
            continue;
        }

        const Spectrum f = hit->bsdf->eval(ls.wi, -ray.direction, hit->normal, lambdas);
        Spectrum contribution = f * ls.radiance * cosTheta;
        if (!light->isDelta()) {
            contribution = contribution / ls.pdf;
        }
        total += contribution;
    }

    return total;
}

}  // namespace nanopt
