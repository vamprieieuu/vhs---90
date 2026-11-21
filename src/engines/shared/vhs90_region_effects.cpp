#include "vhs90_region_effects.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;

inline uint64_t mix64(uint64_t x) noexcept {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}
inline uint64_t keyFor(uint64_t seed, int64_t frame, uint32_t stream,
                       int32_t a, int32_t b, int32_t c = 0) noexcept {
    uint64_t x = seed ^ (static_cast<uint64_t>(frame) * 0x9e3779b97f4a7c15ULL);
    x ^= static_cast<uint64_t>(stream) * 0xd1b54a32d192ed03ULL;
    x ^= static_cast<uint64_t>(static_cast<uint32_t>(a)) * 0x94d049bb133111ebULL;
    x ^= static_cast<uint64_t>(static_cast<uint32_t>(b)) * 0xbf58476d1ce4e5b9ULL;
    x ^= static_cast<uint64_t>(static_cast<uint32_t>(c)) * 0x369dea0f31a53f85ULL;
    return mix64(x);
}
inline float unit01(uint64_t x) noexcept {
    return static_cast<float>((x >> 40) & 0xFFFFFFu) * (1.0f / 16777216.0f);
}
inline float signed01(uint64_t x) noexcept { return unit01(x) * 2.0f - 1.0f; }
inline float smooth01(float t) noexcept {
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
inline float smoothNoise(uint64_t seed, int64_t frame, uint32_t stream,
                         float coordinate, int32_t quantization) noexcept {
    const float q = coordinate * static_cast<float>(quantization);
    const int32_t i0 = static_cast<int32_t>(std::floor(q));
    const float f = smooth01(q - static_cast<float>(i0));
    const float a = signed01(keyFor(seed, frame, stream, i0, 0));
    const float b = signed01(keyFor(seed, frame, stream, i0 + 1, 0));
    return a + (b - a) * f;
}
inline bool valid(FloatRgbaView im) noexcept {
    return im.pixels && im.width > 0 && im.height > 0 &&
           im.rowStrideFloats >= im.width * 4;
}
inline float clamp01(float x) noexcept { return std::clamp(x, 0.0f, 1.0f); }
inline void copyImage(FloatRgbaView im, std::vector<float>& dst) {
    dst.resize(static_cast<size_t>(im.width) * im.height * 4);
    for (int y = 0; y < im.height; ++y) {
        const float* src = im.pixels + static_cast<size_t>(y) * im.rowStrideFloats;
        float* out = dst.data() + static_cast<size_t>(y) * im.width * 4;
        std::copy(src, src + im.width * 4, out);
    }
}
inline void sampleBilinear(const std::vector<float>& src, int w, int h,
                           float x, float y, float out[4]) noexcept {
    x = std::clamp(x, 0.0f, static_cast<float>(w - 1));
    y = std::clamp(y, 0.0f, static_cast<float>(h - 1));
    const int x0 = static_cast<int>(x), y0 = static_cast<int>(y);
    const int x1 = std::min(x0 + 1, w - 1), y1 = std::min(y0 + 1, h - 1);
    const float fx = x - x0, fy = y - y0;
    const size_t i00 = (static_cast<size_t>(y0) * w + x0) * 4;
    const size_t i10 = (static_cast<size_t>(y0) * w + x1) * 4;
    const size_t i01 = (static_cast<size_t>(y1) * w + x0) * 4;
    const size_t i11 = (static_cast<size_t>(y1) * w + x1) * 4;
    for (int c = 0; c < 4; ++c) {
        const float a = src[i00 + c] + (src[i10 + c] - src[i00 + c]) * fx;
        const float b = src[i01 + c] + (src[i11 + c] - src[i01 + c]) * fx;
        out[c] = a + (b - a) * fy;
    }
}
inline float regionEnvelope(float across, float halfExtent, float feather) noexcept {
    if (halfExtent <= 0.0f || std::abs(across) > halfExtent) return 0.0f;
    const float edge = std::max(0.0f, 1.0f - std::abs(across) / halfExtent);
    if (feather <= 0.001f) return 1.0f;
    const float start = std::clamp(feather, 0.0f, 1.0f);
    return smooth01(std::clamp(edge / start, 0.0f, 1.0f));
}
} // namespace

