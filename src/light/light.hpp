//! Light interface — provides direct-lighting samples toward a shading point

#pragma once

#include "math/point3.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
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

    /// Sample a direction toward the light from the shading point.
    /// u supplies the 2D random sample needed by area-style lights; delta lights ignore it.
    virtual LightSample sample(Point3 from, Sample2D u) const = 0;

    /// True for delta-distribution lights (point, directional). Integrators skip pdf division for these.
    virtual bool isDelta() const { return false; }
};

}  // namespace nanopt
