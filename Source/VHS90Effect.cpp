#include "AEConfig.h"
#include "entry.h"
#include "AEFX_SuiteHelper.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_EffectCBSuites.h"
#include "AE_EffectPixelFormat.h"
#include "Param_Utils.h"
#include "VHS90SignalKernels.h"
#include "VHS90RegionEngine.h"
#include "VHS90StaticEngine.h"
#include "VHS90IIRFilter.h"
#include "VHS90CompositeEngine.h"
#include "VHS90OsdEngine.h"
#include "VHS90DeviceConnectionEngine.h"
#include "VHS90SyncEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <new>
#include <vector>

namespace {
enum ParamIndex {
    PARAM_INPUT=0, PARAM_MASTER, PARAM_SIGNAL_TOPIC, PARAM_COMPOSITE_AMOUNT,
    PARAM_SYNC_AMOUNT, PARAM_PAL_MODE, PARAM_SIGNAL_MODEL, PARAM_TAPE_SPEED, PARAM_TAPE_DETAIL, PARAM_TRACK_READ, PARAM_TRACK_MISALIGNMENT, PARAM_CHROMA_SHIFT, PARAM_CHROMA_SUBSAMPLE, PARAM_VERTICAL_CHROMA, PARAM_VERTICAL_LUMA,
    PARAM_LUMA_NOISE, PARAM_CHROMA_NOISE, PARAM_COMPOSITE_NOISE, PARAM_NOISE_HUE,
    PARAM_RF_DRIFT, PARAM_RF_HUM, PARAM_RF_AM, PARAM_AGC_PUMP, PARAM_IMPULSE_NOISE, PARAM_DROPOUTS,
    PARAM_MULTIPATH_GAIN, PARAM_MULTIPATH_DELAY, PARAM_MULTIPATH_ENSEMBLE, PARAM_SIGNAL_GHOST, PARAM_CHROMA_DELAY, PARAM_CHROMA_LOSS,
    PARAM_AFC_HUNT, PARAM_CHROMA_FLUTTER, PARAM_YC_CROSSTALK, PARAM_COLOR_PHASE_DRIFT, PARAM_COLOR_DRIFT,
    PARAM_HSYNC_NOISE, PARAM_VSYNC_NOISE, PARAM_BURST_NOISE,
    PARAM_EDGE_ENHANCE, PARAM_TAPE_FADE, PARAM_BLACK_LIFT, PARAM_TEMPORAL_GHOST, PARAM_RINGING, PARAM_RINGING_POWER, PARAM_CRT_EQUALIZER, PARAM_GRADE_CONTRAST, PARAM_GRADE_BRIGHTNESS,
    PARAM_GRADE_SATURATION, PARAM_VIGNETTE, PARAM_FLICKER,
    PARAM_LUMA_SMEAR, PARAM_BLOOM, PARAM_ANALOG_GLOW, PARAM_TAPE_SURFACE, PARAM_TAPE_TRANSPORT,
    PARAM_GEOMETRY, PARAM_INTERLACE, PARAM_VERTICAL_ROLL, PARAM_EDGE_WAVE, PARAM_EDGE_WAVE_FREQUENCY, PARAM_EDGE_WAVE_SPEED, PARAM_EDGE_WAVE_DETAIL, PARAM_SCANLINES, PARAM_SIGNAL_END, PARAM_STATIC_TOPIC, PARAM_STATIC_AMOUNT,
    PARAM_STATIC_DENSITY, PARAM_STATIC_END, PARAM_TRACKING_TOPIC, PARAM_TRACKING_AMOUNT,
    PARAM_TRACKING_SHAPE, PARAM_TRACKING_CENTER_X, PARAM_TRACKING_CENTER_Y,
    PARAM_TRACKING_WIDTH, PARAM_TRACKING_HEIGHT, PARAM_TRACKING_ANGLE, PARAM_TRACKING_MOTION_X,
    PARAM_TRACKING_MOTION_Y, PARAM_TRACKING_SERVO, PARAM_TRACKING_TEAR,
    PARAM_TRACKING_END, PARAM_HEAD_TOPIC, PARAM_HEAD_AMOUNT, PARAM_HEAD_BOTTOM_Y,
    PARAM_HEAD_HEIGHT, PARAM_HEAD_SHIFT, PARAM_HEAD_END, PARAM_HEAD_CONDITION, PARAM_TAPE_MATERIAL_TOPIC, PARAM_TAPE_WEAR, PARAM_TAPE_DROPOUTS, PARAM_TAPE_SCRATCHES, PARAM_TAPE_CREASES, PARAM_TAPE_EDGE, PARAM_TAPE_MOLD, PARAM_TAPE_VASELINE, PARAM_TAPE_STICKY, PARAM_TAPE_SEED, PARAM_TAPE_BACK_TENSION, PARAM_TAPE_NOISE, PARAM_PHYSICAL_TOPIC, PARAM_MAGNET_MOMENT, PARAM_MAGNET_DISTANCE, PARAM_GROUND_LOOP, PARAM_MAINS_HUM, PARAM_RF_FIELD, PARAM_RF_PICKUP, PARAM_PHYSICAL_END, PARAM_PYVHS_TOPIC, PARAM_PYVHS_LUMA_COMPRESSION, PARAM_PYVHS_CHROMA_COMPRESSION, PARAM_PYVHS_LUMA_NOISE, PARAM_PYVHS_CHROMA_NOISE, PARAM_PYVHS_CHROMA_WAVE, PARAM_PYVHS_GENERATION, PARAM_PYVHS_END, PARAM_OSD_TOPIC, PARAM_OSD_AMOUNT, PARAM_OSD_REC, PARAM_OSD_PLAY, PARAM_OSD_TIMESTAMP, PARAM_OSD_OPACITY, PARAM_OSD_SCALE, PARAM_OSD_GLITCH, PARAM_OSD_GHOST, PARAM_OSD_REC_BLINK, PARAM_OSD_TIME_SPEED, PARAM_OSD_YEAR, PARAM_OSD_MONTH, PARAM_OSD_DAY, PARAM_OSD_HOUR, PARAM_OSD_MINUTE, PARAM_OSD_SECOND, PARAM_OSD_REC_COLOR, PARAM_OSD_END, PARAM_VHS_STYLE_TOPIC, PARAM_VHS_STYLE, PARAM_VHS_STYLE_INTENSITY, PARAM_CAMERA_PROFILE, PARAM_CAMERA_INTENSITY, PARAM_NTSCRS_TOPIC, PARAM_NTSCRS_LOOK, PARAM_NTSCRS_INTENSITY, PARAM_NTSCRS_PHASE_NOISE, PARAM_VHS6RIFT_GRAIN, PARAM_VHS6RIFT_GRAIN_SIZE, PARAM_VHS6RIFT_NOISE_DETAIL, PARAM_VHS6RIFT_SHADOW_BIAS, PARAM_VHS6RIFT_NOISE_ENABLE, PARAM_VHS6RIFT_NOISE_SEED, PARAM_SIGNAL_HUE_NOISE, PARAM_DEVICE_CONNECTION_TOPIC, PARAM_DEVICE_CONNECTION_AMOUNT, PARAM_DEVICE_CONNECTION_DENSITY, PARAM_DEVICE_CONNECTION_SIZE, PARAM_DEVICE_CONNECTION_SPEED, PARAM_DEVICE_CONNECTION_DISTRIBUTION, PARAM_DEVICE_CONNECTION_SEED, PARAM_DEVICE_CONNECTION_END, PARAM_COUNT
};
enum ParamDiskID {
    ID_MASTER=1, ID_SIGNAL_TOPIC, ID_COMPOSITE, ID_SYNC, ID_PAL_MODE, ID_SIGNAL_MODEL, ID_TAPE_SPEED, ID_TAPE_DETAIL, ID_TRACK_READ, ID_TRACK_MISALIGNMENT, ID_CHROMA_SHIFT,
    ID_CHROMA_SUBSAMPLE, ID_VERTICAL_CHROMA, ID_VERTICAL_LUMA, ID_LUMA_NOISE, ID_CHROMA_NOISE, ID_COMPOSITE_NOISE, ID_NOISE_HUE, ID_RF_DRIFT, ID_RF_HUM, ID_RF_AM, ID_AGC_PUMP, ID_IMPULSE_NOISE, ID_DROPOUTS,
    ID_MULTIPATH_GAIN, ID_MULTIPATH_DELAY, ID_MULTIPATH_ENSEMBLE, ID_SIGNAL_GHOST, ID_CHROMA_DELAY, ID_CHROMA_LOSS,
    ID_AFC_HUNT, ID_CHROMA_FLUTTER, ID_YC_CROSSTALK, ID_COLOR_PHASE_DRIFT, ID_COLOR_DRIFT,
    ID_HSYNC_NOISE, ID_VSYNC_NOISE, ID_BURST_NOISE,
    ID_EDGE_ENHANCE, ID_TAPE_FADE, ID_BLACK_LIFT, ID_TEMPORAL_GHOST, ID_RINGING, ID_RINGING_POWER, ID_CRT_EQUALIZER, ID_GRADE_CONTRAST, ID_GRADE_BRIGHTNESS,
    ID_GRADE_SATURATION, ID_VIGNETTE, ID_FLICKER, ID_LUMA_SMEAR, ID_BLOOM, ID_ANALOG_GLOW,
    ID_TAPE_SURFACE, ID_TAPE_TRANSPORT, ID_GEOMETRY, ID_INTERLACE, ID_VERTICAL_ROLL, ID_EDGE_WAVE, ID_EDGE_WAVE_FREQUENCY, ID_EDGE_WAVE_SPEED, ID_EDGE_WAVE_DETAIL, ID_SCANLINES, ID_STATIC_TOPIC,
    ID_STATIC_AMOUNT, ID_STATIC_DENSITY, ID_TRACKING_TOPIC, ID_TRACKING_AMOUNT,
    ID_TRACKING_SHAPE, ID_TRACKING_X, ID_TRACKING_Y, ID_TRACKING_WIDTH,
    ID_TRACKING_HEIGHT, ID_TRACKING_ANGLE, ID_TRACKING_MOTION_X, ID_TRACKING_MOTION_Y,
    ID_TRACKING_SERVO, ID_TRACKING_TEAR, ID_HEAD_TOPIC, ID_HEAD_AMOUNT,
    ID_HEAD_Y, ID_HEAD_HEIGHT, ID_HEAD_SHIFT, ID_HEAD_CONDITION,
    ID_TAPE_MATERIAL_TOPIC, ID_TAPE_WEAR, ID_TAPE_DROPOUTS, ID_TAPE_SCRATCHES, ID_TAPE_CREASES, ID_TAPE_EDGE, ID_TAPE_MOLD, ID_TAPE_VASELINE, ID_TAPE_STICKY, ID_TAPE_SEED, ID_TAPE_BACK_TENSION, ID_TAPE_NOISE, ID_PHYSICAL_TOPIC, ID_MAGNET_MOMENT, ID_MAGNET_DISTANCE, ID_GROUND_LOOP, ID_MAINS_HUM, ID_RF_FIELD, ID_RF_PICKUP, ID_PYVHS_TOPIC, ID_PYVHS_LUMA_COMPRESSION, ID_PYVHS_CHROMA_COMPRESSION, ID_PYVHS_LUMA_NOISE, ID_PYVHS_CHROMA_NOISE, ID_PYVHS_CHROMA_WAVE, ID_PYVHS_GENERATION, ID_OSD_TOPIC, ID_OSD_AMOUNT, ID_OSD_REC, ID_OSD_PLAY, ID_OSD_TIMESTAMP, ID_OSD_OPACITY, ID_OSD_SCALE, ID_OSD_GLITCH, ID_OSD_GHOST, ID_OSD_REC_BLINK, ID_OSD_TIME_SPEED, ID_OSD_YEAR, ID_OSD_MONTH, ID_OSD_DAY, ID_OSD_HOUR, ID_OSD_MINUTE, ID_OSD_SECOND, ID_OSD_REC_COLOR, ID_VHS_STYLE_TOPIC, ID_VHS_STYLE, ID_VHS_STYLE_INTENSITY, ID_CAMERA_PROFILE, ID_CAMERA_INTENSITY, ID_NTSCRS_TOPIC, ID_NTSCRS_LOOK, ID_NTSCRS_INTENSITY, ID_NTSCRS_PHASE_NOISE, ID_VHS6RIFT_GRAIN, ID_VHS6RIFT_GRAIN_SIZE, ID_VHS6RIFT_NOISE_DETAIL, ID_VHS6RIFT_SHADOW_BIAS, ID_VHS6RIFT_NOISE_ENABLE, ID_VHS6RIFT_NOISE_SEED, ID_SIGNAL_HUE_NOISE, ID_DEVICE_CONNECTION_TOPIC, ID_DEVICE_CONNECTION_AMOUNT, ID_DEVICE_CONNECTION_DENSITY, ID_DEVICE_CONNECTION_SIZE, ID_DEVICE_CONNECTION_SPEED, ID_DEVICE_CONNECTION_DISTRIBUTION, ID_DEVICE_CONNECTION_SEED
};
constexpr A_long kInputCheckoutID=1;
static char kTrackingShapeChoices[]="Horizontal Band|Vertical Band|Rectangle";
static char kColorSystemChoices[]="NTSC|PAL";
static char kTapeSpeedChoices[]="SP|LP|EP";
static char kVhsStyleChoices[]="Custom|Clean VHS|Vintage VHS|Heavy VHS Degradation|Bad Tracking|Very Bad Tracking|Unstable VCR Playback|Moving Band Noise|Tape Damage|Bottom Tape Damage|Static Interference|Color Bleed|Color Shift|Ghosting|Signal Loss|Severe Analog Glitch|Damaged Tape Playback";
static char kCameraProfileChoices[]="Off|JVC GR-C7U|JVC GR-AX220U|JVC GR-AX920U|JVC GR-SXM37US|JVC GR-SXM740U|Sony CCD-TR55|JVC GR-S55|Panasonic NV-M1|RCA CMR300";
static char kNtscRsLookChoices[]="Custom|VHS-90 Natural NTSC|VHS-90 Home SP|VHS-90 LP Tape|VHS-90 EP Tape|VHS-90 Tracking Snow|VHS-90 Chroma Loss|VHS-90 Edge Wave|VHS-90 Field Blend|VHS-90 Composite Smear|VHS-90 Chroma Instability";
static char kSignalModelChoices[]="Raster NTSC|Full Waveform NTSC";

struct RenderSettings {
    float master=100, composite=0, sync=0, staticAmount=0, staticDensity=35;
    A_long colorSystem=1, signalModel=1, tapeSpeed=1;
    float tapeDetail=100.0f, trackRead=0.0f, trackMisalignment=0.0f, chromaShift=0.0f;
    float chromaSubsample=0, verticalChroma=0, verticalLuma=0;
    float lumaNoise=0, chromaNoise=0, compositeNoise=0, noiseHue=0;
    float rfDrift=0, rfHum=0, rfAm=0, agcPump=0, impulseNoise=0, dropouts=0;
    float multipathGain=0, multipathDelay=24, multipathEnsemble=0, signalGhost=0, chromaDelay=0, chromaLoss=0;
    float afcHunt=0, chromaFlutter=0, ycCrosstalk=0, colorPhaseDrift=0, colorDrift=0;
    float hsyncNoise=0, vsyncNoise=0, burstNoise=0;
    float edgeEnhance=0, tapeFade=0, blackLift=0, temporalGhost=0, ringing=0, ringingPower=2, crtEqualizer=0, gradeContrast=100, gradeBrightness=0, gradeSaturation=100, vignette=0, flicker=0;
    float lumaSmear=0, bloom=0, analogGlow=0;
    float tapeSurface=0, tapeTransport=0, geometry=0, interlace=0, verticalRoll=0, edgeWave=0, scanlines=0;
    float trackingAmount=0, trackingX=50, trackingY=50, trackingWidth=100, trackingHeight=8, trackingAngle=0;
    float trackingMotionX=0, trackingMotionY=0, trackingServo=4, trackingTear=3;
    A_long trackingShape=3;
    float headAmount=0, headY=100, headHeight=4, headShift=24;
    float headCondition=0;
    float tapeWear=100, tapeDropouts=100, tapeScratches=100, tapeCreases=100, tapeEdge=100, tapeMold=100, tapeVaseline=100, tapeSticky=100, tapeSeed=1, backTension=0;
    float tapeNoise=0;
    float magnetMoment=0.0f, magnetDistance=6.0f, groundLoop=0.0f, mainsHum=0.0f, rfField=0.0f, rfPickup=0.0f;
    float pyVhsLumaCompression=1.0f, pyVhsChromaCompression=1.0f, pyVhsLumaNoise=0.0f, pyVhsChromaNoise=0.0f, pyVhsChromaWave=0.0f, pyVhsGeneration=1.0f;
    float osdAmount=0.0f, osdRec=100.0f, osdPlay=100.0f, osdTimestamp=100.0f, osdOpacity=82.0f, osdScale=100.0f, osdGlitch=50.0f, osdGhost=35.0f, osdRecBlink=1.4f, osdTimeSpeed=1.0f;
    float osdYear=1994.0f, osdMonth=6.0f, osdDay=11.0f, osdHour=12.0f, osdMinute=0.0f, osdSecond=0.0f;
    float vhs6RiftGrain=0.0f, vhs6RiftGrainSize=2.0f, vhs6RiftNoiseDetail=50.0f, vhs6RiftShadowBias=65.0f, vhs6RiftNoiseSeed=1.0f;
    bool vhs6RiftNoiseEnable=true;
    float signalHueNoise=0.0f;
    float deviceConnectionAmount=0.0f,deviceConnectionDensity=35.0f,deviceConnectionSize=1.5f,deviceConnectionSpeed=1.0f,deviceConnectionDistribution=50.0f,deviceConnectionSeed=1.0f;
    float ntscRsPhaseNoise=0.0f, edgeWaveFrequency=0.05f, edgeWaveSpeed=4.0f, edgeWaveDetail=2.0f;
    float osdRecColorR=255.0f, osdRecColorG=46.0f, osdRecColorB=46.0f;
    A_long vhsStyle=1, cameraProfile=1, ntscRsLook=1;
    float vhsStyleIntensity=100.0f, cameraIntensity=100.0f, ntscRsIntensity=100.0f;
    bool previousFrameAvailable=false;
    std::int64_t frame=0;
    double timeSeconds=0;
    float frameRate=29.97f;
};
inline float clamp01(float x) noexcept { return std::max(0.0f,std::min(1.0f,x)); }

static float blendPreset(float current,float target,float intensity) noexcept {
    const float t=clamp01(intensity/100.0f);
    return current+(target-current)*t;
}
static void ApplyVhs6RiftStylePreset(RenderSettings& s) noexcept {
    const float i=clamp01(s.vhsStyleIntensity/100.0f);
    if(s.vhsStyle<=1||i<=0.0f)return;
    auto b=[&](float& v,float target){v=blendPreset(v,target,s.vhsStyleIntensity);};
    switch(s.vhsStyle) {
    case 2: b(s.composite,12);b(s.chromaSubsample,8);b(s.lumaNoise,3);b(s.chromaNoise,2);b(s.trackingAmount,4);b(s.headAmount,3);b(s.tapeSurface,2);break;
    case 3: b(s.composite,24);b(s.chromaSubsample,22);b(s.chromaShift,3);b(s.tapeNoise,20);b(s.colorPhaseDrift,15);b(s.edgeEnhance,12);b(s.trackingAmount,10);b(s.geometry,6);b(s.headAmount,10);b(s.blackLift,10);break;
    case 4: b(s.composite,45);b(s.chromaSubsample,45);b(s.chromaNoise,45);b(s.tapeNoise,60);b(s.trackingAmount,55);b(s.dropouts,60);b(s.geometry,30);b(s.headAmount,35);b(s.staticAmount,30);break;
    case 5: b(s.trackingAmount,80);b(s.trackingHeight,18);b(s.trackingWidth,100);b(s.trackingServo,70);b(s.trackingTear,45);b(s.geometry,25);b(s.edgeWave,35);break;
    case 6: b(s.trackingAmount,100);b(s.trackingHeight,30);b(s.trackingServo,95);b(s.trackingTear,85);b(s.verticalRoll,70);b(s.sync,60);b(s.staticAmount,65);b(s.dropouts,70);break;
    case 7: b(s.tapeTransport,65);b(s.geometry,35);b(s.verticalRoll,25);b(s.colorDrift,35);b(s.colorPhaseDrift,28);b(s.rfDrift,35);break;
    case 8: b(s.trackingAmount,45);b(s.trackingHeight,32);b(s.trackingWidth,100);b(s.trackingMotionY,35);b(s.trackingServo,45);b(s.staticAmount,35);break;
    case 9: b(s.tapeSurface,75);b(s.tapeWear,75);b(s.tapeDropouts,80);b(s.tapeScratches,65);b(s.tapeCreases,75);b(s.dropouts,55);b(s.tapeNoise,35);break;
    case 10: b(s.headAmount,100);b(s.headY,100);b(s.headHeight,45);b(s.headShift,65);b(s.trackingAmount,30);break;
    case 11: b(s.staticAmount,85);b(s.staticDensity,80);b(s.lumaNoise,65);b(s.chromaNoise,70);b(s.compositeNoise,60);b(s.tapeNoise,45);break;
    case 12: b(s.chromaSubsample,45);b(s.verticalChroma,35);b(s.chromaShift,12);b(s.gradeSaturation,145);break;
    case 13: b(s.colorPhaseDrift,60);b(s.colorDrift,55);b(s.chromaFlutter,50);b(s.chromaShift,8);break;
    case 14: b(s.multipathGain,55);b(s.multipathDelay,36);b(s.multipathEnsemble,50);b(s.signalGhost,65);b(s.temporalGhost,35);break;
    case 15: b(s.sync,85);b(s.verticalRoll,75);b(s.staticAmount,50);b(s.blackLift,25);b(s.rfDrift,65);break;
    case 16: b(s.trackingAmount,100);b(s.geometry,55);b(s.chromaSubsample,40);b(s.dropouts,85);b(s.staticAmount,70);b(s.chromaShift,12);b(s.sync,65);break;
    case 17: b(s.dropouts,70);b(s.tapeSurface,75);b(s.tapeTransport,60);b(s.geometry,35);b(s.tapeNoise,50);b(s.ringing,45);b(s.headAmount,35);break;
    default:break;
    }
}
static void ApplyVhs6RiftCameraProfile(RenderSettings& s) noexcept {
    const float i=clamp01(s.cameraIntensity/100.0f);
    if(s.cameraProfile<=1||i<=0.0f)return;
    auto b=[&](float& v,float target){v=blendPreset(v,target,s.cameraIntensity);};
    switch(s.cameraProfile) {
    case 2: b(s.tapeDetail,58);b(s.chromaSubsample,26);b(s.chromaShift,6);b(s.geometry,30);b(s.gradeSaturation,108);break;
    case 3: b(s.tapeDetail,72);b(s.chromaSubsample,14);b(s.geometry,15);b(s.chromaShift,3);break;
    case 4: b(s.tapeDetail,84);b(s.edgeEnhance,35);b(s.geometry,8);b(s.chromaSubsample,10);break;
    case 5: b(s.tapeDetail,92);b(s.chromaSubsample,8);b(s.chromaShift,2);b(s.geometry,6);break;
    case 6: b(s.tapeDetail,96);b(s.chromaSubsample,5);b(s.chromaShift,1);b(s.geometry,3);break;
    case 7: b(s.tapeDetail,76);b(s.chromaSubsample,16);b(s.colorPhaseDrift,12);b(s.gradeSaturation,92);break;
    case 8: b(s.tapeDetail,98);b(s.edgeEnhance,30);b(s.chromaSubsample,4);break;
    case 9: b(s.tapeDetail,52);b(s.chromaSubsample,30);b(s.chromaShift,5);b(s.gradeBrightness,5);b(s.gradeSaturation,112);break;
    case 10: b(s.tapeDetail,64);b(s.edgeEnhance,38);b(s.bloom,18);b(s.gradeSaturation,105);break;
    default:break;
    }
}
static void ApplyNtscRsVhsLook(RenderSettings& s) noexcept {
    const float i=clamp01(s.ntscRsIntensity/100.0f);
    if(s.ntscRsLook<=1||i<=0.0f)return;
    auto b=[&](float& v,float target){v=blendPreset(v,target,s.ntscRsIntensity);};
    auto bi=[&](A_long& v,float target){v=static_cast<A_long>(std::lround(blendPreset(float(v),target,s.ntscRsIntensity)));};
    switch(s.ntscRsLook) {
    case 2: b(s.composite,28);b(s.tapeDetail,94);b(s.chromaSubsample,16);b(s.verticalChroma,14);b(s.lumaSmear,12);b(s.ringing,10);b(s.edgeWave,18);b(s.lumaNoise,1);b(s.chromaNoise,6);b(s.ntscRsPhaseNoise,0.5f);break;
    case 3: b(s.composite,32);bi(s.tapeSpeed,1);b(s.tapeDetail,88);b(s.chromaSubsample,22);b(s.verticalChroma,18);b(s.lumaSmear,18);b(s.ringing,12);b(s.edgeWave,30);b(s.headAmount,6);b(s.trackingAmount,18);b(s.lumaNoise,2);b(s.chromaNoise,10);b(s.colorPhaseDrift,8);b(s.ntscRsPhaseNoise,1);break;
    case 4: b(s.composite,42);bi(s.tapeSpeed,2);b(s.tapeDetail,72);b(s.chromaSubsample,35);b(s.verticalChroma,30);b(s.lumaSmear,25);b(s.edgeWave,42);b(s.headAmount,8);b(s.trackingAmount,22);b(s.chromaNoise,16);break;
    case 5: b(s.composite,52);bi(s.tapeSpeed,3);b(s.tapeDetail,58);b(s.chromaSubsample,48);b(s.verticalChroma,42);b(s.lumaSmear,34);b(s.edgeWave,55);b(s.headAmount,10);b(s.trackingAmount,28);b(s.chromaNoise,22);break;
    case 6: b(s.trackingAmount,55);b(s.trackingHeight,16);b(s.trackingWidth,100);b(s.trackingServo,60);b(s.staticAmount,30);b(s.staticDensity,55);b(s.lumaNoise,20);b(s.headAmount,12);break;
    case 7: b(s.chromaLoss,1600);b(s.chromaNoise,38);b(s.colorPhaseDrift,28);b(s.chromaFlutter,25);b(s.verticalChroma,42);break;
    case 8: b(s.edgeWave,78);b(s.geometry,22);b(s.trackingAmount,12);b(s.trackingMotionY,22);b(s.edgeWaveFrequency,0.05f);b(s.edgeWaveSpeed,4.0f);b(s.edgeWaveDetail,4.0f);break;
    case 9: b(s.interlace,65);b(s.verticalChroma,55);b(s.geometry,10);b(s.temporalGhost,15);break;
    case 10: b(s.composite,68);b(s.lumaSmear,48);b(s.ringing,22);b(s.chromaSubsample,25);b(s.verticalChroma,22);break;
    case 11: b(s.colorPhaseDrift,42);b(s.chromaFlutter,38);b(s.colorDrift,36);b(s.chromaDelay,7);b(s.chromaNoise,18);b(s.ntscRsPhaseNoise,35);break;
    default:break;
    }
}
static void ApplyVhsSourcePresets(RenderSettings& s) noexcept {
    ApplyVhs6RiftStylePreset(s);
    ApplyVhs6RiftCameraProfile(s);
    ApplyNtscRsVhsLook(s);
}
static void DeletePreRenderData(void* data) { delete static_cast<RenderSettings*>(data); }

static PF_Err GlobalSetup(PF_InData* in_data,PF_OutData* out_data) {
    (void)in_data;
    out_data->my_version=PF_VERSION(1,0,0,PF_Stage_DEVELOP,1);
    out_data->out_flags=PF_OutFlag_DEEP_COLOR_AWARE|PF_OutFlag_NON_PARAM_VARY;
    out_data->out_flags2=PF_OutFlag2_SUPPORTS_SMART_RENDER|
        PF_OutFlag2_FLOAT_COLOR_AWARE|PF_OutFlag2_SUPPORTS_THREADED_RENDERING;
    return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData* in_data,PF_OutData* out_data) {
    PF_Err err=PF_Err_NONE;
    PF_ParamDef def;
    AEFX_CLR_STRUCT(def); PF_ADD_INPUT("Source",PF_LayerDefault_MYSELF,PF_PixelFormat_ARGB32);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Master Strength",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_MASTER);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Signal",ID_SIGNAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Composite Signal",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COMPOSITE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Sync Collapse",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Color System",2,1,kColorSystemChoices,ID_PAL_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Signal Model",2,1,kSignalModelChoices,ID_SIGNAL_MODEL);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tape Speed",3,1,kTapeSpeedChoices,ID_TAPE_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Detail",6,100,6,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Adjacent Track Crosstalk",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_READ);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Track Misalignment",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_MISALIGNMENT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",0,20,0,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Subsample / Bleed",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_SUBSAMPLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Chroma Delay",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Luma Blur",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_LUMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_LUMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Composite Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COMPOSITE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Noise Hue Bias",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_NOISE_HUE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Mains Hum",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_HUM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AM Nonlinearity",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_AM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AGC Pump",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_AGC_PUMP);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Impulse Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_IMPULSE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Dropouts",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_DROPOUTS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Ghost Gain",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MULTIPATH_GAIN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Ghost Delay",1,200,1,200,24,1,PF_ValueDisplayFlag_NONE,0,ID_MULTIPATH_DELAY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Ghost Ensemble",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MULTIPATH_ENSEMBLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Signal Ghosting",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SIGNAL_GHOST);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Delay",0,24,0,24,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_DELAY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Loss",0,5000,0,5000,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_LOSS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AFC Hunt",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_AFC_HUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Flutter",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_FLUTTER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Y/C Crosstalk",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_YC_CROSSTALK);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Color Phase Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COLOR_PHASE_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Color Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COLOR_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("H Sync Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_HSYNC_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("V Sync Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VSYNC_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Burst Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BURST_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Edge Enhancement",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_EDGE_ENHANCE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Fade",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_FADE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Black Lift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BLACK_LIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Previous Frame Ghost",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEMPORAL_GHOST);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ringing",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RINGING);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ringing Power",0,8,0,8,2,1,PF_ValueDisplayFlag_NONE,0,ID_RINGING_POWER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("CRT 3-Band EQ",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CRT_EQUALIZER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Contrast",0,200,0,200,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_CONTRAST);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Brightness",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_GRADE_BRIGHTNESS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Saturation",0,200,0,200,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_SATURATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vignette",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VIGNETTE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Flicker",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_FLICKER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Smear",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_LUMA_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Bloom / Glow",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BLOOM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Analog Glow",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_ANALOG_GLOW);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Material Master",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_SURFACE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Transport",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_TRANSPORT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Geometry Warp",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_GEOMETRY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Interlace / Field Weave",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_INTERLACE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Roll",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_ROLL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_EDGE_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Frequency",0.005f,0.2f,0.005f,0.2f,0.05f,3,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_FREQUENCY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Speed",0,20,0,20,4,2,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Detail",1,5,1,5,2,0,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Scanlines",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SCANLINES);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_SIGNAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Static",ID_STATIC_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Static Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_STATIC_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Static Density",0,100,0,100,35,1,PF_ValueDisplayFlag_PERCENT,0,ID_STATIC_DENSITY);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_STATIC_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Noise",ID_TRACKING_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Region Shape",3,3,kTrackingShapeChoices,ID_TRACKING_SHAPE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center X",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Width",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_WIDTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Region Angle",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACKING_ANGLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Motion X",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_MOTION_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Motion Y",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_MOTION_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Servo Drift",0,100,0,100,4,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_SERVO);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Probability",0,100,0,100,3,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_TEAR);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACKING_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Head Switching",ID_HEAD_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Bottom Edge Y",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,4,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Shift",0,100,0,100,24,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_SHIFT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_HEAD_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Head Dirt / Wear",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_CONDITION);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tape Material",ID_TAPE_MATERIAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Wear",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_WEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Dropouts",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_DROPOUTS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Scratches",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_SCRATCHES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Creases / Folds",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_CREASES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Edge Damage",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_EDGE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Mold",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_MOLD);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Vaseline / Binder Film",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_VASELINE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Sticky Binder / Shed",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_STICKY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Cassette Seed",1,10000,1,10000,1,1,PF_ValueDisplayFlag_NONE,0,ID_TAPE_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Back Tension", -100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_TAPE_BACK_TENSION);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TAPE_MATERIAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Noise (Multi-Scale)",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("External Physical Interference",ID_PHYSICAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Magnet Moment (A m^2)",0,10,0,10,0,2,PF_ValueDisplayFlag_NONE,0,ID_MAGNET_MOMENT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Magnet Distance (cm)",0.1f,30,0.1f,30,6,1,PF_ValueDisplayFlag_NONE,0,ID_MAGNET_DISTANCE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ground Loop (mV)",0,500,0,500,0,1,PF_ValueDisplayFlag_NONE,0,ID_GROUND_LOOP);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Mains Hum",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MAINS_HUM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Field (V/m)",0,40,0,40,0,1,PF_ValueDisplayFlag_NONE,0,ID_RF_FIELD);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Pickup",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_PICKUP);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_PHYSICAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Python VHS Generation",ID_PYVHS_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Compression",1,3,1,3,1,2,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_LUMA_COMPRESSION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Compression",1,100,1,100,1,1,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_CHROMA_COMPRESSION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Noise",0,20,0,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_LUMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Noise",0,30,0,30,0,1,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_CHROMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Wave",0,2,0,2,0,2,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_CHROMA_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Generation",0,5,0,5,1,2,PF_ValueDisplayFlag_NONE,0,ID_PYVHS_GENERATION);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_PYVHS_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Camcorder OSD",ID_OSD_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("OSD Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("REC Indicator",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_REC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("PLAY / SP Status",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_PLAY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Date / Time",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TIMESTAMP);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("OSD Opacity",0,100,0,100,82,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_OPACITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("OSD Size",35,300,35,300,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_SCALE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Jitter",0,200,0,200,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_GLITCH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Ghost",0,100,0,100,35,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_GHOST);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("REC Blink (Hz)",0,4,0,4,1.4,2,PF_ValueDisplayFlag_NONE,0,ID_OSD_REC_BLINK);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Timestamp Speed",0,10,0,10,1,2,PF_ValueDisplayFlag_NONE,0,ID_OSD_TIME_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Year",1900,2199,1900,2199,1994,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_YEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Month",1,12,1,12,6,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_MONTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Day",1,31,1,31,11,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_DAY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Hour",0,23,0,23,12,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_HOUR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Minute",0,59,0,59,0,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_MINUTE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Start Second",0,59,0,59,0,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_SECOND);
    AEFX_CLR_STRUCT(def); PF_ADD_COLOR("REC Lamp Color",255,46,46,ID_OSD_REC_COLOR);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS Style & Camera Profiles",ID_VHS_STYLE_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("VHS Style Preset",17,1,kVhsStyleChoices,ID_VHS_STYLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Style Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS_STYLE_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Camera Profile",10,1,kCameraProfileChoices,ID_CAMERA_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Camera Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_CAMERA_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_VHS_STYLE_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS-90 Signal Looks",ID_NTSCRS_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Signal Look",11,1,kNtscRsLookChoices,ID_NTSCRS_LOOK);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Look Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_NTSCRS_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Scanline Chroma Phase Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_NTSCRS_PHASE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Grain Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_GRAIN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grain Particle Size",1,5,1,5,2,0,PF_ValueDisplayFlag_NONE,0,ID_VHS6RIFT_GRAIN_SIZE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Noise Detail / High Frequency",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_NOISE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Shadow Bias",0,100,0,100,65,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_SHADOW_BIAS);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Procedural Noise", "Enable", 1, 0, ID_VHS6RIFT_NOISE_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Noise Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_VHS6RIFT_NOISE_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Hue Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SIGNAL_HUE_NOISE);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_NTSCRS_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Signal Network",ID_DEVICE_CONNECTION_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Device Connections",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_DEVICE_CONNECTION_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Network Density",0,100,0,100,35,1,PF_ValueDisplayFlag_PERCENT,0,ID_DEVICE_CONNECTION_DENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Node Size",0.5,8,0.5,8,1.5,1,PF_ValueDisplayFlag_NONE,0,ID_DEVICE_CONNECTION_SIZE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Animation Speed",0,5,0,5,1,1,PF_ValueDisplayFlag_NONE,0,ID_DEVICE_CONNECTION_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Distribution",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_DEVICE_CONNECTION_DISTRIBUTION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Network Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_DEVICE_CONNECTION_SEED);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_DEVICE_CONNECTION_TOPIC+100);
    out_data->num_params=PARAM_COUNT;
    return err;
}

