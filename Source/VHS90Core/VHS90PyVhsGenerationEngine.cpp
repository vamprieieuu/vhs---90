#include "VHS90PyVhsGenerationEngine.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace vhs90 {
namespace {
constexpr float kPi = 3.14159265358979323846f;
inline float sinc(float x) noexcept {
    if (std::fabs(x) < 1.0e-5f) return 1.0f;
    const float a = kPi * x;
    return std::sin(a) / a;
}
inline float lanczos4(float x) noexcept {
    x = std::fabs(x);
    return x < 4.0f ? sinc(x) * sinc(x * 0.25f) : 0.0f;
}
inline float noiseGaussian(int x, int y, std::uint32_t seed, std::int64_t frame) noexcept {
    const float u1 = std::max(1.0e-7f, hash01(std::uint32_t(x) * 2u, std::uint32_t(y), seed, frame));
    const float u2 = hash01(std::uint32_t(x) * 2u + 1u, std::uint32_t(y), seed ^ 0x9E3779B9u, frame);
    return std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * kPi * u2);
}
void resizeHorizontal(const std::vector<float>& src, std::vector<float>& dst,
                      int width, int height, int targetWidth) {
    dst.assign(std::size_t(targetWidth) * height, 0.0f);
    if (targetWidth == width) { dst = src; return; }
    const float scale = float(width) / float(targetWidth);
    const float filterScale = std::max(1.0f, scale);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < targetWidth; ++x) {
            const float center = (float(x) + 0.5f) * scale - 0.5f;
            const int left = int(std::floor(center - 4.0f * filterScale));
            const int right = int(std::ceil(center + 4.0f * filterScale));
            float sum = 0.0f, weight = 0.0f;
            for (int sx = left; sx <= right; ++sx) {
                const int clamped = std::clamp(sx, 0, width - 1);
                const float w = lanczos4((center - float(sx)) / filterScale) / filterScale;
                sum += src[std::size_t(y) * width + clamped] * w;
                weight += w;
            }
            dst[std::size_t(y) * targetWidth + x] = std::fabs(weight) > 1.0e-7f ? sum / weight : 0.0f;
        }
    }
}
void compressChannel(std::vector<float>& channel, int width, int height, float factor,
                     float gaussianSigma, float gaussianMean, float uniformNoise,
                     std::uint32_t seed, std::int64_t frame, std::uint32_t salt) {
    const int reduced = std::clamp(int(std::lround(float(width) / factor)), 1, width);
    std::vector<float> low, restored;
    resizeHorizontal(channel, low, width, height, reduced);
    const float inv255 = 1.0f / 255.0f;
    for (int y = 0; y < height; ++y) for (int x = 0; x < reduced; ++x) {
        const std::size_t i = std::size_t(y)*reduced+x;
        low[i] += (gaussianMean + gaussianSigma*noiseGaussian(x,y,seed^salt,frame))*inv255;
        low[i] += (hash01(std::uint32_t(x),std::uint32_t(y),seed^(salt+0x9E3779B9u),frame)*2.0f-1.0f)*uniformNoise*inv255;
    }
    resizeHorizontal(low, restored, reduced, height, width);
    channel.swap(restored);
}
}

