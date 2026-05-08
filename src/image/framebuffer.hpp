//! Pixel grid storing accumulated tristimulus radiance in CIE XYZ

#pragma once

#include <cstddef>
#include <vector>

#include "spectrum/cie.hpp"

namespace nanopt {

class Framebuffer {
public:
    Framebuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    /// Replace the pixel at (x, y)
    void setPixel(int x, int y, Xyz color);

    /// Read the pixel at (x, y)
    Xyz pixel(int x, int y) const;

    const std::vector<Xyz>& pixels() const { return pixels_; }

private:
    int width_;
    int height_;
    std::vector<Xyz> pixels_;
};

}  // namespace nanopt
