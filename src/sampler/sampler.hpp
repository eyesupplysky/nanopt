//! Per-pixel sampling interface — supplies 1D and 2D random samples to integrators

#pragma once

namespace nanopt {

struct Sample2D {
    float u = 0.0f;
    float v = 0.0f;
};

class Sampler {
public:
    virtual ~Sampler() = default;

    /// One-dimensional sample in [0, 1)
    virtual float get1D() = 0;

    /// Two-dimensional sample in [0, 1)^2
    virtual Sample2D get2D() = 0;

    /// Begin a new pixel; sampler may reset stratification state
    virtual void startPixel(int pixelX, int pixelY) = 0;
};

}  // namespace nanopt
