//! Spectrum keystone alias — definition swaps to SampledSpectrum at M3 without disturbing call sites

#pragma once

#include "spectrum/rgb_spectrum.hpp"

namespace nanopt {

using Spectrum = RgbSpectrum;

}  // namespace nanopt
