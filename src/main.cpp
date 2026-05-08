//! M1 entry point — renders a hardcoded sphere-on-ground scene to out.ppm

#include <cstdio>
#include <memory>
#include <numbers>

#include "camera/pinhole_camera.hpp"
#include "geometry/sphere.hpp"
#include "image/framebuffer.hpp"
#include "image/ppm.hpp"
#include "integrator/direct_integrator.hpp"
#include "light/point_light.hpp"
#include "material/lambertian.hpp"
#include "math/point3.hpp"
#include "math/ray.hpp"
#include "math/vec3.hpp"
#include "sampler/uniform_sampler.hpp"
#include "scene/scene.hpp"
#include "spectrum/spectrum.hpp"

using namespace nanopt;

int main() {
    constexpr int kWidth = 512;
    constexpr int kHeight = 512;

    Scene scene;

    const Bsdf* red =
        scene.addBsdf(std::make_unique<LambertianBsdf>(Spectrum{0.80f, 0.20f, 0.20f}));
    const Bsdf* gray =
        scene.addBsdf(std::make_unique<LambertianBsdf>(Spectrum{0.60f, 0.60f, 0.60f}));

    scene.addPrimitive(std::make_unique<Sphere>(Point3{0.0f, 1.0f, 0.0f}, 1.0f, red));
    scene.addPrimitive(
        std::make_unique<Sphere>(Point3{0.0f, -1000.0f, 0.0f}, 1000.0f, gray));

    scene.addLight(
        std::make_unique<PointLight>(Point3{4.0f, 6.0f, 4.0f}, Spectrum{200.0f}));

    scene.setCamera(std::make_unique<PinholeCamera>(
        Point3{3.0f, 3.0f, 5.0f},
        Point3{0.0f, 1.0f, 0.0f},
        Vec3{0.0f, 1.0f, 0.0f},
        std::numbers::pi_v<float> / 4.0f,
        static_cast<float>(kWidth) / static_cast<float>(kHeight)));

    DirectIntegrator integrator{Spectrum{0.05f, 0.07f, 0.10f}};
    UniformSampler sampler{42};
    Framebuffer framebuffer{kWidth, kHeight};

    for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
            const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(kWidth);
            const float v =
                1.0f - (static_cast<float>(y) + 0.5f) / static_cast<float>(kHeight);
            const Ray ray = scene.camera().generateRay(u, v);
            const Spectrum color = integrator.Li(ray, scene, sampler);
            framebuffer.setPixel(x, y, color);
        }
        if ((y % 64) == 0) {
            std::printf("Row %d/%d\n", y, kHeight);
        }
    }

    if (!writePpm(framebuffer, "out.ppm")) {
        std::fprintf(stderr, "Failed to write out.ppm\n");
        return 1;
    }
    std::printf("Wrote out.ppm\n");
    return 0;
}
