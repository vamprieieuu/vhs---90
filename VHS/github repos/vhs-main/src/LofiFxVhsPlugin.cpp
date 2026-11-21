// ============================================================
// Lofi FX VHS — OFX plugin entry
//
// macOS-only (Metal) for v1. Linux/Windows compile but render via
// CPU passthrough fallback (since the CUDA kernel is a stub).
// ============================================================

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "ofxCore.h"
#include "ofxImageEffect.h"
#include "ofxParam.h"
#include "ofxProperty.h"

#include "VhsParams.h"
#include "CubeParser.h"
#include "LutPack.h"
#include "PresetParser.h"
#include "OsdRenderer.h"

#ifdef __APPLE__
#include "MetalKernels.h"
#endif
#include "CudaKernels.h"

// ============================================================
// Suite pointers
// ============================================================
static const OfxPropertySuiteV1*    gPropHost   = nullptr;
static const OfxImageEffectSuiteV1* gEffectHost = nullptr;
static const OfxParameterSuiteV1*   gParamHost  = nullptr;
static OfxHost*                     gHost       = nullptr;

// ============================================================
// Per-instance data
// ============================================================
struct InstanceData {
    std::vector<CubeLut> inputCsts;
    std::vector<CubeLut> outputCsts;
    int   currentInputCstIdx;
    int   currentOutputCstIdx;
    void* metalInputCstBuffer;
    void* metalOutputCstBuffer;

    void* metalOsdBuffer;
    int   osdW, osdH;

    std::vector<MasterPreset> masterPresets;
    int   currentMasterPreset;
    bool  applyingPreset;
    std::map<std::string, double> presetOverrides;

    InstanceData()
        : currentInputCstIdx(-1), currentOutputCstIdx(-1),
          metalInputCstBuffer(nullptr), metalOutputCstBuffer(nullptr),
          metalOsdBuffer(nullptr), osdW(0), osdH(0),
          currentMasterPreset(-1), applyingPreset(false) {}
};

// ============================================================
// Utilities
// ============================================================
static std::string getBundlePath(OfxImageEffectHandle effect) {
    OfxPropertySetHandle effectProps;
    gEffectHost->getPropertySet(effect, &effectProps);
    char* path = nullptr;
    gPropHost->propGetString(effectProps, kOfxPluginPropFilePath, 0, &path);
    return path ? std::string(path) : std::string();
}

static InstanceData* getInstanceData(OfxImageEffectHandle effect) {
    OfxPropertySetHandle props;
    gEffectHost->getPropertySet(effect, &props);
    void* ptr = nullptr;
    gPropHost->propGetPointer(props, kOfxPropInstanceData, 0, &ptr);
    return static_cast<InstanceData*>(ptr);
}
static void setInstanceData(OfxImageEffectHandle effect, InstanceData* d) {
    OfxPropertySetHandle props;
    gEffectHost->getPropertySet(effect, &props);
    gPropHost->propSetPointer(props, kOfxPropInstanceData, 0, d);
}
// Thread-local preset override context. When non-null, getInt/getDouble/getBool
// check this map first for the requested param name. Set at the start of
// actionRender / actionIsIdentity and cleared after.
static thread_local const std::map<std::string, double>* t_presetOverride = nullptr;
static double getDouble(OfxParamSetHandle ps, const char* name, double t) {
    if (t_presetOverride) {
        auto it = t_presetOverride->find(name);
        if (it != t_presetOverride->end()) return it->second;
    }
    OfxParamHandle p = nullptr; double v = 0;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return v;
    gParamHost->paramGetValueAtTime(p, t, &v);
    return v;
}
static int getInt(OfxParamSetHandle ps, const char* name) {
    if (t_presetOverride) {
        auto it = t_presetOverride->find(name);
        if (it != t_presetOverride->end()) return (int)it->second;
    }
    OfxParamHandle p = nullptr; int v = 0;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return v;
    gParamHost->paramGetValue(p, &v);
    return v;
}
static bool getBool(OfxParamSetHandle ps, const char* name) {
    return getInt(ps, name) != 0;
}
static std::string getString(OfxParamSetHandle ps, const char* name) {
    OfxParamHandle p = nullptr; char* s = nullptr;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return "";
    if (gParamHost->paramGetValue(p, (void*)&s) != kOfxStatOK || !s) return "";
    std::string r(s);
    return r;
}
// Preset-aware param getters: if an active preset overrides this param, return the preset value
static int getIntP(OfxParamSetHandle ps, const char* name, InstanceData* d) {
    if (d && !d->presetOverrides.empty()) {
        auto it = d->presetOverrides.find(name);
        if (it != d->presetOverrides.end()) return (int)it->second;
    }
    return getInt(ps, name);
}
static double getDoubleP(OfxParamSetHandle ps, const char* name, double t, InstanceData* d) {
    if (d && !d->presetOverrides.empty()) {
        auto it = d->presetOverrides.find(name);
        if (it != d->presetOverrides.end()) return it->second;
    }
    return getDouble(ps, name, t);
}
static bool getBoolP(OfxParamSetHandle ps, const char* name, InstanceData* d) {
    return getIntP(ps, name, d) != 0;
}
static void setIntParam(OfxParamSetHandle ps, const char* name, int v) {
    OfxParamHandle p = nullptr;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return;
    gParamHost->paramSetValue(p, v);
}
static void setDoubleParam(OfxParamSetHandle ps, const char* name, double v) {
    OfxParamHandle p = nullptr;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return;
    gParamHost->paramSetValue(p, v);
}

static std::string getUserPresetDir() {
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
    if (!home || !home[0]) return {};
    std::string dir = std::string(home) + "/Documents/Lofi FX VHS Presets";
#ifdef _WIN32
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    return dir;
}

static void setParamEnabled(OfxImageEffectHandle effect, const char* name, bool en) {
    OfxParamSetHandle ps;
    if (gEffectHost->getParamSet(effect, &ps) != kOfxStatOK) return;
    OfxParamHandle p = nullptr;
    if (gParamHost->paramGetHandle(ps, name, &p, nullptr) != kOfxStatOK || !p) return;
    OfxPropertySetHandle props = nullptr;
    if (gParamHost->paramGetPropertySet(p, &props) != kOfxStatOK || !props) return;
    gPropHost->propSetInt(props, kOfxParamPropEnabled, 0, en ? 1 : 0);
}
static void setGroupOpen(OfxImageEffectHandle effect, const char* g, bool open) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    OfxParamHandle h;
    if (gParamHost->paramGetHandle(ps, g, &h, nullptr) == kOfxStatOK) {
        OfxPropertySetHandle pr;
        gParamHost->paramGetPropertySet(h, &pr);
        gPropHost->propSetInt(pr, kOfxParamPropGroupOpen, 0, open ? 1 : 0);
    }
}

// ============================================================
// Param-define helpers — keep DescribeInContext compact
// ============================================================
static OfxPropertySetHandle defParam(OfxParamSetHandle ps, const char* type,
                                    const char* name, const char* label,
                                    const char* group)
{
    OfxPropertySetHandle pp;
    gParamHost->paramDefine(ps, type, name, &pp);
    gPropHost->propSetString(pp, kOfxPropLabel, 0, label);
    if (group) gPropHost->propSetString(pp, kOfxParamPropParent, 0, group);
    return pp;
}

static void defGroup(OfxParamSetHandle ps, const char* name, const char* label,
                     bool open)
{
    OfxPropertySetHandle pp;
    gParamHost->paramDefine(ps, kOfxParamTypeGroup, name, &pp);
    gPropHost->propSetString(pp, kOfxPropLabel, 0, label);
    gPropHost->propSetInt(pp, kOfxParamPropGroupOpen, 0, open ? 1 : 0);
}

static void defBool(OfxParamSetHandle ps, const char* name, const char* label,
                    const char* group, bool defVal)
{
    OfxPropertySetHandle pp = defParam(ps, kOfxParamTypeBoolean, name, label, group);
    gPropHost->propSetInt(pp, kOfxParamPropDefault, 0, defVal ? 1 : 0);
}

static void defDbl(OfxParamSetHandle ps, const char* name, const char* label,
                   const char* group,
                   double defV, double mn, double mx)
{
    OfxPropertySetHandle pp = defParam(ps, kOfxParamTypeDouble, name, label, group);
    gPropHost->propSetDouble(pp, kOfxParamPropDefault, 0, defV);
    gPropHost->propSetDouble(pp, kOfxParamPropMin, 0, mn);
    gPropHost->propSetDouble(pp, kOfxParamPropMax, 0, mx);
    gPropHost->propSetDouble(pp, kOfxParamPropDisplayMin, 0, mn);
    gPropHost->propSetDouble(pp, kOfxParamPropDisplayMax, 0, mx);
}

