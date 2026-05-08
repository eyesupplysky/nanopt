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
#include "scene/scene.hpp"

namespace nanopt {

DirectIntegrator::DirectIntegrator(Spectrum background) : background_(background) {}

Spectrum DirectIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& /*sampler*/) const {
    const std::optional<Hit> hit = scene.world().intersect(ray);
    if (!hit.has_value()) {
        return background_;
    }

    Spectrum total{0.0f};
    constexpr float kShadowEpsilon = 1e-4f;

    for (const auto& light : scene.lights()) {
        const LightSample sample = light->sample(hit->position);

        const Point3 shadowOrigin = hit->position + sample.wi * kShadowEpsilon;
        const Ray shadow{shadowOrigin, sample.wi, sample.distance - kShadowEpsilon};
        if (scene.world().intersect(shadow).has_value()) {
            continue;
        }

        const float cosTheta = std::max(0.0f, dot(sample.wi, hit->normal));
        if (cosTheta <= 0.0f) {
            continue;
        }

        const Spectrum f = hit->bsdf->eval(sample.wi, -ray.direction, hit->normal);
        total += f * sample.radiance * cosTheta;
    }

    return total;
}

}  // namespace nanopt
