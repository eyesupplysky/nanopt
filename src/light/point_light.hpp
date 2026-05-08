//! Point light — delta distribution at a fixed position with 1/d^2 falloff

#pragma once

#include "light/light.hpp"
#include "math/point3.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class PointLight : public Light {
public:
    PointLight(Point3 position, Spectrum intensity);

    LightSample sample(Point3 from) const override;

private:
    Point3 position_;
    Spectrum intensity_;
};

}  // namespace nanopt
