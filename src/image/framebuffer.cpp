//! Pixel grid implementation

#include "image/framebuffer.hpp"

#include <cassert>
#include <cstddef>

namespace nanopt {

Framebuffer::Framebuffer(int width, int height)
    : width_(width),
      height_(height),
      pixels_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
    assert(width > 0 && height > 0);
}

void Framebuffer::setPixel(int x, int y, Xyz color) {
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)
            + static_cast<std::size_t>(x)] = color;
}

Xyz Framebuffer::pixel(int x, int y) const {
    assert(x >= 0 && x < width_ && y >= 0 && y < height_);
    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_)
                   + static_cast<std::size_t>(x)];
}

}  // namespace nanopt
