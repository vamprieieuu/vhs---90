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
// Direct port of VHS6RIFT's VHS_Hash2D / VHS_HashFloat implementation.
inline std::uint32_t vhs6RiftHash(std::uint32_t x) noexcept {
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    return (x >> 16) ^ x;
}
inline float vhs6RiftHash2D(int x, int y, std::uint32_t seed) noexcept {
    const std::uint32_t h = seed ^
        (static_cast<std::uint32_t>(x) * 374761393u) ^
        (static_cast<std::uint32_t>(y) * 668265263u);
    return static_cast<float>(vhs6RiftHash(h) & 0x00FFFFFFu) / 16777216.0f;
}
inline float smoothHash2D(float x, float y, std::uint32_t seed) noexcept {
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const float tx0 = x - static_cast<float>(x0);
    const float ty0 = y - static_cast<float>(y0);
    const float tx = tx0 * tx0 * (3.0f - 2.0f * tx0);
    const float ty = ty0 * ty0 * (3.0f - 2.0f * ty0);
    const auto sample = [seed](int sx, int sy) noexcept {
        return hash01(static_cast<std::uint32_t>(sx),
                      static_cast<std::uint32_t>(sy), seed, 0);
    };
    const float a0 = sample(x0, y0), a1 = sample(x0 + 1, y0);
    const float b0 = sample(x0, y0 + 1), b1 = sample(x0 + 1, y0 + 1);
    const float a = a0 + (a1 - a0) * tx;
    const float b = b0 + (b1 - b0) * tx;
    return a + (b - a) * ty;
}

// Deterministic NTSC-R row_speckles port. Geometric inter-arrival sampling
// creates short horizontal transients; a smooth spatial offset advects the
// pattern so it travels instead of being regenerated as frame-wide flicker.
inline float fbmNoise1D(float x, std::uint32_t seed) noexcept {
    float value = 0.0f, amplitude = 1.0f, normalization = 0.0f, frequency = 1.0f;
    for (std::uint32_t octave = 0; octave < 3; ++octave) {
        value += (valueNoise1D(x * frequency,
            seed + octave * 0x9e3779b9u, 0) - 0.5f) * amplitude;
        normalization += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }
    return normalization > 0.0f ? value / normalization : 0.0f;
}

inline float gaussianSmooth2D(float x, float y, std::uint32_t seed) noexcept {
    const float u1 = std::max(1.0e-6f, smoothHash2D(x, y, seed));
    const float u2 = smoothHash2D(x + 37.0f, y + 19.0f, seed + 0x9e3779b9u);
    const float value = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * kPi * u2);
    return std::max(-3.0f, std::min(3.0f, value));
}

