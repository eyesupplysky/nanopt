//! Uniform pseudo-random sampler — placeholder until stratified / Sobol arrives at M5

#pragma once

#include <cstdint>

#include "sampler/rng.hpp"
#include "sampler/sampler.hpp"

namespace nanopt {

class UniformSampler : public Sampler {
public:
    explicit UniformSampler(std::uint64_t seed = 1);

    float get1D() override;
    Sample2D get2D() override;
    void startPixel(int pixelX, int pixelY) override;

private:
    Rng rng_;
};

}  // namespace nanopt
