//! Pixel grid storing accumulated radiance in linear Spectrum units

#pragma once

#include <cstddef>
#include <vector>

#include "spectrum/spectrum.hpp"

namespace nanopt {

class Framebuffer {
public:
    Framebuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    /// Replace the pixel at (x, y)
    void setPixel(int x, int y, Spectrum color);

    /// Read the pixel at (x, y)
    Spectrum pixel(int x, int y) const;

    const std::vector<Spectrum>& pixels() const { return pixels_; }

private:
    int width_;
    int height_;
    std::vector<Spectrum> pixels_;
};

}  // namespace nanopt
