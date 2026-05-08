//! Lambertian BSDF implementation

#include "material/lambertian.hpp"

#include <numbers>

#include "math/sampling.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/rgb_upsample.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

LambertianBsdf::LambertianBsdf(RgbSpectrum albedoRgb) : albedoRgb_(albedoRgb) {}

Spectrum LambertianBsdf::eval(Vec3 /*wi*/, Vec3 /*wo*/, Normal3 /*n*/, const SampledWavelengths& lambdas) const {
    constexpr float kInvPi = std::numbers::inv_pi_v<float>;
    return sampledReflectanceFromRgb(albedoRgb_, lambdas) * kInvPi;
}

BsdfSample LambertianBsdf::sample(Vec3 /*wo*/, Normal3 n, Sampler& sampler, const SampledWavelengths& lambdas) const {
    Vec3 tangent;
    Vec3 bitangent;
    buildOrthonormalBasis(n, tangent, bitangent);
    const Vec3 local = cosineHemisphereLocal(sampler.get2D());
    BsdfSample s;
    s.wi = fromTangentSpace(local, tangent, bitangent, n);
    s.pdf = cosineHemispherePdf(local.z);
    s.value = sampledReflectanceFromRgb(albedoRgb_, lambdas) * std::numbers::inv_pi_v<float>;
    return s;
}

}  // namespace nanopt
