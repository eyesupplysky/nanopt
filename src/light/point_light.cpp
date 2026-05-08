//! Point light implementation

#include "light/point_light.hpp"

#include <cmath>

#include "math/vec3.hpp"

namespace nanopt {

PointLight::PointLight(Point3 position, Spectrum intensity)
    : position_(position), intensity_(intensity) {}

LightSample PointLight::sample(Point3 from, Sample2D /*u*/) const {
    const Vec3 toLight = position_ - from;
    const float distSq = lengthSquared(toLight);
    const float dist = std::sqrt(distSq);

    LightSample sample;
    sample.wi = toLight / dist;
    sample.distance = dist;
    sample.radiance = intensity_ / distSq;
    sample.pdf = 1.0f;
    return sample;
}

}  // namespace nanopt
