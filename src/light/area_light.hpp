//! Diffuse area light — emits uniformly from one side of a triangle

#pragma once

#include "light/light.hpp"
#include "math/point3.hpp"
#include "math/vec3.hpp"
#include "sampler/sampler.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class AreaLight : public Light {
public:
    AreaLight(Point3 v0, Point3 v1, Point3 v2, Spectrum emission);

    LightSample sample(Point3 from, Sample2D u) const override;
    bool isDelta() const override { return false; }

    /// Outgoing radiance from this emitter — added to the path on a direct emitter hit
    Spectrum emit() const { return emission_; }

private:
    Point3 v0_;
    Point3 v1_;
    Point3 v2_;
    Spectrum emission_;
    Vec3 normal_;
    float area_ = 0.0f;
};

}  // namespace nanopt