static PF_Err CheckoutFloat(PF_InData* in_data,A_long index,float& value) {
    PF_ParamDef param; AEFX_CLR_STRUCT(param);
    PF_Err err=PF_CHECKOUT_PARAM(in_data,index,in_data->current_time,in_data->time_step,in_data->time_scale,&param);
    if(err!=PF_Err_NONE) return err;
    value=static_cast<float>(param.u.fs_d.value);
    return PF_CHECKIN_PARAM(in_data,&param);
}
static PF_Err CheckoutPopup(PF_InData* in_data,A_long index,A_long& value) {
    PF_ParamDef param; AEFX_CLR_STRUCT(param);
    PF_Err err=PF_CHECKOUT_PARAM(in_data,index,in_data->current_time,in_data->time_step,in_data->time_scale,&param);
    if(err!=PF_Err_NONE) return err;
    value=param.u.pd.value;
    return PF_CHECKIN_PARAM(in_data,&param);
}
static PF_Err ReadSettings(PF_InData* in_data,RenderSettings& s) {
    PF_Err err=PF_Err_NONE;
#define READ_FLOAT(I,F) do { err=CheckoutFloat(in_data,(I),s.F); if(err!=PF_Err_NONE) return err; } while(0)
    READ_FLOAT(PARAM_MASTER,master); READ_FLOAT(PARAM_COMPOSITE_AMOUNT,composite);
    READ_FLOAT(PARAM_SYNC_AMOUNT,sync);
    err=CheckoutPopup(in_data,PARAM_PAL_MODE,s.colorSystem); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_SIGNAL_MODEL,s.signalModel); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TAPE_SPEED,s.tapeSpeed); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TAPE_DETAIL,tapeDetail); READ_FLOAT(PARAM_TRACK_READ,trackRead);
    READ_FLOAT(PARAM_TRACK_MISALIGNMENT,trackMisalignment); READ_FLOAT(PARAM_CHROMA_SHIFT,chromaShift);
    READ_FLOAT(PARAM_CHROMA_SUBSAMPLE,chromaSubsample); READ_FLOAT(PARAM_VERTICAL_CHROMA,verticalChroma);
    READ_FLOAT(PARAM_VERTICAL_LUMA,verticalLuma);
    READ_FLOAT(PARAM_LUMA_NOISE,lumaNoise); READ_FLOAT(PARAM_CHROMA_NOISE,chromaNoise);
    READ_FLOAT(PARAM_COMPOSITE_NOISE,compositeNoise); READ_FLOAT(PARAM_NOISE_HUE,noiseHue);
    READ_FLOAT(PARAM_RF_DRIFT,rfDrift); READ_FLOAT(PARAM_RF_HUM,rfHum); READ_FLOAT(PARAM_RF_AM,rfAm);
    READ_FLOAT(PARAM_AGC_PUMP,agcPump);
    READ_FLOAT(PARAM_IMPULSE_NOISE,impulseNoise); READ_FLOAT(PARAM_DROPOUTS,dropouts);
    READ_FLOAT(PARAM_MULTIPATH_GAIN,multipathGain); READ_FLOAT(PARAM_MULTIPATH_DELAY,multipathDelay);
    READ_FLOAT(PARAM_MULTIPATH_ENSEMBLE,multipathEnsemble);
    READ_FLOAT(PARAM_SIGNAL_GHOST,signalGhost); READ_FLOAT(PARAM_CHROMA_DELAY,chromaDelay); READ_FLOAT(PARAM_CHROMA_LOSS,chromaLoss);
    READ_FLOAT(PARAM_AFC_HUNT,afcHunt); READ_FLOAT(PARAM_CHROMA_FLUTTER,chromaFlutter);
    READ_FLOAT(PARAM_YC_CROSSTALK,ycCrosstalk); READ_FLOAT(PARAM_COLOR_PHASE_DRIFT,colorPhaseDrift); READ_FLOAT(PARAM_COLOR_DRIFT,colorDrift);
    READ_FLOAT(PARAM_HSYNC_NOISE,hsyncNoise); READ_FLOAT(PARAM_VSYNC_NOISE,vsyncNoise);
    READ_FLOAT(PARAM_BURST_NOISE,burstNoise);
    READ_FLOAT(PARAM_EDGE_ENHANCE,edgeEnhance); READ_FLOAT(PARAM_TAPE_FADE,tapeFade); READ_FLOAT(PARAM_BLACK_LIFT,blackLift);
    READ_FLOAT(PARAM_TEMPORAL_GHOST,temporalGhost);
    READ_FLOAT(PARAM_RINGING,ringing); READ_FLOAT(PARAM_RINGING_POWER,ringingPower); READ_FLOAT(PARAM_CRT_EQUALIZER,crtEqualizer);
    READ_FLOAT(PARAM_GRADE_CONTRAST,gradeContrast); READ_FLOAT(PARAM_GRADE_BRIGHTNESS,gradeBrightness);
    READ_FLOAT(PARAM_GRADE_SATURATION,gradeSaturation); READ_FLOAT(PARAM_VIGNETTE,vignette); READ_FLOAT(PARAM_FLICKER,flicker);
    READ_FLOAT(PARAM_LUMA_SMEAR,lumaSmear); READ_FLOAT(PARAM_BLOOM,bloom); READ_FLOAT(PARAM_ANALOG_GLOW,analogGlow);
    READ_FLOAT(PARAM_TAPE_SURFACE,tapeSurface); READ_FLOAT(PARAM_TAPE_TRANSPORT,tapeTransport);
    READ_FLOAT(PARAM_GEOMETRY,geometry); READ_FLOAT(PARAM_INTERLACE,interlace); READ_FLOAT(PARAM_VERTICAL_ROLL,verticalRoll); READ_FLOAT(PARAM_EDGE_WAVE,edgeWave); READ_FLOAT(PARAM_SCANLINES,scanlines); READ_FLOAT(PARAM_TAPE_NOISE,tapeNoise);
    READ_FLOAT(PARAM_STATIC_AMOUNT,staticAmount);
    READ_FLOAT(PARAM_STATIC_DENSITY,staticDensity); READ_FLOAT(PARAM_TRACKING_AMOUNT,trackingAmount);
    err=CheckoutPopup(in_data,PARAM_TRACKING_SHAPE,s.trackingShape); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TRACKING_CENTER_X,trackingX); READ_FLOAT(PARAM_TRACKING_CENTER_Y,trackingY);
    READ_FLOAT(PARAM_TRACKING_WIDTH,trackingWidth); READ_FLOAT(PARAM_TRACKING_HEIGHT,trackingHeight);
    READ_FLOAT(PARAM_TRACKING_ANGLE,trackingAngle);
    READ_FLOAT(PARAM_TRACKING_MOTION_X,trackingMotionX); READ_FLOAT(PARAM_TRACKING_MOTION_Y,trackingMotionY);
    READ_FLOAT(PARAM_TRACKING_SERVO,trackingServo); READ_FLOAT(PARAM_TRACKING_TEAR,trackingTear);
    READ_FLOAT(PARAM_HEAD_AMOUNT,headAmount); READ_FLOAT(PARAM_HEAD_BOTTOM_Y,headY);
    READ_FLOAT(PARAM_HEAD_HEIGHT,headHeight); READ_FLOAT(PARAM_HEAD_SHIFT,headShift);
    READ_FLOAT(PARAM_HEAD_CONDITION,headCondition);
    READ_FLOAT(PARAM_TAPE_WEAR,tapeWear); READ_FLOAT(PARAM_TAPE_DROPOUTS,tapeDropouts); READ_FLOAT(PARAM_TAPE_SCRATCHES,tapeScratches);
    READ_FLOAT(PARAM_TAPE_CREASES,tapeCreases); READ_FLOAT(PARAM_TAPE_EDGE,tapeEdge); READ_FLOAT(PARAM_TAPE_MOLD,tapeMold);
    READ_FLOAT(PARAM_TAPE_VASELINE,tapeVaseline); READ_FLOAT(PARAM_TAPE_STICKY,tapeSticky); READ_FLOAT(PARAM_TAPE_SEED,tapeSeed); READ_FLOAT(PARAM_TAPE_BACK_TENSION,backTension);
    READ_FLOAT(PARAM_MAGNET_MOMENT,magnetMoment); READ_FLOAT(PARAM_MAGNET_DISTANCE,magnetDistance); READ_FLOAT(PARAM_GROUND_LOOP,groundLoop);
    READ_FLOAT(PARAM_MAINS_HUM,mainsHum); READ_FLOAT(PARAM_RF_FIELD,rfField); READ_FLOAT(PARAM_RF_PICKUP,rfPickup);
    READ_FLOAT(PARAM_PYVHS_LUMA_COMPRESSION,pyVhsLumaCompression); READ_FLOAT(PARAM_PYVHS_CHROMA_COMPRESSION,pyVhsChromaCompression);
    READ_FLOAT(PARAM_PYVHS_LUMA_NOISE,pyVhsLumaNoise); READ_FLOAT(PARAM_PYVHS_CHROMA_NOISE,pyVhsChromaNoise);
    READ_FLOAT(PARAM_PYVHS_CHROMA_WAVE,pyVhsChromaWave); READ_FLOAT(PARAM_PYVHS_GENERATION,pyVhsGeneration);
    READ_FLOAT(PARAM_OSD_AMOUNT,osdAmount); READ_FLOAT(PARAM_OSD_REC,osdRec); READ_FLOAT(PARAM_OSD_PLAY,osdPlay); READ_FLOAT(PARAM_OSD_TIMESTAMP,osdTimestamp);
    READ_FLOAT(PARAM_OSD_OPACITY,osdOpacity); READ_FLOAT(PARAM_OSD_SCALE,osdScale); READ_FLOAT(PARAM_OSD_GLITCH,osdGlitch); READ_FLOAT(PARAM_OSD_GHOST,osdGhost);
    READ_FLOAT(PARAM_OSD_REC_BLINK,osdRecBlink); READ_FLOAT(PARAM_OSD_TIME_SPEED,osdTimeSpeed);
    READ_FLOAT(PARAM_OSD_YEAR,osdYear); READ_FLOAT(PARAM_OSD_MONTH,osdMonth); READ_FLOAT(PARAM_OSD_DAY,osdDay);
    READ_FLOAT(PARAM_OSD_HOUR,osdHour); READ_FLOAT(PARAM_OSD_MINUTE,osdMinute); READ_FLOAT(PARAM_OSD_SECOND,osdSecond);
    PF_ParamDef recColor; AEFX_CLR_STRUCT(recColor);
    err=PF_CHECKOUT_PARAM(in_data,PARAM_OSD_REC_COLOR,in_data->current_time,in_data->time_step,in_data->time_scale,&recColor);
    if(err!=PF_Err_NONE) return err;
    s.osdRecColorR=float(recColor.u.cd.value.red); s.osdRecColorG=float(recColor.u.cd.value.green); s.osdRecColorB=float(recColor.u.cd.value.blue);
    err=PF_CHECKIN_PARAM(in_data,&recColor); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_VHS_STYLE,s.vhsStyle); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_VHS_STYLE_INTENSITY,vhsStyleIntensity);
    err=CheckoutPopup(in_data,PARAM_CAMERA_PROFILE,s.cameraProfile); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_CAMERA_INTENSITY,cameraIntensity);
    err=CheckoutPopup(in_data,PARAM_NTSCRS_LOOK,s.ntscRsLook); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_NTSCRS_INTENSITY,ntscRsIntensity);
    READ_FLOAT(PARAM_NTSCRS_PHASE_NOISE,ntscRsPhaseNoise);
    READ_FLOAT(PARAM_SIGNAL_HUE_NOISE,signalHueNoise);
    READ_FLOAT(PARAM_DEVICE_CONNECTION_AMOUNT,deviceConnectionAmount); READ_FLOAT(PARAM_DEVICE_CONNECTION_DENSITY,deviceConnectionDensity); READ_FLOAT(PARAM_DEVICE_CONNECTION_SIZE,deviceConnectionSize); READ_FLOAT(PARAM_DEVICE_CONNECTION_SPEED,deviceConnectionSpeed); READ_FLOAT(PARAM_DEVICE_CONNECTION_DISTRIBUTION,deviceConnectionDistribution); READ_FLOAT(PARAM_DEVICE_CONNECTION_SEED,deviceConnectionSeed);
    READ_FLOAT(PARAM_VHS6RIFT_GRAIN,vhs6RiftGrain); READ_FLOAT(PARAM_VHS6RIFT_GRAIN_SIZE,vhs6RiftGrainSize); READ_FLOAT(PARAM_VHS6RIFT_NOISE_DETAIL,vhs6RiftNoiseDetail); READ_FLOAT(PARAM_VHS6RIFT_SHADOW_BIAS,vhs6RiftShadowBias); READ_FLOAT(PARAM_VHS6RIFT_NOISE_SEED,vhs6RiftNoiseSeed);
    PF_ParamDef noiseEnable; AEFX_CLR_STRUCT(noiseEnable); err=PF_CHECKOUT_PARAM(in_data,PARAM_VHS6RIFT_NOISE_ENABLE,in_data->current_time,in_data->time_step,in_data->time_scale,&noiseEnable); if(err!=PF_Err_NONE) return err; s.vhs6RiftNoiseEnable=noiseEnable.u.bd.value!=0; err=PF_CHECKIN_PARAM(in_data,&noiseEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_EDGE_WAVE_FREQUENCY,edgeWaveFrequency); READ_FLOAT(PARAM_EDGE_WAVE_SPEED,edgeWaveSpeed); READ_FLOAT(PARAM_EDGE_WAVE_DETAIL,edgeWaveDetail);