inline float ntscRowSpeckles(float xPx, float yPx, float intensity,
                             float anisotropy, float cellW, float cellH,
                             float driftX, float driftY, float timeSeconds,
                             std::uint32_t seed) noexcept {
    intensity = clamp01(intensity);
    anisotropy = clamp01(anisotropy);
    if (intensity <= 0.0f) return 0.0f;

    const float patternX = xPx - driftX * timeSeconds * std::max(1.0f, cellW);
    const float patternY = yPx - driftY * timeSeconds * std::max(1.0f, cellH);
    const float lineRandom = smoothHash2D(0.0f, patternY / 4.0f, seed + 0x6a09e667u);
    const float denominator = std::max(1.0e-6f,
        intensity * (1.0f - intensity) * (1.0f - anisotropy));
    const float logistic = std::exp(std::max(-20.0f, std::min(20.0f,
        (lineRandom - intensity) / denominator)));
    const float lineIntensity = clamp01(
        (anisotropy / (1.0f + logistic) + intensity * (1.0f - anisotropy)) * 0.125f);
    if (lineIntensity < 1.0e-5f) return 0.0f;

    const int block = static_cast<int>(std::floor(patternX / 64.0f));
    float result = 0.0f;
    for (int eventBlock = block - 1; eventBlock <= block; ++eventBlock) {
        int pixelIndex = eventBlock * 64;
        for (int eventIndex = 0; eventIndex < 24; ++eventIndex) {
            const float u = std::max(1.0e-6f, std::min(1.0f - 1.0e-6f,
                hash01(static_cast<std::uint32_t>(eventIndex),
                       static_cast<std::uint32_t>(eventBlock), seed + 0x243f6a88u, 0)));
            const float gapValue = std::log(1.0f - u) / std::log(1.0f - lineIntensity);
            pixelIndex += std::max(1, static_cast<int>(std::floor(gapValue)));
            if (pixelIndex >= (eventBlock + 1) * 64) break;

            const float length = 8.0f + hash01(static_cast<std::uint32_t>(eventIndex + 31),
                static_cast<std::uint32_t>(eventBlock), seed + 0x85a308d3u, 0) * 56.0f;
            const float frequency = length * (3.0f + hash01(static_cast<std::uint32_t>(eventIndex + 67),
                static_cast<std::uint32_t>(eventBlock), seed + 0x13198a2eu, 0) * 2.0f);
            const float offset = patternX - static_cast<float>(pixelIndex);
            if (offset >= 0.0f && offset < length) {
                const float amplitude = hash01(static_cast<std::uint32_t>(std::floor(patternX)) +
                    static_cast<std::uint32_t>(eventIndex * 19),
                    static_cast<std::uint32_t>(std::floor(patternY)),
                    seed + 0x03707344u, 0) * 3.0f - 1.0f;
                const float envelope = std::pow(1.0f - offset / length, 2.0f);
                result += std::cos((offset * kPi) / std::max(1.0f, frequency))
                    * envelope * amplitude;
            }
            pixelIndex += 1;
        }
    }
    return result;
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
inline float wrapMovingCenter(float center,float halfExtent,float displacement) noexcept {
    const float half=std::max(0.0f,halfExtent);
    const float span=1.0f+2.0f*half;
    float phase=std::fmod(center+half+displacement,span);
    if(phase<0.0f) phase+=span;
    // The region enters from one edge, traverses the full frame, then exits
    // before wrapping back in from the opposite edge. This is the moving-band
    // behavior used by the VHS6RIFT reference, generalized to any region.
    return phase-half;
}
inline float evaluateMask(const DamageRegion& r, float u, float v,
                          double timeSeconds) noexcept {
    const float elapsed=static_cast<float>(timeSeconds);
    float cx=r.centerX,cy=r.centerY;
    if(r.kind==RegionKind::TrackingNoise) {
        if(std::fabs(r.motionXPerSecond)>1.0e-7f)
            cx=wrapMovingCenter(cx,std::max(0.0f,r.width)*0.5f,r.motionXPerSecond*elapsed);
        if(std::fabs(r.motionYPerSecond)>1.0e-7f)
            cy=wrapMovingCenter(cy,std::max(0.0f,r.height)*0.5f,r.motionYPerSecond*elapsed);
    } else {
        // Keep head-switch regions freely placeable, including partially or
        // fully outside the raster; only tracking bands use cyclic traversal.
        cx+=r.motionXPerSecond*elapsed;
        cy+=r.motionYPerSecond*elapsed;
    }

    const float dx=u-cx,dy=v-cy;
    const float ca=std::cos(r.angleRadians),sa=std::sin(r.angleRadians);
    const float along=std::fabs(dx*ca+dy*sa);
    const float across=std::fabs(-dx*sa+dy*ca);
    switch (r.shape) {
        case RegionShape::HorizontalBand:
            return edgeMask(across,std::max(0.0f,r.height)*0.5f,r.feather)
                 * edgeMask(along,std::max(0.0f,r.width)*0.5f,r.feather);
        case RegionShape::VerticalBand:
            return edgeMask(along,std::max(0.0f,r.width)*0.5f,r.feather)
                 * edgeMask(across,std::max(0.0f,r.height)*0.5f,r.feather);
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
    const float t = static_cast<float>(p.timeSeconds);
    // Move the random field continuously through space. The lattice remains
    // deterministic (seed + absolute time), while adjacent frames are correlated.
    const float noiseDriftX = t * r.noiseDriftXCellsPerSecond;
    const float noiseDriftY = t * r.noiseDriftYCellsPerSecond;
    const float n = smoothHash2D(static_cast<float>(cx) - noiseDriftX,
                                 static_cast<float>(cy) - noiseDriftY,
                                 regionSeed);
    const float rowN = smoothHash2D(-noiseDriftX,
                                    static_cast<float>(iy) / cellH - noiseDriftY,
                                    regionSeed + 0x68bc21ebu);
    const float rowWave = std::sin((float(iy) / std::max(1, src.height)) *
                                   r.waveFrequency * 2.0f * kPi + t * 2.0f * kPi);
    float movingCenterX=r.centerX,movingCenterY=r.centerY;
    if(r.kind==RegionKind::TrackingNoise) {
        if(std::fabs(r.motionXPerSecond)>1.0e-7f)
            movingCenterX=wrapMovingCenter(movingCenterX,std::max(0.0f,r.width)*0.5f,r.motionXPerSecond*t);
        if(std::fabs(r.motionYPerSecond)>1.0e-7f)
            movingCenterY=wrapMovingCenter(movingCenterY,std::max(0.0f,r.height)*0.5f,r.motionYPerSecond*t);
    }
    const float u=(x+0.5f)/float(std::max(1,src.width));
    const float v=(y+0.5f)/float(std::max(1,src.height));
    const float localX=clamp01((u-(movingCenterX-r.width*0.5f))/std::max(1.0e-5f,r.width));
    const float localY=clamp01((v-(movingCenterY-r.height*0.5f))/std::max(1.0e-5f,r.height));
    const float localProgress=r.shape==RegionShape::VerticalBand?localX:localY;
    const float bandFactor=std::sin(localProgress*kPi);
    float sourceNoise=0.0f,sourceSnow=0.0f,sourceSnowValue=0.0f;
    float sourceChromaAttenuation=1.0f;

    float shiftX = (rowN - 0.5f) * r.maxShiftPx + rowWave * r.waveAmplitudePx;
    const float shiftY = (n - 0.5f) * r.verticalShiftPx;
    if (r.kind == RegionKind::TrackingNoise) {
        const float servo=smoothServoNoise(y,r.servoSmoothnessPx,regionSeed+0x165667b1u,p.frame,r.servoEpochFrames);
        const float tearEvent=hashCell(cx,cy,regionSeed+0xd3a2646cu,p.frame);
        const float tearDirection=hashCell(cx+7,cy+11,regionSeed+0x9e3779b9u,p.frame);
        switch (r.trackingProfile) {
            case TrackingProfile::ServoSweep:
                // VHS6RIFT-derived clustered scanline jitter, servo drift and discrete tear slips.
                shiftX += (servo-0.5f)*2.0f*r.servoDriftPx;
                if (tearEvent<clamp01(r.tearProbability)) shiftX += (tearDirection-0.5f)*2.0f*r.tearShiftPx;
                break;
            case TrackingProfile::RfWave: {
                // NTSC-R-derived anisotropic RF wave: smooth row-correlated motion plus fine horizontal snow.
                const float broad=valueNoise1D(y/std::max(1.0f,r.servoSmoothnessPx*2.5f),regionSeed+0x51ed270bu,p.frame/4);
                const float fine=rowN-0.5f;
                shiftX += (broad-0.5f)*2.0f*r.servoDriftPx + fine*r.waveAmplitudePx*0.65f;
                if (tearEvent<clamp01(r.tearProbability*0.45f)) shiftX += (tearDirection-0.5f)*r.tearShiftPx;
                break;
            }
            case TrackingProfile::CompositeRibbon: {
                // Crates-style layered tracking ribbon: multi-scale row field and segmented displacement jumps.
                const float f1=valueNoise1D(y/std::max(1.0f,r.servoSmoothnessPx),regionSeed+0x243f6a88u,p.frame/3);
                const float f2=valueNoise1D(y/std::max(1.0f,r.servoSmoothnessPx*0.22f),regionSeed+0xb7e15162u,p.frame);
                const float ribbon=(f1-0.5f)*1.35f+(f2-0.5f)*0.65f;
                const float segment=hashCell(cx/3,cy,regionSeed+0x94d049bbu,p.frame);
                shiftX += ribbon*r.servoDriftPx*2.0f;
                if(segment<clamp01(r.tearProbability)) shiftX += (tearDirection-0.5f)*2.0f*r.tearShiftPx;
                break;
            }
            case TrackingProfile::TapeFlutter: {
                // VHSPro-inspired slow tape flutter, independent from the shared Snow and Edge Wave stages.
                const float slow=std::sin(static_cast<float>(p.timeSeconds)*1.7f + y/std::max(1.0f,r.servoSmoothnessPx*3.0f));
                const float slip=valueNoise1D(y/std::max(1.0f,r.servoSmoothnessPx*1.8f),regionSeed+0x68bc21ebu,p.frame/12);
                shiftX += slow*r.servoDriftPx*0.9f + (slip-0.5f)*r.servoDriftPx*1.4f;
                if(tearEvent<clamp01(r.tearProbability*0.25f)) shiftX += (tearDirection-0.5f)*r.tearShiftPx*1.5f;
                break;
            }
            case TrackingProfile::FourEdgeCrawl: {
                // Damage-Lab spatial model: irregular edge crawl with independently seeded row/column structure.
                const float edgeNoise=valueNoise1D((r.shape==RegionShape::VerticalBand?x:y)/std::max(1.0f,r.servoSmoothnessPx),regionSeed+0x85ebca6bu,p.frame/2);
                shiftX += (edgeNoise-0.5f)*r.servoDriftPx*2.4f + (rowN-0.5f)*r.waveAmplitudePx;
                if(tearEvent<clamp01(r.tearProbability)) shiftX += (tearDirection-0.5f)*r.tearShiftPx*1.25f;
                break;
            }
            case TrackingProfile::Vhs6RiftBand: {
                // Direct VHS6RIFT port: source VHS_Hash2D scanline jitter
                // at 60 Hz and moving-band snow at 240 Hz. VHS-90 adds the
                // requested editable region position, size and motion.
                const float lineHash=vhs6RiftHash2D(iy,
                    static_cast<int>(t*60.0f),regionSeed+202u);
                shiftX=std::round((lineHash-0.5f)*r.maxShiftPx*bandFactor);
                const float snowHash=vhs6RiftHash2D(x,iy,
                    regionSeed+static_cast<std::uint32_t>(t*240.0f));
                if(bandFactor>0.4f&&snowHash>0.55f) {
                    sourceSnow=0.70f;
                    sourceSnowValue=snowHash;
                }
                break;
            }
            case TrackingProfile::VhsMainTracking: {
                // VHS-main source port: Gaussian luma disturbance, four-line-correlated
                // horizontal sampling jitter and chroma attenuation. The existing
                // region geometry generalizes its original full-width moving band.
                const float trackPhase = 1.0f - std::fabs(localProgress * 2.0f - 1.0f);
                const int rowBlock = cy & ~3;
                const float rowJitter = smoothHash2D(13.0f,
                    static_cast<float>(rowBlock)-noiseDriftY*t,regionSeed+0x4f1bbcdcu)*2.0f-1.0f;
                shiftX = rowJitter * r.maxShiftPx * trackPhase;
                sourceNoise = gaussianSmooth2D(static_cast<float>(cx)-noiseDriftX*t,
                    static_cast<float>(cy)-noiseDriftY*t,regionSeed+0x9e3779b9u)
                    * 0.25f * trackPhase;
                sourceChromaAttenuation = 1.0f - 0.5f * trackPhase;
                break;
            }
            case TrackingProfile::NtscRTracking: {
                // NTSC-R scalar port: gradient-noise row displacement, luma FBM-like noise,
                // bottom-to-top intensity ramp generalized to the selected user region,
                // and sparse 8..64 px transient events.
                const float intensityScale=localProgress;
                const float regionRows=std::max(1.0f,r.shape==RegionShape::VerticalBand
                    ?r.width*float(std::max(1,src.width)):r.height*float(std::max(1,src.height)));
                const float bandwidthScale=std::clamp(r.servoSmoothnessPx/12.0f,0.25f,2.0f);
                const float noiseOffset=smoothHash2D(t*r.noiseDriftXCellsPerSecond,0.0f,
                    regionSeed+0x4e545343u)*regionRows;
                const float shiftNoise=valueNoise1D((localProgress*regionRows+noiseOffset)*0.5f,
                    regionSeed+0x53484946u,0)-0.5f;
                shiftX+=shiftNoise*intensityScale*r.waveAmplitudePx*0.25f*bandwidthScale;
                const float videoNoise=fbmNoise1D(x*(0.25f/bandwidthScale)-noiseDriftX,
                    regionSeed+0x564e4f49u)*intensityScale*intensityScale*r.noiseAmount*4.0f;
                const float speckles=ntscRowSpeckles(x,y,
                    r.snowIntensity*intensityScale*intensityScale,r.snowAnisotropy,
                    cellW*bandwidthScale,cellH,noiseDriftX,noiseDriftY,t,regionSeed+0x53504543u);
                sourceNoise=videoNoise+speckles*0.28f;
                if(tearEvent<clamp01(r.tearProbability*intensityScale))
                    shiftX+=(tearDirection-0.5f)*r.tearShiftPx*intensityScale;
                break;
            }
        }
    } else {
        const float headProgress=clamp01(localProgress);
        if(r.headSwitchProfile==HeadSwitchProfile::Vhs6Rift) {
            // VHS6RIFT source: quadratic head-switch line jitter and localized
            // grayscale snow, advection-enabled inside the user-positioned mask.
            shiftX+=std::pow(headProgress,2.0f)*r.maxShiftPx;
            const int snowX=static_cast<int>(std::floor(x-noiseDriftX*cellW));
            const int snowY=static_cast<int>(std::floor(y-noiseDriftY*cellH));
            const float snowHash=vhs6RiftHash2D(snowX,snowY,
                regionSeed+static_cast<std::uint32_t>(t*240.0f)+55u);
            if(snowHash>1.0f-headProgress*0.7f) {
                sourceSnow=1.0f;
                sourceSnowValue=snowHash;
            }
        } else if(r.headSwitchProfile==HeadSwitchProfile::NtscR) {
            // NTSC-R source: 1.5-power row displacement plus its optional
            // mid-line transient at the leading edge of the selected region.
            shiftX+=r.maxShiftPx*std::pow(headProgress,1.5f);
            const float rowJitter=smoothHash2D(0.0f,
                static_cast<float>(iy)-noiseDriftY*t*cellH,regionSeed+0x4e545343u)-0.5f;
            shiftX+=rowJitter*r.servoDriftPx;
            const float regionRows=std::max(1.0f,r.shape==RegionShape::VerticalBand
                ?r.width*float(std::max(1,src.width))
                :r.height*float(std::max(1,src.height)));
            if(r.tearProbability>0.0f&&headProgress*regionRows<1.0f) {
                const float jitterRand=(hashCell(0,iy,regionSeed+0x4d49444cu,0)
                    +hashCell(1,iy,regionSeed+0x4d49444du,0))*0.5f;
                const float copyStart=float(src.width)*(0.5f+(jitterRand-0.5f)*0.12f);
                if(x<copyStart) shiftX=0.0f;
                const float transientLength=16.0f;
                const float transientX=x-copyStart;
                if(transientX>=0.0f&&transientX<transientLength) {
                    const float transientIntensity=(hashCell(2,iy,regionSeed+0x4d49444eu,0)+0.5f)*0.5f;
                    sourceNoise+=(1.0f-transientX/transientLength)
                        *(1.0f-transientX/transientLength)
                        *(1.0f-transientX/transientLength)*transientIntensity;
                }
            }
        } else {
            const float tear=hashCell(0,iy/2,regionSeed+0x9e3779b9u,p.frame);
            shiftX+=(tear-0.5f)*r.maxShiftPx*1.75f;
        }
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

    if (sourceChromaAttenuation < 0.99999f) {
        const float luma = 0.299f * sr + 0.587f * sg + 0.114f * sb;
        sr = luma + (sr - luma) * sourceChromaAttenuation;
        sg = luma + (sg - luma) * sourceChromaAttenuation;
        sb = luma + (sb - luma) * sourceChromaAttenuation;
    }

    float noise = sourceNoise;
    if(r.trackingProfile!=TrackingProfile::NtscRTracking && r.trackingProfile!=TrackingProfile::VhsMainTracking && n<clamp01(r.noiseDensity)) {
        const float signedNoise = smoothHash2D(static_cast<float>(cx + 41) - noiseDriftX,
                                                static_cast<float>(cy + 97) - noiseDriftY,
                                                regionSeed + 0x85ebca6bu) - 0.5f;
        noise+=signedNoise*r.noiseAmount;
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
    if(sourceSnow>0.0f) {
        sr=sr*(1.0f-sourceSnow)+sourceSnowValue*sourceSnow;
        sg=sg*(1.0f-sourceSnow)+sourceSnowValue*sourceSnow;
        sb=sb*(1.0f-sourceSnow)+sourceSnowValue*sourceSnow;
    }

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
