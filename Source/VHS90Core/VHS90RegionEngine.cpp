#include "VHS90RegionEngine.h"
#include <algorithm>
#include <cmath>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;

inline float clamp01(float v) noexcept {
    return std::max(0.0f, std::min(1.0f, v));
}
inline float smooth01(float t) noexcept {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}
inline float edgeMask(float distance, float halfExtent, float feather) noexcept {
    halfExtent = std::max(0.0f, halfExtent);
    if (halfExtent <= 0.0f) return 0.0f;
    if (feather <= 0.0f) return distance <= halfExtent ? 1.0f : 0.0f;
    const float inner = std::max(0.0f, halfExtent - feather);
    if (distance <= inner) return 1.0f;
    if (distance >= halfExtent) return 0.0f;
    return 1.0f - smooth01((distance - inner) / std::max(feather, 1.0e-6f));
}
inline float sampleChannel(ConstImageView im, float x, float y, int channel) noexcept {
    x = std::max(0.0f, std::min(float(im.width - 1), x));
    y = std::max(0.0f, std::min(float(im.height - 1), y));
    const int x0 = int(std::floor(x));
    const int y0 = int(std::floor(y));
    const int x1 = std::min(im.width - 1, x0 + 1);
    const int y1 = std::min(im.height - 1, y0 + 1);
    const float tx = x - float(x0);
    const float ty = y - float(y0);
    const float* p00 = im.rgba + y0 * im.rowStrideFloats + x0 * 4;
    const float* p10 = im.rgba + y0 * im.rowStrideFloats + x1 * 4;
    const float* p01 = im.rgba + y1 * im.rowStrideFloats + x0 * 4;
    const float* p11 = im.rgba + y1 * im.rowStrideFloats + x1 * 4;
    const float a = p00[channel] + (p10[channel] - p00[channel]) * tx;
    const float b = p01[channel] + (p11[channel] - p01[channel]) * tx;
    return a + (b - a) * ty;
}
inline float hashCell(int x, int y, std::uint32_t seed, std::int64_t frame) noexcept {
    return hash01(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y), seed, frame);
}
inline float smoothServoNoise(float yPx,float smoothnessPx,std::uint32_t seed,
                               std::int64_t frame,int epochFrames) noexcept {
    const float rowCoordinate=yPx/std::max(1.0f,smoothnessPx);
    const std::int64_t span=std::max(1,epochFrames);
    const double epochValue=std::floor(static_cast<double>(frame)/static_cast<double>(span));
    const auto epoch=static_cast<std::int64_t>(epochValue);
    const float phase=static_cast<float>((static_cast<double>(frame)-epochValue*span)/span);
    const float temporal=phase*phase*(3.0f-2.0f*phase);
    const float current=valueNoise1D(rowCoordinate,seed,epoch);
    const float next=valueNoise1D(rowCoordinate,seed,epoch+1);
    return current+(next-current)*temporal;
}
inline float evaluateMask(const DamageRegion& r, float u, float v,
                          double timeSeconds) noexcept {
    float cx = r.centerX + r.motionXPerSecond * static_cast<float>(timeSeconds);
    float cy = r.centerY + r.motionYPerSecond * static_cast<float>(timeSeconds);
    // Keep region centers outside [0,1] when needed. This lets edge-anchored
    // regions (for example Head Switching with Bottom Edge Y near 0) clip
    // naturally instead of being pulled back into the image.

    const float dx=u-cx,dy=v-cy;
    const float ca=std::cos(r.angleRadians),sa=std::sin(r.angleRadians);
    const float along=std::fabs(dx*ca+dy*sa);
    const float across=std::fabs(-dx*sa+dy*ca);
    switch (r.shape) {
        case RegionShape::HorizontalBand:
            return edgeMask(across,std::max(0.0f,r.height)*0.5f,r.feather)
                 * edgeMask(along,std::max(0.0f,r.width)*0.5f,r.feather);
        case RegionShape::VerticalBand:
            return edgeMask(across,std::max(0.0f,r.width)*0.5f,r.feather)
                 * edgeMask(along,std::max(0.0f,r.height)*0.5f,r.feather);
        case RegionShape::Rectangle:
        default:
            return edgeMask(along,std::max(0.0f,r.width)*0.5f,r.feather)
                 * edgeMask(across,std::max(0.0f,r.height)*0.5f,r.feather);
    }
}
inline void applyRegion(ConstImageView src, float x, float y, float centerAlpha,
                        const DamageRegion& r, const RegionEngineParams& p,
                        std::size_t regionIndex, RGB& current) noexcept {
    const int iy = static_cast<int>(y);
    const float cellW = std::max(1.0f, r.cellWidthPx);
    const float cellH = std::max(1.0f, r.cellHeightPx);
    const int cx = static_cast<int>(std::floor(x / cellW));
    const int cy = static_cast<int>(std::floor(y / cellH));
    const std::uint32_t regionSeed = p.seed ^ r.seedOffset ^
        static_cast<std::uint32_t>(regionIndex * 0x9e3779b9u);
    const float n = hashCell(cx, cy, regionSeed, p.frame);
    const float rowN = hashCell(0, iy, regionSeed + 0x68bc21ebu, p.frame);
    const float t = static_cast<float>(p.timeSeconds);
    const float rowWave = std::sin((float(iy) / std::max(1, src.height)) *
                                   r.waveFrequency * 2.0f * kPi + t * 2.0f * kPi);

    float shiftX = (rowN - 0.5f) * r.maxShiftPx + rowWave * r.waveAmplitudePx;
    const float shiftY = (n - 0.5f) * r.verticalShiftPx;
    if (r.kind == RegionKind::TrackingNoise) {
        const float servo=smoothServoNoise(y,r.servoSmoothnessPx,regionSeed+0x165667b1u,p.frame,r.servoEpochFrames);
        shiftX += (servo-0.5f)*2.0f*r.servoDriftPx;
        const float tearEvent=hashCell(cx,cy,regionSeed+0xd3a2646cu,p.frame);
        if (tearEvent<clamp01(r.tearProbability)) {
            const float tearDirection=hashCell(cx+7,cy+11,regionSeed+0x9e3779b9u,p.frame);
            shiftX += (tearDirection-0.5f)*2.0f*r.tearShiftPx;
        }
    } else {
        const float tear = hashCell(0, iy / 2, regionSeed + 0x9e3779b9u, p.frame);
        shiftX += (tear - 0.5f) * r.maxShiftPx * 1.75f;
    }

    const float sx = x + shiftX;
    const float sy = y + shiftY;
    float sr = sampleChannel(src, sx, sy, 0);
    float sg = sampleChannel(src, sx, sy, 1);
    float sb = sampleChannel(src, sx, sy, 2);
    const float sampleAlpha = sampleChannel(src, sx, sy, 3);

    if (p.premultipliedAlpha) {
        if (sampleAlpha > 1.0e-6f) {
            const float invA = 1.0f / sampleAlpha;
            sr = sr * invA * centerAlpha;
            sg = sg * invA * centerAlpha;
            sb = sb * invA * centerAlpha;
        } else {
            sr = current.r; sg = current.g; sb = current.b;
        }
    }

    if (std::fabs(r.chromaShiftPx) > 1.0e-6f) {
        const float red = sampleChannel(src, sx - r.chromaShiftPx, sy, 0);
        const float blue = sampleChannel(src, sx + r.chromaShiftPx, sy, 2);
        if (p.premultipliedAlpha) {
            const float ar = sampleChannel(src, sx - r.chromaShiftPx, sy, 3);
            const float ab = sampleChannel(src, sx + r.chromaShiftPx, sy, 3);
            sr = (ar > 1.0e-6f ? red / ar * centerAlpha : current.r);
            sb = (ab > 1.0e-6f ? blue / ab * centerAlpha : current.b);
        } else {
            sr = red; sb = blue;
        }
    }

    float noise = 0.0f;
    if (n < clamp01(r.noiseDensity)) {
        const float signedNoise = hashCell(cx + 41, cy + 97, regionSeed + 0x85ebca6bu, p.frame) - 0.5f;
        noise = signedNoise * r.noiseAmount;
    }

    float dropout = 0.0f;
    if (r.kind == RegionKind::HeadSwitch) {
        const float event = hashCell(cx + 13, cy + 29, regionSeed + 0xc2b2ae35u, p.frame);
        const float probability = clamp01(r.dropoutProbability);
        if (event < probability) dropout = -r.dropoutAmount;
        else if (event > 1.0f - probability * 0.15f) dropout = r.dropoutAmount * 0.35f;
    } else {
        const float event = hashCell(cx + 71, cy + 19, regionSeed + 0x27d4eb2fu, p.frame);
        if (event < clamp01(r.dropoutProbability) * 0.5f) dropout = -r.dropoutAmount * 0.5f;
    }

    const float alphaScale = p.premultipliedAlpha ? clamp01(centerAlpha) : 1.0f;
    sr += (noise + dropout) * alphaScale;
    sg += (noise + dropout) * alphaScale;
    sb += (noise + dropout) * alphaScale;

    const float amount = clamp01(r.intensity * p.globalIntensity);
    current.r += (sr - current.r) * amount;
    current.g += (sg - current.g) * amount;
    current.b += (sb - current.b) * amount;
}
} // namespace

