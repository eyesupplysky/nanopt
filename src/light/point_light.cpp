//! Point light implementation

#include "light/point_light.hpp"

#include <cmath>

#include "math/vec3.hpp"
#include "spectrum/rgb_upsample.hpp"
#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {

PointLight::PointLight(Point3 position, RgbSpectrum intensityRgb)
    : position_(position), intensityRgb_(intensityRgb) {}

LightSample PointLight::sample(Point3 from, Sample2D /*u*/, const SampledWavelengths& lambdas) const {
    const Vec3 toLight = position_ - from;
    const float distSq = lengthSquared(toLight);
    const float dist = std::sqrt(distSq);

    LightSample sample;
    sample.wi = toLight / dist;
    sample.distance = dist;
    sample.radiance = sampledIlluminantFromRgb(intensityRgb_, lambdas) / distSq;
    sample.pdf = 1.0f;
    return sample;
}

}  // namespace nanopt
