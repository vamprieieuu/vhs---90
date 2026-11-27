#include "VHS90PhysicalInterferenceEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kMu0 = 1.25663706212e-6f;
constexpr float kInvSqrt2 = 0.70710678118f;

inline float clamp01(float x) noexcept { return std::clamp(x, 0.0f, 1.0f); }
inline float finiteOr(float x, float fallback) noexcept { return std::isfinite(x) ? x : fallback; }
inline bool valid(ConstImageView s, ImageView d) noexcept {
    return s.rgba && d.rgba && s.width > 0 && s.height > 0 &&
           d.width == s.width && d.height == s.height &&
           s.rowStrideFloats >= s.width * 4 && d.rowStrideFloats >= d.width * 4 &&
           s.rgba != d.rgba;
}
inline float hashNoise(int x, int y, std::uint32_t seed, std::int64_t frame) noexcept {
    return hash01(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y), seed, frame) * 2.0f - 1.0f;
}
inline RGB readRgb(const float* px, bool premul) noexcept {
    RGB c{px[0], px[1], px[2]};
    if (premul && px[3] > 1.0e-6f) {
        c.r /= px[3]; c.g /= px[3]; c.b /= px[3];
    }
    return c;
}
}

float dipoleAxialFieldTesla(float momentAm2, float distanceMeters) noexcept {
    if (!std::isfinite(momentAm2) || !std::isfinite(distanceMeters) ||
        distanceMeters <= 1.0e-4f) return 0.0f;
    // Axial field of a magnetic dipole: B = mu0 * 2m / (4 pi r^3).
    return (1.0e-7f * 2.0f * std::fabs(momentAm2)) /
           (distanceMeters * distanceMeters * distanceMeters);
}

float magneticHeadSignalGain(const PhysicalInterferenceParams& p) noexcept {
    const float b = dipoleAxialFieldTesla(p.magnetMomentAm2,
                                          std::max(0.01f, p.magnetDistanceCm) * 0.01f);
    const float mu = std::clamp(finiteOr(p.headCoreRelativePermeability, 1500.0f), 1.0f, 10000.0f);
    const float coupling = clamp01(finiteOr(p.headCoreCoupling, 0.1f));
    const float saturation = std::max(1.0e-4f, finiteOr(p.headCoreSaturationTesla, 0.45f));
    const float x = mu * coupling * b / saturation;
    const float c = std::cosh(std::min(x, 20.0f));
    return 1.0f / (c * c);
}

float magneticTapeEraseFraction(const PhysicalInterferenceParams& p) noexcept {
    const float b = dipoleAxialFieldTesla(p.magnetMomentAm2,
                                          std::max(0.01f, p.magnetDistanceCm) * 0.01f);
    if (b <= 0.0f) return 0.0f;
    const float h = b / kMu0;
    const float hc = std::max(1.0f, finiteOr(p.tapeCoercivityAm, 52000.0f));
    const float spread = std::clamp(finiteOr(p.tapeSwitchingSpread, 0.30f), 0.02f, 1.0f);
    const float z = (h - hc) / (1.41421356237f * spread * hc);
    return clamp01(0.5f * (1.0f + std::erf(z)));
}

