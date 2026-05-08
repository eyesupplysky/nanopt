//! Per-wavelength Fresnel reflectance — conductor (complex IOR) form

#pragma once

#include <algorithm>
#include <cmath>

namespace nanopt {

/// Unpolarized Fresnel reflectance at a real-IOR dielectric interface
inline float fresnelDielectric(float cosThetaI, float etaI, float etaT) {
    const float ci = std::clamp(cosThetaI, 0.0f, 1.0f);
    const float sin2I = std::max(0.0f, 1.0f - ci * ci);
    const float sin2T = (etaI / etaT) * (etaI / etaT) * sin2I;
    if (sin2T >= 1.0f) return 1.0f;
    const float ct = std::sqrt(1.0f - sin2T);
    const float Rs = (etaI * ci - etaT * ct) / (etaI * ci + etaT * ct);
    const float Rp = (etaT * ci - etaI * ct) / (etaT * ci + etaI * ct);
    return 0.5f * (Rs * Rs + Rp * Rp);
}

/// Unpolarized Fresnel reflectance at a conductor interface using complex IOR (η + iκ) in vacuum
/// Form per Born & Wolf, Principles of Optics §13.2; same as pbrt-v3 FrConductor
inline float fresnelConductor(float cosThetaI, float eta, float k) {
    const float ci = std::clamp(cosThetaI, 0.0f, 1.0f);
    const float ci2 = ci * ci;
    const float si2 = 1.0f - ci2;
    const float eta2 = eta * eta;
    const float k2 = k * k;

    const float t0 = eta2 - k2 - si2;
    const float a2plusb2 = std::sqrt(t0 * t0 + 4.0f * eta2 * k2);
    const float t1 = a2plusb2 + ci2;
    const float a = std::sqrt(std::max(0.0f, 0.5f * (a2plusb2 + t0)));
    const float t2 = 2.0f * ci * a;
    const float Rs = (t1 - t2) / (t1 + t2);

    const float t3 = a2plusb2 * ci2 + si2 * si2;
    const float t4 = t2 * si2;
    const float Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}

}  // namespace nanopt
