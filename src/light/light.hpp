//! Light interface — provides direct-lighting samples toward a shading point

#pragma once

#include "math/point3.hpp"
#include "math/vec3.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

struct LightSample {
    Vec3 wi;
    float distance = 0.0f;
    Spectrum radiance;
    float pdf = 0.0f;
};

class Light {
public:
    virtual ~Light() = default;

    /// Sample a direction toward the light from the shading point
    virtual LightSample sample(Point3 from) const = 0;
};

}  // namespace nanopt