#undef READ_FLOAT
    s.frame=in_data->time_step!=0?in_data->current_time/in_data->time_step:in_data->current_time;
    s.timeSeconds=in_data->time_scale!=0?double(in_data->current_time)/double(in_data->time_scale):0.0;
    s.frameRate=(in_data->time_step>0&&in_data->time_scale>0)?float(in_data->time_scale)/float(in_data->time_step):29.97f;
    return PF_Err_NONE;
}

static bool ConvertInputWorld(const PF_EffectWorld* world,PF_PixelFormat format,std::vector<float>& rgba) {
    const int w=world->width,h=world->height; rgba.resize(std::size_t(w)*h*4);
    for(int y=0;y<h;++y) {
        const char* row=reinterpret_cast<const char*>(world->data)+y*world->rowbytes;
        for(int x=0;x<w;++x) {
            float r=0,g=0,b=0,a=0;
            if(format==PF_PixelFormat_ARGB32) {
                const auto* px=reinterpret_cast<const PF_Pixel8*>(row)+x;
                a=float(px->alpha)/PF_MAX_CHAN8; r=float(px->red)/PF_MAX_CHAN8; g=float(px->green)/PF_MAX_CHAN8; b=float(px->blue)/PF_MAX_CHAN8;
            } else if(format==PF_PixelFormat_ARGB64) {
                const auto* px=reinterpret_cast<const PF_Pixel16*>(row)+x;
                a=float(px->alpha)/PF_MAX_CHAN16; r=float(px->red)/PF_MAX_CHAN16; g=float(px->green)/PF_MAX_CHAN16; b=float(px->blue)/PF_MAX_CHAN16;
            } else if(format==PF_PixelFormat_ARGB128) {
                const auto* px=reinterpret_cast<const PF_PixelFloat*>(row)+x;
                a=px->alpha; r=px->red; g=px->green; b=px->blue;
            } else return false;
            float* d=rgba.data()+(std::size_t(y)*w+x)*4; d[0]=r; d[1]=g; d[2]=b; d[3]=a;
        }
    }
    return true;
}
static void WritePixel(PF_EffectWorld* world,PF_PixelFormat format,int x,int y,const float* v) {
    char* row=reinterpret_cast<char*>(world->data)+y*world->rowbytes;
    if(format==PF_PixelFormat_ARGB32) {
        auto* p=reinterpret_cast<PF_Pixel8*>(row)+x;
        p->alpha=A_u_char(std::lround(clamp01(v[3])*PF_MAX_CHAN8));
        p->red=A_u_char(std::lround(clamp01(v[0])*PF_MAX_CHAN8));
        p->green=A_u_char(std::lround(clamp01(v[1])*PF_MAX_CHAN8));
        p->blue=A_u_char(std::lround(clamp01(v[2])*PF_MAX_CHAN8));
    } else if(format==PF_PixelFormat_ARGB64) {
        auto* p=reinterpret_cast<PF_Pixel16*>(row)+x;
        p->alpha=A_u_short(std::lround(clamp01(v[3])*PF_MAX_CHAN16));
        p->red=A_u_short(std::lround(clamp01(v[0])*PF_MAX_CHAN16));
        p->green=A_u_short(std::lround(clamp01(v[1])*PF_MAX_CHAN16));
        p->blue=A_u_short(std::lround(clamp01(v[2])*PF_MAX_CHAN16));
    } else {
        auto* p=reinterpret_cast<PF_PixelFloat*>(row)+x;
        p->alpha=v[3]; p->red=v[0]; p->green=v[1]; p->blue=v[2];
    }
}
static void CopyRegion(const PF_EffectWorld* in,PF_EffectWorld* out,PF_PixelFormat fmt) {
    const int bpp=fmt==PF_PixelFormat_ARGB32?sizeof(PF_Pixel8):fmt==PF_PixelFormat_ARGB64?sizeof(PF_Pixel16):sizeof(PF_PixelFloat);
    for(int y=0;y<out->height;++y) {
        const int sy=out->origin_y+y-in->origin_y;
        char* dst=reinterpret_cast<char*>(out->data)+y*out->rowbytes;
        if(sy<0||sy>=in->height) { std::memset(dst,0,out->width*bpp); continue; }
        const char* src=reinterpret_cast<const char*>(in->data)+sy*in->rowbytes;
        for(int x=0;x<out->width;++x) {
            const int sx=out->origin_x+x-in->origin_x;
            if(sx>=0&&sx<in->width) std::memcpy(dst+x*bpp,src+sx*bpp,bpp);
            else std::memset(dst+x*bpp,0,bpp);
        }
    }
}