void applyTrackingNoise(FloatRgbaView im, const TrackingNoiseParams& p,
                        uint64_t globalSeed, int64_t frameIndex) {
    if (!valid(im) || !p.enabled ||
        (p.displacementPixels == 0.0f && p.noiseAmount == 0.0f && p.chromaAmount == 0.0f))
        return;

    std::vector<float> src;
    copyImage(im, src);
    const float ca = std::cos(p.region.angleRadians), sa = std::sin(p.region.angleRadians);
    const float da = std::cos(p.displacementAngleRadians), db = std::sin(p.displacementAngleRadians);
    const float cx = clamp01(p.region.centerX) * (im.width - 1);
    const float cy = clamp01(p.region.centerY) * (im.height - 1);
    const float halfW = std::max(0.5f, clamp01(p.region.width) * im.width * 0.5f);
    const float halfH = std::max(0.5f, clamp01(p.region.height) * im.height * 0.5f);
    const uint64_t seed = globalSeed ^ p.seedOffset;
    const float time = static_cast<float>(frameIndex) * p.temporalSpeed;

    for (int y = 0; y < im.height; ++y) {
        float* dst = im.pixels + static_cast<size_t>(y) * im.rowStrideFloats;
        for (int x = 0; x < im.width; ++x) {
            const float dx = static_cast<float>(x) - cx;
            const float dy = static_cast<float>(y) - cy;
            const float along = dx * ca + dy * sa;
            const float across = -dx * sa + dy * ca;
            if (std::abs(along) > halfW) continue;
            const float env = regionEnvelope(across, halfH, p.region.feather);
            if (env <= 0.0f) continue;

            const float u = (along / std::max(1.0f, halfW) + 1.0f) * 0.5f;
            const float slow = smoothNoise(seed, frameIndex, 0x101u, u + time * 0.002f, 9);
            const float fast = smoothNoise(seed, frameIndex, 0x102u,
                                           across / std::max(1.0f, halfH) + time * 0.011f, 23);
            const float tear = (slow * 0.68f + fast * 0.32f) * p.tearAmount;
            const float shift = p.displacementPixels * env * tear;
            float sample[4];
            sampleBilinear(src, im.width, im.height,
                           static_cast<float>(x) - da * shift,
                           static_cast<float>(y) - db * shift, sample);

            const float grain = signed01(keyFor(seed, frameIndex, 0x103u, x / 4, y / 2));
            const float lumaNoise = grain * p.noiseAmount * env;
            const float chromaNoise = signed01(keyFor(seed, frameIndex, 0x104u, x / 3, y / 2))
                                      * p.chromaAmount * env;
            dst[x * 4 + 0] = sample[0] + lumaNoise + chromaNoise;
            dst[x * 4 + 1] = sample[1] + lumaNoise - chromaNoise * 0.25f;
            dst[x * 4 + 2] = sample[2] + lumaNoise - chromaNoise;
            // Alpha is deliberately not written.
        }
    }
}

