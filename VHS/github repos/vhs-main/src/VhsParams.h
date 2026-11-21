#ifndef VHS_PARAMS_H
#define VHS_PARAMS_H

// ============================================================
// Lofi FX VHS — shared parameter struct
//
// IMPORTANT — colour-space architecture:
//
//   The VHS simulation runs in display-referred Rec.709 / Y'CbCr
//   (BT.601 matrix), NOT in DWG/DI working space. VHS is a
//   video-domain effect: composite encoding, NTSC/PAL bandwidth
//   limits, interlace and tape artifacts are all defined relative
//   to a video signal carrying display-light primaries with
//   601 colorimetry (the standard prevalent when VHS was current).
//
//   The CST pipeline brackets the simulation:
//     Input CST  : raw working space → DWG/DI       (optional)
//     [internal] : DI → DWG-linear → Rec.709 → Y'CbCr (BT.601)
//     [VHS sim runs here]
//     [internal] : Y'CbCr → Rec.709 → DWG-linear → DI
//     Output CST : DWG/DI → target working space    (optional)
//
//   This matches the Film plugin's CST conventions so users in
//   DaVinci YRGB Color Managed (DWG+DI) get correct behavior with
//   "Input/Output Transform" both set to "None".
//
// Layout: identical in C++ and Metal/CUDA. All fields are int or
// float (4 bytes); pad explicitly at the end if needed.
// ============================================================

struct VhsParams {
    // ---- Image / dispatch ----
    int   width;
    int   height;
    int   srcRowBytes;
    int   dstRowBytes;
    float renderScale;     // OFX render scale (proxy / scrub)
    int   frame_index;     // (int)floor(time)
    int   seed_offset;     // user seed variant

    // ---- Format / standard ----
    int   tvStandard;      // 0 = NTSC, 1 = PAL
    float bandwidthLines;  // luma bandwidth target (active scan lines, 100..500)
    float bandwidthRows;   // luma vertical bandwidth target (rows, 50..500)
    float chromaBandwidth; // chroma bandwidth target (40..200 lines)
    int   palSwitch;       // PAL alternate-line phase switch (0/1)

    // ---- Composite encode / decode ----
    float compositeStrength;   // 0 = pure Y/C separated, 1 = full encode→decode
    float dotCrawlGain;        // luma-into-chroma artifact strength
    float crossColorGain;      // chroma-into-luma artifact strength
    float compositeNoise;      // composite-domain noise %

    // ---- Magnetic noise ----
    float lumaNoise;           // luma noise amplitude (0..1)
    float chromaNoise;         // chroma noise amplitude (0..1)
    float noiseTintHue;        // hue bias (degrees) for chroma noise

    // ---- Interlace ----
    int   interlaceMode;       // 0 = off, 1 = NTSC comb
    float interlaceAmount;     // 0..15 field displacement strength in px
    int   fieldOrder;          // 0 = top first, 1 = bottom first

    // ---- Tracking / dropouts ----
    int   headSwitchEnable;    // bottom head-switch noise band
    float headSwitchHeight;    // band height in pixels (typ. 12)
    float headSwitchIntensity; // 0..1 jitter + noise amount
    int   trackingErrorEnable;
    float trackingErrorAmount; // 0..1 severity
    float trackingErrorSpeed;  // vertical drift speed (lines/sec)
    float trackingErrorHeight; // half-band height (px)
    float trackingErrorOpacity; // 0..1 overall visibility
    float dropoutDensity;      // expected dropouts per frame
    float dropoutLength;       // mean horizontal length (pixels)

    // ---- Geometric wobble ----
    float lineJitterPx;        // per-line horizontal jitter sigma (px)
    float topSkewPx;           // top-of-frame characteristic horizontal skew (px)
    float topSkewDecay;        // exponential decay rate (lines)
    float edgePincushion;      // 0..1 edge curvature

