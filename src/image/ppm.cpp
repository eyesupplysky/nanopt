//! PPM writer implementation

#include "image/ppm.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>

#include "image/framebuffer.hpp"
#include "spectrum/spectrum.hpp"

namespace nanopt {

namespace {

std::uint8_t toByte(float linear) {
    const float clamped = std::clamp(linear, 0.0f, 1.0f);
    const float gamma = std::pow(clamped, 1.0f / 2.2f);
    return static_cast<std::uint8_t>(gamma * 255.0f + 0.5f);
}

}  // namespace

bool writePpm(const Framebuffer& framebuffer, std::string_view path) {
    const std::string pathStr(path);
    std::FILE* file = std::fopen(pathStr.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }

    std::fprintf(file, "P6\n%d %d\n255\n", framebuffer.width(), framebuffer.height());

    for (int y = 0; y < framebuffer.height(); ++y) {
        for (int x = 0; x < framebuffer.width(); ++x) {
            const Spectrum c = framebuffer.pixel(x, y);
            const std::uint8_t bytes[3] = {toByte(c.r), toByte(c.g), toByte(c.b)};
            std::fwrite(bytes, 1, 3, file);
        }
    }

    std::fclose(file);
    return true;
}

}  // namespace nanopt