bool processPhysicalInterference(ConstImageView src, ImageView dst,
                                 const PhysicalInterferenceParams& p) noexcept {
    if (!valid(src, dst) ||
        !std::isfinite(p.magnetMomentAm2) || !std::isfinite(p.magnetDistanceCm) ||
        !std::isfinite(p.mainsFrequencyHz) || !std::isfinite(p.mainsPhaseRadians) ||
        !std::isfinite(p.groundLoopMillivolts) || !std::isfinite(p.mainsHumAmount) ||
        !std::isfinite(p.rfFieldVPerM) || !std::isfinite(p.rfCarrierMHz) ||
        !std::isfinite(p.rfPickupAmount) || !std::isfinite(p.framesPerSecond) ||
        p.framesPerSecond <= 0.0f) return false;

    const float hum = clamp01(p.mainsHumAmount);
    const float rf = clamp01(p.rfPickupAmount);
    const float headGain = magneticHeadSignalGain(p);
    const float erase = magneticTapeEraseFraction(p);
    const float signalGain = std::clamp(headGain * (1.0f - 0.82f * erase), 0.0f, 1.0f);
    const float loopDrive = std::clamp(std::fabs(p.groundLoopMillivolts) / 500.0f, 0.0f, 1.0f);
    // Mains Hum is an independent control; a zero Ground Loop setting must not mute it.
    const float humDrive = loopDrive > 0.0f ? loopDrive : 1.0f;
    const float rfDrive = std::clamp(std::fabs(p.rfFieldVPerM) / 40.0f, 0.0f, 1.0f) * rf;

    if (signalGain >= 0.999999f && hum <= 0.0f && rfDrive <= 0.0f) {
        for (int y = 0; y < src.height; ++y)
            std::copy_n(src.rgba + y * src.rowStrideFloats, src.width * 4,
                        dst.rgba + y * dst.rowStrideFloats);
        return true;
    }

    const float frameTime = float(p.frame) / p.framesPerSecond;
    const float mainsHz = std::clamp(p.mainsFrequencyHz, 1.0f, 400.0f);
    const float rfCyclesPerPixel = std::clamp(p.rfCarrierMHz, 0.1f, 19.0f) * 0.03125f;
    const float humOffset = (0.002f + 0.045f * hum) * humDrive;
    const float humGain = 1.0f + 0.018f * hum * humDrive;

    for (int y = 0; y < src.height; ++y) {
        const float lineTime = frameTime + float(y) /
            (float(src.height) * p.framesPerSecond);
        const float mainsPhase = 2.0f * kPi * mainsHz * lineTime + p.mainsPhaseRadians;
        const float humWave = std::sin(mainsPhase) + 0.20f * std::sin(3.0f * mainsPhase) +
                              0.10f * std::sin(5.0f * mainsPhase);
        const float lineBias = humOffset * humWave;
        const float* sr = src.rgba + y * src.rowStrideFloats;
        float* dr = dst.rgba + y * dst.rowStrideFloats;

        for (int x = 0; x < src.width; ++x) {
            const float* px = sr + x * 4;
            float* out = dr + x * 4;
            const float alpha = px[3];
            if (alpha <= 1.0e-6f) {
                std::copy_n(px, 4, out);
                continue;
            }

            RGB rgb = readRgb(px, p.premultipliedAlpha);
            YIQ yiq = rgbToYiq(rgb);

            // Magnetic loss reduces the recovered signal; the missing carrier
            // energy exposes the existing deck noise floor rather than painting
            // a separate snow texture over the whole frame.
            yiq.y *= signalGain;
            yiq.i *= signalGain;
            yiq.q *= signalGain;
            const float lost = 1.0f - signalGain;
            const float snow = lost * lost * 0.075f;
            yiq.y += hashNoise(x, y, p.seed ^ 0x4D41474Eu, p.frame) * snow;

            // Ground-loop pickup moves black level and modulates gain; the
            // mains harmonics make the bars less synthetic than a single sine.
            yiq.y = yiq.y * humGain + lineBias;

            if (rfDrive > 0.0f) {
                const float phase = 2.0f * kPi * rfCyclesPerPixel * float(x) +
                                    mainsPhase * 0.37f + float(y) * 0.11f;
                const float beat = std::sin(phase) + 0.35f * std::sin(phase * 0.5f + 0.8f);
                const float pickup = rfDrive * (0.0025f + 0.035f * rfDrive) * beat;
                yiq.y += pickup * 0.45f;
                yiq.i += pickup * 0.75f;
                yiq.q += pickup * 0.30f;
            }

            rgb = yiqToRgb(yiq);
            if (p.premultipliedAlpha) {
                rgb.r *= alpha; rgb.g *= alpha; rgb.b *= alpha;
            }
            out[0] = rgb.r; out[1] = rgb.g; out[2] = rgb.b; out[3] = alpha;
        }
    }
    return true;
}
} // namespace vhs90
