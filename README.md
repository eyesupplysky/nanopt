# nanopt

A from-scratch spectral path tracer in C++. Light transport is computed per-wavelength across the visible range (380–750 nm), not as RGB. The "nano" is grounded: nanometers.

Spectral rendering pays back the ~2× scope over an RGB tracer with dispersion (rainbow through prisms), iridescence, and the infrastructure to render fluorescence accurately.

## Status

- [x] **M1** — RGB pinhole ray caster: scene graph, sphere primitive, ray-sphere intersection, single-bounce direct lighting, PPM output.
- [x] **M2** — Path tracing (RGB): Russian-roulette-terminated multi-bounce GI, BVH for triangle meshes, OBJ loader, Cornell box.
- [x] **M3** — Spectral upgrade: hero+stratified wavelength sampling (5 samples per ray), Smits RGB-to-spectrum upsampling for scene literals, CIE 1931 spectral integration via Wyman analytic fits, D65-normalized XYZ → linear sRGB output.
- [x] **M4** — Materials: Lambertian, conductor (Fresnel + GGX), dielectric (wavelength-dependent IOR via Sellmeier). Smooth shading from per-vertex normals; the Stanford XYZ RGB Dragon ships as the canonical demo.
- [x] **M5** — Importance sampling: Veach two-strategy MIS for direct lighting (light-strategy NEE + BSDF-strategy continuation, combined with the power-2 heuristic). Cosine-weighted hemisphere and GGX VNDF sampling already in by M4 (Lambertian and rough conductor respectively).
- [ ] **M6** — Multithreading + polish: tile-based parallel renderer, public documentation of the light transport equation, spectral pipeline, BVH layout, and BRDF reference.

M3 is complete. Light transport is computed per-wavelength: each path samples 5 wavelengths (one hero + four stratified secondaries) across 380–750 nm, BSDFs and lights upsample their RGB literals to those wavelengths via the Smits 1999 algorithm, and the integrator returns radiance evaluated at the path's wavelengths. The framebuffer accumulates CIE XYZ tristimulus values; output goes through XYZ → linear sRGB → 1/2.2 gamma. The 256-sample Cornell reference at `references/m3-cornell.png` matches `references/m2-cornell.png` on neutral materials to within ±6 byte values per pixel, confirming the spectral pipeline round-trips RGB correctly.

M4 is complete. Three new BSDFs ship: Lambertian (pre-existing), a rough conductor with measured complex IOR (Au/Cu/Al from McPeak et al. 2015) using GGX VNDF sampling and height-correlated Smith masking, and a smooth dielectric whose index of refraction is computed per wavelength via the Sellmeier 3-term equation (BK7 and SF10 ship as constexpr instances). All BSDFs evaluate in tangent space via an explicit `Frame` carried on every Hit. The dielectric refract branch is wavelength-dependent: it samples the hero direction only and signals the path tracer to terminate the secondary wavelengths via `SampledWavelengths::terminateSecondary` — the dispersion mechanism prepared in M3 finally exercised. Smooth shading from per-vertex normals (interpolated barycentrically with face-forward clamp) supports curved meshes without faceting; the Stanford XYZ RGB Dragon ships as the canonical demo at `references/m4-cornell.png`, with visible chromatic dispersion through the SF10 glass body.

M5 is complete. Direct lighting now uses Veach two-strategy MIS: every non-delta surface combines a light-strategy estimator (next-event estimation, sampling each light directly) with a BSDF-strategy estimator (the path's continuation ray contributing whenever it lands on an emitter), weighted with the power-2 heuristic. The pdf surface that makes this work is small but exact: each `Bsdf` exposes a `pdf(wi, wo, frame, lambdas)` query that returns the same solid-angle density `sample()` would assign to the given direction (Lambertian: cosine; rough conductor: GGX VNDF half-vector pdf carried through the reflection Jacobian; delta BSDFs: zero), and each `Light` exposes `pdfLi(from, wi, lambdas)` (area lights: a Möller-Trumbore intersection plus the standard `t² / (cosθ' · area)`; delta lights: zero). Delta BSDFs receive direct lighting through the BSDF strategy alone (the light strategy is identically zero for a Dirac lobe), which lets the path tracer see emitters refracted through glass — previously dropped by the bounce-0-only emitter rule. The canonical M5 scene at `references/m5-cornell.png` swaps the M4 single ceiling light for a Veach-style row of four equal-flux area lights at decreasing size; small bright lights favour light sampling, large dim lights favour BSDF sampling, and MIS chooses the right strategy per pixel automatically.

## Acknowledgements

- **Stanford XYZ RGB Dragon** — Stanford 3D Scanning Repository (http://graphics.stanford.edu/data/3Dscanrep/). Distributed via the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models) collection.
- **Conductor optical constants** — K. M. McPeak, S. V. Jayanti, S. J. P. Kress, S. Meyer, S. Iotti, A. Rossinelli, D. J. Norris, *"Plasmonic films can easily be better: Rules and recipes,"* ACS Photonics 2:326-333 (2015). Tables resampled from the CC0 data hosted at [refractiveindex.info](https://refractiveindex.info).

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
