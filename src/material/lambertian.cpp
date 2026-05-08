//! Lambertian BSDF implementation

#include "material/lambertian.hpp"

#include <numbers>

#include "math/sampling.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/rgb_upsample.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

LambertianBsdf::LambertianBsdf(RgbSpectrum albedoRgb) : albedoRgb_(albedoRgb) {}

Spectrum LambertianBsdf::eval(Vec3 /*wi*/, Vec3 /*wo*/, const Frame& /*frame*/, const SampledWavelengths& lambdas) const {
    constexpr float kInvPi = std::numbers::inv_pi_v<float>;
    return sampledReflectanceFromRgb(albedoRgb_, lambdas) * kInvPi;
}

BsdfSample LambertianBsdf::sample(Vec3 /*wo*/, const Frame& frame, Sampler& sampler, const SampledWavelengths& lambdas) const {
    const Vec3 local = cosineHemisphereLocal(sampler.get2D());
    BsdfSample s;
    s.wi = frame.toWorld(local);
    s.pdf = cosineHemispherePdf(local.z);
    s.value = sampledReflectanceFromRgb(albedoRgb_, lambdas) * std::numbers::inv_pi_v<float>;
    return s;
}

float LambertianBsdf::pdf(Vec3 wiWorld, Vec3 /*wo*/, const Frame& frame,
                          const SampledWavelengths& /*lambdas*/) const {
    const Vec3 wiLocal = frame.toLocal(wiWorld);
    if (wiLocal.z <= 0.0f) return 0.0f;
    return cosineHemispherePdf(wiLocal.z);
}

}  // namespace nanopt
