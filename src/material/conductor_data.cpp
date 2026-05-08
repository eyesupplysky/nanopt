//! Measured conductor η/κ tables — McPeak et al. 2015, ACS Photonics 2:326-333 (CC0)

#include "material/conductor_data.hpp"

#include <algorithm>
#include <array>

#include "spectrum/sampled_spectrum.hpp"

namespace nanopt {
namespace {

constexpr float kTableLambdaMin = 380.0f;
constexpr float kTableLambdaStep = 10.0f;
constexpr int kTableEntries = 38;  // 380, 390, ..., 750 nm inclusive

struct EtaK {
    float eta;
    float k;
};

// Source: K. M. McPeak et al., "Plasmonic films can easily be better: Rules and recipes",
// ACS Photonics 2:326-333 (2015). DOI: 10.1021/ph5004237.
// CC0 via refractiveindex.info; see assets/conductor/ for the upstream YAML and
// extract.priv.py for the resampling script.

constexpr std::array<EtaK, kTableEntries> kAuTable = {{
    {1.678059f, 1.948382f}, {1.672504f, 1.966512f}, {1.665616f, 1.973924f}, {1.653911f, 1.975227f},
    {1.637437f, 1.970305f}, {1.612343f, 1.959477f}, {1.581523f, 1.940313f}, {1.538326f, 1.910745f},
    {1.475285f, 1.872316f}, {1.386263f, 1.827683f}, {1.252905f, 1.782098f}, {1.064360f, 1.767870f},
    {0.848475f, 1.828280f}, {0.661636f, 1.964526f}, {0.529127f, 2.129736f}, {0.438041f, 2.294990f},
    {0.372502f, 2.451664f}, {0.323931f, 2.597158f}, {0.284960f, 2.738978f}, {0.254069f, 2.871855f},
    {0.228937f, 2.999519f}, {0.207122f, 3.121346f}, {0.188790f, 3.241703f}, {0.172726f, 3.356759f},
    {0.160072f, 3.465635f}, {0.146368f, 3.577628f}, {0.135418f, 3.685965f}, {0.125499f, 3.792328f},
    {0.117832f, 3.896137f}, {0.111118f, 3.999543f}, {0.105745f, 4.102704f}, {0.101509f, 4.205033f},
    {0.098608f, 4.305081f}, {0.097790f, 4.403812f}, {0.098623f, 4.499192f}, {0.096904f, 4.595801f},
    {0.099463f, 4.687205f}, {0.098537f, 4.781078f},
}};

constexpr std::array<EtaK, kTableEntries> kCuTable = {{
    {1.165775f, 2.046321f}, {1.139930f, 2.090129f}, {1.119339f, 2.142246f}, {1.097661f, 2.193481f},
    {1.082884f, 2.251164f}, {1.067185f, 2.306769f}, {1.056311f, 2.361947f}, {1.048210f, 2.413637f},
    {1.044058f, 2.464134f}, {1.040826f, 2.508968f}, {1.040384f, 2.549588f}, {1.035623f, 2.577676f},
    {1.029217f, 2.600959f}, {1.015962f, 2.610628f}, {0.995464f, 2.613857f}, {0.957526f, 2.603585f},
    {0.896412f, 2.584135f}, {0.797460f, 2.564204f}, {0.649914f, 2.566649f}, {0.467668f, 2.633707f},
    {0.308053f, 2.774526f}, {0.206478f, 2.953106f}, {0.153429f, 3.124794f}, {0.129739f, 3.280828f},
    {0.116677f, 3.422223f}, {0.110070f, 3.546564f}, {0.107194f, 3.666809f}, {0.104232f, 3.775694f},
    {0.102539f, 3.879628f}, {0.102449f, 3.981770f}, {0.101216f, 4.082309f}, {0.101604f, 4.175084f},
    {0.101237f, 4.270626f}, {0.101558f, 4.365354f}, {0.101132f, 4.453676f}, {0.100849f, 4.541494f},
    {0.100920f, 4.632838f}, {0.101174f, 4.718605f},
}};

constexpr std::array<EtaK, kTableEntries> kAlTable = {{
    {0.335956f, 4.002607f}, {0.354902f, 4.114238f}, {0.375151f, 4.226433f}, {0.396086f, 4.336806f},
    {0.417648f, 4.447407f}, {0.440996f, 4.559109f}, {0.464233f, 4.669102f}, {0.489220f, 4.778319f},
    {0.514817f, 4.887948f}, {0.539877f, 4.997346f}, {0.568005f, 5.105941f}, {0.596705f, 5.212874f},
    {0.625686f, 5.320478f}, {0.655710f, 5.428163f}, {0.688336f, 5.535956f}, {0.720794f, 5.641768f},
    {0.754468f, 5.746497f}, {0.789405f, 5.851937f}, {0.829205f, 5.958236f}, {0.867377f, 6.060226f},
    {0.908570f, 6.165184f}, {0.948956f, 6.267417f}, {0.992466f, 6.368986f}, {1.038146f, 6.466819f},
    {1.088160f, 6.566589f}, {1.136329f, 6.663126f}, {1.190265f, 6.759418f}, {1.246364f, 6.852330f},
    {1.304383f, 6.942364f}, {1.365410f, 7.031952f}, {1.426024f, 7.116287f}, {1.493231f, 7.197481f},
    {1.559752f, 7.273914f}, {1.631622f, 7.345760f}, {1.706781f, 7.414250f}, {1.786399f, 7.475374f},
    {1.872189f, 7.528353f}, {1.958355f, 7.571404f},
}};

const std::array<EtaK, kTableEntries>& tableFor(MetalKind metal) {
    switch (metal) {
        case MetalKind::Gold:     return kAuTable;
        case MetalKind::Copper:   return kCuTable;
        case MetalKind::Aluminum: return kAlTable;
    }
    return kAuTable;  // unreachable on a defined enum value
}

EtaK lookup(const std::array<EtaK, kTableEntries>& table, float lambda) {
    const float t = (lambda - kTableLambdaMin) / kTableLambdaStep;
    const int i0 = std::clamp(static_cast<int>(t), 0, kTableEntries - 2);
    const float frac = std::clamp(t - static_cast<float>(i0), 0.0f, 1.0f);
    const EtaK& a = table[i0];
    const EtaK& b = table[i0 + 1];
    return EtaK{
        a.eta + frac * (b.eta - a.eta),
        a.k + frac * (b.k - a.k),
    };
}

}  // namespace

ConductorIor evalConductorIor(MetalKind metal, const SampledWavelengths& lambdas) {
    const auto& table = tableFor(metal);
    ConductorIor out;
    for (int i = 0; i < kSpectrumSamples; ++i) {
        const EtaK ek = lookup(table, lambdas.lambda[i]);
        out.eta.values[i] = ek.eta;
        out.k.values[i] = ek.k;
    }
    return out;
}

}  // namespace nanopt