static void defIntP(OfxParamSetHandle ps, const char* name, const char* label,
                    const char* group, int defV, int mn, int mx)
{
    OfxPropertySetHandle pp = defParam(ps, kOfxParamTypeInteger, name, label, group);
    gPropHost->propSetInt(pp, kOfxParamPropDefault, 0, defV);
    gPropHost->propSetInt(pp, kOfxParamPropMin, 0, mn);
    gPropHost->propSetInt(pp, kOfxParamPropMax, 0, mx);
    gPropHost->propSetInt(pp, kOfxParamPropDisplayMin, 0, mn);
    gPropHost->propSetInt(pp, kOfxParamPropDisplayMax, 0, mx);
}

// Choice: caller adds options after via the returned property set.
static OfxPropertySetHandle defChoice(OfxParamSetHandle ps, const char* name,
                                      const char* label, const char* group,
                                      int defV)
{
    OfxPropertySetHandle pp = defParam(ps, kOfxParamTypeChoice, name, label, group);
    gPropHost->propSetInt(pp, kOfxParamPropDefault, 0, defV);
    return pp;
}

static void addChoice(OfxPropertySetHandle pp, int idx, const char* label) {
    gPropHost->propSetString(pp, kOfxParamPropChoiceOption, idx, label);
}

static void def9PosChoice(OfxParamSetHandle ps, const char* name,
                          const char* label, const char* group, int defV)
{
    OfxPropertySetHandle pp = defChoice(ps, name, label, group, defV);
    addChoice(pp, 0, "Top Left");
    addChoice(pp, 1, "Top Center");
    addChoice(pp, 2, "Top Right");
    addChoice(pp, 3, "Middle Left");
    addChoice(pp, 4, "Middle Center");
    addChoice(pp, 5, "Middle Right");
    addChoice(pp, 6, "Bottom Left");
    addChoice(pp, 7, "Bottom Center");
    addChoice(pp, 8, "Bottom Right");
}

// ============================================================
// ACTION: Load
// ============================================================
static OfxStatus actionLoad() {
    gPropHost   = (const OfxPropertySuiteV1*)   gHost->fetchSuite(gHost->host, kOfxPropertySuite, 1);
    gEffectHost = (const OfxImageEffectSuiteV1*)gHost->fetchSuite(gHost->host, kOfxImageEffectSuite, 1);
    gParamHost  = (const OfxParameterSuiteV1*)  gHost->fetchSuite(gHost->host, kOfxParameterSuite, 1);
    if (!gPropHost || !gEffectHost || !gParamHost)
        return kOfxStatErrMissingHostFeature;
    return kOfxStatOK;
}

// ============================================================
// ACTION: Describe
// ============================================================
static OfxStatus actionDescribe(OfxImageEffectHandle effect) {
    OfxPropertySetHandle eP;
    gEffectHost->getPropertySet(effect, &eP);
    gPropHost->propSetString(eP, kOfxPropLabel, 0, PLUGIN_LABEL);
    gPropHost->propSetString(eP, kOfxImageEffectPluginPropGrouping, 0, PLUGIN_GROUP);
    gPropHost->propSetString(eP, kOfxImageEffectPropSupportedContexts, 0,
                             kOfxImageEffectContextFilter);
    gPropHost->propSetString(eP, kOfxImageEffectPropSupportedPixelDepths, 0,
                             kOfxBitDepthFloat);
#ifdef __APPLE__
    gPropHost->propSetString(eP, kOfxImageEffectPropMetalRenderSupported, 0, "true");
#else
    gPropHost->propSetString(eP, kOfxImageEffectPropCudaRenderSupported, 0, "true");
#endif
    gPropHost->propSetInt(eP, kOfxImageEffectFrameVarying, 0, 1);
    gPropHost->propSetInt(eP, kOfxImageEffectPropSupportsTiles, 0, 0);
    return kOfxStatOK;
}