bool processPyVhsGeneration(ConstImageView src, ImageView dst,
                            const PyVhsGenerationParams& p) noexcept {
    if (!src.rgba || !dst.rgba || src.width <= 0 || src.height <= 0 ||
        dst.width != src.width || dst.height != src.height ||
        src.rowStrideFloats < src.width * 4 || dst.rowStrideFloats < dst.width * 4 ||
        src.rgba == dst.rgba) return false;
    const float lc = std::clamp(std::isfinite(p.lumaCompression) ? p.lumaCompression : 1.0f, 1.0f, 8.0f);
    const float cc = std::clamp(std::isfinite(p.chromaCompression) ? p.chromaCompression : 1.0f, 1.0f, 100.0f);
    const float sigma = std::clamp(std::isfinite(p.lumaNoiseSigma) ? p.lumaNoiseSigma : 0.0f, 0.0f, 100.0f) *
                         std::clamp(std::isfinite(p.generation) ? p.generation : 1.0f, 0.0f, 10.0f);
    const float mean = std::clamp(std::isfinite(p.lumaNoiseMean) ? p.lumaNoiseMean : 0.0f, -100.0f, 100.0f) *
                       std::clamp(std::isfinite(p.generation) ? p.generation : 1.0f, 0.0f, 10.0f);
    const float cnoise = std::clamp(std::isfinite(p.chromaNoise) ? p.chromaNoise : 0.0f, 0.0f, 100.0f) *
                         std::clamp(std::isfinite(p.generation) ? p.generation : 1.0f, 0.0f, 10.0f);
    const float wave = std::clamp(std::isfinite(p.chromaWave) ? p.chromaWave : 0.0f, 0.0f, 2.0f);
    const float freq = std::clamp(std::isfinite(p.waveFrequency) ? p.waveFrequency : 0.22f, 0.01f, 4.0f);
    if (lc <= 1.0001f && cc <= 1.0001f && sigma <= 0.0f && std::fabs(mean) <= 0.0f &&
        cnoise <= 0.0f && wave <= 0.0f) {
        for (int y = 0; y < src.height; ++y)
            std::copy_n(src.rgba + y * src.rowStrideFloats, src.width * 4, dst.rgba + y * dst.rowStrideFloats);
        return true;
    }
    try {
        const int w = src.width, h = src.height;
        const std::size_t n = std::size_t(w) * h;
        std::vector<float> yy(n), cr(n), cb(n);
        for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
            const float* px = src.rgba + y * src.rowStrideFloats + x * 4;
            float r = px[0], g = px[1], b = px[2];
            if (p.premultipliedAlpha && px[3] > 1.0e-6f) { r /= px[3]; g /= px[3]; b /= px[3]; }
            const float l = 0.299f*r + 0.587f*g + 0.114f*b;
            const std::size_t i = std::size_t(y)*w+x;
            yy[i] = l;
            cr[i] = (r-l)*0.713f;
            cb[i] = (b-l)*0.564f;
        }
        compressChannel(yy,w,h,lc,sigma,mean,0.0f,p.seed,p.frame,0x4C554D41u);
        compressChannel(cr,w,h,cc,0.0f,0.0f,cnoise,p.seed,p.frame,0x43524E4Fu);
        compressChannel(cb,w,h,cc,0.0f,0.0f,cnoise,p.seed,p.frame,0x43424E4Fu);
        std::vector<float> rowCr(static_cast<std::size_t>(w));
        std::vector<float> rowCb(static_cast<std::size_t>(w));
        for (int y = 0; y < h; ++y) {
            const float phase = float(y) * freq * 2.0f * kPi / float(std::max(1,h)) +
                                float(hash32(std::uint32_t(y) ^ p.seed) & 0xffffu) * (1.0f/65535.0f);
            const float rowWave = wave * std::sin(phase + float(p.frame) * 0.17f);
            for (int x = 0; x < w; ++x) {
                const std::size_t i = std::size_t(y)*w+x;
                yy[i] = std::clamp(yy[i], 0.0f, 1.0f);
            }
            if (wave > 0.0f) {
                const std::size_t rowBase = std::size_t(y)*w;
                std::copy_n(cr.data()+rowBase,w,rowCr.data());
                std::copy_n(cb.data()+rowBase,w,rowCb.data());
                for (int x = 0; x < w; ++x) {
                    const float sx = float(x) + rowWave;
                    const int x0 = int(std::floor(sx));
                    const float t = sx - float(x0);
                    const int a = std::clamp(x0,0,w-1), b = std::clamp(x0+1,0,w-1);
                    cr[rowBase+x] = rowCr[std::size_t(a)]*(1.0f-t)+rowCr[std::size_t(b)]*t;
                    cb[rowBase+x] = rowCb[std::size_t(a)]*(1.0f-t)+rowCb[std::size_t(b)]*t;
                }
            }
        }
        for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
            const std::size_t i = std::size_t(y)*w+x;
            const float l = yy[i], r = std::clamp(l + cr[i]/0.713f, 0.0f, 1.0f);
            const float b = std::clamp(l + cb[i]/0.564f, 0.0f, 1.0f);
            const float g = std::clamp((l - 0.299f*r - 0.114f*b)/0.587f, 0.0f, 1.0f);
            float* out = dst.rgba + y*dst.rowStrideFloats + x*4;
            const float a = src.rgba[y*src.rowStrideFloats+x*4+3];
            out[0] = p.premultipliedAlpha ? r*a : r;
            out[1] = p.premultipliedAlpha ? g*a : g;
            out[2] = p.premultipliedAlpha ? b*a : b;
            out[3] = a;
        }
        return true;
    } catch (...) { return false; }
}
} // namespace vhs90
