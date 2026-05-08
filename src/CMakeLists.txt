add_library(nanopt_core STATIC
    sampler/uniform_sampler.cpp
    image/framebuffer.cpp
    image/ppm.cpp
    geometry/sphere.cpp
    geometry/triangle.cpp
    accel/bvh.cpp
    camera/pinhole_camera.cpp
    light/point_light.cpp
    light/area_light.cpp
    material/lambertian.cpp
    material/conductor.cpp
    material/conductor_data.cpp
    material/dielectric.cpp
    scene/scene.cpp
    spectrum/cie.cpp
    spectrum/rgb_upsample.cpp
    spectrum/wavelength.cpp
    integrator/direct_integrator.cpp
    integrator/path_integrator.cpp
    io/obj_loader.cpp
)

target_include_directories(nanopt_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(nanopt main.cpp)
target_link_libraries(nanopt PRIVATE nanopt_core)