// ============================================================
// ACTION: DescribeInContext
// ============================================================
static OfxStatus actionDescribeInContext(OfxImageEffectHandle effect,
                                        OfxPropertySetHandle inArgs)
{
    OfxPropertySetHandle clipProps;
    gEffectHost->clipDefine(effect, kOfxImageEffectSimpleSourceClipName, &clipProps);
    gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0,
                             kOfxImageComponentRGBA);
    gEffectHost->clipDefine(effect, kOfxImageEffectOutputClipName, &clipProps);
    gPropHost->propSetString(clipProps, kOfxImageEffectPropSupportedComponents, 0,
                             kOfxImageComponentRGBA);

    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    OfxPropertySetHandle pp;

    // ---- Initialisation sentinel (hidden) ----
    pp = defParam(ps, kOfxParamTypeInteger, PARAM_INITIALIZED, "_initialized", nullptr);
    gPropHost->propSetInt(pp, kOfxParamPropSecret, 0, 1);
    gPropHost->propSetInt(pp, kOfxParamPropDefault, 0, 0);
    gPropHost->propSetInt(pp, kOfxParamPropAnimates, 0, 0);

    // ---- Master Preset (top-level, before all groups) ----
    pp = defChoice(ps, PARAM_MASTER_PRESET, "Preset", nullptr, 0);
    addChoice(pp, 0, "Custom");

    // ---- Top-level Color Space ----
    pp = defChoice(ps, PARAM_INPUT_TRANSFORM, "Input Color Space", nullptr, 0);
    addChoice(pp, 0, "DaVinci WG / Intermediate");
    pp = defChoice(ps, PARAM_OUTPUT_TRANSFORM, "Output Color Space", nullptr, 0);
    addChoice(pp, 0, "DaVinci WG / Intermediate");

    // ---- Format ----
    defGroup(ps, "grp_format", "Format", true);
    pp = defChoice(ps, PARAM_TV_STANDARD, "TV Standard", "grp_format", TV_NTSC);
    addChoice(pp, 0, "NTSC");
    addChoice(pp, 1, "PAL");
    defDbl(ps, PARAM_BANDWIDTH,      "Luma Bandwidth (lines)", "grp_format", 240.0, 100.0, 500.0);
    defDbl(ps, PARAM_BANDWIDTH_ROWS, "Luma Bandwidth (rows)",  "grp_format", 240.0,  50.0, 500.0);
    defDbl(ps, PARAM_CHROMA_BW,      "Chroma Bandwidth (lines)", "grp_format",  40.0,  10.0, 240.0);
    defBool(ps, PARAM_PAL_SWITCH, "PAL Phase Switch",         "grp_format", true);

    // ---- Composite ----
    defGroup(ps, "grp_composite", "Composite Encoding", false);
    defBool(ps, PARAM_COMPOSITE_EN, "Enable Composite",   "grp_composite", true);
    defDbl(ps, PARAM_COMP_STRENGTH, "Composite Strength", "grp_composite", 1.0, 0.0, 1.0);
    defDbl(ps, PARAM_DOT_CRAWL,    "Dot Crawl",          "grp_composite", 0.5, 0.0, 1.0);
    defDbl(ps, PARAM_CROSS_COLOR,  "Cross Color",        "grp_composite", 0.5, 0.0, 1.0);
    defDbl(ps, PARAM_COMP_NOISE,   "Composite Noise",    "grp_composite", 0.0, 0.0, 0.5);

    // ---- Noise (magnetic) ----
    defGroup(ps, "grp_noise", "Magnetic Noise", false);
    defBool(ps, PARAM_NOISE_EN,     "Enable Noise",      "grp_noise", true);
    defDbl(ps, PARAM_LUMA_NOISE,    "Luma Noise",        "grp_noise", 0.04, 0.0, 0.5);
    defDbl(ps, PARAM_CHROMA_NOISE,  "Chroma Noise",      "grp_noise", 0.05, 0.0, 0.5);
    defDbl(ps, PARAM_NOISE_TINT_HUE,"Noise Hue Bias",    "grp_noise", 0.0, 0.0, 360.0);
    defIntP(ps, PARAM_SEED_OFFSET,  "Seed Variant",      "grp_noise", 42, 0, 999999);

    // ---- Interlace ----
    defGroup(ps, "grp_interlace", "Interlace", false);
    pp = defChoice(ps, PARAM_INTERLACE_MODE, "Interlace Mode", "grp_interlace", INTERLACE_OFF);
    addChoice(pp, 0, "Off");
    addChoice(pp, 1, "NTSC Comb");
    defDbl(ps, PARAM_INTERLACE_AMOUNT, "Field Displacement (px)", "grp_interlace", 1.0, 0.0, 15.0);
    pp = defChoice(ps, PARAM_FIELD_ORDER, "Field Order", "grp_interlace", 0);
    addChoice(pp, 0, "Top First");
    addChoice(pp, 1, "Bottom First");

    // ---- Tracking / Artifacts ----
    defGroup(ps, "grp_tracking", "Tracking & Dropouts", false);
    defBool(ps, PARAM_HEADSWITCH_EN,   "Enable Head-Switch",  "grp_tracking", true);
    defDbl(ps, PARAM_HEADSWITCH_H,     "Head-Switch Height (px)", "grp_tracking", 12.0, 4.0, 60.0);
    defDbl(ps, PARAM_HEADSWITCH_INT,   "Head-Switch Intensity",   "grp_tracking", 0.5, 0.0, 1.0);
    defBool(ps, PARAM_TRACKING_EN,     "Enable Tracking Error",   "grp_tracking", false);
    defDbl(ps, PARAM_TRACKING_AMT,     "Tracking Amount",         "grp_tracking", 0.3, 0.0, 1.0);
    defDbl(ps, PARAM_TRACKING_SPEED,   "Tracking Drift Speed",    "grp_tracking", 4.0, 0.0, 60.0);
    defDbl(ps, PARAM_TRACKING_HEIGHT, "Tracking Height (px)",    "grp_tracking", 40.0, 5.0, 200.0);
    defDbl(ps, PARAM_TRACKING_OPACITY, "Tracking Opacity",        "grp_tracking", 1.0, 0.0, 1.0);
    defBool(ps, PARAM_DROPOUT_EN,      "Enable Dropouts",         "grp_tracking", false);
    defDbl(ps, PARAM_DROPOUT_DENSITY,  "Dropouts / Frame",        "grp_tracking", 0.0, 0.0, 30.0);
    defDbl(ps, PARAM_DROPOUT_LENGTH,   "Dropout Length (px)",     "grp_tracking", 24.0, 4.0, 200.0);
    defDbl(ps, PARAM_DROPOUT_HEIGHT,   "Dropout Height (px)",     "grp_tracking", 1.0, 1.0, 24.0);
    defDbl(ps, PARAM_DROPOUT_OPACITY,  "Dropout Opacity",         "grp_tracking", 0.85, 0.0, 1.0);
    defDbl(ps, PARAM_DROPOUT_SOFTNESS, "Dropout Softness (px)",   "grp_tracking", 2.0, 0.0, 20.0);

    // ---- Geometry ----
    defGroup(ps, "grp_geometry", "Geometric Wobble", false);
    defBool(ps, PARAM_GEOMETRY_EN,   "Enable Geometry",      "grp_geometry", false);
    defDbl(ps, PARAM_LINE_JITTER,    "Line Jitter (px)",     "grp_geometry", 0.0, 0.0, 8.0);
    defDbl(ps, PARAM_TOP_SKEW,       "Top Skew (px)",        "grp_geometry", 0.0, 0.0, 30.0);
    defDbl(ps, PARAM_TOP_SKEW_DECAY, "Top Skew Decay (lines)","grp_geometry", 30.0, 4.0, 200.0);
    defDbl(ps, PARAM_EDGE_PINCUSHION,"Edge Pincushion",      "grp_geometry", 0.0, 0.0, 1.0);

    // ---- Channel Registration ----
    defGroup(ps, "grp_chroma_reg", "Channel Registration", false);
    defBool(ps, PARAM_CHROMA_REG_EN,  "Enable Registration",      "grp_chroma_reg", false);
    defDbl(ps, PARAM_CHROMA_OFFSET_R, "Red Channel Offset (px)",  "grp_chroma_reg", 0.0, -12.0, 12.0);
    defDbl(ps, PARAM_CHROMA_OFFSET_B, "Blue Channel Offset (px)", "grp_chroma_reg", 0.0, -12.0, 12.0);

    // ---- Age / CRT ----
    defGroup(ps, "grp_age", "Color Fade & CRT Trim", false);
    defBool(ps, PARAM_AGE_EN,          "Enable Color Fade & CRT", "grp_age", true);
    defDbl(ps, PARAM_COLOR_FADE,       "Color Fade",        "grp_age", 0.0, 0.0, 1.0);
    defDbl(ps, PARAM_HUE_TINT,         "Hue Tint (deg)",    "grp_age", 0.0, -180.0, 180.0);
    defDbl(ps, PARAM_BLACK_LIFT,       "Black Lift",        "grp_age", 0.0, 0.0, 0.2);
    defDbl(ps, PARAM_WHITE_KNEE,       "White Knee",        "grp_age", 1.0, 0.0, 1.0);
    defDbl(ps, PARAM_DYNAMIC_RANGE,    "Dynamic Range (stops)", "grp_age", 9.0, 4.0, 14.0);

    // ---- Mist / CRT Bloom ----
    defGroup(ps, "grp_mist", "CRT Bloom (Mist)", false);
    defBool(ps, PARAM_MIST_ENABLED,  "Enable CRT Bloom",    "grp_mist", false);
    defDbl(ps, PARAM_MIST_AMOUNT,    "Bloom Amount",         "grp_mist", 0.5,  0.0, 2.0);
    defDbl(ps, PARAM_MIST_SPREAD,    "Bloom Spread (px)",    "grp_mist", 20.0, 2.0, 80.0);
    defDbl(ps, PARAM_MIST_THRESHOLD, "Highlight Threshold",  "grp_mist", 0.6,  0.0, 1.0);
    defDbl(ps, PARAM_MIST_VEIL,      "Veiling Glare",        "grp_mist", 0.1,  0.0, 1.0);

    // ---- OSD ----
    defGroup(ps, "grp_osd", "Camcorder OSD", false);
    defBool(ps, PARAM_OSD_ENABLE, "Enable OSD", "grp_osd", false);
    defBool(ps, PARAM_OSD_DATE_EN,    "Show Date",        "grp_osd", true);
    defBool(ps, PARAM_OSD_TIME_EN,    "Show Time",        "grp_osd", true);
    defBool(ps, PARAM_OSD_REC_EN,     "Show REC",         "grp_osd", true);
    defBool(ps, PARAM_OSD_MODE_EN,    "Show Play Mode",   "grp_osd", false);
    defBool(ps, PARAM_OSD_COUNTER_EN, "Show Counter",     "grp_osd", false);
    defBool(ps, PARAM_OSD_SPEED_EN,   "Show Speed Label", "grp_osd", false);
    def9PosChoice(ps, PARAM_OSD_DATE_POS,    "Date Position",     "grp_osd", POS_BR);
    def9PosChoice(ps, PARAM_OSD_TIME_POS,    "Time Position",     "grp_osd", POS_BL);
    def9PosChoice(ps, PARAM_OSD_REC_POS,     "REC Position",      "grp_osd", POS_TR);
    def9PosChoice(ps, PARAM_OSD_MODE_POS,    "Play Mode Position","grp_osd", POS_TL);
    def9PosChoice(ps, PARAM_OSD_COUNTER_POS, "Counter Position",  "grp_osd", POS_TC);
    def9PosChoice(ps, PARAM_OSD_SPEED_POS,   "Speed Position",    "grp_osd", POS_BC);
    pp = defChoice(ps, PARAM_OSD_GLYPH_SET, "Glyph Set", "grp_osd", 1);
    addChoice(pp, 0, "7-Segment");
    addChoice(pp, 1, "Blocky Bitmap");
    defBool(ps, PARAM_OSD_SHADOW, "Shadow",  "grp_osd", true);
    defDbl(ps, PARAM_OSD_SCALE,   "Scale",   "grp_osd", 1.0, 0.5, 4.0);
    pp = defChoice(ps, PARAM_OSD_PLAY_MODE, "Play Mode", "grp_osd", OSD_PLAY);
    addChoice(pp, 0, "PLAY"); addChoice(pp, 1, "PAUSE");
    addChoice(pp, 2, "FF");   addChoice(pp, 3, "REW");
    addChoice(pp, 4, "STOP");
    pp = defChoice(ps, PARAM_OSD_SPEED_LABEL, "Tape Speed", "grp_osd", OSD_SPEED_SP);
    addChoice(pp, 0, "SP"); addChoice(pp, 1, "LP"); addChoice(pp, 2, "EP");
    defBool(ps, PARAM_OSD_REC_BLINK, "REC Blinks", "grp_osd", true);
    defIntP(ps, PARAM_OSD_START_YEAR,   "Start Year",   "grp_osd", 1995, 1980, 2099);
    defIntP(ps, PARAM_OSD_START_MONTH,  "Start Month",  "grp_osd",    7,    1,   12);
    defIntP(ps, PARAM_OSD_START_DAY,    "Start Day",    "grp_osd",   14,    1,   31);
    defIntP(ps, PARAM_OSD_START_HOUR,   "Start Hour",   "grp_osd",   14,    0,   23);
    defIntP(ps, PARAM_OSD_START_MINUTE, "Start Minute", "grp_osd",   32,    0,   59);
    defIntP(ps, PARAM_OSD_START_SECOND, "Start Second", "grp_osd",    0,    0,   59);
    pp = defChoice(ps, PARAM_OSD_DATE_FORMAT, "Date Format", "grp_osd", 1);
    addChoice(pp, 0, "YYYY MM DD");
    addChoice(pp, 1, "MM DD YYYY");
    addChoice(pp, 2, "DD MM YYYY");

    // ---- Save Preset group ----
    defGroup(ps, "grp_save_preset", "Save Preset", false);
    pp = defParam(ps, kOfxParamTypeString, PARAM_PRESET_NAME, "Preset Name", "grp_save_preset");
    gPropHost->propSetString(pp, kOfxParamPropDefault, 0, "My Preset");
    pp = defParam(ps, kOfxParamTypePushButton, PARAM_SAVE_PRESET, "Save Preset", "grp_save_preset");
    pp = defParam(ps, kOfxParamTypePushButton, PARAM_REFRESH_PRESETS, "Refresh Presets", "grp_save_preset");

    return kOfxStatOK;
}

