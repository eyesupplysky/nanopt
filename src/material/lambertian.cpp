//! Lambertian BSDF implementation

#include "material/lambertian.hpp"

#include <numbers>

#include "math/sampling.hpp"
#include "sampler/sampler.hpp"

namespace nanopt {

LambertianBsdf::LambertianBsdf(Spectrum albedo) : albedo_(albedo) {}

Spectrum LambertianBsdf::eval(Vec3 /*wi*/, Vec3 /*wo*/, Normal3 /*n*/) const {
    constexpr float kInvPi = std::numbers::inv_pi_v<float>;
    return albedo_ * kInvPi;
}

BsdfSample LambertianBsdf::sample(Vec3 /*wo*/, Normal3 n, Sampler& sampler) const {
    Vec3 tangent;
    Vec3 bitangent;
    buildOrthonormalBasis(n, tangent, bitangent);
    const Vec3 local = cosineHemisphereLocal(sampler.get2D());
    BsdfSample s;
    s.wi = fromTangentSpace(local, tangent, bitangent, n);
    s.pdf = cosineHemispherePdf(local.z);
    s.value = albedo_ * std::numbers::inv_pi_v<float>;
    return s;
}

}  // namespace nanopt
