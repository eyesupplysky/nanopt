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
#include "math/sampling.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/rgb_upsample.hpp"

namespace nanopt {

namespace {

constexpr float kShadowEpsilon = 1e-4f;
constexpr float kSpawnEpsilon = 1e-4f;

// Light-strategy half of the two-strategy direct-lighting MIS estimator. The matching BSDF strategy
// is the path tracer's bounce ray itself: when the continuation hits an emitter, PathIntegrator
// adds that contribution with the complementary MIS weight (Phase C). For delta lights, no BSDF
// strategy can match the delta direction, so weight is 1 here.
Spectrum estimateDirectLighting(const Scene& scene, const Hit& hit, Vec3 wo, Sampler& sampler,
                                const SampledWavelengths& lambdas) {
    if (hit.bsdf->isDelta()) {
        return Spectrum{0.0f};
    }
    Spectrum total{0.0f};
    for (const auto& light : scene.lights()) {
        const LightSample ls = light->sample(hit.position, sampler.get2D(), lambdas);
        if (ls.pdf <= 0.0f || ls.radiance.isBlack()) {
            continue;
        }
        const float cosTheta = std::max(0.0f, dot(ls.wi, hit.frame.n));
        if (cosTheta == 0.0f) {
            continue;
        }
        const Point3 shadowOrigin = hit.position + ls.wi * kShadowEpsilon;
        const Ray shadow{shadowOrigin, ls.wi, ls.distance - kShadowEpsilon};
        if (scene.world().intersect(shadow).has_value()) {
            continue;
        }
        const Spectrum f = hit.bsdf->eval(ls.wi, wo, hit.frame, lambdas);
        const Spectrum contribution = f * ls.radiance * cosTheta;
        if (light->isDelta()) {
            total += contribution;
            continue;
        }
        const float bsdfPdf = hit.bsdf->pdf(ls.wi, wo, hit.frame, lambdas);
        const float weight = powerHeuristic(ls.pdf, bsdfPdf);
        total += contribution * (weight / ls.pdf);
    }
    return total;
}

}  // namespace

PathIntegrator::PathIntegrator(int maxDepth, int rrStartDepth, RgbSpectrum backgroundRgb)
    : maxDepth_(maxDepth), rrStartDepth_(rrStartDepth), backgroundRgb_(backgroundRgb) {}

Spectrum PathIntegrator::Li(const Ray& primary, const Scene& scene, Sampler& sampler,
                            const SampledWavelengths& lambdasIn) const {
    // Working copy: a dispersive BSDF sample collapses the path to its hero wavelength via
    // SampledWavelengths::terminateSecondary, which mutates the carried path state.
    SampledWavelengths lambdas = lambdasIn;
    Spectrum L{0.0f};
    Spectrum throughput{1.0f};
    Ray ray = primary;

    // MIS bookkeeping for indirect emitter hits (BSDF strategy of the direct-lighting MIS).
    // Camera ray has no preceding NEE strategy, so the first emitter hit is unweighted.
    bool prevBounceSpecular = true;
    float prevBsdfPdf = 0.0f;
    Point3 prevHitPos{};

    for (int depth = 0; depth < maxDepth_; ++depth) {
        const std::optional<Hit> hit = scene.world().intersect(ray);
        if (!hit.has_value()) {
            L += throughput * sampledIlluminantFromRgb(backgroundRgb_, lambdas);
            break;
        }

        const Vec3 wo = -ray.direction;

        if (hit->areaLight != nullptr) {
            // BSDF strategy of the direct-lighting MIS: previous bounce sampled this direction
            // from the BSDF, the ray landed on an emitter, contribute Le with the MIS weight
            // against the matching light-strategy pdf.
            if (dot(wo, hit->geometricNormal) > 0.0f) {
                const Spectrum Le = hit->areaLight->emit(lambdas);
                if (prevBounceSpecular) {
                    L += throughput * Le;
                } else {
                    const float lightPdf = hit->areaLight->pdfLi(prevHitPos, ray.direction, lambdas);
                    const float weight = powerHeuristic(prevBsdfPdf, lightPdf);
                    L += throughput * Le * weight;
                }
            }
            break;
        }

        L += throughput * estimateDirectLighting(scene, *hit, wo, sampler, lambdas);

        const BsdfSample bs = hit->bsdf->sample(wo, hit->frame, sampler, lambdas);
        if (bs.pdf <= 0.0f || bs.value.isBlack()) {
            break;
        }
        const float cosThetaWi = dot(bs.wi, hit->frame.n);
        if (cosThetaWi == 0.0f) {
            break;
        }
        throughput = throughput * bs.value * (std::fabs(cosThetaWi) / bs.pdf);

        if (bs.dispersive) {
            lambdas.terminateSecondary();
        }

        // Stash MIS state for the next iteration's emitter-hit check.
        // Delta and dispersive samples have no light-strategy competitor — weight is 1.
        prevBounceSpecular = hit->bsdf->isDelta() || bs.dispersive;
        prevBsdfPdf = bs.pdf;
        prevHitPos = hit->position;

        if (depth >= rrStartDepth_) {
            const float q = std::min(0.95f, throughput.maxComponent());
            if (q <= 0.0f || sampler.get1D() > q) {
                break;
            }
            throughput = throughput / q;
        }

        // Spawn the next ray off the geometric face, not the shading frame, so smooth-shaded
        // meshes don't push the spawn into their own geometry.
        const float geomCosThetaWi = dot(bs.wi, hit->geometricNormal);
        const float spawnSign = geomCosThetaWi >= 0.0f ? 1.0f : -1.0f;
        const Point3 spawnOrigin = hit->position + hit->geometricNormal * (kSpawnEpsilon * spawnSign);
        ray = Ray{spawnOrigin, bs.wi};
    }
    return L;
}

}  // namespace nanopt
