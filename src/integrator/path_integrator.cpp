//! Path-tracing integrator implementation

#include "integrator/path_integrator.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

#include "geometry/hit.hpp"
#include "geometry/primitive.hpp"
#include "light/area_light.hpp"
#include "light/light.hpp"
#include "material/bsdf.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"

namespace nanopt {

namespace {

constexpr float kShadowEpsilon = 1e-4f;
constexpr float kSpawnEpsilon = 1e-4f;

Spectrum estimateDirectLighting(const Scene& scene, const Hit& hit, Vec3 wo, Sampler& sampler) {
    Spectrum total{0.0f};
    for (const auto& light : scene.lights()) {
        const LightSample ls = light->sample(hit.position, sampler.get2D());
        if (ls.pdf <= 0.0f || ls.radiance.isBlack()) {
            continue;
        }
        const float cosTheta = std::max(0.0f, dot(ls.wi, hit.normal));
        if (cosTheta == 0.0f) {
            continue;
        }
        const Point3 shadowOrigin = hit.position + ls.wi * kShadowEpsilon;
        const Ray shadow{shadowOrigin, ls.wi, ls.distance - kShadowEpsilon};
        if (scene.world().intersect(shadow).has_value()) {
            continue;
        }
        const Spectrum f = hit.bsdf->eval(ls.wi, wo, hit.normal);
        Spectrum contribution = f * ls.radiance * cosTheta;
        if (!light->isDelta()) {
            contribution = contribution / ls.pdf;
        }
        total += contribution;
    }
    return total;
}

}  // namespace

PathIntegrator::PathIntegrator(int maxDepth, int rrStartDepth, Spectrum background)
    : maxDepth_(maxDepth), rrStartDepth_(rrStartDepth), background_(background) {}

Spectrum PathIntegrator::Li(const Ray& primary, const Scene& scene, Sampler& sampler) const {
    Spectrum L{0.0f};
    Spectrum throughput{1.0f};
    Ray ray = primary;

    for (int depth = 0; depth < maxDepth_; ++depth) {
        const std::optional<Hit> hit = scene.world().intersect(ray);
        if (!hit.has_value()) {
            L += throughput * background_;
            break;
        }

        const Vec3 wo = -ray.direction;

        if (hit->areaLight != nullptr) {
            // Direct primary-ray hit on emitter: add emission and terminate.
            // NEE owns indirect emitter contributions, so we must drop bounce > 0.
            if (depth == 0 && dot(wo, hit->normal) > 0.0f) {
                L += throughput * hit->areaLight->emit();
            }
            break;
        }

        L += throughput * estimateDirectLighting(scene, *hit, wo, sampler);

        const BsdfSample bs = hit->bsdf->sample(wo, hit->normal, sampler);
        if (bs.pdf <= 0.0f || bs.value.isBlack()) {
            break;
        }
        const float cosThetaWi = dot(bs.wi, hit->normal);
        if (cosThetaWi == 0.0f) {
            break;
        }
        throughput = throughput * bs.value * (std::fabs(cosThetaWi) / bs.pdf);

        if (depth >= rrStartDepth_) {
            const float q = std::min(0.95f, throughput.maxComponent());
            if (q <= 0.0f || sampler.get1D() > q) {
                break;
            }
            throughput = throughput / q;
        }

        const float spawnSign = cosThetaWi > 0.0f ? 1.0f : -1.0f;
        const Point3 spawnOrigin = hit->position + hit->normal * (kSpawnEpsilon * spawnSign);
        ray = Ray{spawnOrigin, bs.wi};
    }
    return L;
}

}  // namespace nanopt
