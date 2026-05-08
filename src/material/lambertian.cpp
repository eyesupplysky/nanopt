//! Lambertian BSDF implementation

#include "material/lambertian.hpp"

#include <numbers>

namespace nanopt {

LambertianBsdf::LambertianBsdf(Spectrum albedo) : albedo_(albedo) {}

Spectrum LambertianBsdf::eval(Vec3 /*wi*/, Vec3 /*wo*/, Normal3 /*n*/) const {
    constexpr float kInvPi = 1.0f / std::numbers::pi_v<float>;
    return albedo_ * kInvPi;
}

}  // namespace nanopt
