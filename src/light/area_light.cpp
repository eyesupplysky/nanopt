//! Diffuse area light implementation

#include "light/area_light.hpp"

#include <cmath>

#include "math/sampling.hpp"
#include "math/vec3.hpp"

namespace nanopt {

AreaLight::AreaLight(Point3 v0, Point3 v1, Point3 v2, Spectrum emission)
    : v0_(v0), v1_(v1), v2_(v2), emission_(emission) {
    const Vec3 e1 = v1_ - v0_;
    const Vec3 e2 = v2_ - v0_;
    const Vec3 cr = cross(e1, e2);
    const float crLen = length(cr);
    area_ = 0.5f * crLen;
    normal_ = crLen > 0.0f ? cr / crLen : Vec3{0.0f, 0.0f, 0.0f};
}

LightSample AreaLight::sample(Point3 from, Sample2D u) const {
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
    s.radiance = emission_;
    s.pdf = distSq / (cosThetaPrime * area_);
    return s;
}

}  // namespace nanopt
