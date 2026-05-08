//! Point light — delta distribution at a fixed position with 1/d^2 falloff

#pragma once

#include "light/light.hpp"
#include "math/point3.hpp"
#include "spectrum/rgb_spectrum.hpp"

namespace nanopt {

class PointLight : public Light {
public:
    PointLight(Point3 position, RgbSpectrum intensityRgb);

    LightSample sample(Point3 from, Sample2D u, const SampledWavelengths& lambdas) const override;
    bool isDelta() const override { return true; }

private:
    Point3 position_;
    RgbSpectrum intensityRgb_;
};

}  // namespace nanopt