    // ---- Color fade / CRT trim ----
    float colorFade;           // 0 = no fade, 1 = monochrome
    float hueTint;             // global hue rotation degrees
    float blackLift;           // 0..0.2
    float whiteKnee;           // 0..1 (1 = clip, 0 = soft roll-off)
    float dynamicRangeStops;   // 4..14 stops of effective range

    // ---- OSD (camcorder burn-in) ----
    int   osdEnable;           // master toggle
    int   osdElements;         // bitmask: bit0=date bit1=time bit2=REC bit3=playMode
                               //          bit4=counter bit5=speedLabel
    int   osdPositionDate;     // 0..8 (TL..BR)
    int   osdPositionTime;
    int   osdPositionRec;
    int   osdPositionMode;
    int   osdPositionCounter;
    int   osdPositionSpeed;
    int   osdGlyphSet;         // 0 = 7-segment, 1 = blocky bitmap
    int   osdShadow;           // 0/1 black 1px shadow
    float osdScale;            // visual scale 0.5..3.0 of native bitmap
    int   osdPlayMode;         // 0=PLAY 1=PAUSE 2=FF 3=REW 4=STOP
    int   osdSpeedLabel;       // 0=SP 1=LP 2=EP
    int   osdRecBlink;         // 0/1
    // Free-running clock — start time is captured at instance creation /
    // user param edit, advances at frame rate per timeline frame.
    int   osdStartYear;
    int   osdStartMonth;
    int   osdStartDay;
    int   osdStartHour;
    int   osdStartMinute;
    int   osdStartSecond;
    int   osdDateFormat;       // 0=YYYY MM DD, 1=MM DD YYYY, 2=DD MM YYYY

    // ---- CST (Input / Output transforms) ----
    int   inTransform;         // 0 = none, 1..N from discovered input CSTs
    int   inputCstLutSize;
    int   outTransform;        // 0 = none, 1..N from discovered output CSTs
    int   cstLutSize;

    // ---- Dropout height ----
    float dropoutHeight;   // vertical stripe height (px), default 1

    // ---- Dropout softness / opacity ----
    float dropoutOpacity;  // 0..1 blend with dropout white
    float dropoutSoftness; // edge feather px, 0..20

    // ---- Channel registration ----
    float chromaOffsetR;   // Cr channel horizontal offset (px), default 0
    float chromaOffsetB;   // Cb channel horizontal offset (px), default 0

    // ---- Mist / CRT bloom ----
    int   mistEnabled;     // 0/1
    float mistAmount;      // bloom intensity, 0..2
    float mistSpread;      // PSF width (px), 2..80
    float mistThreshold;   // highlight extraction threshold, 0..1
    float mistVeil;        // shadow-lift veiling glare, 0..1
};

// Bump the expected size if you add fields. Compile error is a
// strong signal that the kernel-side struct must be updated too.
static_assert(sizeof(VhsParams) % 4 == 0,
              "VhsParams must be 4-byte aligned");

// ---- TV standard IDs ----
#define TV_NTSC 0
#define TV_PAL  1

// ---- Interlace modes ----
#define INTERLACE_OFF        0
#define INTERLACE_COMB       1

// ---- OSD element bitmask ----
#define OSD_BIT_DATE     (1 << 0)
#define OSD_BIT_TIME     (1 << 1)
#define OSD_BIT_REC      (1 << 2)
#define OSD_BIT_MODE     (1 << 3)
#define OSD_BIT_COUNTER  (1 << 4)
#define OSD_BIT_SPEED    (1 << 5)

// ---- 9-corner positions ----
#define POS_TL 0
#define POS_TC 1
#define POS_TR 2
#define POS_ML 3
#define POS_MC 4
#define POS_MR 5
#define POS_BL 6
#define POS_BC 7
#define POS_BR 8

// ---- OSD play-mode IDs ----
#define OSD_PLAY  0
#define OSD_PAUSE 1
#define OSD_FF    2
#define OSD_REW   3
#define OSD_STOP  4

