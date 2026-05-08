//! Diffuse area light implementation

#include "light/area_light.hpp"

#include <cmath>

#include "math/sampling.hpp"
#include "math/vec3.hpp"
#include "spectrum/rgb_upsample.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

AreaLight::AreaLight(Point3 v0, Point3 v1, Point3 v2, RgbSpectrum emissionRgb)
    : v0_(v0), v1_(v1), v2_(v2), emissionRgb_(emissionRgb) {
    const Vec3 e1 = v1_ - v0_;
    const Vec3 e2 = v2_ - v0_;
    const Vec3 cr = cross(e1, e2);
    const float crLen = length(cr);
    area_ = 0.5f * crLen;
    normal_ = crLen > 0.0f ? cr / crLen : Vec3{0.0f, 0.0f, 0.0f};
}

LightSample AreaLight::sample(Point3 from, Sample2D u, const SampledWavelengths& lambdas) const {
    const Sample2D bary = uniformTriangleBarycentric(u);
    const float b0 = bary.u;
    const float b1 = bary.v;
    const float b2 = 1.0f - b0 - b1;
    const Point3 p{
        b0 * v0_.x + b1 * v1_.x + b2 * v2_.x,
        b0 * v0_.y + b1 * v1_.y + b2 * v2_.y,
        b0 * v0_.z + b1 * v1_.z + b2 * v2_.z,
    };

    const Vec3 toLight = p - from;
    const float distSq = lengthSquared(toLight);
    const float dist = std::sqrt(distSq);

    LightSample s;
    s.distance = dist;
    if (dist <= 0.0f || area_ <= 0.0f) {
        s.wi = Vec3{0.0f, 0.0f, 0.0f};
        s.radiance = Spectrum{0.0f};
        s.pdf = 0.0f;
        return s;
    }
    s.wi = toLight / dist;
    const float cosThetaPrime = dot(-s.wi, normal_);
    if (cosThetaPrime <= 0.0f) {
        s.radiance = Spectrum{0.0f};
        s.pdf = 0.0f;
        return s;
    }
    s.radiance = sampledIlluminantFromRgb(emissionRgb_, lambdas);
    s.pdf = distSq / (cosThetaPrime * area_);
    return s;
}

float AreaLight::pdfLi(Point3 from, Vec3 wi, const SampledWavelengths& /*lambdas*/) const {
    if (area_ <= 0.0f) {
        return 0.0f;
    }
    // Möller-Trumbore intersection of (from, wi) with this triangle.
    const Vec3 e1 = v1_ - v0_;
    const Vec3 e2 = v2_ - v0_;
    const Vec3 p = cross(wi, e2);
    const float det = dot(e1, p);
    constexpr float kParallelEpsilon = 1e-8f;
    if (std::fabs(det) < kParallelEpsilon) {
        return 0.0f;
    }
    const float invDet = 1.0f / det;
    const Vec3 tvec = from - v0_;
    const float b1 = dot(tvec, p) * invDet;
    if (b1 < 0.0f || b1 > 1.0f) {
        return 0.0f;
    }
    const Vec3 q = cross(tvec, e1);
    const float b2 = dot(wi, q) * invDet;
    if (b2 < 0.0f || b1 + b2 > 1.0f) {
        return 0.0f;
    }
    const float t = dot(e2, q) * invDet;
    if (t <= 0.0f) {
        return 0.0f;
    }
    const float cosThetaPrime = dot(-wi, normal_);
    if (cosThetaPrime <= 0.0f) {
        return 0.0f;
    }
    return (t * t) / (cosThetaPrime * area_);
}

Spectrum AreaLight::emit(const SampledWavelengths& lambdas) const {
    return sampledIlluminantFromRgb(emissionRgb_, lambdas);
}

}  // namespace nanopt