// ============================================================
// Enable/disable update helpers
// ============================================================
static void updateOsdEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_OSD_ENABLE);
    const char* names[] = {
        PARAM_OSD_DATE_EN, PARAM_OSD_TIME_EN, PARAM_OSD_REC_EN,
        PARAM_OSD_MODE_EN, PARAM_OSD_COUNTER_EN, PARAM_OSD_SPEED_EN,
        PARAM_OSD_DATE_POS, PARAM_OSD_TIME_POS, PARAM_OSD_REC_POS,
        PARAM_OSD_MODE_POS, PARAM_OSD_COUNTER_POS, PARAM_OSD_SPEED_POS,
        PARAM_OSD_GLYPH_SET, PARAM_OSD_SHADOW, PARAM_OSD_SCALE,
        PARAM_OSD_PLAY_MODE, PARAM_OSD_SPEED_LABEL, PARAM_OSD_REC_BLINK,
        PARAM_OSD_START_YEAR, PARAM_OSD_START_MONTH, PARAM_OSD_START_DAY,
        PARAM_OSD_START_HOUR, PARAM_OSD_START_MINUTE, PARAM_OSD_START_SECOND,
        PARAM_OSD_DATE_FORMAT
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateCompositeEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_COMPOSITE_EN);
    const char* names[] = {
        PARAM_COMP_STRENGTH, PARAM_DOT_CRAWL, PARAM_CROSS_COLOR, PARAM_COMP_NOISE
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateNoiseEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_NOISE_EN);
    const char* names[] = {
        PARAM_LUMA_NOISE, PARAM_CHROMA_NOISE, PARAM_NOISE_TINT_HUE, PARAM_SEED_OFFSET
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateDropoutsEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_DROPOUT_EN);
    const char* names[] = {
        PARAM_DROPOUT_DENSITY, PARAM_DROPOUT_LENGTH, PARAM_DROPOUT_HEIGHT,
        PARAM_DROPOUT_OPACITY, PARAM_DROPOUT_SOFTNESS
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateGeometryEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_GEOMETRY_EN);
    const char* names[] = {
        PARAM_LINE_JITTER, PARAM_TOP_SKEW, PARAM_TOP_SKEW_DECAY, PARAM_EDGE_PINCUSHION
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateChromaRegEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_CHROMA_REG_EN);
    const char* names[] = { PARAM_CHROMA_OFFSET_R, PARAM_CHROMA_OFFSET_B };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateAgeEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_AGE_EN);
    const char* names[] = {
        PARAM_COLOR_FADE, PARAM_HUE_TINT, PARAM_BLACK_LIFT, PARAM_WHITE_KNEE, PARAM_DYNAMIC_RANGE
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

static void updateMistEnabled(OfxImageEffectHandle effect) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    bool en = getBool(ps, PARAM_MIST_ENABLED);
    const char* names[] = {
        PARAM_MIST_AMOUNT, PARAM_MIST_SPREAD, PARAM_MIST_THRESHOLD, PARAM_MIST_VEIL
    };
    for (auto n : names) setParamEnabled(effect, n, en);
}

// ============================================================
// Preset system — param table, save/load/apply/detect/refresh
// ============================================================

struct PresetParamDef {
    const char* name;
    bool isInt;  // true = int/bool/choice, false = double
};

static const PresetParamDef kPresetParams[] = {
    { PARAM_TV_STANDARD,       true },
    { PARAM_BANDWIDTH,         false },
    { PARAM_BANDWIDTH_ROWS,    false },
    { PARAM_CHROMA_BW,         false },
    { PARAM_PAL_SWITCH,        true },
    { PARAM_COMPOSITE_EN,      true },
    { PARAM_COMP_STRENGTH,     false },
    { PARAM_DOT_CRAWL,         false },
    { PARAM_CROSS_COLOR,       false },
    { PARAM_COMP_NOISE,        false },
    { PARAM_NOISE_EN,          true },
    { PARAM_LUMA_NOISE,        false },
    { PARAM_CHROMA_NOISE,      false },
    { PARAM_NOISE_TINT_HUE,    false },
    { PARAM_SEED_OFFSET,       true },
    { PARAM_INTERLACE_MODE,    true },
    { PARAM_INTERLACE_AMOUNT,  false },
    { PARAM_FIELD_ORDER,       true },
    { PARAM_HEADSWITCH_EN,     true },
    { PARAM_HEADSWITCH_H,      false },
    { PARAM_HEADSWITCH_INT,    false },
    { PARAM_TRACKING_EN,       true },
    { PARAM_TRACKING_AMT,      false },
    { PARAM_TRACKING_SPEED,    false },
    { PARAM_TRACKING_HEIGHT,   false },
    { PARAM_TRACKING_OPACITY,  false },
    { PARAM_DROPOUT_EN,        true },
    { PARAM_DROPOUT_DENSITY,   false },
    { PARAM_DROPOUT_LENGTH,    false },
    { PARAM_DROPOUT_HEIGHT,    false },
    { PARAM_DROPOUT_OPACITY,   false },
    { PARAM_DROPOUT_SOFTNESS,  false },
    { PARAM_GEOMETRY_EN,       true },
    { PARAM_LINE_JITTER,       false },
    { PARAM_TOP_SKEW,          false },
    { PARAM_TOP_SKEW_DECAY,    false },
    { PARAM_EDGE_PINCUSHION,   false },
    { PARAM_CHROMA_REG_EN,     true },
    { PARAM_CHROMA_OFFSET_R,   false },
    { PARAM_CHROMA_OFFSET_B,   false },
    { PARAM_AGE_EN,            true },
    { PARAM_COLOR_FADE,        false },
    { PARAM_HUE_TINT,          false },
    { PARAM_BLACK_LIFT,        false },
    { PARAM_WHITE_KNEE,        false },
    { PARAM_DYNAMIC_RANGE,     false },
    { PARAM_MIST_ENABLED,      true },
    { PARAM_MIST_AMOUNT,       false },
    { PARAM_MIST_SPREAD,       false },
    { PARAM_MIST_THRESHOLD,    false },
    { PARAM_MIST_VEIL,         false },
    { PARAM_OSD_ENABLE,        true },
    { PARAM_OSD_DATE_EN,       true },
    { PARAM_OSD_TIME_EN,       true },
    { PARAM_OSD_REC_EN,        true },
    { PARAM_OSD_MODE_EN,       true },
    { PARAM_OSD_COUNTER_EN,    true },
    { PARAM_OSD_SPEED_EN,      true },
    { PARAM_OSD_DATE_POS,      true },
    { PARAM_OSD_TIME_POS,      true },
    { PARAM_OSD_REC_POS,       true },
    { PARAM_OSD_MODE_POS,      true },
    { PARAM_OSD_COUNTER_POS,   true },
    { PARAM_OSD_SPEED_POS,     true },
    { PARAM_OSD_GLYPH_SET,     true },
    { PARAM_OSD_SHADOW,        true },
    { PARAM_OSD_SCALE,         false },
    { PARAM_OSD_PLAY_MODE,     true },
    { PARAM_OSD_SPEED_LABEL,   true },
    { PARAM_OSD_REC_BLINK,     true },
    { PARAM_OSD_START_YEAR,    true },
    { PARAM_OSD_START_MONTH,   true },
    { PARAM_OSD_START_DAY,     true },
    { PARAM_OSD_START_HOUR,    true },
    { PARAM_OSD_START_MINUTE,  true },
    { PARAM_OSD_START_SECOND,  true },
    { PARAM_OSD_DATE_FORMAT,   true },
    { PARAM_INPUT_TRANSFORM,   true },
    { PARAM_OUTPUT_TRANSFORM,  true },
};
static const int kNumPresetParams =
    (int)(sizeof(kPresetParams) / sizeof(kPresetParams[0]));