// ---- OSD speed labels ----
#define OSD_SPEED_SP 0
#define OSD_SPEED_LP 1
#define OSD_SPEED_EP 2

// ============================================================
// Parameter name constants — canonical strings used by
// LofiFxVhsPlugin.cpp, OFX param IDs, and preset files.
// ============================================================
#define PARAM_MASTER_PRESET     "master_preset"
#define PARAM_INPUT_TRANSFORM   "input_transform"
#define PARAM_OUTPUT_TRANSFORM  "output_transform"
#define PARAM_PRESET_NAME       "preset_name"
#define PARAM_SAVE_PRESET       "save_preset"
#define PARAM_REFRESH_PRESETS   "refresh_presets"
#define PARAM_INITIALIZED       "_initialized"


// Format
#define PARAM_TV_STANDARD       "tv_standard"
#define PARAM_BANDWIDTH         "bandwidth_lines"
#define PARAM_BANDWIDTH_ROWS    "bandwidth_rows"
#define PARAM_CHROMA_BW         "chroma_bandwidth_lines"
#define PARAM_PAL_SWITCH        "pal_switch"

// Composite
#define PARAM_COMPOSITE_EN      "composite_enable"
#define PARAM_COMP_STRENGTH     "composite_strength"
#define PARAM_DOT_CRAWL         "dot_crawl"
#define PARAM_CROSS_COLOR       "cross_color"
#define PARAM_COMP_NOISE        "composite_noise"

// Noise
#define PARAM_NOISE_EN          "noise_enable"
#define PARAM_LUMA_NOISE        "luma_noise"
#define PARAM_CHROMA_NOISE      "chroma_noise"
#define PARAM_NOISE_TINT_HUE    "noise_tint_hue"
#define PARAM_SEED_OFFSET       "seed_offset"

// Interlace
#define PARAM_INTERLACE_MODE    "interlace_mode"
#define PARAM_INTERLACE_AMOUNT  "interlace_amount"
#define PARAM_FIELD_ORDER       "field_order"

// Tracking / artifacts
#define PARAM_HEADSWITCH_EN     "headswitch_enable"
#define PARAM_HEADSWITCH_H      "headswitch_height"
#define PARAM_HEADSWITCH_INT    "headswitch_intensity"
#define PARAM_TRACKING_EN       "tracking_enable"
#define PARAM_TRACKING_AMT      "tracking_amount"
#define PARAM_TRACKING_SPEED    "tracking_speed"
#define PARAM_TRACKING_HEIGHT   "tracking_height"
#define PARAM_TRACKING_OPACITY  "tracking_opacity"
#define PARAM_DROPOUT_EN        "dropout_enable"
#define PARAM_DROPOUT_DENSITY   "dropout_density"
#define PARAM_DROPOUT_LENGTH    "dropout_length"
#define PARAM_DROPOUT_HEIGHT    "dropout_height"
#define PARAM_DROPOUT_OPACITY   "dropout_opacity"
#define PARAM_DROPOUT_SOFTNESS  "dropout_softness"

// Geometry
#define PARAM_GEOMETRY_EN       "geometry_enable"
#define PARAM_LINE_JITTER       "line_jitter_px"
#define PARAM_TOP_SKEW          "top_skew_px"
#define PARAM_TOP_SKEW_DECAY    "top_skew_decay"
#define PARAM_EDGE_PINCUSHION   "edge_pincushion"

// Channel registration
#define PARAM_CHROMA_REG_EN     "chroma_reg_enable"
#define PARAM_CHROMA_OFFSET_R   "chroma_offset_r"
#define PARAM_CHROMA_OFFSET_B   "chroma_offset_b"

// Mist / CRT bloom
#define PARAM_MIST_ENABLED      "mist_enabled"
#define PARAM_MIST_AMOUNT       "mist_amount"
#define PARAM_MIST_SPREAD       "mist_spread"
#define PARAM_MIST_THRESHOLD    "mist_threshold"
#define PARAM_MIST_VEIL         "mist_veil"

