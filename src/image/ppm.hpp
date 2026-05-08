//! PPM (P6 binary) image writer — placeholder until EXR at M6

#pragma once

#include <string_view>

namespace nanopt {

class Framebuffer;

/// Write the framebuffer to a binary PPM (P6) file with sRGB-style gamma encoding
bool writePpm(const Framebuffer& framebuffer, std::string_view path);

}  // namespace nanopt