static void refreshPresets(OfxImageEffectHandle effect) {
    InstanceData* data = getInstanceData(effect);
    if (!data) return;

    std::string bundlePath = getBundlePath(effect);

    data->masterPresets.clear();
    if (!bundlePath.empty()) {
        std::string presetDir = bundlePath + "/Contents/Resources/Presets";
        data->masterPresets = discoverPresets(presetDir.c_str());
    }
    {
        std::string userDir = getUserPresetDir();
        if (!userDir.empty()) {
            auto userPresets = discoverPresets(userDir.c_str());
            for (auto& p : userPresets)
                data->masterPresets.push_back(std::move(p));
        }
    }

    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    OfxParamHandle presetParam;
    gParamHost->paramGetHandle(ps, PARAM_MASTER_PRESET, &presetParam, nullptr);
    OfxPropertySetHandle presetProps;
    gParamHost->paramGetPropertySet(presetParam, &presetProps);

    gPropHost->propSetString(presetProps, kOfxParamPropChoiceOption, 0, "Custom");
    for (size_t i = 0; i < data->masterPresets.size(); ++i) {
        gPropHost->propSetString(presetProps, kOfxParamPropChoiceOption,
                                 (int)(i + 1),
                                 data->masterPresets[i].name.c_str());
    }
}

static void applyMasterPreset(OfxImageEffectHandle effect,
                              const MasterPreset& preset,
                              double time) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);

    for (int i = 0; i < kNumPresetParams; ++i) {
        auto it = preset.values.find(kPresetParams[i].name);
        if (it == preset.values.end()) continue;

        if (kPresetParams[i].isInt) {
            setIntParam(ps, kPresetParams[i].name, (int)it->second);
        } else {
            setDoubleParam(ps, kPresetParams[i].name, it->second);
        }
    }
}

static int detectMasterPreset(OfxImageEffectHandle effect,
                              const std::vector<MasterPreset>& presets,
                              double time) {
    if (presets.empty()) return 0;

    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);

    for (size_t pi = 0; pi < presets.size(); ++pi) {
        if (presets[pi].hasEmbeddedLut) continue;

        bool match = true;
        for (int i = 0; i < kNumPresetParams && match; ++i) {
            auto it = presets[pi].values.find(kPresetParams[i].name);
            if (it == presets[pi].values.end()) continue;

            if (kPresetParams[i].isInt) {
                int cur = getInt(ps, kPresetParams[i].name);
                if (cur != (int)it->second) match = false;
            } else {
                double cur = getDouble(ps, kPresetParams[i].name, time);
                if (fabs(cur - it->second) > 0.001) match = false;
            }
        }
        if (match) return (int)(pi + 1);
    }
    return 0;
}

static void saveCurrentPreset(OfxImageEffectHandle effect, double time) {
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);

    std::string presetName = getString(ps, PARAM_PRESET_NAME);
    if (presetName.empty()) presetName = "My Preset";

    std::string safeFilename;
    for (char c : presetName) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == ' ')
            safeFilename += c;
        else
            safeFilename += '_';
    }
    if (safeFilename.empty()) safeFilename = "preset";

    std::string dir = getUserPresetDir();
    if (dir.empty()) return;

    std::string path = dir + "/" + safeFilename + ".preset";

    std::ofstream f(path);
    if (!f.is_open()) return;

    f << "# ============================================================\n";
    f << "# Lofi FX VHS — Preset: " << presetName << "\n";
    f << "# ============================================================\n\n";
    f << "name = " << presetName << "\n\n";

    for (int i = 0; i < kNumPresetParams; ++i) {
        if (kPresetParams[i].isInt) {
            int v = getInt(ps, kPresetParams[i].name);
            f << kPresetParams[i].name << " = " << v << "\n";
        } else {
            double v = getDouble(ps, kPresetParams[i].name, time);
            f << kPresetParams[i].name << " = " << v << "\n";
        }
    }

    f.close();
}

// ============================================================
// ACTION: CreateInstance
// ============================================================
static OfxStatus actionCreateInstance(OfxImageEffectHandle effect) {
    InstanceData* d = new InstanceData();

    std::string bundlePath = getBundlePath(effect);
    if (!bundlePath.empty()) {
        std::string pakPath = bundlePath + "/Contents/Resources/resources.pak";
        loadLutPackSplit(pakPath.c_str(), d->inputCsts, d->outputCsts);
    }

    // Populate input/output CST choice params
    {
        OfxParamSetHandle ps;
        gEffectHost->getParamSet(effect, &ps);

        if (!d->inputCsts.empty()) {
            OfxParamHandle h; gParamHost->paramGetHandle(ps, PARAM_INPUT_TRANSFORM, &h, nullptr);
            OfxPropertySetHandle pr; gParamHost->paramGetPropertySet(h, &pr);
            gPropHost->propSetString(pr, kOfxParamPropChoiceOption, 0, "DaVinci WG / Intermediate");
            for (size_t i = 0; i < d->inputCsts.size(); ++i)
                gPropHost->propSetString(pr, kOfxParamPropChoiceOption, (int)(i+1),
                                         d->inputCsts[i].title.c_str());
        }
        if (!d->outputCsts.empty()) {
            OfxParamHandle h; gParamHost->paramGetHandle(ps, PARAM_OUTPUT_TRANSFORM, &h, nullptr);
            OfxPropertySetHandle pr; gParamHost->paramGetPropertySet(h, &pr);
            gPropHost->propSetString(pr, kOfxParamPropChoiceOption, 0, "DaVinci WG / Intermediate");
            for (size_t i = 0; i < d->outputCsts.size(); ++i)
                gPropHost->propSetString(pr, kOfxParamPropChoiceOption, (int)(i+1),
                                         d->outputCsts[i].title.c_str());
        }
    }

    setInstanceData(effect, d);

    // Populate master preset dropdown
    refreshPresets(effect);

    // Auto-select "VHS Camcorder" preset on fresh instances
    {
        OfxParamSetHandle ps2;
        gEffectHost->getParamSet(effect, &ps2);
        OfxParamHandle initP;
        int initialized = 0;
        if (gParamHost->paramGetHandle(ps2, PARAM_INITIALIZED, &initP, nullptr) == kOfxStatOK)
            gParamHost->paramGetValue(initP, &initialized);
        if (initialized == 0 && !d->masterPresets.empty()) {
            int defaultIdx = 0;
            for (size_t i = 0; i < d->masterPresets.size(); ++i) {
                if (d->masterPresets[i].name == "VHS Camcorder") {
                    defaultIdx = (int)i;
                    break;
                }
            }
            d->applyingPreset = true;
            applyMasterPreset(effect, d->masterPresets[defaultIdx], 0);
            d->presetOverrides = d->masterPresets[defaultIdx].values;
            d->currentMasterPreset = defaultIdx;
            setIntParam(ps2, PARAM_MASTER_PRESET, defaultIdx + 1);
            d->applyingPreset = false;
        }
    }
    // Mark initialised so future copies / project-reloads skip the auto-select
    {
        OfxParamSetHandle ps2;
        gEffectHost->getParamSet(effect, &ps2);
        setIntParam(ps2, PARAM_INITIALIZED, 1);
    }

    updateOsdEnabled(effect);
    updateCompositeEnabled(effect);
    updateNoiseEnabled(effect);
    updateDropoutsEnabled(effect);
    updateGeometryEnabled(effect);
    updateChromaRegEnabled(effect);
    updateAgeEnabled(effect);
    updateMistEnabled(effect);
    setGroupOpen(effect, "grp_format", true);

    return kOfxStatOK;
}