void applyHeadSwitching(FloatRgbaView im, const HeadSwitchParams& p,
                        uint64_t globalSeed, int64_t frameIndex) {
    if (!valid(im) || !p.enabled || p.intensity <= 0.0f || p.heightFraction <= 0.0f)
        return;

    std::vector<float> src;
    copyImage(im, src);
    const float anchor = clamp01(p.anchorY);
    const float height = std::clamp(p.heightFraction, 0.0f, 1.0f);
    const float y0n = std::max(0.0f, anchor - height);
    const float y1n = anchor;
    if (y1n <= y0n) return;
    const int y0 = std::clamp(static_cast<int>(std::floor(y0n * im.height)), 0, im.height - 1);
    const int y1 = std::clamp(static_cast<int>(std::ceil(y1n * im.height)), y0 + 1, im.height);
    const int bandH = y1 - y0;
    const uint64_t seed = globalSeed ^ p.seedOffset;

    for (int y = y0; y < y1; ++y) {
        const float t = bandH <= 1 ? 1.0f : static_cast<float>(y - y0) / (bandH - 1);
        const float ramp = std::pow(std::clamp(t, 0.0f, 1.0f), std::max(0.05f, p.rampPower));
        const float lineNoise = smoothNoise(seed, frameIndex, 0x201u,
                                            static_cast<float>(y) / std::max(1.0f, im.height * 0.08f), 7);
        const float lineShift = p.horizontalShiftPixels * p.intensity * ramp * (0.72f + 0.28f * lineNoise);
        const float flicker = 0.5f + 0.5f * signed01(keyFor(seed, frameIndex, 0x202u, y, 0));
        float* dst = im.pixels + static_cast<size_t>(y) * im.rowStrideFloats;
        for (int x = 0; x < im.width; ++x) {
            float sample[4], red[4], blue[4];
            sampleBilinear(src, im.width, im.height, static_cast<float>(x) - lineShift,
                           static_cast<float>(y), sample);
            const float n = signed01(keyFor(seed, frameIndex, 0x203u, x / 2, y));
            const float stripe = std::sin(static_cast<float>(x) * 0.17f + flicker * 6.2831853f);
            const float grain = n * p.noiseAmount * p.intensity * (0.35f + 0.65f * ramp);
            const float band = stripe * p.stripeAmount * p.intensity * flicker * 0.08f;
            const float fringe = p.colorFringePixels * p.intensity * ramp;
            sampleBilinear(src, im.width, im.height, static_cast<float>(x) - lineShift - fringe,
                           static_cast<float>(y), red);
            sampleBilinear(src, im.width, im.height, static_cast<float>(x) - lineShift + fringe,
                           static_cast<float>(y), blue);
            dst[x * 4 + 0] = sample[0] + grain + band + (red[0] - sample[0]) * 0.25f;
            dst[x * 4 + 1] = sample[1] + grain * 0.55f - band * 0.35f;
            dst[x * 4 + 2] = sample[2] + grain - band + (blue[2] - sample[2]) * 0.25f;
            // Alpha is deliberately not written.
        }
    }
}

void applyDropouts(FloatRgbaView im, const DropoutParams& p,
                   uint64_t globalSeed, int64_t frameIndex) {
    if (!valid(im) || !p.enabled || p.intensity <= 0.0f || p.densityPerFrame <= 0.0f)
        return;
    const uint64_t seed = globalSeed ^ p.seedOffset;
    const int count = std::clamp(static_cast<int>(std::round(p.densityPerFrame * p.intensity)), 0, 128);
    for (int i = 0; i < count; ++i) {
        const uint64_t k = keyFor(seed, frameIndex, 0x301u, i, 0);
        const int y = static_cast<int>(unit01(k) * im.height) % im.height;
        const float lenU = unit01(keyFor(seed, frameIndex, 0x302u, i, 0));
        const float lenF = std::clamp(p.minLengthFraction +
                          (p.maxLengthFraction - p.minLengthFraction) * lenU, 0.001f, 1.0f);
        const int length = std::clamp(static_cast<int>(std::round(lenF * im.width)), 1, im.width);
        const int x0 = static_cast<int>(unit01(keyFor(seed, frameIndex, 0x303u, i, 0)) *
                                       std::max(1, im.width - length + 1));
        const int rows = std::clamp(static_cast<int>(std::round(p.heightPixels)), 1, im.height);
        const bool white = unit01(keyFor(seed, frameIndex, 0x304u, i, 0)) < p.whiteProbability;
        const float value = white ? 0.92f : 0.025f;
        for (int yy = y; yy < std::min(im.height, y + rows); ++yy) {
            float* row = im.pixels + static_cast<size_t>(yy) * im.rowStrideFloats;
            for (int x = x0; x < std::min(im.width, x0 + length); ++x) {
                const float a = std::clamp(p.intensity, 0.0f, 1.0f);
                for (int c = 0; c < 3; ++c)
                    row[x * 4 + c] = row[x * 4 + c] * (1.0f - a) + value * a;
            }
        }
    }
}
} // namespace vhs90
