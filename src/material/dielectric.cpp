//! Smooth-dielectric BSDF implementation

#include "material/dielectric.hpp"

#include <algorithm>
#include <cmath>

#include "material/fresnel.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

DielectricBsdf::DielectricBsdf(SellmeierGlass glass) : glass_(glass) {}

Spectrum DielectricBsdf::eval(Vec3 /*wi*/, Vec3 /*wo*/, const Frame& /*frame*/,
                              const SampledWavelengths& /*lambdas*/) const {
    // Delta BSDF: zero against any direction not aligned to the (single) sampled lobe.
    return Spectrum{0.0f};
}

BsdfSample DielectricBsdf::sample(Vec3 woWorld, const Frame& frame, Sampler& sampler,
                                  const SampledWavelengths& lambdas) const {
    const Vec3 woLocal = frame.toLocal(woWorld);
    const float cosThetaO = woLocal.z;
    if (cosThetaO == 0.0f) {
        return BsdfSample{};
    }
    const bool entering = cosThetaO > 0.0f;

    // Hero IOR drives the refraction direction and the RR-pick Fresnel.
    const float etaHeroGlass = iorAtNm(glass_, lambdas.lambda[0]);
    const float etaIHero = entering ? 1.0f : etaHeroGlass;
    const float etaTHero = entering ? etaHeroGlass : 1.0f;
    const float etaRel = etaIHero / etaTHero;

    const float cosThetaI = std::abs(cosThetaO);
    const float sin2I = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
    const float sin2THero = etaRel * etaRel * sin2I;

    const float fHero = (sin2THero >= 1.0f)
                           ? 1.0f
                           : fresnelDielectric(cosThetaI, etaIHero, etaTHero);

    const float u = sampler.get1D();
    if (u < fHero) {
        // Reflection branch — direction is wavelength-independent
        const Vec3 wiLocal{-woLocal.x, -woLocal.y, woLocal.z};

        Spectrum F;
        for (int i = 0; i < kSpectrumSamples; ++i) {
            if (lambdas.pdf[i] == 0.0f) {
                F.values[i] = 0.0f;
                continue;
            }
            const float nGlass = iorAtNm(glass_, lambdas.lambda[i]);
            const float etaIi = entering ? 1.0f : nGlass;
            const float etaTi = entering ? nGlass : 1.0f;
            F.values[i] = fresnelDielectric(cosThetaI, etaIi, etaTi);
        }

        BsdfSample s;
        s.wi = frame.toWorld(wiLocal);
        s.value = F * (1.0f / cosThetaI);  // F / |cosθ_wi| (== cosθ_o for reflection)
        s.pdf = fHero;
        s.dispersive = false;
        return s;
    }

    // Refraction branch — wavelength-dependent direction; use hero IOR only
    if (sin2THero >= 1.0f) {
        // TIR at hero — RR shouldn't have selected refraction (fHero would be 1.0). Defensive.
        return BsdfSample{};
    }
    const float cosThetaT = std::sqrt(1.0f - sin2THero);
    const Vec3 wiLocal = entering
                            ? Vec3{-etaRel * woLocal.x, -etaRel * woLocal.y, -cosThetaT}
                            : Vec3{-etaRel * woLocal.x, -etaRel * woLocal.y, cosThetaT};

    Spectrum value{};  // zero-initialised
    // Backward-tracing radiance scaling for refractive interfaces: (etaI/etaT)²
    const float etaScale = (etaIHero * etaIHero) / (etaTHero * etaTHero);
    value.values[0] = (1.0f - fHero) * etaScale / cosThetaT;

    BsdfSample s;
    s.wi = frame.toWorld(wiLocal);
    s.value = value;
    s.pdf = 1.0f - fHero;
    s.dispersive = true;
    return s;
}

}  // namespace nanopt
