//! Direct-lighting integrator implementation

#include "integrator/direct_integrator.hpp"

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
#include "math/sampling.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/rgb_upsample.hpp"

namespace nanopt {

namespace {

constexpr float kShadowEpsilon = 1e-4f;
constexpr float kSpawnEpsilon = 1e-4f;

}  // namespace

DirectIntegrator::DirectIntegrator(RgbSpectrum backgroundRgb) : backgroundRgb_(backgroundRgb) {}

Spectrum DirectIntegrator::Li(const Ray& ray, const Scene& scene, Sampler& sampler,
                              const SampledWavelengths& lambdas) const {
    const std::optional<Hit> hit = scene.world().intersect(ray);
    if (!hit.has_value()) {
        return sampledIlluminantFromRgb(backgroundRgb_, lambdas);
    }

    Spectrum total{0.0f};
    const Vec3 wo = -ray.direction;
    const bool deltaBsdf = hit->bsdf->isDelta();

    // Light strategy — NEE over each light, MIS-weighted against the BSDF strategy.
    // Skipped for delta BSDFs (eval is identically zero against a light-sampled direction).
    if (!deltaBsdf) {
        for (const auto& light : scene.lights()) {
            const LightSample ls = light->sample(hit->position, sampler.get2D(), lambdas);
            if (ls.pdf <= 0.0f || ls.radiance.isBlack()) {
                continue;
            }
            const float cosTheta = std::max(0.0f, dot(ls.wi, hit->frame.n));
            if (cosTheta <= 0.0f) {
                continue;
            }
            const Point3 shadowOrigin = hit->position + ls.wi * kShadowEpsilon;
            const Ray shadow{shadowOrigin, ls.wi, ls.distance - kShadowEpsilon};
            if (scene.world().intersect(shadow).has_value()) {
                continue;
            }
            const Spectrum f = hit->bsdf->eval(ls.wi, wo, hit->frame, lambdas);
            const Spectrum contribution = f * ls.radiance * cosTheta;
            if (light->isDelta()) {
                total += contribution;
                continue;
            }
            const float bsdfPdf = hit->bsdf->pdf(ls.wi, wo, hit->frame, lambdas);
            const float weight = powerHeuristic(ls.pdf, bsdfPdf);
            total += contribution * (weight / ls.pdf);
        }
    }

    // BSDF strategy — sample BSDF, intersect; if the ray lands on an emitter, contribute the
    // MIS-weighted radiance. Delta BSDFs use weight=1 (light strategy contributes zero).
    const BsdfSample bs = hit->bsdf->sample(wo, hit->frame, sampler, lambdas);
    if (bs.pdf > 0.0f && !bs.value.isBlack()) {
        const float geomCosThetaWi = dot(bs.wi, hit->geometricNormal);
        const float spawnSign = geomCosThetaWi >= 0.0f ? 1.0f : -1.0f;
        const Point3 spawnOrigin = hit->position + hit->geometricNormal * (kSpawnEpsilon * spawnSign);
        const Ray bsdfRay{spawnOrigin, bs.wi};
        const std::optional<Hit> bHit = scene.world().intersect(bsdfRay);
        if (bHit.has_value() && bHit->areaLight != nullptr) {
            const Spectrum Le = bHit->areaLight->emit(lambdas);
            const bool frontFace = dot(-bs.wi, bHit->geometricNormal) > 0.0f;
            if (frontFace && !Le.isBlack()) {
                if (deltaBsdf) {
                    // Delta BSDF: bs.value bakes the cosine; weight = 1.
                    total += bs.value * Le / bs.pdf;
                } else {
                    const float cosThetaWi = std::max(0.0f, dot(bs.wi, hit->frame.n));
                    const float lightPdf = bHit->areaLight->pdfLi(hit->position, bs.wi, lambdas);
                    const float weight = powerHeuristic(bs.pdf, lightPdf);
                    total += bs.value * Le * (cosThetaWi * weight / bs.pdf);
                }
            }
        }
    }

    return total;
}

}  // namespace nanopt