// Age / color
#define PARAM_AGE_EN            "age_enable"
#define PARAM_COLOR_FADE        "color_fade"
#define PARAM_HUE_TINT          "hue_tint"
#define PARAM_BLACK_LIFT        "black_lift"
#define PARAM_WHITE_KNEE        "white_knee"
#define PARAM_DYNAMIC_RANGE     "dynamic_range_stops"

// OSD
#define PARAM_OSD_ENABLE        "osd_enable"
#define PARAM_OSD_DATE_EN       "osd_date_enable"
#define PARAM_OSD_TIME_EN       "osd_time_enable"
#define PARAM_OSD_REC_EN        "osd_rec_enable"
#define PARAM_OSD_MODE_EN       "osd_mode_enable"
#define PARAM_OSD_COUNTER_EN    "osd_counter_enable"
#define PARAM_OSD_SPEED_EN      "osd_speed_enable"
#define PARAM_OSD_DATE_POS      "osd_date_pos"
#define PARAM_OSD_TIME_POS      "osd_time_pos"
#define PARAM_OSD_REC_POS       "osd_rec_pos"
#define PARAM_OSD_MODE_POS      "osd_mode_pos"
#define PARAM_OSD_COUNTER_POS   "osd_counter_pos"
#define PARAM_OSD_SPEED_POS     "osd_speed_pos"
#define PARAM_OSD_GLYPH_SET     "osd_glyph_set"
#define PARAM_OSD_SHADOW        "osd_shadow"
#define PARAM_OSD_SCALE         "osd_scale"
#define PARAM_OSD_PLAY_MODE     "osd_play_mode"
#define PARAM_OSD_SPEED_LABEL   "osd_speed_label"
#define PARAM_OSD_REC_BLINK     "osd_rec_blink"
#define PARAM_OSD_START_YEAR    "osd_start_year"
#define PARAM_OSD_START_MONTH   "osd_start_month"
#define PARAM_OSD_START_DAY     "osd_start_day"
#define PARAM_OSD_START_HOUR    "osd_start_hour"
#define PARAM_OSD_START_MINUTE  "osd_start_minute"
#define PARAM_OSD_START_SECOND  "osd_start_second"
#define PARAM_OSD_DATE_FORMAT   "osd_date_format"

// ============================================================
// Plugin identity
// ============================================================
#define PLUGIN_ID             "com.lofifxvhs.tape"
#define PLUGIN_LABEL          "LoFi FX VHS"
#define PLUGIN_GROUP          "LoFi FX"
#define PLUGIN_VERSION_MAJOR  1
#define PLUGIN_VERSION_MINOR  0

// ============================================================
// OFX GPU render property strings — fallbacks for older headers
// ============================================================
#ifndef kOfxImageEffectPropCudaRenderSupported
#define kOfxImageEffectPropCudaRenderSupported   "OfxImageEffectPropCudaRenderSupported"
#endif
#ifndef kOfxImageEffectPropCudaEnabled
#define kOfxImageEffectPropCudaEnabled           "OfxImageEffectPropCudaEnabled"
#endif
#ifndef kOfxImageEffectPropCudaStream
#define kOfxImageEffectPropCudaStream            "OfxImageEffectPropCudaStream"
#endif
#ifndef kOfxImageEffectPropMetalRenderSupported
#define kOfxImageEffectPropMetalRenderSupported   "OfxImageEffectPropMetalRenderSupported"
#endif
#ifndef kOfxImageEffectPropMetalEnabled
#define kOfxImageEffectPropMetalEnabled           "OfxImageEffectPropMetalEnabled"
#endif
#ifndef kOfxImageEffectPropMetalCommandQueue
#define kOfxImageEffectPropMetalCommandQueue      "OfxImageEffectPropMetalCommandQueue"
#endif

#endif // VHS_PARAMS_H
