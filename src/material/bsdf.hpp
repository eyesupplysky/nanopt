//! BSDF interface — bidirectional scattering distribution function

#pragma once

#include "math/normal3.hpp"
#include "math/vec3.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

class Bsdf {
public:
    virtual ~Bsdf() = default;

    /// Evaluate the BSDF for incoming wi and outgoing wo at a surface with normal n
    virtual Spectrum eval(Vec3 wi, Vec3 wo, Normal3 n) const = 0;
};

}  // namespace nanopt