static PF_Err RenderWorld(RenderSettings s,PF_EffectWorld* input,PF_EffectWorld* output,PF_PixelFormat format,PF_EffectWorld* previousInput=nullptr) {
    try {
        ApplyVhsSourcePresets(s);
        const int w=input->width,h=input->height;
        if(w<=0||h<=0) return PF_Err_NONE;
        const float master=clamp01(s.master/100.0f);
        const bool compositeStages=s.tapeDetail<100||s.trackRead>0||s.headCondition>0||s.chromaShift>0||s.composite>0||s.chromaSubsample>0||s.verticalChroma>0||s.verticalLuma>0||s.lumaNoise>0||s.chromaNoise>0||s.signalHueNoise>0||s.chromaLoss>0||s.compositeNoise>0||s.tapeNoise>0||s.rfDrift>0||s.rfHum>0||s.rfAm>0||s.agcPump>0||s.impulseNoise>0||s.dropouts>0||s.multipathGain>0||s.multipathEnsemble>0||s.signalGhost>0||s.chromaDelay>0||s.afcHunt>0||s.chromaFlutter>0||s.ycCrosstalk>0||s.colorPhaseDrift>0||s.colorDrift>0||s.hsyncNoise>0||s.vsyncNoise>0||s.burstNoise>0||s.edgeEnhance>0||s.tapeFade>0||s.blackLift>0||s.temporalGhost>0||s.ringing>0||s.crtEqualizer>0||s.gradeContrast!=100||s.gradeBrightness!=0||s.gradeSaturation!=100||s.vignette>0||s.flicker>0||s.lumaSmear>0||s.bloom>0||s.analogGlow>0||s.tapeSurface>0||s.tapeTransport>0||(s.trackingAmount>0&&s.trackingServo>0)||s.geometry>0||s.interlace>0||s.verticalRoll>0||s.edgeWave>0||s.scanlines>0||s.magnetMoment>0||s.groundLoop>0||s.mainsHum>0||s.rfPickup>0||s.pyVhsLumaCompression>1.0001f||s.pyVhsChromaCompression>1.0001f||s.pyVhsLumaNoise>0||s.pyVhsChromaNoise>0||s.pyVhsChromaWave>0||s.ntscRsPhaseNoise>0;
        const bool osdEnabled=s.osdAmount>0.0f;
        const bool any=compositeStages||s.sync>0||s.staticAmount>0||s.trackingAmount>0||s.headAmount>0||s.deviceConnectionAmount>0||osdEnabled;
        if(!any||(master<=0.0f&&!osdEnabled)) { CopyRegion(input,output,format); return PF_Err_NONE; }
        std::vector<float> source,working,scratch;
        if(!ConvertInputWorld(input,format,source)) return PF_Err_UNRECOGNIZED_PARAM_TYPE;
        std::vector<float> previousPixels;
        if(previousInput && s.temporalGhost>0 && previousInput->width==w && previousInput->height==h) {
            if(!ConvertInputWorld(previousInput,format,previousPixels)) previousPixels.clear();
        }
        working=source; scratch.resize(source.size());
        if(compositeStages&&master>0.0f) {
            CompositeParams p;
            p.strength=master;
            if(s.magnetMoment>0||s.groundLoop>0||s.mainsHum>0||s.rfPickup>0) {
                p.physicalInterferenceEnabled=true;
                p.physicalInterference.magnetMomentAm2=s.magnetMoment;
                p.physicalInterference.magnetDistanceCm=s.magnetDistance;
                p.physicalInterference.groundLoopMillivolts=s.groundLoop;
                p.physicalInterference.mainsHumAmount=clamp01(s.mainsHum/100.0f);
                p.physicalInterference.mainsFrequencyHz=(s.colorSystem==2)?50.0f:60.0f;
                p.physicalInterference.rfFieldVPerM=s.rfField;
                p.physicalInterference.rfPickupAmount=clamp01(s.rfPickup/100.0f);
                p.physicalInterference.framesPerSecond=s.frameRate;
                p.physicalInterference.seed=0x90A11E5Eu;
            }
            if(s.pyVhsLumaCompression>1.0001f||s.pyVhsChromaCompression>1.0001f||s.pyVhsLumaNoise>0||s.pyVhsChromaNoise>0||s.pyVhsChromaWave>0) {
                p.pyVhsGenerationEnabled=true;
                p.pyVhsGeneration.lumaCompression=s.pyVhsLumaCompression;
                p.pyVhsGeneration.chromaCompression=s.pyVhsChromaCompression;
                p.pyVhsGeneration.lumaNoiseSigma=s.pyVhsLumaNoise;
                p.pyVhsGeneration.chromaNoise=s.pyVhsChromaNoise;
                p.pyVhsGeneration.chromaWave=s.pyVhsChromaWave;
                p.pyVhsGeneration.generation=s.pyVhsGeneration;
                p.pyVhsGeneration.seed=0x56534850u;
            }
            if(s.tapeDetail<100) { p.tapeDetailEnabled=true; p.tapeDetail.detail=std::max(0.06f,s.tapeDetail/100.0f); p.tapeDetail.minimumWidth=48; }
            if(s.trackRead>0) { p.trackReadEnabled=true; p.trackRead.amount=clamp01(s.trackRead/100.0f); p.trackRead.trackingOffset=s.trackMisalignment/100.0f; p.trackRead.speed=std::clamp(int(s.tapeSpeed)-1,0,2); }
            if(s.headCondition>0) {
                const float a=clamp01(s.headCondition/100.0f);
                p.headConditionEnabled=true; p.headCondition.amount=a;
                p.headCondition.seed=0x90A11D3Bu; p.headCondition.frame=s.frame;
                p.headCondition.wear=0.35f*a; p.headCondition.dirt=0.65f*a;
                p.headCondition.buildup=0.8f*a; p.headCondition.mismatch=0.25f*a;
                p.headCondition.grease=0.3f*a;
                p.headCondition.usePhysicalTapeSamples=true;
                p.headCondition.tapeSurface=p.tapeSurface;
            }
            if(s.chromaShift>0) { p.chromaChannelShiftEnabled=true; p.chromaChannelShift.shiftPixels=s.chromaShift; } p.compositeSignalStrength=clamp01(s.composite/100.0f); p.frame=s.frame;
            p.inputBandwidthEnabled=(s.composite>0);
            p.outputBandwidthEnabled=(s.composite>0||s.verticalLuma>0);
            if(s.composite<=0) { p.outputBandwidth.iCutoffHz=0.0f; p.outputBandwidth.qCutoffHz=0.0f; p.outputBandwidth.lumaCutoffHz=0.0f; }
            p.palMode=(s.colorSystem==2);
            if(s.ringing>0) { p.ringingEnabled=true; p.ringing.strength=clamp01(s.ringing/100.0f); p.ringing.power=s.ringingPower; }
            if(s.crtEqualizer>0) { p.crtEqualizerEnabled=true; p.crtEqualizer.amount=clamp01(s.crtEqualizer/100.0f); }
            // The full raw-waveform simulator currently models NTSC 525/60 only.
            p.waveformCompositeEnabled=(s.signalModel==2 && s.colorSystem==1);
            if(!previousPixels.empty() && s.temporalGhost>0) {
                p.temporalGhostEnabled=true;
                p.previousFrame={previousPixels.data(),w,h,w*4};
                p.previousFrameIndex=s.frame-1;
                p.temporalGhost.amount=clamp01(s.temporalGhost/100.0f)*0.35f;
                p.temporalGhost.lumaAmount=1.0f;
                p.temporalGhost.chromaAmount=0.8f;
                p.temporalGhost.offsetXPixels=2.0f;
                p.temporalGhost.redFringePixels=0.6f;
                p.temporalGhost.blueFringePixels=-0.6f;
            }
            if(p.waveformCompositeEnabled) {
                auto& e=p.waveformComposite.effects;
                auto& d=p.waveformComposite.decoder;
                const std::uint32_t frameSeed=0x90A11E5Eu+std::uint32_t(s.frame)*0x9E3779B9u;
                e.random_seed=frameSeed;
                d.random_seed=frameSeed^0x85EBCA6Bu;
                const float speedLuma=(s.tapeSpeed==1)?3.0e6f:((s.tapeSpeed==2)?2.4e6f:2.0e6f);
                const float speedI=(s.tapeSpeed==1)?627000.0f:((s.tapeSpeed==2)?400000.0f:370000.0f);
                const float speedQ=(s.tapeSpeed==1)?627000.0f:((s.tapeSpeed==2)?400000.0f:370000.0f);
                p.waveformComposite.signal.luma_cutoff_hz=speedLuma;
                p.waveformComposite.signal.i_cutoff_hz=speedI;
                p.waveformComposite.signal.q_cutoff_hz=speedQ;
                d.saturation=1.0f;
                d.h_lock_instability=clamp01(s.afcHunt/100.0f);
                d.v_hold_instability=clamp01(s.vsyncNoise/100.0f);
                d.burst_lock_instability=clamp01(std::max(s.colorPhaseDrift,s.colorDrift)/100.0f);
                e.rf_drift=clamp01(s.rfDrift/100.0f);
                e.hum=clamp01(s.rfHum/100.0f);
                e.am_nonlinearity=clamp01(s.rfAm/100.0f);
                e.agc_pump=clamp01(s.agcPump/100.0f);
                e.impulse_noise=clamp01(s.impulseNoise/100.0f);
                e.multipath_gain=clamp01(s.multipathGain/100.0f)*0.45f;
                e.multipath_delay_samples=static_cast<std::uint32_t>(std::max(1.0f,s.multipathDelay*1.05f));
                e.multipath_ensemble=clamp01(s.multipathEnsemble/100.0f);
                e.afc_hunt=clamp01(s.afcHunt/100.0f);
                e.chroma_flutter=clamp01(s.chromaFlutter/100.0f);
                e.yc_crosstalk=clamp01(s.ycCrosstalk/100.0f);
                e.h_sync_noise_ire=clamp01(s.hsyncNoise/100.0f)*5.0f;
                e.v_sync_noise_ire=clamp01(s.vsyncNoise/100.0f)*5.0f;
                e.burst_noise_ire=clamp01(s.burstNoise/100.0f)*5.0f;
                e.noise_stddev_ire=clamp01(s.compositeNoise/100.0f)*4.0f;
            }
            if(s.composite>0) {
                const float lumaCut=(s.tapeSpeed==1)?3000000.0f:((s.tapeSpeed==2)?2400000.0f:2000000.0f);
                const float chromaCut=(s.tapeSpeed==1)?627000.0f:((s.tapeSpeed==2)?400000.0f:370000.0f);
                const int chromaDelay=(s.tapeSpeed==1)?9:((s.tapeSpeed==2)?12:14);
                p.inputBandwidth.lumaCutoffHz=lumaCut;p.outputBandwidth.lumaCutoffHz=lumaCut;
                p.inputBandwidth.iCutoffHz=chromaCut;p.inputBandwidth.qCutoffHz=chromaCut;
                p.outputBandwidth.iCutoffHz=chromaCut;p.outputBandwidth.qCutoffHz=chromaCut;
                p.inputBandwidth.iDelaySamples=chromaDelay;p.inputBandwidth.qDelaySamples=chromaDelay;
                p.outputBandwidth.iDelaySamples=chromaDelay;p.outputBandwidth.qDelaySamples=chromaDelay;
            }
            if(s.geometry>0||s.interlace>0||s.verticalRoll>0||s.edgeWave>0) {
                p.geometryEnabled=true; p.geometry.seed=0x90A1106Eu; p.geometry.frame=s.frame;
                p.geometry.lineJitterPixels=s.geometry/100.0f*1.5f;
                p.geometry.topSkewPixels=s.geometry/100.0f*2.0f;
                p.geometry.edgePincushionPixels=s.geometry/100.0f*1.2f;
                p.geometry.interlaceAmount=clamp01(s.interlace/100.0f);
                p.geometry.verticalRoll=clamp01(s.verticalRoll/100.0f);
                p.geometry.edgeWaveAmplitudePixels=s.edgeWave/100.0f*8.0f;
                p.geometry.edgeWaveFrequency=s.edgeWaveFrequency; p.geometry.edgeWaveSpeed=s.edgeWaveSpeed; p.geometry.edgeWaveDetail=static_cast<int>(std::lround(s.edgeWaveDetail));
                p.geometry.horizontalScale=std::max(0.1f,float(w)/720.0f);
                p.geometry.verticalScale=std::max(0.1f,float(h)/480.0f);
            }
            if(s.tapeTransport>0||(s.trackingAmount>0&&s.trackingServo>0)||(s.tapeSurface>0&&(s.tapeSticky>0||s.tapeMold>0))) {
                const float a=clamp01(s.tapeTransport/100.0f);
                p.tapeTransportEnabled=true; p.tapeTransport.seed=0x90A11A6Eu; p.tapeTransport.frame=s.frame;
                p.tapeTransport.framesPerSecond=s.frameRate; p.tapeTransport.fieldRateHz=s.frameRate*2.0f;
                p.tapeTransport.drumFlutterPixels=1.2f*a; p.tapeTransport.flutterPixels=0.5f*a;
                p.tapeTransport.stickSlipPixels=0.35f*a; p.tapeTransport.lineJitterPixels=0.2f*a;
                p.tapeTransport.slowDriftPixelsPerSecond=0.08f*a;
                p.tapeTransport.servoJitterPixels=0.0f;
                p.tapeTransport.controlTrackServoAmount=(s.trackingAmount>0)?clamp01(s.trackingServo/100.0f):0.0f;
                p.tapeTransport.controlTrackPulseStrength=1.0f;
                p.tapeTransport.controlTrackPulseDropout=clamp01(s.dropouts/100.0f*0.35f);
                p.tapeTransport.controlTrackFalsePulseChance=clamp01(s.headAmount/100.0f*0.08f);
                p.tapeTransport.pictureSearchSpeed=1.0f; p.tapeTransport.backTension=std::clamp(s.backTension/100.0f,-1.0f,1.0f);
                p.tapeTransport.trackingTargetPixels=(s.trackingX-50.0f)*0.02f;
                p.tapeTransport.verticalScale=std::max(0.1f,float(h)/480.0f);
            }
            if(s.tapeSurface>0) {
                TapeMaterialControls tc;
                tc.master=s.tapeSurface; tc.wear=s.tapeWear; tc.dropouts=s.tapeDropouts;
                tc.scratches=s.tapeScratches; tc.creases=s.tapeCreases; tc.edgeDamage=s.tapeEdge;
                tc.mold=s.tapeMold; tc.vaseline=s.tapeVaseline; tc.sticky=s.tapeSticky;
                tc.cassetteSeed=static_cast<std::uint32_t>(std::max(1.0f,std::min(10000.0f,std::round(s.tapeSeed))));
                tc.tapeSpeed=static_cast<std::uint32_t>(std::clamp(int(s.tapeSpeed)-1,0,2));
                tc.trackingOffsetTracks=p.trackRead.trackingOffset; tc.frame=s.frame;
                p.tapeSurfaceEnabled=true; p.tapeSurface=makeTapeSurfaceParams(tc);
            }
            p.tapeSurface.tapeSpeed=static_cast<std::uint32_t>(std::clamp(int(s.tapeSpeed)-1,0,2));
            p.tapeSurface.trackingOffsetTracks=p.trackRead.trackingOffset;
            if(p.tapeTransportEnabled) {
                p.tapeTransport.usePhysicalControlTrackMap=true;
                p.tapeTransport.controlTrackTape=p.tapeSurface;
            }
            if(s.chromaSubsample>0) {
                p.chromaSubsampleEnabled=true; p.chromaSubsample.amount=clamp01(s.chromaSubsample/100.0f);
                p.chromaSubsample.horizontalFactor=2; p.chromaSubsample.verticalFactor=2;
                p.chromaSubsample.blurRadius=2.0f;
            }
            if(s.verticalChroma>0) {
                p.verticalChromaBlendEnabled=true; p.verticalChromaBlendAmount=clamp01(s.verticalChroma/100.0f);
            }
            if(s.verticalLuma>0) {
                p.outputBandwidth.verticalLumaRadiusRows=std::max(1,int(std::lround(s.verticalLuma/100.0f*4.0f)));
                p.outputBandwidth.verticalLumaSigmaRows=0.6f+s.verticalLuma/100.0f*1.4f;
            }
            if(s.edgeEnhance>0||s.tapeFade>0||s.gradeContrast!=100||s.gradeBrightness!=0||s.gradeSaturation!=100||s.vignette>0||s.flicker>0) {
                p.imageGradeEnabled=true; p.imageGrade.seed=0x90A11E5Eu; p.imageGrade.frame=s.frame;
                p.imageGrade.edgeEnhancement=clamp01(s.edgeEnhance/100.0f);
                p.imageGrade.fade=clamp01(s.tapeFade/100.0f);
                p.imageGrade.blackLift=clamp01(s.blackLift/100.0f);
                p.imageGrade.contrast=s.gradeContrast/100.0f;
                p.imageGrade.brightness=s.gradeBrightness/255.0f;
                p.imageGrade.saturation=s.gradeSaturation/100.0f;
                p.imageGrade.vignette=clamp01(s.vignette/100.0f);
                p.imageGrade.flicker=clamp01(s.flicker/100.0f);
            }
            if(s.afcHunt>0||s.chromaFlutter>0||s.ycCrosstalk>0||s.hsyncNoise>0||s.vsyncNoise>0||s.burstNoise>0||s.colorPhaseDrift>0||s.colorDrift>0) {
                p.receiverInstabilityEnabled=true; p.receiverInstability.seed=0x90A11E5Eu; p.receiverInstability.frame=s.frame;
                p.receiverInstability.afcHunt=clamp01(s.afcHunt/100.0f);
                p.receiverInstability.chromaFlutter=clamp01(s.chromaFlutter/100.0f);
                p.receiverInstability.ycCrosstalk=clamp01(s.ycCrosstalk/100.0f);
                p.receiverInstability.chromaPhaseDrift=clamp01(s.colorPhaseDrift/100.0f);
                p.receiverInstability.chromaColorDrift=clamp01(s.colorDrift/100.0f);
                p.receiverInstability.verticalScale=std::max(0.1f,float(h)/480.0f);
                p.receiverInstability.horizontalSyncNoise=clamp01(s.hsyncNoise/100.0f);
                p.receiverInstability.verticalSyncNoise=clamp01(s.vsyncNoise/100.0f);
                p.receiverInstability.burstNoise=clamp01(s.burstNoise/100.0f);
            }
            if(s.signalGhost>0||s.chromaDelay>0) {
                p.signalGhostEnabled=true; p.signalGhost.amount=clamp01(s.signalGhost/100.0f);
                p.signalGhost.chromaDelayPixels=s.chromaDelay;
                p.signalGhost.horizontalScale=std::max(0.1f,float(w)/720.0f);
            }
            if(s.multipathGain>0||s.multipathEnsemble>0) {
                p.multipathEchoEnabled=true; p.multipathEcho.seed=0x90A11E5Eu; p.multipathEcho.frame=s.frame;
                p.multipathEcho.gain=clamp01(s.multipathGain/100.0f);
                p.multipathEcho.delayPixels=s.multipathDelay;
                p.multipathEcho.ensemble=clamp01(s.multipathEnsemble/100.0f);
            }
            if(s.dropouts>0) {
                p.dropoutEnabled=true; p.dropouts.amount=clamp01(s.dropouts/100.0f);
                p.dropouts.seed=0x90D09A75u; p.dropouts.frame=s.frame;
                p.dropouts.horizontalScale=std::max(0.1f,float(w)/720.0f);
                p.dropouts.verticalScale=std::max(0.1f,float(h)/480.0f);
            }
            if(s.impulseNoise>0) {
                p.impulseNoiseEnabled=true; p.impulseNoise.seed=0x90A11E5Eu; p.impulseNoise.frame=s.frame;
                p.impulseNoise.amount=clamp01(s.impulseNoise/100.0f);
            }
            if(s.rfDrift>0||s.rfHum>0||s.rfAm>0) {
                p.rfInstabilityEnabled=true; p.rfInstability.seed=0x90A11E5Eu; p.rfInstability.frame=s.frame;
                p.rfInstability.drift=clamp01(s.rfDrift/100.0f);
                p.rfInstability.hum=clamp01(s.rfHum/100.0f);
                p.rfInstability.amNonlinearity=clamp01(s.rfAm/100.0f);
                p.rfInstability.agcPump=clamp01(s.agcPump/100.0f);
            }
            if(s.lumaNoise>0||s.chromaNoise>0||s.chromaLoss>0||s.compositeNoise>0||s.ntscRsPhaseNoise>0||s.signalHueNoise>0||(s.vhs6RiftNoiseEnable&&s.vhs6RiftGrain>0)) {
                p.analogNoiseEnabled=true; p.analogNoise.seed=static_cast<std::uint32_t>(std::clamp(std::lround(s.vhs6RiftNoiseSeed),1l,32767l)); p.analogNoise.frame=s.frame;
                p.analogNoise.proceduralNoiseEnabled=s.vhs6RiftNoiseEnable;
                p.analogNoise.grainAmount=clamp01(s.vhs6RiftGrain/100.0f); p.analogNoise.grainSize=std::clamp(int(std::lround(s.vhs6RiftGrainSize)),1,5);
                p.analogNoise.noiseDetail=clamp01(s.vhs6RiftNoiseDetail/100.0f); p.analogNoise.shadowBias=clamp01(s.vhs6RiftShadowBias/100.0f);
                p.analogNoise.lumaAmount=clamp01(s.lumaNoise/100.0f);
                p.analogNoise.chromaAmount=clamp01(s.chromaNoise/100.0f);
                p.analogNoise.hueNoiseAmount=clamp01(s.signalHueNoise/100.0f);
                p.analogNoise.compositeAmount=clamp01(s.compositeNoise/100.0f);
                p.analogNoise.chromaLoss=s.chromaLoss;
                p.analogNoise.chromaPhaseNoise=clamp01(s.ntscRsPhaseNoise/100.0f);
                p.analogNoise.hueBiasDegrees=s.noiseHue;
                p.analogNoise.tapeLineAmount=clamp01(s.tapeNoise/100.0f); p.analogNoise.tapeLineSpeed=1.0f; p.analogNoise.framesPerSecond=s.frameRate;
            }
            if(s.lumaSmear>0) {
                p.lumaSmearEnabled=true; p.lumaSmear.strength=clamp01(s.lumaSmear/100.0f);
                p.lumaSmear.tapCount=16; p.lumaSmear.decay=0.65f; p.lumaSmear.offsetPx=1.0f;
                p.lumaSmear.highlightStrength=0.18f;
            }
            if(s.analogGlow>0) {
                p.analogGlowEnabled=true; p.analogGlow.amount=clamp01(s.analogGlow/100.0f);
                p.analogGlow.horizontalScale=std::max(0.1f,float(w)/720.0f);
            }
            if(s.bloom>0) {
                p.bloomEnabled=true; p.bloom.threshold=0.72f; p.bloom.intensity=clamp01(s.bloom/100.0f);
                p.bloom.spread=2.0f; p.bloom.levels=3;
            }
            if(s.scanlines>0) {
                p.scanlinesEnabled=true; p.scanlines.amount=clamp01(s.scanlines/100.0f);
                p.scanlines.frequency=0.5f; p.scanlines.speed=0.0f; p.scanlines.frame=s.frame;
            }
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processCompositeRoundTrip(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&s.staticAmount>0) {
            StaticParams p; p.intensity=clamp01(s.staticAmount/100.0f*master);
            p.sourceMix=1.0f; p.density=clamp01(s.staticDensity/100.0f); p.seed=0x905A71C1u;
            p.frame=s.frame; p.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processStaticPattern(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&s.sync>0) {
            SyncCollapseParams p; p.strength=clamp01(s.sync/100.0f*master);
            p.frame=s.frame; p.timeSeconds=s.timeSeconds; p.seed=0x90C011A5u;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processSyncCollapse(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&(s.trackingAmount>0||s.headAmount>0)) {
            DamageRegion regions[2]; std::size_t count=0;
            if(s.trackingAmount>0) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::TrackingNoise;
                r.shape=static_cast<RegionShape>(std::max<A_long>(1,std::min<A_long>(3,s.trackingShape))-1);
                r.centerX=clamp01(s.trackingX/100.0f); r.centerY=clamp01(s.trackingY/100.0f);
                r.angleRadians=s.trackingAngle*0.01745329251994329577f;
                r.width=clamp01(s.trackingWidth/100.0f); r.height=clamp01(s.trackingHeight/100.0f);
                r.intensity=clamp01(s.trackingAmount/100.0f*master);
                r.motionXPerSecond=s.trackingMotionX/100.0f; r.motionYPerSecond=s.trackingMotionY/100.0f;
                r.servoDriftPx=s.trackingServo; r.tearProbability=clamp01(s.trackingTear/100.0f);
            }
            if(s.headAmount>0) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::HeadSwitch; r.shape=RegionShape::HorizontalBand;
                r.centerX=0.5f; r.width=1.0f; r.height=clamp01(s.headHeight/100.0f);
                r.centerY=clamp01(s.headY/100.0f)-r.height*0.5f;
                r.intensity=clamp01(s.headAmount/100.0f*master); r.maxShiftPx=s.headShift;
                r.noiseAmount=0.25f; r.dropoutProbability=0.2f;
            }
            RegionEngineParams p; p.regions=regions; p.regionCount=count; p.seed=0x90DA6A6Eu;
            p.frame=s.frame; p.timeSeconds=s.timeSeconds; p.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processDamageRegions(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(s.deviceConnectionAmount>0.0f) {
            DeviceConnectionParams dp; dp.amount=clamp01(s.deviceConnectionAmount/100.0f*master);
            dp.density=clamp01(s.deviceConnectionDensity/100.0f); dp.nodeSize=s.deviceConnectionSize;
            dp.speed=s.deviceConnectionSpeed; dp.distribution=clamp01(s.deviceConnectionDistribution/100.0f);
            dp.seed=static_cast<std::uint32_t>(std::clamp(std::lround(s.deviceConnectionSeed),1l,32767l));
            dp.frame=s.frame; dp.framesPerSecond=s.frameRate; dp.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processDeviceConnections(in,out,dp)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(osdEnabled) {
            VhsOsdParams op;
            op.amount=clamp01(s.osdAmount/100.0f); op.opacity=clamp01(s.osdOpacity/100.0f);
            op.scale=std::max(0.35f,s.osdScale/100.0f); op.glitch=std::max(0.0f,s.osdGlitch/100.0f);
            op.ghost=clamp01(s.osdGhost/100.0f); op.recAmount=clamp01(s.osdRec/100.0f);
            op.playAmount=clamp01(s.osdPlay/100.0f); op.timestampAmount=clamp01(s.osdTimestamp/100.0f);
            op.recBlinkHz=std::max(0.0f,s.osdRecBlink); op.timestampSpeed=std::max(0.0f,s.osdTimeSpeed);
            op.timeSeconds=s.timeSeconds; op.framesPerSecond=s.frameRate; op.frame=s.frame;
            op.startYear=static_cast<int>(std::lround(s.osdYear)); op.startMonth=static_cast<int>(std::lround(s.osdMonth));
            op.startDay=static_cast<int>(std::lround(s.osdDay)); op.startHour=static_cast<int>(std::lround(s.osdHour));
            op.startMinute=static_cast<int>(std::lround(s.osdMinute)); op.startSecond=static_cast<int>(std::lround(s.osdSecond));
            op.recColorR=s.osdRecColorR/255.0f; op.recColorG=s.osdRecColorG/255.0f; op.recColorB=s.osdRecColorB/255.0f;
            op.seed=0x9050D5u; op.premultipliedAlpha=true;
            ImageView osdView{working.data(),w,h,w*4};
            if(!applyVhsOsd(osdView,op)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
        }
        for(int y=0;y<output->height;++y) for(int x=0;x<output->width;++x) {
            const int sx=output->origin_x+x-input->origin_x, sy=output->origin_y+y-input->origin_y;
            if(sx<0||sx>=w||sy<0||sy>=h) { const float z[4]={0,0,0,0}; WritePixel(output,format,x,y,z); continue; }
            WritePixel(output,format,x,y,working.data()+(std::size_t(sy)*w+sx)*4);
        }
        return PF_Err_NONE;
    } catch(...) { return PF_Err_OUT_OF_MEMORY; }
}

static PF_Err SmartPreRender(PF_InData* in_data,PF_PreRenderExtra* extra) {
    auto* s=new(std::nothrow) RenderSettings();
    if(!s) return PF_Err_OUT_OF_MEMORY;
    PF_Err err=ReadSettings(in_data,*s);
    if(err!=PF_Err_NONE) { delete s; return err; }
    PF_RenderRequest req=extra->input->output_request;
    req.rect.left=0; req.rect.top=0; req.rect.right=in_data->width; req.rect.bottom=in_data->height;
    PF_CheckoutResult result; AEFX_CLR_STRUCT(result);
    err=extra->cb->checkout_layer(in_data->effect_ref,PARAM_INPUT,kInputCheckoutID,&req,
        in_data->current_time,in_data->time_step,in_data->time_scale,&result);
    if(err!=PF_Err_NONE) { delete s; return err; }
    // Checkout the immediately preceding source frame through SmartFX's declared
    // checkout ID; no global frame cache is used, so MFR/out-of-order renders stay safe.
    if(s->temporalGhost>0 && in_data->time_step>0 && in_data->current_time>=in_data->time_step) {
        PF_CheckoutResult previousResult; AEFX_CLR_STRUCT(previousResult);
        const A_long previousTime=in_data->current_time-in_data->time_step;
        const PF_Err previousErr=extra->cb->checkout_layer(in_data->effect_ref,PARAM_INPUT,kInputCheckoutID+1,&req,
            previousTime,in_data->time_step,in_data->time_scale,&previousResult);
        s->previousFrameAvailable=(previousErr==PF_Err_NONE);
    }
    extra->output->result_rect=extra->input->output_request.rect;
    extra->output->max_result_rect=result.max_result_rect;
    extra->output->solid=result.solid;
    extra->output->pre_render_data=s;
    extra->output->delete_pre_render_data_func=DeletePreRenderData;
    return PF_Err_NONE;
}
static PF_Err SmartRender(PF_InData* in_data,PF_OutData* out_data,PF_SmartRenderExtra* extra) {
    const auto* s=static_cast<const RenderSettings*>(extra->input->pre_render_data);
    if(!s) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    PF_EffectWorld* inWorld=nullptr; PF_EffectWorld* previousWorld=nullptr; PF_EffectWorld* outWorld=nullptr; bool checked=false,previousChecked=false;
    PF_Err err=extra->cb->checkout_layer_pixels(in_data->effect_ref,kInputCheckoutID,&inWorld);
    if(err==PF_Err_NONE) checked=true;
    if(err==PF_Err_NONE && s->previousFrameAvailable) {
        const PF_Err previousErr=extra->cb->checkout_layer_pixels(in_data->effect_ref,kInputCheckoutID+1,&previousWorld);
        if(previousErr==PF_Err_NONE) previousChecked=true;
    }
    if(err==PF_Err_NONE) err=extra->cb->checkout_output(in_data->effect_ref,&outWorld);
    if(err==PF_Err_NONE) {
        AEFX_SuiteScoper<PF_WorldSuite2> suite(in_data,kPFWorldSuite,kPFWorldSuiteVersion2,out_data);
        PF_PixelFormat inFmt=PF_PixelFormat_INVALID,outFmt=PF_PixelFormat_INVALID;
        err=suite->PF_GetPixelFormat(inWorld,&inFmt);
        if(err==PF_Err_NONE) err=suite->PF_GetPixelFormat(outWorld,&outFmt);
        if(err==PF_Err_NONE&&inFmt!=outFmt) err=PF_Err_UNRECOGNIZED_PARAM_TYPE;
        if(err==PF_Err_NONE&&previousChecked&&previousWorld) {
            PF_PixelFormat previousFmt=PF_PixelFormat_INVALID;
            const PF_Err previousFmtErr=suite->PF_GetPixelFormat(previousWorld,&previousFmt);
            if(previousFmtErr!=PF_Err_NONE||previousFmt!=inFmt) previousWorld=nullptr;
        }
        if(err==PF_Err_NONE) err=RenderWorld(*s,inWorld,outWorld,inFmt,previousWorld);
    }
    if(previousChecked) {
        const PF_Err previousCheckin=extra->cb->checkin_layer_pixels(in_data->effect_ref,kInputCheckoutID+1);
        if(err==PF_Err_NONE) err=previousCheckin;
    }
    if(checked) {
        const PF_Err checkin=extra->cb->checkin_layer_pixels(in_data->effect_ref,kInputCheckoutID);
        if(err==PF_Err_NONE) err=checkin;
    }
    return err;
}
static PF_Err LegacyRender(PF_InData* in_data,PF_OutData* out_data,PF_ParamDef* params[],PF_LayerDef* output) {
    if(!params||!params[PARAM_INPUT]||!output) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    RenderSettings s;
    s.master=float(params[PARAM_MASTER]->u.fs_d.value);
    s.composite=float(params[PARAM_COMPOSITE_AMOUNT]->u.fs_d.value);
    s.sync=float(params[PARAM_SYNC_AMOUNT]->u.fs_d.value);
    s.colorSystem=params[PARAM_PAL_MODE]->u.pd.value;
    s.signalModel=params[PARAM_SIGNAL_MODEL]->u.pd.value;
    s.tapeSpeed=params[PARAM_TAPE_SPEED]->u.pd.value;
    s.tapeDetail=float(params[PARAM_TAPE_DETAIL]->u.fs_d.value);
    s.trackRead=float(params[PARAM_TRACK_READ]->u.fs_d.value);
    s.headCondition=float(params[PARAM_HEAD_CONDITION]->u.fs_d.value);
    s.tapeWear=float(params[PARAM_TAPE_WEAR]->u.fs_d.value); s.tapeDropouts=float(params[PARAM_TAPE_DROPOUTS]->u.fs_d.value); s.tapeScratches=float(params[PARAM_TAPE_SCRATCHES]->u.fs_d.value);
    s.tapeCreases=float(params[PARAM_TAPE_CREASES]->u.fs_d.value); s.tapeEdge=float(params[PARAM_TAPE_EDGE]->u.fs_d.value); s.tapeMold=float(params[PARAM_TAPE_MOLD]->u.fs_d.value);
    s.tapeVaseline=float(params[PARAM_TAPE_VASELINE]->u.fs_d.value); s.tapeSticky=float(params[PARAM_TAPE_STICKY]->u.fs_d.value); s.tapeSeed=float(params[PARAM_TAPE_SEED]->u.fs_d.value); s.backTension=float(params[PARAM_TAPE_BACK_TENSION]->u.fs_d.value);
    s.trackMisalignment=float(params[PARAM_TRACK_MISALIGNMENT]->u.fs_d.value);
    s.chromaShift=float(params[PARAM_CHROMA_SHIFT]->u.fs_d.value);
    s.chromaSubsample=float(params[PARAM_CHROMA_SUBSAMPLE]->u.fs_d.value);
    s.verticalChroma=float(params[PARAM_VERTICAL_CHROMA]->u.fs_d.value);
    s.verticalLuma=float(params[PARAM_VERTICAL_LUMA]->u.fs_d.value);
    s.lumaNoise=float(params[PARAM_LUMA_NOISE]->u.fs_d.value);
    s.chromaNoise=float(params[PARAM_CHROMA_NOISE]->u.fs_d.value);
    s.chromaLoss=float(params[PARAM_CHROMA_LOSS]->u.fs_d.value);
    s.compositeNoise=float(params[PARAM_COMPOSITE_NOISE]->u.fs_d.value);
    s.rfDrift=float(params[PARAM_RF_DRIFT]->u.fs_d.value);
    s.rfHum=float(params[PARAM_RF_HUM]->u.fs_d.value);
    s.rfAm=float(params[PARAM_RF_AM]->u.fs_d.value);
    s.agcPump=float(params[PARAM_AGC_PUMP]->u.fs_d.value);
    s.impulseNoise=float(params[PARAM_IMPULSE_NOISE]->u.fs_d.value);
    s.dropouts=float(params[PARAM_DROPOUTS]->u.fs_d.value);
    s.multipathGain=float(params[PARAM_MULTIPATH_GAIN]->u.fs_d.value);
    s.multipathDelay=float(params[PARAM_MULTIPATH_DELAY]->u.fs_d.value);
    s.multipathEnsemble=float(params[PARAM_MULTIPATH_ENSEMBLE]->u.fs_d.value);
    s.signalGhost=float(params[PARAM_SIGNAL_GHOST]->u.fs_d.value);
    s.chromaDelay=float(params[PARAM_CHROMA_DELAY]->u.fs_d.value);
    s.afcHunt=float(params[PARAM_AFC_HUNT]->u.fs_d.value);
    s.chromaFlutter=float(params[PARAM_CHROMA_FLUTTER]->u.fs_d.value);
    s.ycCrosstalk=float(params[PARAM_YC_CROSSTALK]->u.fs_d.value);
    s.colorPhaseDrift=float(params[PARAM_COLOR_PHASE_DRIFT]->u.fs_d.value);
    s.colorDrift=float(params[PARAM_COLOR_DRIFT]->u.fs_d.value);
    s.hsyncNoise=float(params[PARAM_HSYNC_NOISE]->u.fs_d.value);
    s.vsyncNoise=float(params[PARAM_VSYNC_NOISE]->u.fs_d.value);
    s.burstNoise=float(params[PARAM_BURST_NOISE]->u.fs_d.value);
    s.edgeEnhance=float(params[PARAM_EDGE_ENHANCE]->u.fs_d.value);
    s.tapeFade=float(params[PARAM_TAPE_FADE]->u.fs_d.value);
    s.blackLift=float(params[PARAM_BLACK_LIFT]->u.fs_d.value);
    s.temporalGhost=float(params[PARAM_TEMPORAL_GHOST]->u.fs_d.value);
    s.ringing=float(params[PARAM_RINGING]->u.fs_d.value);
    s.ringingPower=float(params[PARAM_RINGING_POWER]->u.fs_d.value);
    s.crtEqualizer=float(params[PARAM_CRT_EQUALIZER]->u.fs_d.value);
    s.gradeContrast=float(params[PARAM_GRADE_CONTRAST]->u.fs_d.value);
    s.gradeBrightness=float(params[PARAM_GRADE_BRIGHTNESS]->u.fs_d.value);
    s.gradeSaturation=float(params[PARAM_GRADE_SATURATION]->u.fs_d.value);
    s.vignette=float(params[PARAM_VIGNETTE]->u.fs_d.value);
    s.flicker=float(params[PARAM_FLICKER]->u.fs_d.value);
    s.noiseHue=float(params[PARAM_NOISE_HUE]->u.fs_d.value);
    s.lumaSmear=float(params[PARAM_LUMA_SMEAR]->u.fs_d.value);
    s.bloom=float(params[PARAM_BLOOM]->u.fs_d.value);
    s.analogGlow=float(params[PARAM_ANALOG_GLOW]->u.fs_d.value);
    s.tapeSurface=float(params[PARAM_TAPE_SURFACE]->u.fs_d.value);
    s.tapeTransport=float(params[PARAM_TAPE_TRANSPORT]->u.fs_d.value);
    s.geometry=float(params[PARAM_GEOMETRY]->u.fs_d.value);
    s.interlace=float(params[PARAM_INTERLACE]->u.fs_d.value);
    s.verticalRoll=float(params[PARAM_VERTICAL_ROLL]->u.fs_d.value);
    s.edgeWave=float(params[PARAM_EDGE_WAVE]->u.fs_d.value);
    s.scanlines=float(params[PARAM_SCANLINES]->u.fs_d.value);
    s.tapeNoise=float(params[PARAM_TAPE_NOISE]->u.fs_d.value);
    s.staticAmount=float(params[PARAM_STATIC_AMOUNT]->u.fs_d.value);
    s.staticDensity=float(params[PARAM_STATIC_DENSITY]->u.fs_d.value);
    s.trackingAmount=float(params[PARAM_TRACKING_AMOUNT]->u.fs_d.value);
    s.trackingShape=params[PARAM_TRACKING_SHAPE]->u.pd.value;
    s.trackingX=float(params[PARAM_TRACKING_CENTER_X]->u.fs_d.value);
    s.trackingY=float(params[PARAM_TRACKING_CENTER_Y]->u.fs_d.value);
    s.trackingWidth=float(params[PARAM_TRACKING_WIDTH]->u.fs_d.value);
    s.trackingHeight=float(params[PARAM_TRACKING_HEIGHT]->u.fs_d.value);
    s.trackingAngle=float(params[PARAM_TRACKING_ANGLE]->u.fs_d.value);
    s.trackingMotionX=float(params[PARAM_TRACKING_MOTION_X]->u.fs_d.value);
    s.trackingMotionY=float(params[PARAM_TRACKING_MOTION_Y]->u.fs_d.value);
    s.trackingServo=float(params[PARAM_TRACKING_SERVO]->u.fs_d.value);
    s.trackingTear=float(params[PARAM_TRACKING_TEAR]->u.fs_d.value);
    s.headAmount=float(params[PARAM_HEAD_AMOUNT]->u.fs_d.value);
    s.headY=float(params[PARAM_HEAD_BOTTOM_Y]->u.fs_d.value);
    s.headHeight=float(params[PARAM_HEAD_HEIGHT]->u.fs_d.value);
    s.headShift=float(params[PARAM_HEAD_SHIFT]->u.fs_d.value);
    s.osdAmount=float(params[PARAM_OSD_AMOUNT]->u.fs_d.value); s.osdRec=float(params[PARAM_OSD_REC]->u.fs_d.value);
    s.osdPlay=float(params[PARAM_OSD_PLAY]->u.fs_d.value); s.osdTimestamp=float(params[PARAM_OSD_TIMESTAMP]->u.fs_d.value);
    s.osdOpacity=float(params[PARAM_OSD_OPACITY]->u.fs_d.value); s.osdScale=float(params[PARAM_OSD_SCALE]->u.fs_d.value);
    s.osdGlitch=float(params[PARAM_OSD_GLITCH]->u.fs_d.value); s.osdGhost=float(params[PARAM_OSD_GHOST]->u.fs_d.value);
    s.osdRecBlink=float(params[PARAM_OSD_REC_BLINK]->u.fs_d.value); s.osdTimeSpeed=float(params[PARAM_OSD_TIME_SPEED]->u.fs_d.value);
    s.osdYear=float(params[PARAM_OSD_YEAR]->u.fs_d.value); s.osdMonth=float(params[PARAM_OSD_MONTH]->u.fs_d.value); s.osdDay=float(params[PARAM_OSD_DAY]->u.fs_d.value);
    s.osdHour=float(params[PARAM_OSD_HOUR]->u.fs_d.value); s.osdMinute=float(params[PARAM_OSD_MINUTE]->u.fs_d.value); s.osdSecond=float(params[PARAM_OSD_SECOND]->u.fs_d.value);
    s.osdRecColorR=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.red); s.osdRecColorG=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.green); s.osdRecColorB=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.blue);
    s.vhsStyle=params[PARAM_VHS_STYLE]->u.pd.value; s.vhsStyleIntensity=float(params[PARAM_VHS_STYLE_INTENSITY]->u.fs_d.value);
    s.cameraProfile=params[PARAM_CAMERA_PROFILE]->u.pd.value; s.cameraIntensity=float(params[PARAM_CAMERA_INTENSITY]->u.fs_d.value);
    s.ntscRsLook=params[PARAM_NTSCRS_LOOK]->u.pd.value; s.ntscRsIntensity=float(params[PARAM_NTSCRS_INTENSITY]->u.fs_d.value);
    s.ntscRsPhaseNoise=float(params[PARAM_NTSCRS_PHASE_NOISE]->u.fs_d.value);
    s.signalHueNoise=float(params[PARAM_SIGNAL_HUE_NOISE]->u.fs_d.value);
    s.deviceConnectionAmount=float(params[PARAM_DEVICE_CONNECTION_AMOUNT]->u.fs_d.value); s.deviceConnectionDensity=float(params[PARAM_DEVICE_CONNECTION_DENSITY]->u.fs_d.value); s.deviceConnectionSize=float(params[PARAM_DEVICE_CONNECTION_SIZE]->u.fs_d.value); s.deviceConnectionSpeed=float(params[PARAM_DEVICE_CONNECTION_SPEED]->u.fs_d.value); s.deviceConnectionDistribution=float(params[PARAM_DEVICE_CONNECTION_DISTRIBUTION]->u.fs_d.value); s.deviceConnectionSeed=float(params[PARAM_DEVICE_CONNECTION_SEED]->u.fs_d.value);
    s.edgeWaveFrequency=float(params[PARAM_EDGE_WAVE_FREQUENCY]->u.fs_d.value); s.edgeWaveSpeed=float(params[PARAM_EDGE_WAVE_SPEED]->u.fs_d.value); s.edgeWaveDetail=float(params[PARAM_EDGE_WAVE_DETAIL]->u.fs_d.value);
    s.frame=in_data->time_step!=0?in_data->current_time/in_data->time_step:in_data->current_time;
    s.timeSeconds=in_data->time_scale!=0?double(in_data->current_time)/double(in_data->time_scale):0.0;
    s.frameRate=(in_data->time_step>0&&in_data->time_scale>0)?float(in_data->time_scale)/float(in_data->time_step):29.97f;
    PF_EffectWorld* inWorld=&params[PARAM_INPUT]->u.ld; PF_EffectWorld* outWorld=output;
    AEFX_SuiteScoper<PF_WorldSuite2> suite(in_data,kPFWorldSuite,kPFWorldSuiteVersion2,out_data);
    PF_PixelFormat inFmt=PF_PixelFormat_INVALID,outFmt=PF_PixelFormat_INVALID;
    PF_Err err=suite->PF_GetPixelFormat(inWorld,&inFmt);
    if(err==PF_Err_NONE) err=suite->PF_GetPixelFormat(outWorld,&outFmt);
    if(err==PF_Err_NONE&&inFmt!=outFmt) err=PF_Err_UNRECOGNIZED_PARAM_TYPE;
    if(err==PF_Err_NONE) err=RenderWorld(s,inWorld,outWorld,inFmt);
    return err;
}
} // namespace

extern "C" DllExport PF_Err EffectMain(PF_Cmd cmd,PF_InData* in_data,PF_OutData* out_data,
    PF_ParamDef* params[],PF_LayerDef* output,void* extra) {
    (void)params; (void)output;
    PF_Err err=PF_Err_NONE;
    try {
        switch(cmd) {
            case PF_Cmd_ABOUT: PF_STRCPY(out_data->return_msg,"VHS-90 1.0.0 - Native procedural VHS / NTSC signal engine."); break;
            case PF_Cmd_GLOBAL_SETUP: err=GlobalSetup(in_data,out_data); break;
            case PF_Cmd_PARAMS_SETUP: err=ParamsSetup(in_data,out_data); break;
            case PF_Cmd_RENDER: err=LegacyRender(in_data,out_data,params,output); break;
            case PF_Cmd_SMART_PRE_RENDER: err=SmartPreRender(in_data,static_cast<PF_PreRenderExtra*>(extra)); break;
            case PF_Cmd_SMART_RENDER: err=SmartRender(in_data,out_data,static_cast<PF_SmartRenderExtra*>(extra)); break;
            default: break;
        }
    } catch(...) { err=PF_Err_INTERNAL_STRUCT_DAMAGED; }
    return err;
}

extern "C" DllExport PF_Err PluginDataEntryFunction2(PF_PluginDataPtr inPtr,
    PF_PluginDataCB2 callback,SPBasicSuite* spBasic,const char* hostName,const char* hostVersion) {
    (void)spBasic; (void)hostName; (void)hostVersion;
    return PF_REGISTER_EFFECT_EXT2(inPtr,callback,"VHS-90","com.vhs90.effect",
        "Analog & Tape Effects",AE_RESERVED_INFO,"EffectMain",
        "https://github.com/vamprieieuu/vhs---90");
}