// ============================================================
// ACTION: DestroyInstance
// ============================================================
static OfxStatus actionDestroyInstance(OfxImageEffectHandle effect) {
    InstanceData* d = getInstanceData(effect);
    if (d) {
#ifdef __APPLE__
        if (d->metalInputCstBuffer)  metalFreeLut(d->metalInputCstBuffer);
        if (d->metalOutputCstBuffer) metalFreeLut(d->metalOutputCstBuffer);
        if (d->metalOsdBuffer)       metalFreeOsd(d->metalOsdBuffer);
#endif
        delete d;
    }
    setInstanceData(effect, nullptr);
    return kOfxStatOK;
}

// ============================================================
// ACTION: InstanceChanged
// ============================================================
static OfxStatus actionInstanceChanged(OfxImageEffectHandle effect,
                                       OfxPropertySetHandle inArgs)
{
    char* objType = nullptr;
    char* objName = nullptr;
    gPropHost->propGetString(inArgs, kOfxPropType, 0, &objType);
    gPropHost->propGetString(inArgs, kOfxPropName, 0, &objName);
    if (!objType || !objName) return kOfxStatReplyDefault;
    if (strcmp(objType, kOfxTypeParameter) != 0) return kOfxStatReplyDefault;

    double time = 0;
    gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);

    InstanceData* instData = getInstanceData(effect);
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);

    // ---- Save Preset button ----
    if (strcmp(objName, PARAM_SAVE_PRESET) == 0) {
        saveCurrentPreset(effect, time);
        refreshPresets(effect);
        // Auto-select the newly saved preset (its values match current params)
        if (instData) {
            int detected = detectMasterPreset(effect, instData->masterPresets, time);
            if (detected != getInt(ps, PARAM_MASTER_PRESET)) {
                instData->applyingPreset = true;
                setIntParam(ps, PARAM_MASTER_PRESET, detected);
                instData->applyingPreset = false;
            }
            if (detected >= 1 && detected <= (int)instData->masterPresets.size()) {
                instData->presetOverrides = instData->masterPresets[detected - 1].values;
                instData->currentMasterPreset = detected - 1;
            }
        }
        return kOfxStatOK;
    }

    // ---- Refresh Presets button ----
    if (strcmp(objName, PARAM_REFRESH_PRESETS) == 0) {
        refreshPresets(effect);
        return kOfxStatOK;
    }

    // ---- Enable/disable groups ----
    if      (strcmp(objName, PARAM_OSD_ENABLE)    == 0) updateOsdEnabled(effect);
    else if (strcmp(objName, PARAM_COMPOSITE_EN)  == 0) updateCompositeEnabled(effect);
    else if (strcmp(objName, PARAM_NOISE_EN)      == 0) updateNoiseEnabled(effect);
    else if (strcmp(objName, PARAM_DROPOUT_EN)    == 0) updateDropoutsEnabled(effect);
    else if (strcmp(objName, PARAM_GEOMETRY_EN)   == 0) updateGeometryEnabled(effect);
    else if (strcmp(objName, PARAM_CHROMA_REG_EN) == 0) updateChromaRegEnabled(effect);
    else if (strcmp(objName, PARAM_AGE_EN)        == 0) updateAgeEnabled(effect);
    else if (strcmp(objName, PARAM_MIST_ENABLED)  == 0) updateMistEnabled(effect);

    // ---- Explicit Preset selected ----
    if (strcmp(objName, PARAM_MASTER_PRESET) == 0) {
        int idx = getInt(ps, PARAM_MASTER_PRESET);
        if (instData && idx >= 1 && idx <= (int)instData->masterPresets.size()) {
            instData->applyingPreset = true;
            applyMasterPreset(effect, instData->masterPresets[idx - 1], time);
            instData->presetOverrides = instData->masterPresets[idx - 1].values;
            instData->currentMasterPreset = idx - 1;
            instData->applyingPreset = false;
        } else if (instData && idx == 0) {
            instData->presetOverrides.clear();
            instData->currentMasterPreset = -1;
        }
        return kOfxStatOK;
    }

    // ---- Auto-detect master preset after any other param change ----
    if (instData && !instData->applyingPreset) {
        int detected = detectMasterPreset(effect, instData->masterPresets, time);
        int current = getInt(ps, PARAM_MASTER_PRESET);
        if (detected != current) {
            setIntParam(ps, PARAM_MASTER_PRESET, detected);
        }
    }

    return kOfxStatOK;
}

// ============================================================
// ACTION: IsIdentity
// ============================================================
static OfxStatus actionIsIdentity(OfxImageEffectHandle effect,
                                  OfxPropertySetHandle inArgs,
                                  OfxPropertySetHandle outArgs)
{
    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);
    double t = 0;
    gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &t);

    InstanceData* d = getInstanceData(effect);
    t_presetOverride = d ? &d->presetOverrides : nullptr;

    // VHS sim is always "active" once the user adds it because the composite
    // encode/decode roundtrip alone alters the image. The only true bypass
    // condition is: composite at 0, no noise, no artifacts, no OSD, no fade,
    // both transforms on default, no look — in which case the encode/decode
    // collapses to identity.
    bool compositeEn = getBool(ps, PARAM_COMPOSITE_EN);
    bool noiseEn     = getBool(ps, PARAM_NOISE_EN);
    bool dropoutEn   = getBool(ps, PARAM_DROPOUT_EN);
    bool geoEn       = getBool(ps, PARAM_GEOMETRY_EN);
    bool chromaRegEn = getBool(ps, PARAM_CHROMA_REG_EN);
    bool ageEn       = getBool(ps, PARAM_AGE_EN);
    bool any =
        (compositeEn && (getDouble(ps, PARAM_COMP_STRENGTH, t) > 1e-6 ||
                         getDouble(ps, PARAM_COMP_NOISE,    t) > 1e-6)) ||
        (noiseEn && (getDouble(ps, PARAM_LUMA_NOISE,   t) > 1e-6 ||
                     getDouble(ps, PARAM_CHROMA_NOISE, t) > 1e-6)) ||
        getInt(ps, PARAM_INTERLACE_MODE)        != 0 ||
        getBool(ps, PARAM_HEADSWITCH_EN)            ||
        getBool(ps, PARAM_TRACKING_EN)              ||
        (dropoutEn && getDouble(ps, PARAM_DROPOUT_DENSITY, t) > 1e-6) ||
        (geoEn && (getDouble(ps, PARAM_LINE_JITTER,     t) > 1e-6 ||
                   getDouble(ps, PARAM_TOP_SKEW,        t) > 1e-6 ||
                   getDouble(ps, PARAM_EDGE_PINCUSHION, t) > 1e-6)) ||
        (chromaRegEn && (std::fabs(getDouble(ps, PARAM_CHROMA_OFFSET_R, t)) > 1e-3 ||
                         std::fabs(getDouble(ps, PARAM_CHROMA_OFFSET_B, t)) > 1e-3)) ||
        (ageEn && (getDouble(ps, PARAM_COLOR_FADE,  t) > 1e-6 ||
                   std::fabs(getDouble(ps, PARAM_HUE_TINT, t)) > 1e-3 ||
                   getDouble(ps, PARAM_BLACK_LIFT,  t) > 1e-6 ||
                   getDouble(ps, PARAM_WHITE_KNEE,  t) < 1.0 - 1e-6 ||
                   getDouble(ps, PARAM_DYNAMIC_RANGE, t) < 13.5 - 1e-3)) ||
        getBool(ps, PARAM_OSD_ENABLE)               ||
        getBool(ps, PARAM_MIST_ENABLED)             ||
        getInt(ps, PARAM_INPUT_TRANSFORM)       != 0 ||
        getInt(ps, PARAM_OUTPUT_TRANSFORM)      != 0 ||
        getDouble(ps, PARAM_BANDWIDTH, t)     < 480.0 ||
        getDouble(ps, PARAM_BANDWIDTH_ROWS, t) < 480.0 ||
        getDouble(ps, PARAM_CHROMA_BW, t)     < 240.0;

    if (!any) {
        t_presetOverride = nullptr;
        gPropHost->propSetString(outArgs, kOfxPropName, 0,
                                 kOfxImageEffectSimpleSourceClipName);
        return kOfxStatOK;
    }
    t_presetOverride = nullptr;
    return kOfxStatReplyDefault;
}

