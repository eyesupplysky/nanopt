add_library(nanopt_core STATIC
    sampler/uniform_sampler.cpp
    image/framebuffer.cpp
    image/ppm.cpp
    geometry/sphere.cpp
    accel/primitive_list.cpp
    camera/pinhole_camera.cpp
    light/point_light.cpp
    material/lambertian.cpp
    scene/scene.cpp
    integrator/direct_integrator.cpp
)

target_include_directories(nanopt_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(nanopt main.cpp)
target_link_libraries(nanopt PRIVATE nanopt_core)
