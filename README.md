# nanopt

A from-scratch spectral path tracer in C++. Light transport is computed per-wavelength across the visible range (380–750 nm), not as RGB. The "nano" is grounded: nanometers.

Spectral rendering pays back the ~2× scope over an RGB tracer with dispersion (rainbow through prisms), iridescence, and the infrastructure to render fluorescence accurately.

## Status

- [x] **M1** — RGB pinhole ray caster: scene graph, sphere primitive, ray-sphere intersection, single-bounce direct lighting, PPM output.
- [x] **M2** — Path tracing (RGB): Russian-roulette-terminated multi-bounce GI, BVH for triangle meshes, OBJ loader, Cornell box.
- [x] **M3** — Spectral upgrade: hero+stratified wavelength sampling (5 samples per ray), Smits RGB-to-spectrum upsampling for scene literals, CIE 1931 spectral integration via Wyman analytic fits, D65-normalized XYZ → linear sRGB output.
- [ ] **M4** — Materials: Lambertian, conductor (Fresnel + GGX), dielectric (wavelength-dependent IOR via Sellmeier).
- [ ] **M5** — Importance sampling: MIS for direct lighting, cosine-weighted hemisphere sampling, GGX VNDF sampling.
- [ ] **M6** — Multithreading + polish: tile-based parallel renderer, public documentation of the light transport equation, spectral pipeline, BVH layout, and BRDF reference.

M3 is complete. Light transport is computed per-wavelength: each path samples 5 wavelengths (one hero + four stratified secondaries) across 380–750 nm, BSDFs and lights upsample their RGB literals to those wavelengths via the Smits 1999 algorithm, and the integrator returns radiance evaluated at the path's wavelengths. The framebuffer accumulates CIE XYZ tristimulus values; output goes through XYZ → linear sRGB → 1/2.2 gamma. The 256-sample Cornell reference at `references/m3-cornell.png` matches `references/m2-cornell.png` on neutral materials to within ±6 byte values per pixel, confirming the spectral pipeline round-trips RGB correctly.

## Build

Requires CMake 3.25+ and a C++20 compiler.

```sh
cmake -B build
cmake --build build --config Release
```

Run the resulting `nanopt` binary from the project root so it can find `assets/cornell-box.obj`. Output is written to `out.ppm` in the working directory.

## Design notes

- **No external renderer libraries.** No Embree, no OpenImageDenoise, no third-party BSDF library. The goal is to derive light transport from first principles.
- **CMake** for the build; `vcpkg` will arrive once later milestones need the small set of supporting deps.
- **Reference scenes** are tracked at every milestone; image diffs against them catch silent regressions.