// ============================================================
// Helper: ensure a CST LUT is uploaded to GPU and return device ptr
// ============================================================
#ifdef __APPLE__
static void* ensureMetalLut(void* queue, void*& slot, int& currentIdx,
                            int newIdx, const std::vector<CubeLut>& list)
{
    if (newIdx < 0 || newIdx >= (int)list.size()) {
        if (slot) { metalFreeLut(slot); slot = nullptr; }
        currentIdx = -1;
        return nullptr;
    }
    if (newIdx != currentIdx) {
        if (slot) metalFreeLut(slot);
        slot = metalUploadLut(queue, list[newIdx].data.data(), list[newIdx].size);
        currentIdx = newIdx;
    }
    return slot;
}
#endif

// ============================================================
// ACTION: Render
// ============================================================
static OfxStatus actionRender(OfxImageEffectHandle effect,
                              OfxPropertySetHandle inArgs)
{
    double time = 0;
    gPropHost->propGetDouble(inArgs, kOfxPropTime, 0, &time);

    double rsX = 1.0, rsY = 1.0;
    gPropHost->propGetDouble(inArgs, kOfxImageEffectPropRenderScale, 0, &rsX);
    gPropHost->propGetDouble(inArgs, kOfxImageEffectPropRenderScale, 1, &rsY);
    float renderScale = (float)(0.5 * (rsX + rsY));
    if (renderScale < 1e-3f) renderScale = 1.0f;

    OfxParamSetHandle ps;
    gEffectHost->getParamSet(effect, &ps);

    VhsParams p; memset(&p, 0, sizeof(p));
    p.renderScale  = renderScale;
    p.frame_index  = (int)time;

    InstanceData* d = getInstanceData(effect);
    t_presetOverride = d ? &d->presetOverrides : nullptr;

    p.seed_offset  = getInt(ps, PARAM_SEED_OFFSET);

    p.tvStandard       = getInt(ps, PARAM_TV_STANDARD);
    p.bandwidthLines   = (float)getDouble(ps, PARAM_BANDWIDTH, time);
    p.bandwidthRows    = (float)getDouble(ps, PARAM_BANDWIDTH_ROWS, time);
    p.chromaBandwidth  = (float)getDouble(ps, PARAM_CHROMA_BW, time);
    p.palSwitch        = getBool(ps, PARAM_PAL_SWITCH) ? 1 : 0;

    bool compositeEn = getBool(ps, PARAM_COMPOSITE_EN);
    bool noiseEn     = getBool(ps, PARAM_NOISE_EN);
    bool dropoutEn   = getBool(ps, PARAM_DROPOUT_EN);
    bool geoEn_r     = getBool(ps, PARAM_GEOMETRY_EN);
    bool chromaRegEn = getBool(ps, PARAM_CHROMA_REG_EN);
    bool ageEn_r     = getBool(ps, PARAM_AGE_EN);

    p.compositeStrength = compositeEn ? (float)getDouble(ps, PARAM_COMP_STRENGTH, time) : 0.0f;
    p.dotCrawlGain      = compositeEn ? (float)getDouble(ps, PARAM_DOT_CRAWL,    time) : 0.0f;
    p.crossColorGain    = compositeEn ? (float)getDouble(ps, PARAM_CROSS_COLOR,  time) : 0.0f;
    p.compositeNoise    = compositeEn ? (float)getDouble(ps, PARAM_COMP_NOISE,   time) : 0.0f;

    p.lumaNoise    = noiseEn ? (float)getDouble(ps, PARAM_LUMA_NOISE,    time) : 0.0f;
    p.chromaNoise  = noiseEn ? (float)getDouble(ps, PARAM_CHROMA_NOISE,  time) : 0.0f;
    p.noiseTintHue = noiseEn ? (float)getDouble(ps, PARAM_NOISE_TINT_HUE,time) : 0.0f;

    p.interlaceMode   = getInt(ps, PARAM_INTERLACE_MODE);
    p.interlaceAmount = (float)getDouble(ps, PARAM_INTERLACE_AMOUNT, time);
    p.fieldOrder     = getInt(ps, PARAM_FIELD_ORDER);

    p.headSwitchEnable    = getBool(ps, PARAM_HEADSWITCH_EN) ? 1 : 0;
    p.headSwitchHeight    = (float)getDouble(ps, PARAM_HEADSWITCH_H,   time);
    p.headSwitchIntensity = (float)getDouble(ps, PARAM_HEADSWITCH_INT, time);
    p.trackingErrorEnable = getBool(ps, PARAM_TRACKING_EN) ? 1 : 0;
    p.trackingErrorAmount = (float)getDouble(ps, PARAM_TRACKING_AMT,   time);
    p.trackingErrorSpeed  = (float)getDouble(ps, PARAM_TRACKING_SPEED, time);
    p.trackingErrorHeight = (float)getDouble(ps, PARAM_TRACKING_HEIGHT, time);
    p.trackingErrorOpacity = (float)getDouble(ps, PARAM_TRACKING_OPACITY, time);
    p.dropoutDensity      = dropoutEn ? (float)getDouble(ps, PARAM_DROPOUT_DENSITY, time) : 0.0f;
    p.dropoutLength       = (float)getDouble(ps, PARAM_DROPOUT_LENGTH, time);
    p.dropoutHeight       = (float)getDouble(ps, PARAM_DROPOUT_HEIGHT, time);
    p.dropoutOpacity      = (float)getDouble(ps, PARAM_DROPOUT_OPACITY, time);
    p.dropoutSoftness     = (float)getDouble(ps, PARAM_DROPOUT_SOFTNESS, time);

    p.lineJitterPx   = geoEn_r ? (float)getDouble(ps, PARAM_LINE_JITTER,    time) : 0.0f;
    p.topSkewPx      = geoEn_r ? (float)getDouble(ps, PARAM_TOP_SKEW,       time) : 0.0f;
    p.topSkewDecay   = (float)getDouble(ps, PARAM_TOP_SKEW_DECAY, time);
    p.edgePincushion = geoEn_r ? (float)getDouble(ps, PARAM_EDGE_PINCUSHION, time) : 0.0f;

    p.chromaOffsetR  = chromaRegEn ? (float)getDouble(ps, PARAM_CHROMA_OFFSET_R, time) : 0.0f;
    p.chromaOffsetB  = chromaRegEn ? (float)getDouble(ps, PARAM_CHROMA_OFFSET_B, time) : 0.0f;

    p.colorFade         = ageEn_r ? (float)getDouble(ps, PARAM_COLOR_FADE,    time) : 0.0f;
    p.hueTint           = ageEn_r ? (float)getDouble(ps, PARAM_HUE_TINT,      time) : 0.0f;
    p.blackLift         = ageEn_r ? (float)getDouble(ps, PARAM_BLACK_LIFT,    time) : 0.0f;
    p.whiteKnee         = ageEn_r ? (float)getDouble(ps, PARAM_WHITE_KNEE,    time) : 1.0f;
    p.dynamicRangeStops = ageEn_r ? (float)getDouble(ps, PARAM_DYNAMIC_RANGE, time) : 14.0f;

    p.mistEnabled   = getBool(ps, PARAM_MIST_ENABLED) ? 1 : 0;
    p.mistAmount    = (float)getDouble(ps, PARAM_MIST_AMOUNT,    time);
    p.mistSpread    = (float)getDouble(ps, PARAM_MIST_SPREAD,    time);
    p.mistThreshold = (float)getDouble(ps, PARAM_MIST_THRESHOLD, time);
    p.mistVeil      = (float)getDouble(ps, PARAM_MIST_VEIL,      time);

    // OSD
    p.osdEnable = getBool(ps, PARAM_OSD_ENABLE) ? 1 : 0;
    int osdMask = 0;
    if (getBool(ps, PARAM_OSD_DATE_EN))    osdMask |= OSD_BIT_DATE;
    if (getBool(ps, PARAM_OSD_TIME_EN))    osdMask |= OSD_BIT_TIME;
    if (getBool(ps, PARAM_OSD_REC_EN))     osdMask |= OSD_BIT_REC;
    if (getBool(ps, PARAM_OSD_MODE_EN))    osdMask |= OSD_BIT_MODE;
    if (getBool(ps, PARAM_OSD_COUNTER_EN)) osdMask |= OSD_BIT_COUNTER;
    if (getBool(ps, PARAM_OSD_SPEED_EN))   osdMask |= OSD_BIT_SPEED;
    p.osdElements        = osdMask;
    p.osdPositionDate    = getInt(ps, PARAM_OSD_DATE_POS);
    p.osdPositionTime    = getInt(ps, PARAM_OSD_TIME_POS);
    p.osdPositionRec     = getInt(ps, PARAM_OSD_REC_POS);
    p.osdPositionMode    = getInt(ps, PARAM_OSD_MODE_POS);
    p.osdPositionCounter = getInt(ps, PARAM_OSD_COUNTER_POS);
    p.osdPositionSpeed   = getInt(ps, PARAM_OSD_SPEED_POS);
    p.osdGlyphSet        = getInt(ps, PARAM_OSD_GLYPH_SET);
    p.osdShadow          = getBool(ps, PARAM_OSD_SHADOW) ? 1 : 0;
    p.osdScale           = (float)getDouble(ps, PARAM_OSD_SCALE, time);
    p.osdPlayMode        = getInt(ps, PARAM_OSD_PLAY_MODE);
    p.osdSpeedLabel      = getInt(ps, PARAM_OSD_SPEED_LABEL);
    p.osdRecBlink        = getBool(ps, PARAM_OSD_REC_BLINK) ? 1 : 0;
    p.osdStartYear       = getInt(ps, PARAM_OSD_START_YEAR);
    p.osdStartMonth      = getInt(ps, PARAM_OSD_START_MONTH);
    p.osdStartDay        = getInt(ps, PARAM_OSD_START_DAY);
    p.osdStartHour       = getInt(ps, PARAM_OSD_START_HOUR);
    p.osdStartMinute     = getInt(ps, PARAM_OSD_START_MINUTE);
    p.osdStartSecond     = getInt(ps, PARAM_OSD_START_SECOND);
    p.osdDateFormat      = getInt(ps, PARAM_OSD_DATE_FORMAT);

    // CST
    int inXform  = getInt(ps, PARAM_INPUT_TRANSFORM);
    int outXform = getInt(ps, PARAM_OUTPUT_TRANSFORM);
    p.inTransform     = inXform;
    p.outTransform    = outXform;
    p.inputCstLutSize = (d && inXform > 0  && (inXform-1)  < (int)d->inputCsts.size())  ? d->inputCsts[inXform-1].size  : 0;
    p.cstLutSize      = (d && outXform > 0 && (outXform-1) < (int)d->outputCsts.size()) ? d->outputCsts[outXform-1].size : 0;

    // ---- Fetch images ----
    OfxImageClipHandle srcClip, dstClip;
    gEffectHost->clipGetHandle(effect, kOfxImageEffectSimpleSourceClipName, &srcClip, nullptr);
    gEffectHost->clipGetHandle(effect, kOfxImageEffectOutputClipName,       &dstClip, nullptr);
    OfxPropertySetHandle srcImg = nullptr, dstImg = nullptr;
    gEffectHost->clipGetImage(srcClip, time, nullptr, &srcImg);
    gEffectHost->clipGetImage(dstClip, time, nullptr, &dstImg);
    if (!srcImg || !dstImg) {
        if (srcImg) gEffectHost->clipReleaseImage(srcImg);
        if (dstImg) gEffectHost->clipReleaseImage(dstImg);
        t_presetOverride = nullptr;
        return kOfxStatFailed;
    }

    void* srcPtr = nullptr; void* dstPtr = nullptr;
    int srcRowBytes = 0, dstRowBytes = 0;
    OfxRectI dstBounds;
    gPropHost->propGetPointer(srcImg, kOfxImagePropData, 0, &srcPtr);
    gPropHost->propGetPointer(dstImg, kOfxImagePropData, 0, &dstPtr);
    gPropHost->propGetInt(srcImg, kOfxImagePropRowBytes, 0, &srcRowBytes);
    gPropHost->propGetInt(dstImg, kOfxImagePropRowBytes, 0, &dstRowBytes);
    gPropHost->propGetIntN(dstImg, kOfxImagePropBounds, 4, &dstBounds.x1);
    p.width  = dstBounds.x2 - dstBounds.x1;
    p.height = dstBounds.y2 - dstBounds.y1;
    p.srcRowBytes = srcRowBytes;
    p.dstRowBytes = dstRowBytes;

    OfxStatus status = kOfxStatFailed;

#ifdef __APPLE__
    {
        int metalEnabled = 0;
        gPropHost->propGetInt(inArgs, kOfxImageEffectPropMetalEnabled, 0, &metalEnabled);
        if (metalEnabled) {
            void* queuePtr = nullptr;
            gPropHost->propGetPointer(inArgs, kOfxImageEffectPropMetalCommandQueue,
                                      0, &queuePtr);
            if (queuePtr && metalInit(queuePtr)) {
                // Upload LUTs as needed
                void* inCstBuf = (inXform > 0)
                    ? ensureMetalLut(queuePtr, d->metalInputCstBuffer,
                                     d->currentInputCstIdx, inXform - 1, d->inputCsts)
                    : nullptr;
                void* outCstBuf = (outXform > 0)
                    ? ensureMetalLut(queuePtr, d->metalOutputCstBuffer,
                                     d->currentOutputCstIdx, outXform - 1, d->outputCsts)
                    : nullptr;

                // Render OSD mask on CPU and upload (full-frame R8)
                void* osdBuf = nullptr;
                int   osdW = 0, osdH = 0;
                if (p.osdEnable && p.osdElements != 0) {
                    std::vector<uint8_t> mask = renderOsdMask(p, p.frame_index, 30.0);
                    if (!mask.empty()) {
                        if (d->metalOsdBuffer) metalFreeOsd(d->metalOsdBuffer);
                        d->metalOsdBuffer = metalUploadOsd(queuePtr, mask.data(),
                                                           p.width, p.height);
                        d->osdW = p.width;
                        d->osdH = p.height;
                        osdBuf = d->metalOsdBuffer;
                        osdW = d->osdW;
                        osdH = d->osdH;
                    }
                }

                metalDispatchVhs(queuePtr, srcPtr, dstPtr,
                                 outCstBuf, inCstBuf,
                                 osdBuf, osdW, osdH, p, false);
                status = kOfxStatOK;
            }
        }
    }
#endif

    // CPU passthrough fallback
    if (status != kOfxStatOK && srcPtr && dstPtr) {
        for (int y = 0; y < p.height; y++) {
            memcpy((char*)dstPtr + (size_t)y * dstRowBytes,
                   (const char*)srcPtr + (size_t)y * srcRowBytes,
                   (size_t)p.width * 4 * sizeof(float));
        }
        status = kOfxStatOK;
    }

    gEffectHost->clipReleaseImage(srcImg);
    gEffectHost->clipReleaseImage(dstImg);
    t_presetOverride = nullptr;
    return status;
}