bool processDamageRegions(ConstImageView src, ImageView dst,
                          const RegionEngineParams& params) noexcept {
    if (!src.rgba || !dst.rgba || src.width <= 0 || src.height <= 0 ||
        dst.width != src.width || dst.height != src.height ||
        src.rowStrideFloats < src.width * 4 || dst.rowStrideFloats < dst.width * 4 ||
        (params.regionCount > 0 && params.regions == nullptr)) return false;
    if (src.rgba == dst.rgba) return false;

    for (int y = 0; y < src.height; ++y) {
        const float v = (float(y) + 0.5f) / float(src.height);
        const float* srcRow = src.rgba + y * src.rowStrideFloats;
        float* dstRow = dst.rgba + y * dst.rowStrideFloats;
        for (int x = 0; x < src.width; ++x) {
            const float u = (float(x) + 0.5f) / float(src.width);
            const float* sourcePixel = srcRow + x * 4;
            const float alpha = sourcePixel[3];
            RGB out{sourcePixel[0], sourcePixel[1], sourcePixel[2]};
            if (alpha > 0.0f && params.globalIntensity > 0.0f) {
                for (std::size_t i = 0; i < params.regionCount; ++i) {
                    const DamageRegion& r = params.regions[i];
                    if (!r.enabled || r.intensity <= 0.0f) continue;
                    if (!std::isfinite(r.angleRadians)) return false;
                    const float mask = evaluateMask(r, u, v, params.timeSeconds);
                    if (mask <= 0.0f) continue;
                    RGB regionOut = out;
                    applyRegion(src, float(x), float(y), alpha, r, params, i, regionOut);
                    out.r += (regionOut.r - out.r) * mask;
                    out.g += (regionOut.g - out.g) * mask;
                    out.b += (regionOut.b - out.b) * mask;
                }
            }
            float* dstPixel = dstRow + x * 4;
            dstPixel[0] = out.r;
            dstPixel[1] = out.g;
            dstPixel[2] = out.b;
            dstPixel[3] = alpha;
        }
    }
    return true;
}

} // namespace vhs90
