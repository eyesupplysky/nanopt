//! Rough-conductor BSDF implementation

#include "material/conductor.hpp"

#include <algorithm>
#include <cmath>

#include "material/fresnel.hpp"
#include "material/microfacet.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

namespace {

constexpr float kAlphaMin = 1e-3f;

}  // namespace

ConductorBsdf::ConductorBsdf(MetalKind metal, float roughness)
    : metal_(metal), alpha_(std::max(kAlphaMin, roughness)) {}

Spectrum ConductorBsdf::eval(Vec3 wiWorld, Vec3 woWorld, const Frame& frame,
                             const SampledWavelengths& lambdas) const {
    const Vec3 wi = frame.toLocal(wiWorld);
    const Vec3 wo = frame.toLocal(woWorld);
    if (wi.z <= 0.0f || wo.z <= 0.0f) {
        return Spectrum{0.0f};
    }

    Vec3 h = wi + wo;
    if (h.x == 0.0f && h.y == 0.0f && h.z == 0.0f) {
        return Spectrum{0.0f};
    }
    h = normalize(h);
    if (h.z <= 0.0f) {
        return Spectrum{0.0f};
    }

    const float D = ggxD(h, alpha_);
    const float G2 = ggxG2(wi, wo, alpha_);
    const float wiDotH = std::max(0.0f, dot(wi, h));

    const ConductorIor ior = evalConductorIor(metal_, lambdas);
    Spectrum F;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        F.values[i] = fresnelConductor(wiDotH, ior.eta.values[i], ior.k.values[i]);
    }

    const float invDenom = 1.0f / (4.0f * wi.z * wo.z);
    return F * (D * G2 * invDenom);
}

BsdfSample ConductorBsdf::sample(Vec3 woWorld, const Frame& frame, Sampler& sampler,
                                 const SampledWavelengths& lambdas) const {
    const Vec3 wo = frame.toLocal(woWorld);
    if (wo.z <= 0.0f) {
        return BsdfSample{};
    }

    const Sample2D u = sampler.get2D();
    const Vec3 h = sampleGgxVndf(wo, alpha_, u);
    const float woDotH = dot(wo, h);
    if (woDotH <= 0.0f) {
        return BsdfSample{};
    }

    // Reflection of -wo across h
    const Vec3 wi = h * (2.0f * woDotH) - wo;
    if (wi.z <= 0.0f) {
        return BsdfSample{};
    }

    const float D = ggxD(h, alpha_);
    const float G1wo = ggxG1(wo, alpha_);
    const float G2 = ggxG2(wi, wo, alpha_);

    const ConductorIor ior = evalConductorIor(metal_, lambdas);
    Spectrum F;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        F.values[i] = fresnelConductor(woDotH, ior.eta.values[i], ior.k.values[i]);
    }

    const float invDenom = 1.0f / (4.0f * wi.z * wo.z);

    BsdfSample s;
    s.wi = frame.toWorld(wi);
    s.value = F * (D * G2 * invDenom);
    s.pdf = G1wo * D / (4.0f * wo.z);
    return s;
}

}  // namespace nanopt