// ============================================================
// Main entry dispatcher
// ============================================================
static OfxStatus pluginMain(const char* action, const void* handle,
                            OfxPropertySetHandle inArgs,
                            OfxPropertySetHandle outArgs)
{
    OfxImageEffectHandle effect = (OfxImageEffectHandle)handle;

    if (strcmp(action, kOfxActionLoad) == 0) return actionLoad();
    if (strcmp(action, kOfxActionUnload) == 0) {
#ifdef __APPLE__
        metalRelease();
#endif
        return kOfxStatOK;
    }
    if (strcmp(action, kOfxActionDescribe) == 0)
        return actionDescribe(effect);
    if (strcmp(action, kOfxImageEffectActionDescribeInContext) == 0)
        return actionDescribeInContext(effect, inArgs);
    if (strcmp(action, kOfxActionCreateInstance) == 0)
        return actionCreateInstance(effect);
    if (strcmp(action, kOfxActionDestroyInstance) == 0)
        return actionDestroyInstance(effect);
    if (strcmp(action, kOfxActionInstanceChanged) == 0)
        return actionInstanceChanged(effect, inArgs);
    if (strcmp(action, kOfxImageEffectActionIsIdentity) == 0)
        return actionIsIdentity(effect, inArgs, outArgs);
    if (strcmp(action, kOfxImageEffectActionRender) == 0)
        return actionRender(effect, inArgs);

    return kOfxStatReplyDefault;
}

static void setHost(OfxHost* host) { gHost = host; }

static OfxPlugin pluginStruct = {
    kOfxImageEffectPluginApi,
    1,
    PLUGIN_ID,
    PLUGIN_VERSION_MAJOR,
    PLUGIN_VERSION_MINOR,
    setHost,
    pluginMain
};

extern "C" {
    int OfxGetNumberOfPlugins(void) { return 1; }
    OfxPlugin* OfxGetPlugin(int nth) { return (nth == 0) ? &pluginStruct : nullptr; }
}
