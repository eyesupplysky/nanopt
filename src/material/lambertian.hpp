//! Lambertian (perfect diffuse) BSDF

#pragma once

#include "material/bsdf.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class LambertianBsdf : public Bsdf {
public:
    explicit LambertianBsdf(Spectrum albedo);

    Spectrum eval(Vec3 wi, Vec3 wo, Normal3 n) const override;

private:
    Spectrum albedo_;
};

}  // namespace nanopt
