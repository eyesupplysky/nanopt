//! Random number generator wrapping std::mt19937 — replaceable with PCG later

#pragma once

#include <cstdint>
#include <random>

namespace nanopt {

class Rng {
public:
    explicit Rng(std::uint64_t seed = 1)
        : engine_(static_cast<std::mt19937::result_type>(seed)) {}

    /// Uniform float in [0, 1)
    float uniform() { return distribution_(engine_); }

private:
    std::mt19937 engine_;
    std::uniform_real_distribution<float> distribution_{0.0f, 1.0f};
};

}  // namespace nanopt
