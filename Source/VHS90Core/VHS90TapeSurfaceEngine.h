#pragma once
#include "VHS90SignalKernels.h"
#include <cstdint>
namespace vhs90 {
struct TapeSurfaceParams {
    float defectDensity=0.0f, oxideLoss=0.0f, scratchDensity=0.0f;
    float creaseStrength=0.0f, debrisAmount=0.0f, edgeLift=0.0f;
    float surfaceRoughness=0.0f, tapeAdvancePerFrame=0.012f;
    float moldAmount=0.0f, vaselineAmount=0.0f, pinholeAmount=0.0f, flakeAmount=0.0f;
    float alongCellSize=0.018f, acrossCellSize=0.035f;
    std::uint32_t seed=7936u;
    std::uint32_t tapeSpeed=0; // 0 SP, 1 LP, 2 EP
    float trackingOffsetTracks=0.0f;
    int rasterActiveLines=480;
    int rasterVbiLines=40;
    std::int64_t frame=0;
    bool premultipliedAlpha=true;
};
struct TapeSurfaceSample {
    float magnetization=1.0f, spacingLift=0.0f, roughness=0.0f, creaseLoss=0.0f, film=0.0f;
    float shed=0.0f;
};
// NTSCRT physical tape coordinates: x is millimetres along the tape and y
// is millimetres across its 12.65 mm width. Deterministic across render order.
struct TapePointMm { double x=0.0; double y=0.0; };
double tapeWidthMm() noexcept;
double tapeTrackStartMm() noexcept;
double tapeTrackLengthMm() noexcept;
double tapeTrackPitchUm(std::uint32_t speed) noexcept;
double tapeTrackOffsetLines(std::uint32_t speed) noexcept;
TapePointMm tapeTrackPoint(std::int64_t field,double alongMm,double acrossMm,std::uint32_t speed) noexcept;
enum class TapeDamageControl { Wear, Dropouts, Scratches, EdgeDamage, Creases, TransportFlutter };
float ntscrtTapeAmount(TapeDamageControl control,float knob) noexcept;
float ntscrtTapeKnobForAmount(TapeDamageControl control,float amount) noexcept;
float ntscrtTapeAmountIn027(TapeDamageControl control,float knob) noexcept;
float ntscrtTransportFlutterAmount(float knob) noexcept;
float ntscrtTapeWearAmount(float knob) noexcept;
float ntscrtTapeDropoutAmount(float knob) noexcept;
float ntscrtTapeScratchAmount(float knob) noexcept;
float ntscrtTapeEdgeAmount(float knob) noexcept;
float ntscrtTapeCreaseAmount(float knob) noexcept;
std::uint32_t ntscrtTapeSeed(std::uint32_t tapeSeed) noexcept;
struct TapeMaterialControls {
    float master=0.0f;
    float wear=100.0f, dropouts=100.0f, scratches=100.0f, creases=100.0f, edgeDamage=100.0f;
    float mold=100.0f, vaseline=100.0f, sticky=100.0f;
    float surfaceRoughness=0.0f, pinholes=0.0f, flakes=0.0f;
    std::uint32_t cassetteSeed=1u, tapeSpeed=0u;
    float trackingOffsetTracks=0.0f;
    std::int64_t frame=0;
};
TapeSurfaceParams makeTapeSurfaceParams(const TapeMaterialControls& controls) noexcept;
TapeSurfaceSample sampleTapeSurfaceAtMm(double xMm, float yMm, const TapeSurfaceParams& params, bool detail=true, bool flakes=true) noexcept;
TapeSurfaceSample sampleTapeSurface(float u,float v,const TapeSurfaceParams& params) noexcept;
bool processTapeSurface(ConstImageView src,ImageView dst,const TapeSurfaceParams& params) noexcept;
} // namespace vhs90
