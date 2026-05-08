//! Uniform pseudo-random sampler implementation

#include "sampler/uniform_sampler.hpp"

namespace nanopt {

UniformSampler::UniformSampler(std::uint64_t seed) : rng_(seed) {}

float UniformSampler::get1D() {
    return rng_.uniform();
}

Sample2D UniformSampler::get2D() {
    return {rng_.uniform(), rng_.uniform()};
}

void UniformSampler::startPixel(int /*pixelX*/, int /*pixelY*/) {}

}  // namespace nanopt
