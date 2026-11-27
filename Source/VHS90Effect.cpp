#include "AEConfig.h"
#include "entry.h"
#include "AEFX_SuiteHelper.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_EffectCBSuites.h"
#include "AE_EffectPixelFormat.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "VHS90SignalKernels.h"
#include "VHS90RegionEngine.h"
#include "VHS90StaticEngine.h"
#include "VHS90TearChromaEngine.h"
#include "VHS90IIRFilter.h"
#include "VHS90CompositeEngine.h"
#include "VHS90ChromaShiftEngine.h"
#include "VHS90OsdEngine.h"
#include "VHS90OsdLocalization.h"
#include "VHS90OsdArbitraryData.h"
#include "VHS90DirectWriteText.h"
#include "VHS90OsdTextDialog.h"
#include "VHS90SyncEngine.h"
#include "VHS90SyncCollapseLabEngine.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <new>
#include <vector>

namespace {
using namespace vhs90;

enum ParamIndex {
    PARAM_INPUT=0, PARAM_VHS_STYLE_TOPIC, PARAM_VHS_STYLE, PARAM_VHS_STYLE_INTENSITY, PARAM_CAMERA_PROFILE, PARAM_CAMERA_INTENSITY, PARAM_VHS_STYLE_END, PARAM_MASTER, PARAM_SIGNAL_TOPIC, PARAM_COMPOSITE_AMOUNT,
    PARAM_SYNC_AMOUNT, PARAM_PAL_MODE, PARAM_SIGNAL_MODEL,   PARAM_TRACK_READ, PARAM_TRACK_MISALIGNMENT,      
       
    PARAM_RF_DRIFT, PARAM_RF_HUM, PARAM_RF_AM, PARAM_AGC_PUMP, PARAM_IMPULSE_NOISE, PARAM_DROPOUTS,
    PARAM_MULTIPATH_GAIN, PARAM_MULTIPATH_DELAY, PARAM_MULTIPATH_ENSEMBLE, PARAM_SIGNAL_GHOST,  
    PARAM_AFC_HUNT,    
    PARAM_HSYNC_NOISE, PARAM_VSYNC_NOISE, PARAM_BURST_NOISE,
    PARAM_EDGE_ENHANCE, PARAM_TAPE_FADE, PARAM_BLACK_LIFT, PARAM_TEMPORAL_GHOST,     
           
        
    PARAM_GEOMETRY, PARAM_INTERLACE, PARAM_VERTICAL_ROLL, PARAM_EDGE_WAVE, PARAM_EDGE_WAVE_FREQUENCY, PARAM_EDGE_WAVE_SPEED, PARAM_EDGE_WAVE_DETAIL, PARAM_SCANLINES, PARAM_NTSCRS_TOPIC, PARAM_NTSCRS_LOOK, PARAM_NTSCRS_INTENSITY, PARAM_NTSCRS_PHASE_NOISE, PARAM_NTSCRS_END, PARAM_SIGNAL_END, PARAM_RINGING_TOPIC, PARAM_RINGING, PARAM_RINGING_POWER, PARAM_CRT_EQUALIZER, PARAM_RINGING_END, PARAM_GLOW_TOPIC, PARAM_LUMA_SMEAR, PARAM_BLOOM, PARAM_ANALOG_GLOW, PARAM_GLOW_END, PARAM_GENERAL_TOPIC, PARAM_GRADE_EXPOSURE, PARAM_GRADE_CONTRAST, PARAM_GRADE_BRIGHTNESS, PARAM_GRADE_SATURATION, PARAM_VIGNETTE, PARAM_FLICKER, PARAM_GENERAL_END, PARAM_CHROMA_TOPIC, PARAM_CHROMA_SHIFT, PARAM_CHROMA_REG_RED, PARAM_CHROMA_REG_GREEN, PARAM_CHROMA_REG_BLUE, PARAM_CHROMA_REG_MODE, PARAM_CHROMA_SUBSAMPLE, PARAM_VERTICAL_CHROMA, PARAM_VERTICAL_LUMA, PARAM_CHROMA_DELAY, PARAM_CHROMA_LOSS, PARAM_CHROMA_FLUTTER, PARAM_YC_CROSSTALK, PARAM_COLOR_PHASE_DRIFT, PARAM_COLOR_DRIFT, PARAM_GRADE_I_GAIN, PARAM_GRADE_Q_GAIN, PARAM_GRADE_I_OFFSET, PARAM_GRADE_Q_OFFSET, PARAM_GRADE_CHROMA_QUANTIZATION, PARAM_CHROMA_EDGE_ENHANCE, PARAM_CHROMA_END, PARAM_STATIC_TOPIC, PARAM_STATIC_AMOUNT,
    PARAM_STATIC_DENSITY, PARAM_STATIC_END, PARAM_GRAIN_TOPIC, PARAM_LUMA_NOISE, PARAM_CHROMA_NOISE, PARAM_COMPOSITE_NOISE, PARAM_NOISE_HUE, PARAM_VHS6RIFT_GRAIN, PARAM_VHS6RIFT_GRAIN_SIZE, PARAM_VHS6RIFT_NOISE_DETAIL, PARAM_VHS6RIFT_SHADOW_BIAS, PARAM_VHS6RIFT_NOISE_ENABLE, PARAM_VHS6RIFT_NOISE_SEED, PARAM_SIGNAL_HUE_NOISE, PARAM_GRAIN_END, PARAM_TRACKING_TOPIC, PARAM_TRACKING_AMOUNT,
    PARAM_TRACKING_SHAPE, PARAM_TRACKING_CENTER_X, PARAM_TRACKING_CENTER_Y,
    PARAM_TRACKING_WIDTH, PARAM_TRACKING_HEIGHT, PARAM_TRACKING_ANGLE, PARAM_TRACKING_MOTION_X,
    PARAM_TRACKING_MOTION_Y, PARAM_TRACKING_SERVO, PARAM_TRACKING_TEAR,
    PARAM_TRACKING_END, PARAM_HEAD_TOPIC, PARAM_HEAD_AMOUNT, PARAM_HEAD_BOTTOM_Y,
    PARAM_HEAD_HEIGHT, PARAM_HEAD_SHIFT, PARAM_HEAD_END, PARAM_HEAD_CONDITION, PARAM_TAPE_MATERIAL_TOPIC, PARAM_TAPE_SPEED, PARAM_TAPE_DETAIL, PARAM_TAPE_SURFACE, PARAM_TAPE_TRANSPORT, PARAM_TAPE_WEAR, PARAM_TAPE_DROPOUTS, PARAM_TAPE_SCRATCHES, PARAM_TAPE_CREASES, PARAM_TAPE_EDGE, PARAM_TAPE_MOLD, PARAM_TAPE_VASELINE, PARAM_TAPE_STICKY, PARAM_TAPE_SEED, PARAM_TAPE_BACK_TENSION, PARAM_TAPE_NOISE, PARAM_TAPE_MATERIAL_END, PARAM_PHYSICAL_TOPIC,    PARAM_MAINS_HUM,   PARAM_PHYSICAL_END,         PARAM_OSD_TOPIC, PARAM_OSD_AMOUNT, PARAM_OSD_REC, PARAM_OSD_PLAY, PARAM_OSD_TIMESTAMP, PARAM_OSD_OPACITY, PARAM_OSD_SCALE, PARAM_OSD_GLITCH, PARAM_OSD_GHOST, PARAM_OSD_REC_BLINK, PARAM_OSD_TIME_SPEED, PARAM_OSD_YEAR, PARAM_OSD_MONTH, PARAM_OSD_DAY, PARAM_OSD_HOUR, PARAM_OSD_MINUTE, PARAM_OSD_SECOND, PARAM_OSD_REC_COLOR, PARAM_OSD_END,                               PARAM_TRACKING_PROFILE,               PARAM_TEAR_CHROMA_TOPIC, PARAM_TEAR_CHROMA_AMOUNT, PARAM_TEAR_CHROMA_MODE, PARAM_SPECTRAL_SMEAR, PARAM_SPECTRAL_HUE_SPREAD, PARAM_CHROMA_SATURATION, PARAM_SPECTRAL_CENTER_Y, PARAM_SPECTRAL_COVERAGE, PARAM_TEAR_BAND_START_Y, PARAM_TEAR_BAND_HEIGHT, PARAM_TEAR_BAND_COUNT, PARAM_TEAR_DISPLACEMENT, PARAM_TEAR_FRAGMENTATION, PARAM_TEAR_CHROMA_SHIFT, PARAM_TEAR_SMEAR, PARAM_TEAR_DROPOUT, PARAM_TEAR_SEED, PARAM_TEAR_CHROMA_END,
    PARAM_TRACK_LAYER2_TOPIC, PARAM_TRACK_LAYER2_AMOUNT, PARAM_TRACK_LAYER2_PROFILE, PARAM_TRACK_LAYER2_SHAPE, PARAM_TRACK_LAYER2_X, PARAM_TRACK_LAYER2_Y, PARAM_TRACK_LAYER2_WIDTH, PARAM_TRACK_LAYER2_HEIGHT, PARAM_TRACK_LAYER2_SERVO, PARAM_TRACK_LAYER2_SHIFT, PARAM_TRACK_LAYER2_WAVE, PARAM_TRACK_LAYER2_TEAR, PARAM_TRACK_LAYER2_END,
    PARAM_TRACK_LAYER3_TOPIC, PARAM_TRACK_LAYER3_AMOUNT, PARAM_TRACK_LAYER3_PROFILE, PARAM_TRACK_LAYER3_SHAPE, PARAM_TRACK_LAYER3_X, PARAM_TRACK_LAYER3_Y, PARAM_TRACK_LAYER3_WIDTH, PARAM_TRACK_LAYER3_HEIGHT, PARAM_TRACK_LAYER3_SERVO, PARAM_TRACK_LAYER3_SHIFT, PARAM_TRACK_LAYER3_WAVE, PARAM_TRACK_LAYER3_TEAR, PARAM_TRACK_LAYER3_END,
    PARAM_TRACK_LAYER4_TOPIC, PARAM_TRACK_LAYER4_AMOUNT, PARAM_TRACK_LAYER4_PROFILE, PARAM_TRACK_LAYER4_SHAPE, PARAM_TRACK_LAYER4_X, PARAM_TRACK_LAYER4_Y, PARAM_TRACK_LAYER4_WIDTH, PARAM_TRACK_LAYER4_HEIGHT, PARAM_TRACK_LAYER4_SERVO, PARAM_TRACK_LAYER4_SHIFT, PARAM_TRACK_LAYER4_WAVE, PARAM_TRACK_LAYER4_TEAR, PARAM_TRACK_LAYER4_END, PARAM_TRACKING_PRESETS_TOPIC, PARAM_TRACKING_PRESET_BASE, PARAM_TRACKING_PRESET_2, PARAM_TRACKING_PRESET_3, PARAM_TRACKING_PRESET_4, PARAM_TRACKING_PRESETS_END, PARAM_PATTERN_TOPIC, PARAM_PATTERN_MODE, PARAM_PATTERN_ANGLE, PARAM_PATTERN_WIDTH, PARAM_PATTERN_GREEN, PARAM_PATTERN_PHASE_COUNT, PARAM_PATTERN_END,
    PARAM_TEAR_BAND2_TOPIC, PARAM_TEAR_BAND2_AMOUNT, PARAM_TEAR_BAND2_Y, PARAM_TEAR_BAND2_HEIGHT, PARAM_TEAR_BAND2_SHIFT, PARAM_TEAR_BAND2_FRAGMENTATION, PARAM_TEAR_BAND2_CHROMA, PARAM_TEAR_BAND2_SMEAR, PARAM_TEAR_BAND2_DROPOUT, PARAM_TEAR_BAND2_END,
    PARAM_TEAR_BAND3_TOPIC, PARAM_TEAR_BAND3_AMOUNT, PARAM_TEAR_BAND3_Y, PARAM_TEAR_BAND3_HEIGHT, PARAM_TEAR_BAND3_SHIFT, PARAM_TEAR_BAND3_FRAGMENTATION, PARAM_TEAR_BAND3_CHROMA, PARAM_TEAR_BAND3_SMEAR, PARAM_TEAR_BAND3_DROPOUT, PARAM_TEAR_BAND3_END,
    PARAM_TEAR_BAND4_TOPIC, PARAM_TEAR_BAND4_AMOUNT, PARAM_TEAR_BAND4_Y, PARAM_TEAR_BAND4_HEIGHT, PARAM_TEAR_BAND4_SHIFT, PARAM_TEAR_BAND4_FRAGMENTATION, PARAM_TEAR_BAND4_CHROMA, PARAM_TEAR_BAND4_SMEAR, PARAM_TEAR_BAND4_DROPOUT, PARAM_TEAR_BAND4_END,
    PARAM_TEAR_BAND5_TOPIC, PARAM_TEAR_BAND5_AMOUNT, PARAM_TEAR_BAND5_Y, PARAM_TEAR_BAND5_HEIGHT, PARAM_TEAR_BAND5_SHIFT, PARAM_TEAR_BAND5_FRAGMENTATION, PARAM_TEAR_BAND5_CHROMA, PARAM_TEAR_BAND5_SMEAR, PARAM_TEAR_BAND5_DROPOUT, PARAM_TEAR_BAND5_END,
    PARAM_OSD_LAYOUT_TOPIC, PARAM_OSD_DECK_MODE, PARAM_OSD_DATE_FORMAT, PARAM_OSD_REC_X, PARAM_OSD_REC_Y,
    PARAM_OSD_STATUS_X, PARAM_OSD_STATUS_Y, PARAM_OSD_TIMESTAMP_X, PARAM_OSD_TIMESTAMP_Y, PARAM_OSD_LAYOUT_END,
    PARAM_OSD_COUNTER_TOPIC, PARAM_OSD_COUNTER_FORMAT, PARAM_OSD_COUNTER_AMOUNT, PARAM_OSD_COUNTER_X, PARAM_OSD_COUNTER_Y, PARAM_OSD_COUNTER_OFFSET, PARAM_OSD_SHOW_COUNTER, PARAM_OSD_COUNTER_END, PARAM_OSD_TEXT_DATA, PARAM_OSD_LOCALIZATION_TOPIC,
    PARAM_OSD_LANGUAGE, PARAM_OSD_DIGITS, PARAM_OSD_DATE_LAYOUT, PARAM_OSD_CLOCK_MODE, PARAM_OSD_MONTH_STYLE, PARAM_OSD_CLOCK_RUNS, PARAM_OSD_FONT, PARAM_OSD_EXTRA_LABEL, PARAM_OSD_EXTRA_X, PARAM_OSD_EXTRA_Y, PARAM_OSD_LOCALIZATION_END,
    PARAM_OSD_WORDS_TOPIC, PARAM_OSD_SHOW_TEXT, PARAM_OSD_SHOW_REC_LABEL, PARAM_OSD_SHOW_PLAY_LABEL, PARAM_OSD_SHOW_PAUSE_LABEL, PARAM_OSD_SHOW_STOP_LABEL, PARAM_OSD_SHOW_FF_LABEL, PARAM_OSD_SHOW_REW_LABEL, PARAM_OSD_SHOW_SPEED_LABEL, PARAM_OSD_SHOW_TAPE_LABEL, PARAM_OSD_SHOW_ERROR_LABEL, PARAM_OSD_SHOW_PROGRAM_LABEL, PARAM_OSD_SHOW_RES_LABEL, PARAM_OSD_SHOW_FORWARD_LABEL, PARAM_OSD_TEXT_GLOW, PARAM_OSD_TEXT_SIZE, PARAM_OSD_TEXT_GLOW_AMOUNT, PARAM_OSD_TEXT_SEED, PARAM_OSD_TEXT_OPACITY, PARAM_OSD_SHOW_DATE, PARAM_OSD_WORDS_X, PARAM_OSD_WORDS_Y, PARAM_OSD_WORDS_END, PARAM_TAPE_SURFACE_DETAIL_TOPIC, PARAM_TAPE_PINHOLES, PARAM_TAPE_FLAKES, PARAM_TAPE_ROUGHNESS, PARAM_TAPE_SURFACE_DETAIL_END, PARAM_OSD_TEXT_EDIT_TOPIC, PARAM_OSD_TEXT_EDIT, PARAM_OSD_TEXT_EDIT_END, PARAM_PLAYBACK_TOPIC, PARAM_DECODER_MODE, PARAM_FRAME_REPEAT_FREQUENCY, PARAM_FRAME_REPEAT_STRENGTH, PARAM_PLAYBACK_END, PARAM_GROUND_LOOP_TOPIC, PARAM_GROUND_LOOP, PARAM_GROUND_LOOP_END, PARAM_TRACK_LAYER_MOTION_TOPIC, PARAM_TRACK_LAYER2_ANGLE, PARAM_TRACK_LAYER2_MOTION_X, PARAM_TRACK_LAYER2_MOTION_Y, PARAM_TRACK_LAYER3_ANGLE, PARAM_TRACK_LAYER3_MOTION_X, PARAM_TRACK_LAYER3_MOTION_Y, PARAM_TRACK_LAYER4_ANGLE, PARAM_TRACK_LAYER4_MOTION_X, PARAM_TRACK_LAYER4_MOTION_Y, PARAM_TRACK_LAYER_MOTION_END, PARAM_SYNC_DAMAGE_TOPIC, PARAM_SYNC_DAMAGE_ENABLE, PARAM_SYNC_DAMAGE_TEAR_AMOUNT, PARAM_SYNC_DAMAGE_BAND_INTENSITY, PARAM_SYNC_DAMAGE_BAND_DENSITY, PARAM_SYNC_DAMAGE_BAND_HEIGHT, PARAM_SYNC_DAMAGE_BAND_IRREGULARITY, PARAM_SYNC_DAMAGE_DISPLACEMENT, PARAM_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION, PARAM_SYNC_DAMAGE_COVERAGE, PARAM_SYNC_DAMAGE_LUMA_ENABLE, PARAM_SYNC_DAMAGE_LUMA_INSTABILITY, PARAM_SYNC_DAMAGE_BRIGHT_FLASH, PARAM_SYNC_DAMAGE_DARK_COLLAPSE, PARAM_SYNC_DAMAGE_EVENT_FREQUENCY, PARAM_SYNC_DAMAGE_EVENT_DURATION, PARAM_SYNC_DAMAGE_BRIGHT_PROBABILITY, PARAM_SYNC_DAMAGE_DARK_PROBABILITY, PARAM_SYNC_DAMAGE_TRANSITION_SHARPNESS, PARAM_SYNC_DAMAGE_LOCAL_LUMA, PARAM_SYNC_DAMAGE_MANUAL_BRIGHT, PARAM_SYNC_DAMAGE_MANUAL_DARK, PARAM_SYNC_DAMAGE_DROPOUTS_ENABLE, PARAM_SYNC_DAMAGE_DROPOUT_DENSITY, PARAM_SYNC_DAMAGE_STREAK_LENGTH, PARAM_SYNC_DAMAGE_STREAK_BRIGHTNESS, PARAM_SYNC_DAMAGE_DARK_DROPOUT, PARAM_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION, PARAM_SYNC_DAMAGE_TEMPORAL_VARIATION, PARAM_SYNC_DAMAGE_CHROMA_ENABLE, PARAM_SYNC_DAMAGE_CHROMA_TINT, PARAM_SYNC_DAMAGE_MAGENTA_TINT, PARAM_SYNC_DAMAGE_GREEN_TINT, PARAM_SYNC_DAMAGE_RGB_OFFSET, PARAM_SYNC_DAMAGE_CHROMA_BLEED, PARAM_SYNC_DAMAGE_ANIMATION_MODE, PARAM_SYNC_DAMAGE_INPUT_MODE, PARAM_SYNC_DAMAGE_ANIMATION_SPEED, PARAM_SYNC_DAMAGE_RANDOM_SEED, PARAM_SYNC_DAMAGE_BLEND_INPUT, PARAM_SYNC_DAMAGE_EFFECT_AMOUNT, PARAM_SYNC_DAMAGE_END, PARAM_COUNT
};
enum ParamDiskID {
    ID_VHS_STYLE_TOPIC=136,
    ID_VHS_STYLE=137,
    ID_VHS_STYLE_INTENSITY=138,
    ID_CAMERA_PROFILE=139,
    ID_CAMERA_INTENSITY=140,
    ID_MASTER=1,
    ID_SIGNAL_TOPIC=2,
    ID_COMPOSITE=3,
    ID_SYNC=4,
    ID_PAL_MODE=5,
    ID_SIGNAL_MODEL=6,
    ID_TRACK_READ=9,
    ID_TRACK_MISALIGNMENT=10,
    ID_RF_DRIFT=21,
    ID_RF_HUM=22,
    ID_RF_AM=23,
    ID_AGC_PUMP=24,
    ID_IMPULSE_NOISE=25,
    ID_DROPOUTS=26,
    ID_MULTIPATH_GAIN=27,
    ID_MULTIPATH_DELAY=28,
    ID_MULTIPATH_ENSEMBLE=29,
    ID_SIGNAL_GHOST=30,
    ID_AFC_HUNT=33,
    ID_HSYNC_NOISE=38,
    ID_VSYNC_NOISE=39,
    ID_BURST_NOISE=40,
    ID_EDGE_ENHANCE=41,
    ID_TAPE_FADE=42,
    ID_BLACK_LIFT=43,
    ID_TEMPORAL_GHOST=44,
    ID_GEOMETRY=63,
    ID_INTERLACE=64,
    ID_VERTICAL_ROLL=65,
    ID_EDGE_WAVE=66,
    ID_EDGE_WAVE_FREQUENCY=67,
    ID_EDGE_WAVE_SPEED=68,
    ID_EDGE_WAVE_DETAIL=69,
    ID_SCANLINES=70,
    ID_NTSCRS_TOPIC=141,
    ID_NTSCRS_LOOK=142,
    ID_NTSCRS_INTENSITY=143,
    ID_NTSCRS_PHASE_NOISE=144,
    ID_RINGING_TOPIC=338,
    ID_RINGING=45,
    ID_RINGING_POWER=46,
    ID_CRT_EQUALIZER=47,
    ID_GLOW_TOPIC=339,
    ID_LUMA_SMEAR=58,
    ID_BLOOM=59,
    ID_ANALOG_GLOW=60,
    ID_GENERAL_TOPIC=340,
    ID_GRADE_CONTRAST=48,
    ID_GRADE_BRIGHTNESS=49,
    ID_GRADE_SATURATION=50,
    ID_VIGNETTE=56,
    ID_FLICKER=57,
    ID_CHROMA_TOPIC=341,
    ID_CHROMA_SHIFT=11,
    ID_CHROMA_REG_RED=12,
    ID_CHROMA_REG_BLUE=13,
    ID_CHROMA_SUBSAMPLE=14,
    ID_VERTICAL_CHROMA=15,
    ID_VERTICAL_LUMA=16,
    ID_CHROMA_DELAY=31,
    ID_CHROMA_LOSS=32,
    ID_CHROMA_FLUTTER=34,
    ID_YC_CROSSTALK=35,
    ID_COLOR_PHASE_DRIFT=36,
    ID_COLOR_DRIFT=37,
    ID_GRADE_I_GAIN=51,
    ID_GRADE_Q_GAIN=52,
    ID_GRADE_I_OFFSET=53,
    ID_GRADE_Q_OFFSET=54,
    ID_GRADE_CHROMA_QUANTIZATION=55,
    ID_STATIC_TOPIC=71,
    ID_STATIC_AMOUNT=72,
    ID_STATIC_DENSITY=73,
    ID_TRACKING_TOPIC=74,
    ID_TRACKING_AMOUNT=75,
    ID_TRACKING_SHAPE=76,
    ID_TRACKING_X=77,
    ID_TRACKING_Y=78,
    ID_TRACKING_WIDTH=79,
    ID_TRACKING_HEIGHT=80,
    ID_TRACKING_ANGLE=81,
    ID_TRACKING_MOTION_X=82,
    ID_TRACKING_MOTION_Y=83,
    ID_TRACKING_SERVO=84,
    ID_TRACKING_TEAR=85,
    ID_HEAD_TOPIC=86,
    ID_HEAD_AMOUNT=87,
    ID_HEAD_Y=88,
    ID_HEAD_HEIGHT=89,
    ID_HEAD_SHIFT=90,
    ID_HEAD_CONDITION=91,
    ID_TAPE_MATERIAL_TOPIC=92,
    ID_TAPE_SPEED=7,
    ID_TAPE_DETAIL=8,
    ID_TAPE_SURFACE=61,
    ID_TAPE_TRANSPORT=62,
    ID_TAPE_WEAR=93,
    ID_TAPE_DROPOUTS=94,
    ID_TAPE_SCRATCHES=95,
    ID_TAPE_CREASES=96,
    ID_TAPE_EDGE=97,
    ID_TAPE_MOLD=98,
    ID_TAPE_VASELINE=99,
    ID_TAPE_STICKY=100,
    ID_TAPE_SEED=101,
    ID_TAPE_BACK_TENSION=102,
    ID_TAPE_NOISE=103,
    ID_PHYSICAL_TOPIC=104,
    ID_MAINS_HUM=108,
    ID_OSD_TOPIC=118,
    ID_OSD_AMOUNT=119,
    ID_OSD_REC=120,
    ID_OSD_PLAY=121,
    ID_OSD_TIMESTAMP=122,
    ID_OSD_OPACITY=123,
    ID_OSD_SCALE=124,
    ID_OSD_GLITCH=125,
    ID_OSD_GHOST=126,
    ID_OSD_REC_BLINK=127,
    ID_OSD_TIME_SPEED=128,
    ID_OSD_YEAR=129,
    ID_OSD_MONTH=130,
    ID_OSD_DAY=131,
    ID_OSD_HOUR=132,
    ID_OSD_MINUTE=133,
    ID_OSD_SECOND=134,
    ID_OSD_REC_COLOR=135,
    ID_VHS6RIFT_GRAIN=145,
    ID_VHS6RIFT_GRAIN_SIZE=146,
    ID_VHS6RIFT_NOISE_DETAIL=147,
    ID_VHS6RIFT_SHADOW_BIAS=148,
    ID_VHS6RIFT_NOISE_ENABLE=149,
    ID_VHS6RIFT_NOISE_SEED=150,
    ID_SIGNAL_HUE_NOISE=151,
    ID_CHROMA_EDGE_ENHANCE=160,
    ID_TRACKING_PROFILE=173,
    ID_TEAR_CHROMA_TOPIC=187,
    ID_TEAR_CHROMA_AMOUNT=188,
    ID_TEAR_CHROMA_MODE=189,
    ID_SPECTRAL_SMEAR=190,
    ID_SPECTRAL_HUE_SPREAD=191,
    ID_CHROMA_SATURATION=192,
    ID_SPECTRAL_CENTER_Y=193,
    ID_SPECTRAL_COVERAGE=194,
    ID_TEAR_BAND_START_Y=195,
    ID_TEAR_BAND_HEIGHT=196,
    ID_TEAR_BAND_COUNT=197,
    ID_TEAR_DISPLACEMENT=198,
    ID_TEAR_FRAGMENTATION=199,
    ID_TEAR_CHROMA_SHIFT=200,
    ID_TEAR_SMEAR=201,
    ID_TEAR_DROPOUT=202,
    ID_TEAR_SEED=203,
    ID_TRACK_LAYER2_TOPIC=204,
    ID_TRACK_LAYER2_AMOUNT=205,
    ID_TRACK_LAYER2_PROFILE=206,
    ID_TRACK_LAYER2_SHAPE=207,
    ID_TRACK_LAYER2_X=208,
    ID_TRACK_LAYER2_Y=209,
    ID_TRACK_LAYER2_WIDTH=210,
    ID_TRACK_LAYER2_HEIGHT=211,
    ID_TRACK_LAYER2_SERVO=212,
    ID_TRACK_LAYER2_SHIFT=213,
    ID_TRACK_LAYER2_WAVE=214,
    ID_TRACK_LAYER2_TEAR=215,
    ID_TRACK_LAYER3_TOPIC=216,
    ID_TRACK_LAYER3_AMOUNT=217,
    ID_TRACK_LAYER3_PROFILE=218,
    ID_TRACK_LAYER3_SHAPE=219,
    ID_TRACK_LAYER3_X=220,
    ID_TRACK_LAYER3_Y=221,
    ID_TRACK_LAYER3_WIDTH=222,
    ID_TRACK_LAYER3_HEIGHT=223,
    ID_TRACK_LAYER3_SERVO=224,
    ID_TRACK_LAYER3_SHIFT=225,
    ID_TRACK_LAYER3_WAVE=226,
    ID_TRACK_LAYER3_TEAR=227,
    ID_TRACK_LAYER4_TOPIC=228,
    ID_TRACK_LAYER4_AMOUNT=229,
    ID_TRACK_LAYER4_PROFILE=230,
    ID_TRACK_LAYER4_SHAPE=231,
    ID_TRACK_LAYER4_X=232,
    ID_TRACK_LAYER4_Y=233,
    ID_TRACK_LAYER4_WIDTH=234,
    ID_TRACK_LAYER4_HEIGHT=235,
    ID_TRACK_LAYER4_SERVO=236,
    ID_TRACK_LAYER4_SHIFT=237,
    ID_TRACK_LAYER4_WAVE=238,
    ID_TRACK_LAYER4_TEAR=239,
    ID_TRACKING_PRESETS_TOPIC=240,
    ID_TRACKING_PRESET_BASE=241,
    ID_TRACKING_PRESET_2=242,
    ID_TRACKING_PRESET_3=243,
    ID_TRACKING_PRESET_4=244,
    ID_PATTERN_TOPIC=245,
    ID_PATTERN_MODE=246,
    ID_PATTERN_ANGLE=247,
    ID_PATTERN_WIDTH=248,
    ID_PATTERN_GREEN=249,
    ID_PATTERN_PHASE_COUNT=250,
    ID_TEAR_BAND2_TOPIC=251,
    ID_TEAR_BAND2_AMOUNT=252,
    ID_TEAR_BAND2_Y=253,
    ID_TEAR_BAND2_HEIGHT=254,
    ID_TEAR_BAND2_SHIFT=255,
    ID_TEAR_BAND2_FRAGMENTATION=256,
    ID_TEAR_BAND2_CHROMA=257,
    ID_TEAR_BAND2_SMEAR=258,
    ID_TEAR_BAND2_DROPOUT=259,
    ID_TEAR_BAND3_TOPIC=260,
    ID_TEAR_BAND3_AMOUNT=261,
    ID_TEAR_BAND3_Y=262,
    ID_TEAR_BAND3_HEIGHT=263,
    ID_TEAR_BAND3_SHIFT=264,
    ID_TEAR_BAND3_FRAGMENTATION=265,
    ID_TEAR_BAND3_CHROMA=266,
    ID_TEAR_BAND3_SMEAR=267,
    ID_TEAR_BAND3_DROPOUT=268,
    ID_TEAR_BAND4_TOPIC=269,
    ID_TEAR_BAND4_AMOUNT=270,
    ID_TEAR_BAND4_Y=271,
    ID_TEAR_BAND4_HEIGHT=272,
    ID_TEAR_BAND4_SHIFT=273,
    ID_TEAR_BAND4_FRAGMENTATION=274,
    ID_TEAR_BAND4_CHROMA=275,
    ID_TEAR_BAND4_SMEAR=276,
    ID_TEAR_BAND4_DROPOUT=277,
    ID_TEAR_BAND5_TOPIC=278,
    ID_TEAR_BAND5_AMOUNT=279,
    ID_TEAR_BAND5_Y=280,
    ID_TEAR_BAND5_HEIGHT=281,
    ID_TEAR_BAND5_SHIFT=282,
    ID_TEAR_BAND5_FRAGMENTATION=283,
    ID_TEAR_BAND5_CHROMA=284,
    ID_TEAR_BAND5_SMEAR=285,
    ID_TEAR_BAND5_DROPOUT=286,
    ID_OSD_LAYOUT_TOPIC=9025,
    ID_OSD_DECK_MODE=9001,
    ID_OSD_DATE_FORMAT=9002,
    ID_OSD_REC_X=9003,
    ID_OSD_REC_Y=9004,
    ID_OSD_STATUS_X=9005,
    ID_OSD_STATUS_Y=9006,
    ID_OSD_TIMESTAMP_X=9007,
    ID_OSD_TIMESTAMP_Y=9008,
    ID_OSD_COUNTER_TOPIC=9026,
    ID_OSD_COUNTER_FORMAT=9009,
    ID_OSD_COUNTER_AMOUNT=9010,
    ID_OSD_COUNTER_X=9011,
    ID_OSD_COUNTER_Y=9012,
    ID_OSD_COUNTER_OFFSET=9024,
    ID_OSD_SHOW_COUNTER=9047,
    ID_OSD_TEXT_DATA=9013,
    ID_OSD_LOCALIZATION_TOPIC=9027,
    ID_OSD_LANGUAGE=9014,
    ID_OSD_DIGITS=9015,
    ID_OSD_DATE_LAYOUT=9016,
    ID_OSD_CLOCK_MODE=9017,
    ID_OSD_MONTH_STYLE=9018,
    ID_OSD_CLOCK_RUNS=9019,
    ID_OSD_FONT=9020,
    ID_OSD_EXTRA_LABEL=9021,
    ID_OSD_EXTRA_X=9022,
    ID_OSD_EXTRA_Y=9023,
    ID_OSD_WORDS_TOPIC=9028,
    ID_OSD_SHOW_TEXT=9029,
    ID_OSD_SHOW_REC_LABEL=9030,
    ID_OSD_SHOW_PLAY_LABEL=9031,
    ID_OSD_SHOW_PAUSE_LABEL=9032,
    ID_OSD_SHOW_STOP_LABEL=9033,
    ID_OSD_SHOW_FF_LABEL=9034,
    ID_OSD_SHOW_REW_LABEL=9035,
    ID_OSD_SHOW_SPEED_LABEL=9036,
    ID_OSD_SHOW_TAPE_LABEL=9037,
    ID_OSD_SHOW_ERROR_LABEL=9038,
    ID_OSD_SHOW_PROGRAM_LABEL=9039,
    ID_OSD_SHOW_RES_LABEL=9040,
    ID_OSD_SHOW_FORWARD_LABEL=9041,
    ID_OSD_TEXT_GLOW=9042,
    ID_OSD_TEXT_SIZE=9048,
    ID_OSD_TEXT_GLOW_AMOUNT=9049,
    ID_OSD_TEXT_SEED=9050,
    ID_OSD_TEXT_OPACITY=9051,
    ID_OSD_SHOW_DATE=9052,
    ID_OSD_WORDS_X=9053,
    ID_OSD_WORDS_Y=9054,
    ID_GRAIN_TOPIC=9055,
    ID_LUMA_NOISE=17,
    ID_CHROMA_NOISE=18,
    ID_COMPOSITE_NOISE=19,
    ID_NOISE_HUE=20,

    ID_CHROMA_REG_GREEN=9056,
    ID_CHROMA_REG_MODE=9057,
    ID_GRADE_EXPOSURE=9058,
    ID_TAPE_SURFACE_DETAIL_TOPIC=350,
    ID_TAPE_PINHOLES=351,
    ID_TAPE_FLAKES=352,
    ID_TAPE_ROUGHNESS=353,
    ID_OSD_TEXT_EDIT_TOPIC=354,
    ID_OSD_TEXT_EDIT=355,
    ID_PLAYBACK_TOPIC=356,
    ID_DECODER_MODE=357,
    ID_FRAME_REPEAT_FREQUENCY=358,
    ID_FRAME_REPEAT_STRENGTH=359,
    ID_GROUND_LOOP_TOPIC=360,
    ID_GROUND_LOOP=361,
    ID_TRACK_LAYER_MOTION_TOPIC=362,
    ID_TRACK_LAYER2_ANGLE=363,
    ID_TRACK_LAYER2_MOTION_X=364,
    ID_TRACK_LAYER2_MOTION_Y=365,
    ID_TRACK_LAYER3_ANGLE=366,
    ID_TRACK_LAYER3_MOTION_X=367,
    ID_TRACK_LAYER3_MOTION_Y=368,
    ID_TRACK_LAYER4_ANGLE=369,
    ID_TRACK_LAYER4_MOTION_X=370,
    ID_TRACK_LAYER4_MOTION_Y=371,
    ID_SYNC_DAMAGE_TOPIC=372,
    ID_SYNC_DAMAGE_ENABLE=373,
    ID_SYNC_DAMAGE_TEAR_AMOUNT=374,
    ID_SYNC_DAMAGE_BAND_INTENSITY=375,
    ID_SYNC_DAMAGE_BAND_DENSITY=376,
    ID_SYNC_DAMAGE_BAND_HEIGHT=377,
    ID_SYNC_DAMAGE_BAND_IRREGULARITY=378,
    ID_SYNC_DAMAGE_DISPLACEMENT=379,
    ID_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION=380,
    ID_SYNC_DAMAGE_COVERAGE=381,
    ID_SYNC_DAMAGE_LUMA_ENABLE=382,
    ID_SYNC_DAMAGE_LUMA_INSTABILITY=383,
    ID_SYNC_DAMAGE_BRIGHT_FLASH=384,
    ID_SYNC_DAMAGE_DARK_COLLAPSE=385,
    ID_SYNC_DAMAGE_EVENT_FREQUENCY=386,
    ID_SYNC_DAMAGE_EVENT_DURATION=387,
    ID_SYNC_DAMAGE_BRIGHT_PROBABILITY=411,
    ID_SYNC_DAMAGE_DARK_PROBABILITY=412,
    ID_SYNC_DAMAGE_TRANSITION_SHARPNESS=388,
    ID_SYNC_DAMAGE_LOCAL_LUMA=389,
    ID_SYNC_DAMAGE_MANUAL_BRIGHT=390,
    ID_SYNC_DAMAGE_MANUAL_DARK=391,
    ID_SYNC_DAMAGE_DROPOUTS_ENABLE=392,
    ID_SYNC_DAMAGE_DROPOUT_DENSITY=393,
    ID_SYNC_DAMAGE_STREAK_LENGTH=394,
    ID_SYNC_DAMAGE_STREAK_BRIGHTNESS=395,
    ID_SYNC_DAMAGE_DARK_DROPOUT=396,
    ID_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION=397,
    ID_SYNC_DAMAGE_TEMPORAL_VARIATION=398,
    ID_SYNC_DAMAGE_CHROMA_ENABLE=399,
    ID_SYNC_DAMAGE_CHROMA_TINT=400,
    ID_SYNC_DAMAGE_MAGENTA_TINT=401,
    ID_SYNC_DAMAGE_GREEN_TINT=402,
    ID_SYNC_DAMAGE_RGB_OFFSET=403,
    ID_SYNC_DAMAGE_CHROMA_BLEED=404,
    ID_SYNC_DAMAGE_ANIMATION_MODE=405,
    ID_SYNC_DAMAGE_INPUT_MODE=406,
    ID_SYNC_DAMAGE_ANIMATION_SPEED=407,
    ID_SYNC_DAMAGE_RANDOM_SEED=408,
    ID_SYNC_DAMAGE_BLEND_INPUT=409,
    ID_SYNC_DAMAGE_EFFECT_AMOUNT=410,
};
constexpr A_long kInputCheckoutID=1;
static char kTrackingShapeChoices[]="Horizontal Band|Vertical Band|Rectangle";
static char kTrackingProfileChoices[]="Servo Sweep|RF Wave|Composite Ribbon|Tape Flutter|Four-Edge Crawl";
static char kTrackingPresetChoices[]="Custom|Fine Sweep|Fragmented Sweep|RF Snowline|Composite Flagging|Tape Flutter|Four-Edge Crawl|Full-Frame Breakdown";
static char kTearChromaModeChoices[]="Spectral Chroma Smear|Localized Headbleed|Chroma Streaks & Interleave|Multi-Band Tear|Spectral + Lower Tear";
static char kStaticPatternChoices[]="Dark-Field Speckles|Full-Frame Horizontal|Top Strip|Bottom-Weighted Streaks|Monochrome Streaks|Split-Band Chromatic|Static to Blackout|Dark-Field Luma Streaks|Multi-Phase Green Line|Two-Stage Angular Ribbon|Diagonal Sync Collapse|Colored Speckles|Vertical Columns";
static char kSyncDamageAnimationChoices[]="Automatic Procedural|Manual / Keyframed|Frozen";
static char kSyncDamageInputChoices[]="Apply Over Original|Generated Signal";
static char kColorSystemChoices[]="NTSC|PAL";
static char kTapeSpeedChoices[]="SP|LP|EP";
static char kVhsStyleChoices[]="Custom|VHS6RIFT - Clean VHS|VHS6RIFT - Vintage VHS|VHS6RIFT - Heavy VHS|VHS6RIFT - Bad Tracking|VHS6RIFT - Very Bad Tracking|VHS6RIFT - Unstable VCR|VHS6RIFT - Tape Damage|VHS6RIFT - Static Interference|VHS6RIFT - Color Bleed|VHS6RIFT - Color Shift|VHS6RIFT - Signal Loss|VHS6RIFT - Severe Analog Glitch|VHS Add-on - Soft Transfer|VHS Add-on - Classic Transfer|Lost Signal - Reference Analog|Crates VHS Master - Archive Standard|Crates VHS Master - Violet Transfer|Crates VHS Master - Signal Corruption|Crates VHS Master - Handheld Camcorder|VHS Pro - Neutral Playback|VHS Pro - Fresh Cassette|VHS Pro - Aged Cassette|VHS Pro - Tracking Failure|VHS Pro - Home Video Camera|VHS Pro - Chroma Bleed|VHS Pro - Multi-Generation Dub|VHS Craft - Clean|VHS Craft - Sharp|VHS Craft - Violet Glow|VHS Craft - Green Glow|VHS Craft - Tape Monochrome|VHS6RIFT - Moving Band Noise|VHS6RIFT - Bottom Tape Damage|VHS6RIFT - Ghosting|VHS6RIFT - Damaged Tape Playback|VHS Add-on - Heavy|VHS Add-on - Worn|VHS Add-on - Clean TV|CVS - VHS SP|CVS - VHS LP|CVS - VHS EP|LS - Real VHS|LS - Backrooms|LS - Poolrooms|LS - Liminal Night|LS - Subtle|LS - Found Footage|LS - Damaged Tape|LS - Rental Tape|LS - Camcorder 1996|LS - Broken Signal|LS - Composite Decode|LS - Analog Fog|Crates - Clean|Crates - Sharp|Crates - Soft|Crates - Green|Crates - Streaks|Crates - Cool|Crates - Warm|Crates - Blue Bloom|Crates - Echo|Crates - Retro|Crates - Hue Wobble|Crates - Black & White|VHSPro - Rental Copy|VHSPro - Broadcast|VHSPro - Music Video|VHSCraft - Standard|VHSCraft - Damaged Tape|VHSCraft - Camcorder";
static char kCameraProfileChoices[]="Off|JVC GR-C7U|JVC GR-AX220U|JVC GR-AX920U|JVC GR-SXM37US|JVC GR-SXM740U|Sony CCD-TR55|JVC GR-S55|Panasonic NV-M1|RCA CMR300|FXC6 Direct Capture|Gen 7 Reference";
static char kRgbChannelModeChoices[]="RGB Split|Red/Cyan|Green/Magenta|Blue/Yellow";
static char kNtscRsLookChoices[]="Custom|NTSCRT Natural Composite|NTSCRT Home SP|NTSCRT LP Tape|NTSCRT EP Tape|NTSCRT Tracking Snow|NTSCRT Chroma Loss|NTSCRT Edge Wave|NTSCRT Field Blend|NTSCRT Composite Smear|NTSCRT Chroma Instability";
static char kSignalModelChoices[]="Raster NTSC|Full Waveform NTSC";
static char kCompositeDecoderChoices[]="Consumer Notch|One-Line Comb|Two-Line Comb";
static char kOsdDeckModeChoices[]="Play|Pause|Stop|Fast Forward|Rewind";
static char kOsdDateFormatChoices[]="ISO Date + Time|US Date + Time|European Date + Time|Date Only|Time Only";
static char kOsdCounterFormatChoices[]="Six-Digit Frames|HH:MM:SS|HH:MM:SS:FF";
static char kOsdLanguageChoices[]="English|Arabic|English + Arabic";
static char kOsdDigitsChoices[]="Latin|Arabic-Indic|Persian";
static char kOsdDateLayoutChoices[]="JAN.21.1994|1.21.1994|21.1.1994|1994.1.21";
static char kOsdClockModeChoices[]="Off|12 Hour|24 Hour";
static char kOsdMonthStyleChoices[]="Short|Full";
static char kOsdFontChoices[]="A Massir Ballpoint|A Naskh Tahrir|a-bad-khat|Guesswhat Exceptional|Arabic Childhood|Arial Narrow Bold|Evogria|K Kamran|Maqroo Regular|Milan Display Black|Nasma|Real Vhs Font Regular|RooyinFree Bold|YR Sponge Arabic|ZIAN Black";
static char kOsdExtraLabelChoices[]="Off|TAPE|ERROR|P01|RES|FORWARD|EJECT|SEARCH|TRACKING|AUTO|CH|CAM|DECK|TIMER|STEREO|SLOW|STILL|MEMORY";

struct RenderSettings {
    float master=100, composite=0, sync=0, staticAmount=0, staticDensity=35, staticAngle=24, staticRibbonWidth=12, staticGreenLine=75, staticPhaseCount=3;
    A_long syncDamageEnable=0, syncDamageLumaEnable=1, syncDamageDropoutsEnable=1, syncDamageChromaEnable=1, syncDamageAnimationMode=1, syncDamageInputMode=1;
    float syncDamageTearAmount=85, syncDamageBandIntensity=85, syncDamageBandDensity=7, syncDamageBandHeight=55, syncDamageBandIrregularity=82, syncDamageDisplacement=22, syncDamageBoundaryFragmentation=62, syncDamageCoverage=100;
    float syncDamageLumaInstability=90, syncDamageBrightFlash=85, syncDamageDarkCollapse=85, syncDamageEventFrequency=1.7f, syncDamageEventDuration=0.30f, syncDamageBrightProbability=50, syncDamageDarkProbability=50, syncDamageTransitionSharpness=90, syncDamageLocalLuma=70, syncDamageManualBright=0, syncDamageManualDark=0;
    float syncDamageDropoutDensity=65, syncDamageStreakLength=18, syncDamageStreakBrightness=75, syncDamageDarkDropout=70, syncDamageHorizontalFragmentation=65, syncDamageTemporalVariation=85;
    float syncDamageChromaTint=20, syncDamageMagentaTint=20, syncDamageGreenTint=20, syncDamageRgbOffset=0.35f, syncDamageChromaBleed=18, syncDamageAnimationSpeed=100, syncDamageRandomSeed=1, syncDamageBlendInput=80, syncDamageEffectAmount=100;
    A_long staticMode=2;
    A_long colorSystem=1, signalModel=1, tapeSpeed=1, compositeDecoder=1, vhs43Frame=0;
    float roundedOverscan=0.0f, lensDistortion=0.0f, digitalZoom=1.0f, osdBattery=0.0f;
    float frameRepeatFrequency=0.0f, frameRepeatStrength=0.0f;
    float tapeDetail=100.0f, trackRead=0.0f, trackMisalignment=0.0f, chromaShift=0.0f, redChannelOffset=0.0f, greenChannelOffset=0.0f, blueChannelOffset=0.0f; A_long chromaRegMode=1;
    float chromaSubsample=0, verticalChroma=0, verticalLuma=0;
    float lumaNoise=0, chromaNoise=0, compositeNoise=0, noiseHue=0;
    float rfDrift=0, rfHum=0, rfAm=0, agcPump=0, impulseNoise=0, dropouts=0;
    float multipathGain=0, multipathDelay=24, multipathEnsemble=0, signalGhost=0, chromaDelay=0, chromaLoss=0;
    float afcHunt=0, chromaFlutter=0, ycCrosstalk=0, colorPhaseDrift=0, colorDrift=0;
    float hsyncNoise=0, vsyncNoise=0, burstNoise=0;
    float edgeEnhance=0, chromaEdgeEnhance=0, tapeFade=0, blackLift=0, temporalGhost=0, ringing=0, ringingPower=2, crtEqualizer=0, gradeExposure=0, gradeContrast=100, gradeBrightness=0, gradeSaturation=100, chromaIGain=100, chromaQGain=100, chromaIOffset=0, chromaQOffset=0, chromaQuantization=0, vignette=0, flicker=0;
    float lumaSmear=0, bloom=0, analogGlow=0;
    float tapeSurface=0, tapeTransport=0, geometry=0, interlace=0, verticalRoll=0, edgeWave=0, scanlines=0;
    float trackingAmount=0, trackingX=50, trackingY=50, trackingWidth=100, trackingHeight=8, trackingAngle=0;
    float trackingMotionX=0, trackingMotionY=0, trackingServo=4, trackingTear=3;
    A_long trackingShape=3, trackingProfile=1, trackingPresetBase=1, trackingLayerPreset[3]={3,4,5};
    A_long trackingLayerProfile[3]={2,3,4}, trackingLayerShape[3]={1,1,1};
    float trackingLayerAmount[3]={0,0,0}, trackingLayerX[3]={50,50,50}, trackingLayerY[3]={25,50,75}, trackingLayerWidth[3]={100,100,100}, trackingLayerHeight[3]={8,8,8}, trackingLayerServo[3]={8,12,5}, trackingLayerShift[3]={24,36,12}, trackingLayerWave[3]={4,8,2}, trackingLayerTear[3]={10,25,5}, trackingLayerAngle[3]={0,0,0}, trackingLayerMotionX[3]={0,0,0}, trackingLayerMotionY[3]={0,0,0};
    
    A_long tearChromaMode=5;
    float tearBandCount=2.0f;
    float tearChromaAmount=0, spectralSmear=75, spectralHueSpread=70, chromaSaturation=140, spectralCenterY=38, spectralCoverage=72, tearBandStartY=60, tearBandHeight=18, tearDisplacement=18, tearFragmentation=55, tearChromaShift=5, tearSmear=6, tearDropout=12, tearSeed=1;
    float tearBandAmount[4]={70,60,80,55}, tearBandY[4]={75,45,90,30}, tearBandHeightExtra[4]={15,12,8,10}, tearBandShift[4]={30,-24,40,-18}, tearBandFragmentation[4]={60,70,45,65}, tearBandChroma[4]={8,-6,12,-10}, tearBandSmear[4]={8,12,5,10}, tearBandDropout[4]={20,15,25,10};
    float headAmount=0, headY=100, headHeight=4, headShift=24;
    float headCondition=0;
    float tapeWear=100, tapeDropouts=100, tapeScratches=100, tapeCreases=100, tapeEdge=100, tapeMold=100, tapeVaseline=100, tapeSticky=100, tapePinholes=0, tapeFlakes=0, tapeRoughness=0, tapeSeed=1, backTension=0;
    float tapeNoise=0;
    float magnetMoment=0.0f, magnetDistance=6.0f, groundLoop=0.0f, mainsHum=0.0f, rfField=0.0f, rfPickup=0.0f;
        float osdAmount=0.0f, osdRec=100.0f, osdPlay=100.0f, osdTimestamp=100.0f, osdOpacity=82.0f, osdScale=100.0f, osdGlitch=50.0f, osdGhost=35.0f, osdRecBlink=1.4f, osdTimeSpeed=1.0f;
    float osdYear=1994.0f, osdMonth=6.0f, osdDay=11.0f, osdHour=12.0f, osdMinute=0.0f, osdSecond=0.0f;
    A_long osdDeckMode=1, osdDateFormat=1;
    A_long osdLanguage=1, osdDigits=1, osdDateLayout=1, osdClockMode=3, osdMonthStyle=1, osdClockRuns=1, osdFont=13, osdExtraLabel=1;
    float osdExtraX=50.0f, osdExtraY=50.0f;
    A_long osdShowText=1, osdShowRecLabel=1, osdShowPlayLabel=1, osdShowPauseLabel=1, osdShowStopLabel=1, osdShowFfLabel=1, osdShowRewLabel=1, osdShowSpeedLabel=1, osdShowTapeLabel=0, osdShowErrorLabel=0, osdShowProgramLabel=0, osdShowResLabel=0, osdShowForwardLabel=0, osdTextGlow=0, osdShowDate=1;
    float osdWordsX=50.0f, osdWordsY=50.0f, osdTextSize=100.0f, osdTextGlowAmount=50.0f, osdTextSeed=1.0f, osdTextOpacity=100.0f;
    float osdRecX=2.2f, osdRecY=3.0f, osdStatusX=98.0f, osdStatusY=3.0f, osdTimestampX=98.0f, osdTimestampY=96.0f;
    A_long osdCounterFormat=1, osdShowCounter=0;
    float osdCounterAmount=0.0f, osdCounterX=2.2f, osdCounterY=96.0f, osdCounterOffset=0.0f;
    std::string osdText;
    float vhs6RiftGrain=0.0f, vhs6RiftGrainSize=2.0f, vhs6RiftNoiseDetail=50.0f, vhs6RiftShadowBias=65.0f, vhs6RiftNoiseSeed=1.0f;
    bool vhs6RiftNoiseEnable=true;
    float signalHueNoise=0.0f;
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

static float masterDrive(float value) noexcept { return std::clamp(value/100.0f,0.0f,3.0f); }
static float masterMix(float value) noexcept { return std::clamp(value/100.0f,0.0f,3.0f); }

static float blendPreset(float current,float target,float intensity) noexcept {
    const float t=clamp01(intensity/100.0f);
    return current+(target-current)*t;
}
static void ApplyTrackingRecipe(DamageRegion& r,A_long preset) noexcept {
    switch(preset) {
        case 2: // Fine Sweep
            r.maxShiftPx*=0.65f; r.waveAmplitudePx*=0.65f; r.tearProbability*=0.45f;
            r.noiseDensity*=0.65f; r.chromaShiftPx*=0.6f; break;
        case 3: // Fragmented Sweep
            r.maxShiftPx*=1.25f; r.waveAmplitudePx*=1.2f; r.tearProbability=clamp01(r.tearProbability*1.7f);
            r.tearShiftPx*=1.4f; r.noiseDensity*=1.25f; break;
        case 4: // RF Snowline
            r.noiseDensity=0.62f; r.noiseAmount=0.22f; r.waveAmplitudePx*=1.5f;
            r.servoSmoothnessPx*=1.6f; r.chromaShiftPx*=0.5f; break;
        case 5: // Composite Flagging
            r.maxShiftPx*=1.6f; r.waveAmplitudePx*=1.8f; r.tearProbability=clamp01(r.tearProbability*1.3f);
            r.tearShiftPx*=1.5f; r.chromaShiftPx*=2.0f; break;
        case 6: // Tape Flutter
            r.servoDriftPx*=0.55f; r.servoSmoothnessPx*=2.4f; r.waveFrequency*=0.65f;
            r.tearProbability*=0.4f; break;
        case 7: // Four-Edge Crawl
            r.waveAmplitudePx*=1.8f; r.noiseDensity=0.4f; r.tearProbability*=0.7f;
            r.chromaShiftPx*=1.4f; break;
        case 8: // Full-Frame Breakdown recipe; geometry remains editable by the user.
            r.maxShiftPx*=1.8f; r.noiseDensity=0.8f; r.noiseAmount=0.28f;
            r.tearProbability=std::max(r.tearProbability,0.5f); break;
        default: break;
    }
}
static void ApplyCratesMasterPreset(RenderSettings& s,int preset) noexcept {
    // Exact selected source fields recovered from the supplied CratesVHSMaster
    // AEX metadata (44 of its 66 controls). Unrepresented source-only controls
    // remain in the audit manifest; mappings below adapt them to VHS-90's DSP.
    static constexpr float kCrates[16][44]={
        {50,50,0,100,100,10,5,50,50,100,30,24,50,100,50,30,50,100,100,-5,0,0,5,19,50,294,22,40,100,50,540.2f,70,7,-10,10,100,15,11,56,75,75,75,100,100},
        {5,1,0,200,100,20,5,50,50,100,30,0,50,100,50,30,50,100,100,-5,0,0,5,19,351,141,3,40,100,50,540.2f,70,7,-10,5,100,2,167,56,78.3f,100,100,100,100},
        {5,1,0,30,100,20,0,74,50,100,6,0,50,100,50,30,50,100,100,-5,0,0,5,19,351,141,3,40,100,50,540.2f,44,7,-10,5,100,5,200,56,69.3f,72,183,100,100},
        {50,50,0,100,100,10,12,100,68,110,6,6,50,100,100,30,92,38,100,173,-12,0,5,17,5,1049,3,40,100,50,270.3f,50,14,0,10,100,8,4,346,100,100,100,100,84},
        {25,10,0,150,100,50,5,50,50,100,0,39,50,100,50,30,100,50,100,-5,0,0,5,19,351,141,3,40,100,97,540.2f,70,7,-10,10,100,15,11,56,50,197,43,100,100},
        {50,50,0,100,100,40,15,45,159,100,30,67,50,13,93,30,50,100,100,50,0,0,5,215,351,141,3,40,124,78,540.2f,70,7,-10,10,100,15,11,56,70,100,50,90,130},
        {50,50,0,543,100,10,5,50,268,100,30,39,50,100,200,30,205,100,100,-5,0,0,5,94,351,141,3,40,100,50,270.3f,70,7,-10,10,100,42,11,56,100,100,100,100,100},
        {50,50,0,100,100,10,5,50,50,100,30,12,50,100,50,30,11,9,532,20,87,50,5,100,59,53,3,40,5,100,270.3f,70,7,-10,10,100,15,11,56,100,100,100,100,100},
        {50,50,0,100,100,10,8,100,50,100,17,22,6.9f,797,50,30,50,100,1519,30,-12,100,5,19,133,92,3,40,100,50,540.2f,70,7,-10,10,100,15,11,56,61,100,100,100,100},
        {50,50,0,150,100,20,15,100,100,100,17,22,6.9f,797,50,30,122,30,1519,30,-12,100,5,19,133,92,3,40,100,50,270.3f,70,7,-10,10,100,15,11,56,100,100,100,90,100},
        {5,1,0,200,100,25,12,100,150,100,150,0,50,100,50,30,50,100,100,22,0,0,5,19,351,141,3,40,100,50,540.2f,70,7,-10,5,100,10,130,137,91,100,183,96,100},
        {50,50,0,100,100,-49,0,86,103,100,31,8,50,100,92,30,50,100,100,108,-13.8f,-19,5,19,351,141,19,40,1836,46,270.3f,70,213,-10,10,100,42,11,56,100,100,100,100,100},
        {50,50,0,100,100,-79,-11,77,50,100,30,39,18,100,67,122,50,100,100,-5,0,0,5,19,39,141,3,40,100,50,540.2f,70,7,-10,10,100,15,11,56,100,100,100,100,100},
        {50,54,0,200,100,20,-9,50,50,100,14,0,50,100,79,30,50,100,100,40,38,0,5,19,21,329,3,40,14,50,540.2f,70,7,-10,5,100,15,23,56,90,143,150,104,117},
        {50,50,0,200,100,36,4,49,50,100,30,20,13,100,50,30,50,100,17,772,0,0,44,14,21,597,3,40,22,50,540.2f,43,7,-10,33,100,15,11,56,75,75,75,100,100},
        {50,50,0,100,100,3,0,0,50,100,30,39,50,100,78,110,15,100,100,13,0,0,5,40,25,319,3,40,100,80,540.2f,70,7,-10,10,100,9,52,56,75,75,75,100,138}
    };
    if(preset<0||preset>=16)return;
    const float* v=kCrates[preset];
    auto b=[&](float& dst,float value){dst=blendPreset(dst,value,s.vhsStyleIntensity);};
    const float noise=v[0],distortion=v[1];
    b(s.composite,std::clamp(noise*0.65f+distortion*0.35f,0.0f,100.0f));
    b(s.lumaNoise,noise);b(s.chromaNoise,noise*0.8f);b(s.compositeNoise,noise);
    b(s.lumaSmear,std::max(v[2],v[31]));
    b(s.chromaSubsample,std::clamp((200.0f-v[3])*0.5f,0.0f,100.0f));
    b(s.bloom,std::clamp(v[4]*v[5]/100.0f,0.0f,100.0f));
    b(s.analogGlow,std::clamp(v[6]*1.5f,0.0f,100.0f));
    b(s.gradeSaturation,v[7]*2.0f);
    b(s.staticAmount,std::clamp(v[8]*0.5f,0.0f,100.0f));b(s.staticDensity,v[9]);
    b(s.tapeNoise,v[10]);
    b(s.headHeight,std::max(v[11],v[27]));b(s.headAmount,std::max(v[12],v[26]));
    b(s.headShift,std::clamp(v[13]*0.125f,0.0f,100.0f));
    b(s.impulseNoise,std::clamp((v[14]+v[15])*0.5f,0.0f,100.0f));
    
    b(s.trackingAmount,std::clamp(v[18],0.0f,100.0f));
    b(s.trackingServo,std::clamp(4.0f+std::fabs(v[19])*0.1f,0.0f,100.0f));
    b(s.trackingHeight,std::clamp(std::fabs(v[20]),0.0f,100.0f));b(s.chromaLoss,v[21]);
    b(s.geometry,std::clamp(v[22]+v[23]*0.3f,0.0f,100.0f));
    b(s.edgeWave,std::clamp(v[24]/3.51f,0.0f,100.0f));
    b(s.trackingWidth,std::clamp(v[25]/15.0f,0.0f,100.0f));
    b(s.trackingTear,std::clamp(v[28]/18.36f,0.0f,100.0f));
    b(s.verticalRoll,v[29]);b(s.scanlines,std::clamp(v[30]/5.402f,0.0f,100.0f));
    b(s.multipathGain,std::clamp(v[32]/2.13f,0.0f,100.0f));b(s.chromaDelay,v[33]);
    b(s.colorPhaseDrift,v[34]);b(s.chromaFlutter,std::clamp(v[35]*0.1f,0.0f,100.0f));
    b(s.rfDrift,v[36]);b(s.rfField,v[37]);b(s.rfHum,std::clamp(v[38]/3.46f,0.0f,100.0f));
    b(s.ringing,std::clamp(v[40]*0.5f,0.0f,100.0f));b(s.edgeEnhance,std::clamp(v[41]*0.5f,0.0f,100.0f));
    b(s.gradeBrightness,v[42]-100.0f);b(s.gradeContrast,v[43]);
}
static void ApplyVhsStyleRecipe(RenderSettings& s,int styleRecipe) noexcept {
    const float i=clamp01(s.vhsStyleIntensity/100.0f);
    if(styleRecipe<=1||i<=0.0f)return;
    auto b=[&](float& v,float target){v=blendPreset(v,target,s.vhsStyleIntensity);};
    auto bi=[&](A_long& v,float target){v=static_cast<A_long>(std::lround(blendPreset(float(v),target,s.vhsStyleIntensity)));};
    // Lost Signal VHS v1.9: 13 source profiles. These settings are translated
    // to matching VHS-90 controls; Minecraft-only depth fog, camera matrices,
    // and depth-reprojected history are not fabricated for an AE RGB input.
    static constexpr float kLostSignalProfiles[13][45]={
        {0.10f,0.22f,0.08f,7.0f,1.2f,0.04f,0.35f,0.35f,0.65f,0.04f,0.70f,0.20f,4.0f,0.020f,0.8f,1.0f,4.0f,0.18f,0.08f,4.0f,12.0f,0.55f,0.42f,1.55f,0.40f,0.50f,0.52f,0,0,0.00f,0.92f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,0,0, 0.040f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.10f,0.22f,0.12f,16.0f,1.8f,0.08f,0.35f,0.35f,0.65f,0.08f,0.70f,0.20f,4.0f,0.020f,0.8f,1.0f,4.0f,0.18f,0.08f,4.0f,9.0f,0.45f,0.42f,1.35f,0.55f,0.35f,0.65f,0,0,0.00f,0.92f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,0,0, 0.040f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.06f,0.12f,0.08f,7.0f,1.2f,0.04f,0.20f,0.50f,0.50f,0.02f,0.50f,0.15f,2.5f,0.035f,0.8f,1.0f,2.5f,0.13f,0.05f,3.0f,9.0f,0.25f,0.20f,0.80f,0.30f,0.50f,0.52f,0,0,0.00f,0.85f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,1,2, 0.060f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.03f,0.12f,0.04f,4.0f,0.8f,0.04f,0.10f,0.35f,0.25f,0.02f,0.25f,0.10f,1.5f,0.010f,0.4f,0.5f,1.5f,0.08f,0.05f,2.0f,4.0f,0.10f,0.10f,0.80f,0.15f,0.70f,0.32f,0,0,0.00f,0.85f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,2,3, 0.025f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.10f,0.22f,0.08f,7.0f,1.2f,0.08f,0.20f,0.70f,0.50f,0.04f,0.70f,0.20f,2.5f,0.035f,0.8f,1.0f,2.5f,0.18f,0.08f,4.0f,6.0f,0.25f,0.55f,0.60f,0.30f,0.20f,0.80f,0,0,0.00f,0.92f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,3,4, 0.040f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.03f,0.12f,0.04f,4.0f,0.4f,0.04f,0.10f,0.15f,0.25f,0.02f,0.25f,0.05f,1.0f,0.010f,0.4f,0.5f,1.0f,0.08f,0.03f,1.0f,2.0f,0.10f,0.10f,1.00f,0.15f,0.10f,0.32f,0,0,0.00f,0.50f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,0,0, 0.015f, 0.0f, 0.0f, 1.0f, 0.00f, 0.00f},
        {0.10f,0.22f,0.16f,10.0f,1.8f,0.14f,0.50f,0.50f,0.80f,0.08f,0.85f,0.30f,2.5f,0.035f,1.2f,1.5f,2.5f,0.13f,0.08f,3.0f,6.0f,0.35f,0.30f,1.15f,0.40f,0.35f,0.52f,0,0,0.00f,0.85f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,0,0, 0.025f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.20f,0.48f,0.35f,24.0f,4.0f,0.35f,0.70f,0.70f,1.00f,0.14f,1.00f,0.45f,6.0f,0.080f,2.6f,2.5f,6.0f,0.25f,0.12f,7.0f,16.0f,0.70f,0.55f,1.55f,0.90f,0.50f,0.80f,0,0,0.00f,1.00f,0.0f,0.00f,0.00f,0.15f,0.25f,0.0f,0,0, 0.060f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.10f,0.22f,0.12f,16.0f,1.8f,0.08f,0.35f,0.35f,0.65f,0.08f,0.70f,0.20f,4.0f,0.020f,0.8f,1.0f,4.0f,0.18f,0.08f,4.0f,9.0f,0.45f,0.42f,1.35f,0.55f,0.35f,0.65f,4,4,0.70f,0.92f,0.4f,0.20f,0.20f,0.15f,0.25f,0.0f,0,0, 0.040f, 1.0f, 1.0f, 1.0f, 0.04f, 0.50f},
        {0.10f,0.22f,0.16f,10.0f,1.8f,0.14f,0.50f,0.50f,0.80f,0.08f,0.85f,0.30f,2.5f,0.035f,1.2f,1.5f,2.5f,0.13f,0.08f,3.0f,6.0f,0.35f,0.30f,1.15f,0.40f,0.35f,0.52f,3,1,0.10f,0.85f,0.0f,0.10f,0.00f,0.15f,0.25f,1.0f,0,0, 0.025f, 1.0f, 1.0f, 1.1f, 0.02f, 0.25f},
        {0.20f,0.48f,0.35f,24.0f,4.0f,0.35f,0.70f,0.70f,1.00f,0.14f,1.00f,0.45f,6.0f,0.080f,2.6f,2.5f,6.0f,0.25f,0.12f,7.0f,16.0f,0.70f,0.55f,1.55f,0.90f,0.50f,0.80f,4,5,1.00f,1.00f,0.7f,0.70f,0.70f,0.15f,0.25f,0.0f,0,0, 0.060f, 1.0f, 1.0f, 1.0f, 0.22f, 0.85f},
        {0.10f,0.22f,0.12f,16.0f,1.8f,0.08f,0.35f,0.35f,0.65f,0.08f,0.70f,0.20f,4.0f,0.020f,0.8f,1.0f,4.0f,0.18f,0.08f,4.0f,9.0f,0.45f,0.42f,1.35f,0.55f,0.35f,0.65f,0,0,0.00f,0.92f,0.0f,0.00f,0.00f,0.20f,0.35f,0.0f,0,0, 0.040f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f},
        {0.10f,0.22f,0.12f,16.0f,1.8f,0.08f,0.35f,0.35f,0.65f,0.08f,0.70f,0.20f,4.0f,0.020f,0.8f,1.0f,4.0f,0.18f,0.08f,4.0f,9.0f,0.45f,0.42f,1.35f,0.55f,0.35f,0.65f,0,0,0.00f,0.92f,0.0f,0.00f,0.00f,0.20f,0.35f,0.0f,0,1, 0.040f, 1.0f, 1.0f, 1.0f, 0.00f, 0.00f}
    };
    auto lostSignal=[&](int index){
        const float* v=kLostSignalProfiles[index];
        b(s.composite,100);b(s.lumaNoise,v[0]*100);b(s.chromaNoise,v[0]*85);b(s.tapeNoise,v[0]*100);
        b(s.scanlines,v[1]*100);b(s.trackingAmount,v[2]*200);b(s.trackingTear,v[3]/36.0f*100.0f);
        b(s.trackingServo,v[4]/6.0f*100.0f);b(s.staticAmount,v[5]/0.5f*100.0f);
        b(s.colorPhaseDrift,v[6]*100);b(s.agcPump,v[7]*100);b(s.chromaLoss,v[8]*100);
        b(s.verticalRoll,v[9]/0.35f*100.0f);b(s.sync,v[10]*100);b(s.chromaDelay,v[11]*100);
        b(s.chromaShift,v[12]/9.0f*100.0f);b(s.flicker,v[13]*100);
        b(s.geometry,std::max(v[14]/4.0f*100.0f,v[15]/6.0f*100.0f));
        b(s.lumaSmear,v[16]/6.0f*100.0f);b(s.temporalGhost,v[17]*100);
        b(s.signalGhost,v[18]/0.25f*100.0f);b(s.tapeDetail,100.0f-v[19]*8.0f);
        b(s.chromaSubsample,v[20]/24.0f*100.0f);b(s.ringing,v[21]*100.0f);
        b(s.gradeContrast,100.0f+v[22]*25.0f);b(s.gradeBrightness,-v[22]*8.0f);
        b(s.gradeSaturation,v[23]*100.0f);b(s.headAmount,v[24]*100.0f);
        b(s.headHeight,v[24]*50.0f);b(s.analogGlow,v[25]*100.0f);b(s.vignette,v[26]*100.0f);
        const int format=int(v[27]+0.5f);bi(s.tapeSpeed,format==1?2:(format==2||format==4?3:1));
        b(s.tapeSurface,std::max(v[29],std::max(v[34],v[35]))*100.0f);b(s.tapeWear,v[29]*100.0f);
        b(s.interlace,v[30]*100.0f);b(s.trackingServo,4.0f+std::fabs(v[31])*60.0f);
        b(s.rfDrift,v[32]*100.0f);b(s.rfField,v[32]*100.0f);b(s.tapeTransport,v[33]*100.0f);
        b(s.tapeDropouts,v[34]*100.0f);b(s.dropouts,v[35]*100.0f);
        b(s.osdAmount,v[36]>0.5f?82.0f:0.0f);b(s.osdRec,v[36]>0.5f?100.0f:0.0f);
        b(s.osdPlay,v[36]>0.5f?100.0f:0.0f);b(s.osdTimestamp,v[36]>0.5f?100.0f:0.0f);b(s.osdBattery,v[36]>0.5f?75.0f:0.0f);b(s.frameRepeatFrequency,v[43]*100.0f);b(s.frameRepeatStrength,v[44]*100.0f);
        if(v[37]>0.5f){b(s.colorDrift,v[37]==1.0f?28.0f:(v[37]==2.0f?18.0f:42.0f));}
        if(v[38]>0.5f){b(s.analogGlow,std::max(s.analogGlow,28.0f));}
        b(s.lensDistortion,v[39]*1000.0f);bi(s.vhs43Frame,v[40]>0.5f?1.0f:0.0f);
        b(s.roundedOverscan,v[41]*100.0f);b(s.digitalZoom,v[42]);
    };
    switch(styleRecipe) {
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
    // VHS Effect Add-on's five source presets, translated into the existing VHS-90
    // signal/tape controls. The source is a Python RGB/YCbCr processor; this is a
    // parameter-level adaptation, not a byte-for-byte renderer clone.
    case 18: // light
        b(s.chromaSubsample,25);b(s.chromaShift,1);b(s.lumaNoise,30);b(s.chromaNoise,20);
        b(s.tapeNoise,25);b(s.scanlines,22);b(s.vignette,18);b(s.geometry,15);
        b(s.verticalRoll,10);b(s.headAmount,28);b(s.headHeight,3.5f);b(s.trackingAmount,18);
        b(s.dropouts,10);b(s.tapeDetail,70);b(s.bloom,20);b(s.edgeEnhance,15);
        b(s.gradeSaturation,110);b(s.gradeContrast,103);b(s.osdAmount,82);break;
    case 19: // classic
        b(s.chromaSubsample,60);b(s.chromaShift,3);b(s.lumaNoise,75);b(s.chromaNoise,55);
        b(s.tapeNoise,60);b(s.scanlines,50);b(s.vignette,40);b(s.geometry,45);
        b(s.verticalRoll,25);b(s.headAmount,70);b(s.headHeight,3.5f);b(s.trackingAmount,55);
        b(s.dropouts,35);b(s.tapeDetail,40);b(s.bloom,45);b(s.edgeEnhance,25);
        b(s.gradeSaturation,122);b(s.gradeContrast,105);b(s.osdAmount,82);break;
    case 20: // heavy
        b(s.chromaSubsample,95);b(s.chromaShift,5);b(s.lumaNoise,100);b(s.chromaNoise,95);
        b(s.tapeNoise,90);b(s.scanlines,72);b(s.vignette,60);b(s.geometry,80);
        b(s.verticalRoll,40);b(s.headAmount,100);b(s.headHeight,3.5f);b(s.trackingAmount,95);
        b(s.dropouts,65);b(s.tapeDetail,8);b(s.bloom,70);b(s.edgeEnhance,35);
        b(s.gradeSaturation,132);b(s.gradeContrast,110);b(s.osdAmount,82);b(s.osdGlitch,90);break;
    case 21: // worn
        b(s.chromaSubsample,100);b(s.chromaShift,7);b(s.lumaNoise,100);b(s.chromaNoise,100);
        b(s.tapeNoise,100);b(s.scanlines,85);b(s.vignette,85);b(s.geometry,95);
        b(s.verticalRoll,60);b(s.headAmount,100);b(s.headHeight,6);b(s.trackingAmount,100);
        b(s.dropouts,100);b(s.tapeDetail,6);b(s.bloom,85);b(s.edgeEnhance,30);
        b(s.gradeSaturation,125);b(s.gradeContrast,108);b(s.gradeBrightness,-3);
        b(s.osdAmount,82);b(s.osdGlitch,90);break;
    case 22: // clean_tv
        b(s.chromaSubsample,35);b(s.chromaShift,1);b(s.lumaNoise,25);b(s.chromaNoise,15);
        b(s.tapeNoise,20);b(s.scanlines,35);b(s.vignette,25);b(s.geometry,15);
        b(s.verticalRoll,0);b(s.headAmount,20);b(s.headHeight,3.5f);b(s.trackingAmount,10);
        b(s.dropouts,0);b(s.tapeDetail,65);b(s.bloom,25);b(s.edgeEnhance,0);
        b(s.gradeSaturation,115);b(s.gradeContrast,105);b(s.osdAmount,82);break;
    case 23: // Composite Video Simulator VHS SP mode
        b(s.composite,100);b(s.verticalChroma,50);b(s.edgeEnhance,100);b(s.chromaEdgeEnhance,85);bi(s.tapeSpeed,1);break;
    case 24: // Composite Video Simulator VHS LP mode
        b(s.composite,100);b(s.verticalChroma,50);b(s.edgeEnhance,100);b(s.chromaEdgeEnhance,85);bi(s.tapeSpeed,2);break;
    case 25: // Composite Video Simulator VHS EP mode
        b(s.composite,100);b(s.verticalChroma,50);b(s.edgeEnhance,100);b(s.chromaEdgeEnhance,85);bi(s.tapeSpeed,3);break;
    case 26: lostSignal(0);break;
    case 27: lostSignal(1);break;
    case 28: lostSignal(2);break;
    case 29: lostSignal(3);break;
    case 30: lostSignal(4);break;
    case 31: lostSignal(5);break;
    case 32: lostSignal(6);break;
    case 33: lostSignal(7);break;
    case 34: lostSignal(8);break;
    case 35: lostSignal(9);break;
    case 36: lostSignal(10);break;
    case 37: lostSignal(11);break;
    case 38: lostSignal(12);break;
    // Crates VHS Master: all 16 named metadata looks are represented as native
    // VHS-90 recipes. The original embedded JSON was parsed in the audit, but
    // its per-preset payload is not checked into this repository; these are
    // source-informed parameter translations, not claimed byte-exact values.
    case 39: case 40: case 41: case 42: case 43: case 44: case 45: case 46:
    case 47: case 48: case 49: case 50: case 51: case 52: case 53: case 54:
        ApplyCratesMasterPreset(s,static_cast<int>(styleRecipe-39));break;
    // VHSPro: ten recovered menu looks. The supplied decompilation preserves
    // labels but not reliable full slider tuples, so these native mappings
    // reproduce each look's intent and remain explicitly source-inspired.
    case 55: // Neutral Playback
        b(s.composite,42);b(s.tapeDetail,92);b(s.chromaSubsample,14);b(s.lumaNoise,4);b(s.chromaNoise,4);b(s.tapeNoise,5);break;
    case 56: // Fresh Cassette
        b(s.composite,28);b(s.tapeDetail,100);b(s.tapeWear,0);b(s.tapeSurface,0);b(s.tapeDropouts,0);b(s.tapeScratches,0);b(s.tapeCreases,0);b(s.trackingAmount,0);b(s.headAmount,0);break;
    case 57: // Rental Cassette
        b(s.composite,52);b(s.tapeDetail,82);b(s.tapeWear,24);b(s.tapeSurface,18);b(s.tapeNoise,22);b(s.chromaSubsample,22);b(s.lumaNoise,12);b(s.trackingAmount,10);break;
    case 58: // Aged Cassette
        b(s.composite,72);b(s.tapeDetail,58);b(s.tapeWear,82);b(s.tapeSurface,76);b(s.tapeDropouts,58);b(s.tapeScratches,54);b(s.tapeCreases,48);b(s.tapeNoise,62);b(s.lumaNoise,42);break;
    case 59: // Tracking Failure
        b(s.trackingAmount,100);b(s.trackingHeight,32);b(s.trackingServo,88);b(s.trackingTear,92);b(s.sync,55);b(s.rfDrift,45);b(s.staticAmount,48);b(s.dropouts,45);break;
    case 60: // Home Video Camera
        b(s.composite,48);b(s.tapeDetail,72);b(s.chromaSubsample,30);b(s.chromaShift,4);b(s.colorDrift,22);b(s.lumaNoise,16);b(s.chromaNoise,18);b(s.geometry,12);b(s.osdAmount,62);break;
    case 61: // Chroma Bleed
        b(s.composite,68);b(s.chromaSubsample,58);b(s.verticalChroma,62);b(s.chromaShift,12);b(s.chromaDelay,16);b(s.ycCrosstalk,55);b(s.gradeSaturation,118);break;
    case 62: // Broadcast Deck
        b(s.composite,35);b(s.tapeDetail,108);b(s.edgeEnhance,28);b(s.chromaSubsample,10);b(s.lumaNoise,2);b(s.chromaNoise,3);b(s.tapeNoise,4);b(s.gradeContrast,104);break;
    case 63: // Multi-Generation Dub
        b(s.composite,76);b(s.tapeDetail,54);b(s.chromaSubsample,42);b(s.chromaShift,8);b(s.lumaSmear,38);b(s.temporalGhost,38);b(s.signalGhost,48);b(s.tapeNoise,46);b(s.lumaNoise,32);b(s.chromaNoise,30);break;
    case 64: // Music Video Grade
        b(s.composite,40);b(s.tapeDetail,86);b(s.gradeSaturation,138);b(s.gradeContrast,116);b(s.bloom,48);b(s.analogGlow,34);b(s.chromaShift,4);b(s.colorDrift,18);break;
    // VHSCraft: all eight recovered named looks, adapted to the shared engine.
    case 65: // Craft Neutral
        b(s.composite,48);b(s.tapeDetail,88);b(s.chromaSubsample,18);b(s.lumaNoise,8);b(s.chromaNoise,8);b(s.tapeNoise,10);break;
    case 66: // Craft Clean
        b(s.composite,20);b(s.tapeDetail,100);b(s.chromaSubsample,6);b(s.lumaNoise,1);b(s.chromaNoise,1);b(s.tapeNoise,1);b(s.trackingAmount,0);b(s.headAmount,0);break;
    case 67: // Craft Sharp
        b(s.tapeDetail,118);b(s.edgeEnhance,48);b(s.chromaEdgeEnhance,32);b(s.lumaSmear,0);b(s.ringing,8);break;
    case 68: // Violet Glow
        b(s.gradeSaturation,118);b(s.chromaIGain,108);b(s.chromaQGain,126);b(s.chromaIOffset,-10);b(s.chromaQOffset,22);b(s.bloom,24);b(s.analogGlow,18);break;
    case 69: // Green Glow
        b(s.gradeSaturation,102);b(s.chromaIGain,94);b(s.chromaQGain,112);b(s.chromaIOffset,18);b(s.chromaQOffset,-12);b(s.bloom,18);break;
    case 70: // Worn Cassette
        b(s.composite,82);b(s.tapeDetail,48);b(s.tapeWear,86);b(s.tapeSurface,82);b(s.tapeDropouts,64);b(s.tapeScratches,72);b(s.tapeCreases,58);b(s.tapeNoise,70);b(s.trackingAmount,42);break;
    case 71: // Handheld Cinema
        b(s.composite,48);b(s.tapeDetail,70);b(s.chromaSubsample,34);b(s.chromaShift,5);b(s.geometry,18);b(s.flicker,16);b(s.gradeSaturation,112);b(s.osdAmount,48);break;
    case 72: // Tape Monochrome
        b(s.gradeSaturation,0);b(s.chromaLoss,100);b(s.tapeDetail,82);b(s.lumaNoise,18);b(s.tapeNoise,22);b(s.scanlines,28);break;
    default:break;
    }
    if(styleRecipe<26){bi(s.vhs43Frame,0.0f);b(s.roundedOverscan,0.0f);b(s.lensDistortion,0.0f);b(s.digitalZoom,1.0f);b(s.osdBattery,0.0f);b(s.frameRepeatFrequency,0.0f);b(s.frameRepeatStrength,0.0f);}
}

static constexpr int kLegacyVhsStyleRecipeMap[33]={
    0,1,2,3,4,5,6,7,9,11,12,13,15,16,18,19,26,39,42,45,52,55,56,58,59,60,61,63,66,67,68,69,72
};
static constexpr int kExtendedVhsStyleRecipeMap[41]={
    0,8,10,14,17,20,21,22,23,24,25,27,28,29,30,31,32,33,34,35,36,37,38,40,41,43,44,46,47,48,49,50,51,53,54,57,62,64,65,70,71
};
static void ApplyVhs6RiftStylePreset(RenderSettings& s) noexcept {
    // The original first 32 popup values are preserved for saved projects.
    // Values 33–72 append all 40 source-derived recipes previously omitted.
    const int selected=std::clamp<int>(s.vhsStyle,1,72);
    const int styleRecipe=selected<=32?kLegacyVhsStyleRecipeMap[selected]:kExtendedVhsStyleRecipeMap[selected-32];
    ApplyVhsStyleRecipe(s,styleRecipe);
}
static void ApplyVhs6RiftCameraProfile(RenderSettings& s) noexcept {
    const float i=clamp01(s.cameraIntensity/100.0f);
    if(s.cameraProfile<=1||i<=0.0f)return;
    auto b=[&](float& v,float target){v=blendPreset(v,target,s.cameraIntensity);};
    switch(s.cameraProfile) {
    case 2: // JVC GR-C7U — early VHS-C, warm chroma, soft luma and fine cassette grain.
        b(s.tapeDetail,70);b(s.composite,36);b(s.chromaSubsample,24);b(s.verticalChroma,18);b(s.chromaShift,3.5f);
        b(s.gradeContrast,102);b(s.gradeBrightness,2);b(s.gradeSaturation,108);b(s.chromaIGain,108);b(s.chromaQGain,102);
        b(s.chromaIOffset,2);b(s.chromaQOffset,-2);b(s.lumaNoise,7);b(s.chromaNoise,12);b(s.compositeNoise,5);
        b(s.vhs6RiftGrain,18);b(s.vhs6RiftNoiseDetail,55);b(s.vhs6RiftShadowBias,65);b(s.ringing,8);break;
    case 3: // JVC GR-AX220U — early-90s VHS-C, slightly cool color and steadier playback.
        b(s.tapeDetail,78);b(s.composite,30);b(s.chromaSubsample,20);b(s.verticalChroma,12);b(s.chromaShift,2);
        b(s.gradeContrast,100);b(s.gradeBrightness,1);b(s.gradeSaturation,104);b(s.chromaIGain,105);b(s.chromaQGain,108);
        b(s.chromaIOffset,-1);b(s.chromaQOffset,2);b(s.lumaNoise,6);b(s.chromaNoise,10);b(s.compositeNoise,4);
        b(s.vhs6RiftGrain,14);b(s.vhs6RiftNoiseDetail,58);b(s.vhs6RiftShadowBias,60);break;
    case 4: // JVC GR-AX920U — cleaner consumer VHS-C with firmer edges and restrained noise.
        b(s.tapeDetail,82);b(s.composite,25);b(s.chromaSubsample,14);b(s.verticalChroma,10);b(s.chromaShift,1.5f);
        b(s.edgeEnhance,14);b(s.gradeContrast,105);b(s.gradeSaturation,108);b(s.chromaIGain,108);b(s.chromaQGain,104);
        b(s.lumaNoise,4);b(s.chromaNoise,6);b(s.compositeNoise,2);b(s.vhs6RiftGrain,10);b(s.vhs6RiftNoiseDetail,55);
        b(s.vhs6RiftShadowBias,50);b(s.ringing,12);break;
    case 5: // JVC GR-SXM37US — S-VHS-C detail, wider luma response and clean chroma edges.
        b(s.tapeDetail,92);b(s.composite,18);b(s.chromaSubsample,8);b(s.verticalChroma,6);b(s.chromaShift,0.8f);
        b(s.edgeEnhance,22);b(s.gradeContrast,106);b(s.gradeSaturation,112);b(s.chromaIGain,102);b(s.chromaQGain,100);
        b(s.lumaNoise,3);b(s.chromaNoise,4);b(s.compositeNoise,1);b(s.vhs6RiftGrain,8);b(s.vhs6RiftNoiseDetail,60);
        b(s.vhs6RiftShadowBias,45);b(s.ringing,18);b(s.bloom,6);break;
    case 6: // JVC GR-SXM740U — higher-end S-VHS-C, crisp luma, stable color and subtle optics.
        b(s.tapeDetail,96);b(s.composite,15);b(s.chromaSubsample,6);b(s.verticalChroma,4);b(s.chromaShift,0.5f);
        b(s.edgeEnhance,26);b(s.gradeContrast,108);b(s.gradeBrightness,1);b(s.gradeSaturation,115);
        b(s.lumaNoise,2);b(s.chromaNoise,3);b(s.compositeNoise,1);b(s.vhs6RiftGrain,6);b(s.ringing,20);
        b(s.bloom,8);b(s.vignette,3);b(s.flicker,1);break;
    case 7: // Sony CCD-TR55 — Video8 look, warm highlights, softer chroma and visible fine grain.
        b(s.tapeDetail,74);b(s.composite,42);b(s.chromaSubsample,28);b(s.verticalChroma,22);b(s.chromaShift,4);
        b(s.gradeContrast,96);b(s.gradeBrightness,4);b(s.gradeSaturation,116);b(s.chromaIGain,112);b(s.chromaQGain,106);
        b(s.chromaIOffset,3);b(s.chromaQOffset,-2);b(s.lumaNoise,12);b(s.chromaNoise,16);b(s.compositeNoise,8);
        b(s.vhs6RiftGrain,24);b(s.vhs6RiftNoiseDetail,70);b(s.vhs6RiftShadowBias,70);b(s.analogGlow,7);b(s.flicker,3);
        b(s.colorPhaseDrift,6);break;
    case 8: // JVC GR-S55 — older compact VHS-C, softer detail, stronger tape texture and warm chroma.
        b(s.tapeDetail,68);b(s.composite,48);b(s.chromaSubsample,34);b(s.verticalChroma,28);b(s.chromaShift,5);
        b(s.gradeContrast,94);b(s.gradeBrightness,-1);b(s.gradeSaturation,102);b(s.chromaIGain,106);b(s.chromaQGain,100);
        b(s.lumaNoise,16);b(s.chromaNoise,18);b(s.compositeNoise,10);b(s.vhs6RiftGrain,28);b(s.vhs6RiftNoiseDetail,70);
        b(s.vhs6RiftShadowBias,75);b(s.ringing,6);b(s.analogGlow,3);break;
    case 9: // Panasonic NV-M1 — early VHS-C color, warm saturation and mild RF instability.
        b(s.tapeDetail,62);b(s.composite,52);b(s.chromaSubsample,36);b(s.verticalChroma,32);b(s.chromaShift,5.5f);
        b(s.gradeContrast,102);b(s.gradeBrightness,2);b(s.gradeSaturation,125);b(s.chromaIGain,118);b(s.chromaQGain,110);
        b(s.chromaIOffset,2);b(s.chromaQOffset,4);b(s.lumaNoise,14);b(s.chromaNoise,22);b(s.compositeNoise,12);
        b(s.lumaSmear,12);b(s.rfDrift,5);b(s.colorPhaseDrift,8);b(s.vhs6RiftGrain,24);b(s.vhs6RiftShadowBias,72);break;
    case 10: // RCA CMR300 — 90s consumer camcorder, faded color, visible luma noise and mild instability.
        b(s.tapeDetail,70);b(s.composite,38);b(s.chromaSubsample,26);b(s.verticalChroma,20);b(s.chromaShift,3.5f);
        b(s.gradeContrast,92);b(s.gradeBrightness,3);b(s.gradeSaturation,92);b(s.chromaIGain,95);b(s.chromaQGain,98);
        b(s.chromaIOffset,-2);b(s.chromaQOffset,1);b(s.lumaNoise,18);b(s.chromaNoise,20);b(s.compositeNoise,12);
        b(s.rfDrift,8);b(s.colorPhaseDrift,10);b(s.flicker,4);b(s.vhs6RiftGrain,22);b(s.vhs6RiftShadowBias,78);break;
    case 11: // FXC6 Direct Capture — cool blue/cyan cast, fine grain and softened detail; no severe tracking bars.
        b(s.tapeDetail,70);b(s.chromaSubsample,18);b(s.chromaShift,2.5f);
        b(s.gradeContrast,94);b(s.gradeBrightness,2);b(s.gradeSaturation,112);
        b(s.chromaIGain,105);b(s.chromaQGain,108);b(s.chromaIOffset,-3);b(s.chromaQOffset,5);
        b(s.lumaNoise,8);b(s.chromaNoise,10);b(s.compositeNoise,6);
        b(s.vhs6RiftGrain,20);b(s.vhs6RiftNoiseDetail,65);b(s.vhs6RiftShadowBias,70);
        break;
    case 12: // Gen 7 Reference — teal/yellow camcorder color, stronger chroma instability and intermittent signal damage.
        b(s.tapeDetail,50);b(s.chromaSubsample,32);b(s.chromaShift,6);
        b(s.gradeContrast,108);b(s.gradeBrightness,-3);b(s.gradeSaturation,132);
        b(s.chromaIGain,130);b(s.chromaQGain,115);b(s.chromaIOffset,-6);b(s.chromaQOffset,8);
        b(s.verticalChroma,25);b(s.verticalLuma,14);b(s.lumaNoise,18);b(s.chromaNoise,24);b(s.compositeNoise,20);
        b(s.rfDrift,16);b(s.colorPhaseDrift,18);b(s.chromaFlutter,14);
        b(s.vhs6RiftGrain,28);b(s.vhs6RiftNoiseDetail,80);b(s.vhs6RiftShadowBias,75);
        b(s.staticAmount,22);b(s.staticDensity,70);b(s.trackingAmount,20);
        s.trackingProfile=2;s.trackingPresetBase=4;s.staticMode=9;
        b(s.tearChromaAmount,15);s.tearChromaMode=5;s.tearBandCount=3;
        break;
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
    out_data->out_flags=PF_OutFlag_DEEP_COLOR_AWARE|PF_OutFlag_CUSTOM_UI;
    // Use the proven VHS6RIFT-style PF_Cmd_RENDER path. SmartFX was causing
    // black output on host re-renders; do not advertise Smart Render until its
    // complete host integration has dedicated AE runtime coverage.
    out_data->out_flags2=PF_OutFlag2_SUPPORTS_THREADED_RENDERING;
    return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData* in_data,PF_OutData* out_data) {
    PF_Err err=PF_Err_NONE;
    PF_ParamDef def;
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS Style & Presets",ID_VHS_STYLE_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("VHS Style Preset",72,1,kVhsStyleChoices,ID_VHS_STYLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Style Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS_STYLE_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Camera Profile Preset",12,1,kCameraProfileChoices,ID_CAMERA_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Camera Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_CAMERA_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_VHS_STYLE_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Master Strength",0,300,0,300,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_MASTER);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Signal",ID_SIGNAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Composite Signal",0,100,0,100,28,1,PF_ValueDisplayFlag_PERCENT,0,ID_COMPOSITE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Sync Collapse",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("TV System",2,1,kColorSystemChoices,ID_PAL_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Signal Renderer",2,1,kSignalModelChoices,ID_SIGNAL_MODEL);


    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Adjacent Track Crosstalk",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_READ);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Track Misalignment",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_MISALIGNMENT);










    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RF Hum",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_HUM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AM Nonlinearity",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RF_AM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AGC Pump",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_AGC_PUMP);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Impulse Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_IMPULSE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Signal Dropouts",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_DROPOUTS);
    AEFX_CLR_STRUCT(def); def.ui_flags |= PF_PUI_NO_ECW_UI; PF_ADD_FLOAT_SLIDERX("RF Ghost Gain",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MULTIPATH_GAIN);
    AEFX_CLR_STRUCT(def); def.ui_flags |= PF_PUI_NO_ECW_UI; PF_ADD_FLOAT_SLIDERX("RF Ghost Delay",1,200,1,200,24,1,PF_ValueDisplayFlag_NONE,0,ID_MULTIPATH_DELAY);
    AEFX_CLR_STRUCT(def); def.ui_flags |= PF_PUI_NO_ECW_UI; PF_ADD_FLOAT_SLIDERX("RF Ghost Ensemble",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MULTIPATH_ENSEMBLE);
    AEFX_CLR_STRUCT(def); def.ui_flags |= PF_PUI_NO_ECW_UI; PF_ADD_FLOAT_SLIDERX("Signal Ghosting",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SIGNAL_GHOST);


    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("AFC Hunt",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_AFC_HUNT);




    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("H Sync Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_HSYNC_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("V Sync Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VSYNC_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Burst Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BURST_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Edge Enhancement",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_EDGE_ENHANCE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Fade",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_FADE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Black Lift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BLACK_LIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Previous Frame Ghost",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEMPORAL_GHOST);












    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Geometry Warp",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_GEOMETRY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Interlace / Field Weave",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_INTERLACE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Roll",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_ROLL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_EDGE_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Frequency",0.005f,0.2f,0.005f,0.2f,0.05f,3,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_FREQUENCY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Speed",0,20,0,20,4,2,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("VHS Edge Wave Detail",1,5,1,5,2,0,PF_ValueDisplayFlag_NONE,0,ID_EDGE_WAVE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Scanlines",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_SCANLINES);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Signal Looks",ID_NTSCRS_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Signal Look",11,1,kNtscRsLookChoices,ID_NTSCRS_LOOK);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Look Intensity",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_NTSCRS_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Scanline Chroma Phase Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_NTSCRS_PHASE_NOISE);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_NTSCRS_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_SIGNAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Ringing",ID_RINGING_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ringing",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_RINGING);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ringing Power",0,8,0,8,2,1,PF_ValueDisplayFlag_NONE,0,ID_RINGING_POWER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("CRT 3-Band EQ",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CRT_EQUALIZER);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_RINGING_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Glow",ID_GLOW_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Smear",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_LUMA_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Bloom",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_BLOOM);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Analog Glow",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_ANALOG_GLOW);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_GLOW_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("General Adjustments",ID_GENERAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Exposure (EV)",-3,3,-3,3,0,2,PF_ValueDisplayFlag_NONE,0,ID_GRADE_EXPOSURE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Contrast",0,200,0,200,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_CONTRAST);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Brightness",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_GRADE_BRIGHTNESS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grade Saturation",0,500,0,500,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_SATURATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vignette",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VIGNETTE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Flicker",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_FLICKER);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_GENERAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Chroma",ID_CHROMA_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",0,50,0,50,1.5f,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Red Channel Registration",-20,20,-20,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_REG_RED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Green Channel Registration",-20,20,-20,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_REG_GREEN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Blue Channel Registration",-20,20,-20,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_REG_BLUE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("RGB Channel Mode",4,1,kRgbChannelModeChoices,ID_CHROMA_REG_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Subsample / Bleed",0,100,0,100,16,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_SUBSAMPLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Chroma Delay",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Vertical Luma Blur",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VERTICAL_LUMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Delay",0,24,0,24,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_DELAY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Loss",0,5000,0,5000,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_LOSS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Flutter",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_FLUTTER);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Y/C Crosstalk",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_YC_CROSSTALK);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Color Phase Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COLOR_PHASE_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Color Drift",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COLOR_DRIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma I Gain",0,300,0,300,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_I_GAIN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Q Gain",0,300,0,300,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_Q_GAIN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma I Offset",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_GRADE_I_OFFSET);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Q Offset",-100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_GRADE_Q_OFFSET);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Quantization",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_CHROMA_QUANTIZATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Edge Enhancement",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_EDGE_ENHANCE);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_CHROMA_TOPIC+100);

    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Waves / Static",ID_STATIC_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Static Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_STATIC_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Static Density",0,100,0,100,35,1,PF_ValueDisplayFlag_PERCENT,0,ID_STATIC_DENSITY);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_STATIC_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Grain",ID_GRAIN_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_LUMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Composite Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_COMPOSITE_NOISE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Noise Hue Bias",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_NOISE_HUE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grain Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_GRAIN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Grain Size",1,5,1,5,2,0,PF_ValueDisplayFlag_NONE,0,ID_VHS6RIFT_GRAIN_SIZE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Detail",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_NOISE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Shadow Bias",0,100,0,100,65,1,PF_ValueDisplayFlag_PERCENT,0,ID_VHS6RIFT_SHADOW_BIAS);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Grain", "Enable", 1, 0, ID_VHS6RIFT_NOISE_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_VHS6RIFT_NOISE_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Hue Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SIGNAL_HUE_NOISE);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_GRAIN_TOPIC+100);
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
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tape",ID_TAPE_MATERIAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tape Speed",3,1,kTapeSpeedChoices,ID_TAPE_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Detail",6,100,6,100,84,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_DETAIL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Material Master",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_SURFACE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Transport",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_TRANSPORT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Wear",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_WEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Dropouts",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_DROPOUTS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Scratches",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_SCRATCHES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Creases / Folds",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_CREASES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Edge Damage",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_EDGE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Mold",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_MOLD);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Vaseline / Binder Film",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_VASELINE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Sticky Binder / Shed",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_STICKY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Cassette Seed",1,10000,1,10000,1,1,PF_ValueDisplayFlag_NONE,0,ID_TAPE_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Back Tension", -100,100,-100,100,0,1,PF_ValueDisplayFlag_NONE,0,ID_TAPE_BACK_TENSION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tape Noise (Multi-Scale)",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_NOISE);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TAPE_MATERIAL_TOPIC+100);

    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Mains Hum",ID_PHYSICAL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Mains Hum",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_MAINS_HUM);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_PHYSICAL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Camcorder OSD",ID_OSD_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("OSD Mix / Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_AMOUNT);
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


    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tracking Type",5,1,kTrackingProfileChoices,ID_TRACKING_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tear & Chroma",ID_TEAR_CHROMA_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear & Chroma Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_CHROMA_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tear & Chroma Mode",5,5,kTearChromaModeChoices,ID_TEAR_CHROMA_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Spectral Smear",0,100,0,100,75,1,PF_ValueDisplayFlag_PERCENT,0,ID_SPECTRAL_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Spectral Hue Spread",0,100,0,100,70,1,PF_ValueDisplayFlag_PERCENT,0,ID_SPECTRAL_HUE_SPREAD);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Saturation",0,300,0,300,140,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_SATURATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Spectral Center Y",0,100,0,100,38,1,PF_ValueDisplayFlag_PERCENT,0,ID_SPECTRAL_CENTER_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Spectral Coverage",0,100,0,100,72,1,PF_ValueDisplayFlag_PERCENT,0,ID_SPECTRAL_COVERAGE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Band Start Y",0,100,0,100,60,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND_START_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Band Height",0,100,0,100,18,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Band Count",1,5,1,5,2,0,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND_COUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Displacement",-200,200,-200,200,18,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_DISPLACEMENT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Fragmentation",0,100,0,100,55,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",-100,100,-100,100,5,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_CHROMA_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Smear",0,100,0,100,6,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout Probability",0,100,0,100,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_TEAR_SEED);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TEAR_CHROMA_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Layer 2",ID_TRACK_LAYER2_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tracking Type",5,1,kTrackingProfileChoices,ID_TRACK_LAYER2_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Region Shape",3,1,kTrackingShapeChoices,ID_TRACK_LAYER2_SHAPE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center X",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,25,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Width",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_WIDTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Servo Drift",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_SERVO);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Displacement",0,200,0,200,24,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER2_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Wave Intensity",0,100,0,100,4,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Probability",0,100,0,100,10,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_TEAR);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACK_LAYER2_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Layer 3",ID_TRACK_LAYER3_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tracking Type",5,2,kTrackingProfileChoices,ID_TRACK_LAYER3_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Region Shape",3,1,kTrackingShapeChoices,ID_TRACK_LAYER3_SHAPE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center X",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Width",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_WIDTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Servo Drift",0,100,0,100,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_SERVO);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Displacement",0,200,0,200,36,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER3_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Wave Intensity",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Probability",0,100,0,100,25,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_TEAR);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACK_LAYER3_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Layer 4",ID_TRACK_LAYER4_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Tracking Type",5,3,kTrackingProfileChoices,ID_TRACK_LAYER4_PROFILE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Region Shape",3,1,kTrackingShapeChoices,ID_TRACK_LAYER4_SHAPE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center X",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,75,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Width",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_WIDTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Servo Drift",0,100,0,100,5,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_SERVO);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Displacement",0,200,0,200,12,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER4_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Wave Intensity",0,100,0,100,2,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_WAVE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Tear Probability",0,100,0,100,5,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_TEAR);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACK_LAYER4_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Presets",ID_TRACKING_PRESETS_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Base Tracking Preset",8,1,kTrackingPresetChoices,ID_TRACKING_PRESET_BASE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Layer 2 Preset",8,3,kTrackingPresetChoices,ID_TRACKING_PRESET_2);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Layer 3 Preset",8,4,kTrackingPresetChoices,ID_TRACKING_PRESET_3);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Layer 4 Preset",8,5,kTrackingPresetChoices,ID_TRACKING_PRESET_4);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACKING_PRESETS_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Procedural Static Pattern",ID_PATTERN_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Pattern Type",13,2,kStaticPatternChoices,ID_PATTERN_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Pattern Angle",-90,90,-90,90,24,1,PF_ValueDisplayFlag_NONE,0,ID_PATTERN_ANGLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ribbon / Diagonal Width",1,50,1,50,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_PATTERN_WIDTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Green Line Intensity",0,100,0,100,75,1,PF_ValueDisplayFlag_PERCENT,0,ID_PATTERN_GREEN);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Phase Count",2,8,2,8,3,0,PF_ValueDisplayFlag_NONE,0,ID_PATTERN_PHASE_COUNT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_PATTERN_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tear Band 2",ID_TEAR_BAND2_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Amount",0,100,0,100,70,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,75,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,15,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Shift",-200,200,-200,200,30,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND2_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Fragmentation",0,100,0,100,60,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",-100,100,-100,100,8,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND2_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Smear",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout",0,100,0,100,20,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND2_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TEAR_BAND2_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tear Band 3",ID_TEAR_BAND3_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Amount",0,100,0,100,60,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,45,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Shift",-200,200,-200,200,-24,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND3_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Fragmentation",0,100,0,100,70,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",-100,100,-100,100,-6,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND3_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Smear",0,100,0,100,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout",0,100,0,100,15,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND3_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TEAR_BAND3_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tear Band 4",ID_TEAR_BAND4_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Amount",0,100,0,100,80,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,90,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,8,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Shift",-200,200,-200,200,40,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND4_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Fragmentation",0,100,0,100,45,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",-100,100,-100,100,12,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND4_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Smear",0,100,0,100,5,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout",0,100,0,100,25,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND4_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TEAR_BAND4_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tear Band 5",ID_TEAR_BAND5_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Amount",0,100,0,100,55,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Center Y",0,100,0,100,30,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,10,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Shift",-200,200,-200,200,-18,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND5_SHIFT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Fragmentation",0,100,0,100,65,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Shift",-100,100,-100,100,-10,1,PF_ValueDisplayFlag_NONE,0,ID_TEAR_BAND5_CHROMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Smear",0,100,0,100,10,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_SMEAR);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout",0,100,0,100,10,1,PF_ValueDisplayFlag_PERCENT,0,ID_TEAR_BAND5_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TEAR_BAND5_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS Tape OSD Layout",ID_OSD_LAYOUT_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Deck Transport",5,1,kOsdDeckModeChoices,ID_OSD_DECK_MODE);
    AEFX_CLR_STRUCT(def); def.ui_flags=PF_PUI_NO_ECW_UI; PF_ADD_POPUP("Legacy Timestamp Format",5,1,kOsdDateFormatChoices,ID_OSD_DATE_FORMAT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("REC Horizontal Position",0,100,0,100,2.2,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_REC_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("REC Vertical Position",0,100,0,100,3,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_REC_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Deck Status Horizontal Position",0,100,0,100,98,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_STATUS_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Deck Status Vertical Position",0,100,0,100,3,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_STATUS_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Timestamp Horizontal Position",0,100,0,100,98,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TIMESTAMP_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Timestamp Vertical Position",0,100,0,100,96,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TIMESTAMP_Y);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_LAYOUT_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tape Counter",ID_OSD_COUNTER_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Counter Format",3,1,kOsdCounterFormatChoices,ID_OSD_COUNTER_FORMAT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Counter Amount",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_COUNTER_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Counter Horizontal Position",0,100,0,100,2.2,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_COUNTER_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Counter Vertical Position",0,100,0,100,96,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_COUNTER_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Counter Offset (seconds)",-3600,3600,-3600,3600,0,1,PF_ValueDisplayFlag_NONE,0,ID_OSD_COUNTER_OFFSET);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Show Counter", "Enable", 0, 0, ID_OSD_SHOW_COUNTER);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_COUNTER_TOPIC+100);
    PF_ArbitraryH osdTextDefault = nullptr;
    err = VHS90CreateOsdTextDefault(in_data, &osdTextDefault);
    if (err != PF_Err_NONE) return err;
    AEFX_CLR_STRUCT(def); PF_ADD_ARBITRARY2("OSD Text Data",1,1,0,PF_PUI_NO_ECW_UI,osdTextDefault,ID_OSD_TEXT_DATA,nullptr);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS Text Localization",ID_OSD_LOCALIZATION_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("OSD Language",3,1,kOsdLanguageChoices,ID_OSD_LANGUAGE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("OSD Digits",3,1,kOsdDigitsChoices,ID_OSD_DIGITS);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("VHS Date Layout",4,1,kOsdDateLayoutChoices,ID_OSD_DATE_LAYOUT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Clock",3,3,kOsdClockModeChoices,ID_OSD_CLOCK_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Month Names",2,1,kOsdMonthStyleChoices,ID_OSD_MONTH_STYLE);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Clock Runs", "Enable", 1, 0, ID_OSD_CLOCK_RUNS);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("OSD Font Family",15,13,kOsdFontChoices,ID_OSD_FONT);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Extra VHS Label",18,1,kOsdExtraLabelChoices,ID_OSD_EXTRA_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Extra Label Horizontal Position",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_EXTRA_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Extra Label Vertical Position",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_EXTRA_Y);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_LOCALIZATION_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("VHS Text Words",ID_OSD_WORDS_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Show Text", "Enable", 1, 0, ID_OSD_SHOW_TEXT);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("REC Label", "Show", 1, 0, ID_OSD_SHOW_REC_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("PLAY", "Show", 1, 0, ID_OSD_SHOW_PLAY_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("PAUSE", "Show", 1, 0, ID_OSD_SHOW_PAUSE_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("STOP", "Show", 1, 0, ID_OSD_SHOW_STOP_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("FF", "Show", 1, 0, ID_OSD_SHOW_FF_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("REW", "Show", 1, 0, ID_OSD_SHOW_REW_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("SP / LP / EP", "Show", 1, 0, ID_OSD_SHOW_SPEED_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("TAPE", "Show", 0, 0, ID_OSD_SHOW_TAPE_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("ERROR", "Show", 0, 0, ID_OSD_SHOW_ERROR_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("P.O.1 / P01", "Show", 0, 0, ID_OSD_SHOW_PROGRAM_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("RES", "Show", 0, 0, ID_OSD_SHOW_RES_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("FORWARD", "Show", 0, 0, ID_OSD_SHOW_FORWARD_LABEL);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Text Glow", "Enable", 0, 0, ID_OSD_TEXT_GLOW);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Size (%)",10,500,10,500,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TEXT_SIZE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Glow Amount",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TEXT_GLOW_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_OSD_TEXT_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Text Opacity (%)",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_TEXT_OPACITY);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Show Date", "Enable", 1, 0, ID_OSD_SHOW_DATE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Word Group Horizontal Position",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_WORDS_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Word Group Vertical Position",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_OSD_WORDS_Y);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_WORDS_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tape Surface Detail",ID_TAPE_SURFACE_DETAIL_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Pinholes",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_PINHOLES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Oxide Flakes",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_FLAKES);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Surface Roughness",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_ROUGHNESS);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TAPE_SURFACE_DETAIL_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("OSD Text Input",ID_OSD_TEXT_EDIT_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_BUTTON("OSD Text Editor","Edit Text...",0,PF_ParamFlag_SUPERVISE,ID_OSD_TEXT_EDIT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_OSD_TEXT_EDIT_TOPIC+100);
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Playback & Decoder",ID_PLAYBACK_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Raster Decoder",3,1,kCompositeDecoderChoices,ID_DECODER_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Frame Repeat Frequency",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_FRAME_REPEAT_FREQUENCY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Frame Repeat Strength",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_FRAME_REPEAT_STRENGTH);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_PLAYBACK_TOPIC+100);

    // Appended to preserve all existing serialized AE parameter indices.
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Ground Loop",ID_GROUND_LOOP_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Ground Loop (mV)",0,500,0,500,0,1,PF_ValueDisplayFlag_NONE,0,ID_GROUND_LOOP);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_GROUND_LOOP_TOPIC+100);

    // Extra layers get the same region direction and motion controls as the primary layer.
    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Tracking Layer Direction / Motion",ID_TRACK_LAYER_MOTION_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 2 Angle",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER2_ANGLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 2 Motion X",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_MOTION_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 2 Motion Y",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER2_MOTION_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 3 Angle",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER3_ANGLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 3 Motion X",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_MOTION_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 3 Motion Y",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER3_MOTION_Y);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 4 Angle",-180,180,-180,180,0,1,PF_ValueDisplayFlag_NONE,0,ID_TRACK_LAYER4_ANGLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 4 Motion X",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_MOTION_X);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Layer 4 Motion Y",-100,100,-100,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACK_LAYER4_MOTION_Y);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_TRACK_LAYER_MOTION_TOPIC+100);

    AEFX_CLR_STRUCT(def); PF_ADD_TOPIC("Sync Collapse & Luma Dropout",ID_SYNC_DAMAGE_TOPIC);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Sync Collapse","Enable",0,0,ID_SYNC_DAMAGE_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Tear Amount",0,100,0,100,85,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_TEAR_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Intensity",0,100,0,100,85,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BAND_INTENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Density",1,32,1,32,7,0,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_BAND_DENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Height",1,100,1,100,55,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BAND_HEIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Band Irregularity",0,100,0,100,82,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BAND_IRREGULARITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Displacement (px)",0,160,0,160,22,1,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_DISPLACEMENT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Boundary Fragmentation",0,100,0,100,62,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Full-Frame Coverage",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_COVERAGE);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Luma Instability","Enable",1,0,ID_SYNC_DAMAGE_LUMA_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Luma Instability",0,100,0,100,90,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_LUMA_INSTABILITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Bright Flash Intensity",0,100,0,100,85,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BRIGHT_FLASH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dark Collapse Intensity",0,100,0,100,85,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_DARK_COLLAPSE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Event Frequency (Hz)",0.1,10,0.1,10,1.7,1,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_EVENT_FREQUENCY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Event Duration (s)",0.05,2,0.05,2,0.30,2,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_EVENT_DURATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Bright Event Probability",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BRIGHT_PROBABILITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dark Event Probability",0,100,0,100,50,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_DARK_PROBABILITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Transition Sharpness",0,100,0,100,90,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_TRANSITION_SHARPNESS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Local Band Luma Variation",0,100,0,100,70,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_LOCAL_LUMA);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Manual Bright Pulse",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_MANUAL_BRIGHT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Manual Dark Collapse",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_MANUAL_DARK);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Fine Dropouts","Enable",1,0,ID_SYNC_DAMAGE_DROPOUTS_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dropout Streak Density",0,100,0,100,65,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_DROPOUT_DENSITY);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Streak Length (px)",2,120,2,120,18,1,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_STREAK_LENGTH);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Streak Brightness",0,100,0,100,75,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_STREAK_BRIGHTNESS);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Dark Dropout Amount",0,100,0,100,70,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_DARK_DROPOUT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Horizontal Fragmentation",0,100,0,100,65,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Temporal Variation",0,100,0,100,85,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_TEMPORAL_VARIATION);
    AEFX_CLR_STRUCT(def); PF_ADD_CHECKBOX("Enable Chromatic Variation","Enable",1,0,ID_SYNC_DAMAGE_CHROMA_ENABLE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Subtle Chroma Tint",0,100,0,100,20,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_CHROMA_TINT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Magenta Tint",0,100,0,100,20,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_MAGENTA_TINT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Green Tint",0,100,0,100,20,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_GREEN_TINT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("RGB Offset (px)",0,8,0,8,0.35,2,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_RGB_OFFSET);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Chroma Bleed",0,100,0,100,18,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_CHROMA_BLEED);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Animation Mode",3,1,kSyncDamageAnimationChoices,ID_SYNC_DAMAGE_ANIMATION_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_POPUP("Input Mode",2,1,kSyncDamageInputChoices,ID_SYNC_DAMAGE_INPUT_MODE);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Animation Speed",0,400,0,400,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_ANIMATION_SPEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Random Seed",1,32767,1,32767,1,0,PF_ValueDisplayFlag_NONE,0,ID_SYNC_DAMAGE_RANDOM_SEED);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Blend With Input",0,100,0,100,80,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_BLEND_INPUT);
    AEFX_CLR_STRUCT(def); PF_ADD_FLOAT_SLIDERX("Effect Amount",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_SYNC_DAMAGE_EFFECT_AMOUNT);
    AEFX_CLR_STRUCT(def); PF_END_TOPIC(ID_SYNC_DAMAGE_TOPIC+100);
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
static PF_Err CheckoutCheckbox(PF_InData* in_data,A_long index,A_long& value) {
    PF_ParamDef param; AEFX_CLR_STRUCT(param);
    PF_Err err=PF_CHECKOUT_PARAM(in_data,index,in_data->current_time,in_data->time_step,in_data->time_scale,&param);
    if(err!=PF_Err_NONE) return err;
    value=param.u.bd.value?1:0;
    return PF_CHECKIN_PARAM(in_data,&param);
}
static PF_Err CheckoutPopup(PF_InData* in_data,A_long index,A_long& value) {
    PF_ParamDef param; AEFX_CLR_STRUCT(param);
    PF_Err err=PF_CHECKOUT_PARAM(in_data,index,in_data->current_time,in_data->time_step,in_data->time_scale,&param);
    if(err!=PF_Err_NONE) return err;
    value=param.u.pd.value;
    return PF_CHECKIN_PARAM(in_data,&param);
}
static PF_Err CheckoutOsdText(PF_InData* in_data, std::string& value) {
    PF_ParamDef param; AEFX_CLR_STRUCT(param);
    PF_Err err=PF_CHECKOUT_PARAM(in_data,PARAM_OSD_TEXT_DATA,in_data->current_time,
        in_data->time_step,in_data->time_scale,&param);
    if(err!=PF_Err_NONE) return err;
    err=VHS90ReadOsdText(in_data,param.u.arb_d.value,value);
    const PF_Err checkin=PF_CHECKIN_PARAM(in_data,&param);
    return err!=PF_Err_NONE?err:checkin;
}

static PF_Err ReadSettings(PF_InData* in_data,RenderSettings& s) {
    PF_Err err=PF_Err_NONE;
#define READ_FLOAT(I,F) do { err=CheckoutFloat(in_data,(I),s.F); if(err!=PF_Err_NONE) return err; } while(0)
    READ_FLOAT(PARAM_MASTER,master); READ_FLOAT(PARAM_COMPOSITE_AMOUNT,composite);
    READ_FLOAT(PARAM_SYNC_AMOUNT,sync);
    err=CheckoutPopup(in_data,PARAM_PAL_MODE,s.colorSystem); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_SIGNAL_MODEL,s.signalModel); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TAPE_SPEED,s.tapeSpeed); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_DECODER_MODE,s.compositeDecoder); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_FRAME_REPEAT_FREQUENCY,frameRepeatFrequency);
    READ_FLOAT(PARAM_FRAME_REPEAT_STRENGTH,frameRepeatStrength);
    READ_FLOAT(PARAM_GROUND_LOOP,groundLoop);
    READ_FLOAT(PARAM_TRACK_LAYER2_ANGLE,trackingLayerAngle[0]); READ_FLOAT(PARAM_TRACK_LAYER2_MOTION_X,trackingLayerMotionX[0]); READ_FLOAT(PARAM_TRACK_LAYER2_MOTION_Y,trackingLayerMotionY[0]);
    READ_FLOAT(PARAM_TRACK_LAYER3_ANGLE,trackingLayerAngle[1]); READ_FLOAT(PARAM_TRACK_LAYER3_MOTION_X,trackingLayerMotionX[1]); READ_FLOAT(PARAM_TRACK_LAYER3_MOTION_Y,trackingLayerMotionY[1]);
    READ_FLOAT(PARAM_TRACK_LAYER4_ANGLE,trackingLayerAngle[2]); READ_FLOAT(PARAM_TRACK_LAYER4_MOTION_X,trackingLayerMotionX[2]); READ_FLOAT(PARAM_TRACK_LAYER4_MOTION_Y,trackingLayerMotionY[2]);
    err=CheckoutCheckbox(in_data,PARAM_SYNC_DAMAGE_ENABLE,s.syncDamageEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_SYNC_DAMAGE_TEAR_AMOUNT,syncDamageTearAmount); READ_FLOAT(PARAM_SYNC_DAMAGE_BAND_INTENSITY,syncDamageBandIntensity);
    READ_FLOAT(PARAM_SYNC_DAMAGE_BAND_DENSITY,syncDamageBandDensity); READ_FLOAT(PARAM_SYNC_DAMAGE_BAND_HEIGHT,syncDamageBandHeight);
    READ_FLOAT(PARAM_SYNC_DAMAGE_BAND_IRREGULARITY,syncDamageBandIrregularity); READ_FLOAT(PARAM_SYNC_DAMAGE_DISPLACEMENT,syncDamageDisplacement);
    READ_FLOAT(PARAM_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION,syncDamageBoundaryFragmentation); READ_FLOAT(PARAM_SYNC_DAMAGE_COVERAGE,syncDamageCoverage);
    err=CheckoutCheckbox(in_data,PARAM_SYNC_DAMAGE_LUMA_ENABLE,s.syncDamageLumaEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_SYNC_DAMAGE_LUMA_INSTABILITY,syncDamageLumaInstability); READ_FLOAT(PARAM_SYNC_DAMAGE_BRIGHT_FLASH,syncDamageBrightFlash);
    READ_FLOAT(PARAM_SYNC_DAMAGE_DARK_COLLAPSE,syncDamageDarkCollapse); READ_FLOAT(PARAM_SYNC_DAMAGE_EVENT_FREQUENCY,syncDamageEventFrequency);
    READ_FLOAT(PARAM_SYNC_DAMAGE_EVENT_DURATION,syncDamageEventDuration); READ_FLOAT(PARAM_SYNC_DAMAGE_BRIGHT_PROBABILITY,syncDamageBrightProbability); READ_FLOAT(PARAM_SYNC_DAMAGE_DARK_PROBABILITY,syncDamageDarkProbability); READ_FLOAT(PARAM_SYNC_DAMAGE_TRANSITION_SHARPNESS,syncDamageTransitionSharpness);
    READ_FLOAT(PARAM_SYNC_DAMAGE_LOCAL_LUMA,syncDamageLocalLuma); READ_FLOAT(PARAM_SYNC_DAMAGE_MANUAL_BRIGHT,syncDamageManualBright);
    READ_FLOAT(PARAM_SYNC_DAMAGE_MANUAL_DARK,syncDamageManualDark);
    err=CheckoutCheckbox(in_data,PARAM_SYNC_DAMAGE_DROPOUTS_ENABLE,s.syncDamageDropoutsEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_SYNC_DAMAGE_DROPOUT_DENSITY,syncDamageDropoutDensity); READ_FLOAT(PARAM_SYNC_DAMAGE_STREAK_LENGTH,syncDamageStreakLength);
    READ_FLOAT(PARAM_SYNC_DAMAGE_STREAK_BRIGHTNESS,syncDamageStreakBrightness); READ_FLOAT(PARAM_SYNC_DAMAGE_DARK_DROPOUT,syncDamageDarkDropout);
    READ_FLOAT(PARAM_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION,syncDamageHorizontalFragmentation); READ_FLOAT(PARAM_SYNC_DAMAGE_TEMPORAL_VARIATION,syncDamageTemporalVariation);
    err=CheckoutCheckbox(in_data,PARAM_SYNC_DAMAGE_CHROMA_ENABLE,s.syncDamageChromaEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_SYNC_DAMAGE_CHROMA_TINT,syncDamageChromaTint); READ_FLOAT(PARAM_SYNC_DAMAGE_MAGENTA_TINT,syncDamageMagentaTint);
    READ_FLOAT(PARAM_SYNC_DAMAGE_GREEN_TINT,syncDamageGreenTint); READ_FLOAT(PARAM_SYNC_DAMAGE_RGB_OFFSET,syncDamageRgbOffset);
    READ_FLOAT(PARAM_SYNC_DAMAGE_CHROMA_BLEED,syncDamageChromaBleed);
    err=CheckoutPopup(in_data,PARAM_SYNC_DAMAGE_ANIMATION_MODE,s.syncDamageAnimationMode); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_SYNC_DAMAGE_INPUT_MODE,s.syncDamageInputMode); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_SYNC_DAMAGE_ANIMATION_SPEED,syncDamageAnimationSpeed); READ_FLOAT(PARAM_SYNC_DAMAGE_RANDOM_SEED,syncDamageRandomSeed);
    READ_FLOAT(PARAM_SYNC_DAMAGE_BLEND_INPUT,syncDamageBlendInput); READ_FLOAT(PARAM_SYNC_DAMAGE_EFFECT_AMOUNT,syncDamageEffectAmount);
    READ_FLOAT(PARAM_TAPE_DETAIL,tapeDetail); READ_FLOAT(PARAM_TRACK_READ,trackRead);
    READ_FLOAT(PARAM_TRACK_MISALIGNMENT,trackMisalignment); READ_FLOAT(PARAM_CHROMA_SHIFT,chromaShift);
    READ_FLOAT(PARAM_CHROMA_REG_RED,redChannelOffset); READ_FLOAT(PARAM_CHROMA_REG_GREEN,greenChannelOffset); READ_FLOAT(PARAM_CHROMA_REG_BLUE,blueChannelOffset); err=CheckoutPopup(in_data,PARAM_CHROMA_REG_MODE,s.chromaRegMode); if(err!=PF_Err_NONE) return err;
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
    READ_FLOAT(PARAM_GRADE_EXPOSURE,gradeExposure); READ_FLOAT(PARAM_GRADE_CONTRAST,gradeContrast); READ_FLOAT(PARAM_GRADE_BRIGHTNESS,gradeBrightness);
    READ_FLOAT(PARAM_GRADE_SATURATION,gradeSaturation); READ_FLOAT(PARAM_GRADE_I_GAIN,chromaIGain); READ_FLOAT(PARAM_GRADE_Q_GAIN,chromaQGain); READ_FLOAT(PARAM_GRADE_I_OFFSET,chromaIOffset); READ_FLOAT(PARAM_GRADE_Q_OFFSET,chromaQOffset); READ_FLOAT(PARAM_GRADE_CHROMA_QUANTIZATION,chromaQuantization); READ_FLOAT(PARAM_VIGNETTE,vignette); READ_FLOAT(PARAM_FLICKER,flicker);
    READ_FLOAT(PARAM_LUMA_SMEAR,lumaSmear); READ_FLOAT(PARAM_BLOOM,bloom); READ_FLOAT(PARAM_ANALOG_GLOW,analogGlow);
    READ_FLOAT(PARAM_TAPE_SURFACE,tapeSurface); READ_FLOAT(PARAM_TAPE_TRANSPORT,tapeTransport);
    READ_FLOAT(PARAM_GEOMETRY,geometry); READ_FLOAT(PARAM_INTERLACE,interlace); READ_FLOAT(PARAM_VERTICAL_ROLL,verticalRoll); READ_FLOAT(PARAM_EDGE_WAVE,edgeWave); READ_FLOAT(PARAM_SCANLINES,scanlines); READ_FLOAT(PARAM_TAPE_NOISE,tapeNoise);
    READ_FLOAT(PARAM_STATIC_AMOUNT,staticAmount);
    READ_FLOAT(PARAM_STATIC_DENSITY,staticDensity); READ_FLOAT(PARAM_PATTERN_ANGLE,staticAngle); READ_FLOAT(PARAM_PATTERN_WIDTH,staticRibbonWidth); READ_FLOAT(PARAM_PATTERN_GREEN,staticGreenLine); READ_FLOAT(PARAM_PATTERN_PHASE_COUNT,staticPhaseCount); READ_FLOAT(PARAM_TRACKING_AMOUNT,trackingAmount);
    err=CheckoutPopup(in_data,PARAM_PATTERN_MODE,s.staticMode); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACKING_SHAPE,s.trackingShape); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TRACKING_CENTER_X,trackingX); READ_FLOAT(PARAM_TRACKING_CENTER_Y,trackingY);
    READ_FLOAT(PARAM_TRACKING_WIDTH,trackingWidth); READ_FLOAT(PARAM_TRACKING_HEIGHT,trackingHeight);
    READ_FLOAT(PARAM_TRACKING_ANGLE,trackingAngle);
    READ_FLOAT(PARAM_TRACKING_MOTION_X,trackingMotionX); READ_FLOAT(PARAM_TRACKING_MOTION_Y,trackingMotionY);
    READ_FLOAT(PARAM_TRACKING_SERVO,trackingServo); READ_FLOAT(PARAM_TRACKING_TEAR,trackingTear);
    err=CheckoutPopup(in_data,PARAM_TRACKING_PROFILE,s.trackingProfile); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACKING_PRESET_BASE,s.trackingPresetBase); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACKING_PRESET_2,s.trackingLayerPreset[0]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACKING_PRESET_3,s.trackingLayerPreset[1]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACKING_PRESET_4,s.trackingLayerPreset[2]); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TEAR_CHROMA_AMOUNT,tearChromaAmount); READ_FLOAT(PARAM_SPECTRAL_SMEAR,spectralSmear);
    READ_FLOAT(PARAM_SPECTRAL_HUE_SPREAD,spectralHueSpread); READ_FLOAT(PARAM_CHROMA_SATURATION,chromaSaturation);
    READ_FLOAT(PARAM_SPECTRAL_CENTER_Y,spectralCenterY); READ_FLOAT(PARAM_SPECTRAL_COVERAGE,spectralCoverage);
    READ_FLOAT(PARAM_TEAR_BAND_START_Y,tearBandStartY); READ_FLOAT(PARAM_TEAR_BAND_HEIGHT,tearBandHeight);
    READ_FLOAT(PARAM_TEAR_BAND_COUNT,tearBandCount); READ_FLOAT(PARAM_TEAR_DISPLACEMENT,tearDisplacement);
    READ_FLOAT(PARAM_TEAR_FRAGMENTATION,tearFragmentation); READ_FLOAT(PARAM_TEAR_CHROMA_SHIFT,tearChromaShift);
    READ_FLOAT(PARAM_TEAR_SMEAR,tearSmear); READ_FLOAT(PARAM_TEAR_DROPOUT,tearDropout); READ_FLOAT(PARAM_TEAR_SEED,tearSeed);
    READ_FLOAT(PARAM_TRACK_LAYER2_AMOUNT,trackingLayerAmount[0]); READ_FLOAT(PARAM_TRACK_LAYER2_X,trackingLayerX[0]); READ_FLOAT(PARAM_TRACK_LAYER2_Y,trackingLayerY[0]);
    READ_FLOAT(PARAM_TRACK_LAYER2_WIDTH,trackingLayerWidth[0]); READ_FLOAT(PARAM_TRACK_LAYER2_HEIGHT,trackingLayerHeight[0]);
    READ_FLOAT(PARAM_TRACK_LAYER2_SERVO,trackingLayerServo[0]); READ_FLOAT(PARAM_TRACK_LAYER2_SHIFT,trackingLayerShift[0]); READ_FLOAT(PARAM_TRACK_LAYER2_WAVE,trackingLayerWave[0]); READ_FLOAT(PARAM_TRACK_LAYER2_TEAR,trackingLayerTear[0]);
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER2_PROFILE,s.trackingLayerProfile[0]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER2_SHAPE,s.trackingLayerShape[0]); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TRACK_LAYER3_AMOUNT,trackingLayerAmount[1]); READ_FLOAT(PARAM_TRACK_LAYER3_X,trackingLayerX[1]); READ_FLOAT(PARAM_TRACK_LAYER3_Y,trackingLayerY[1]);
    READ_FLOAT(PARAM_TRACK_LAYER3_WIDTH,trackingLayerWidth[1]); READ_FLOAT(PARAM_TRACK_LAYER3_HEIGHT,trackingLayerHeight[1]);
    READ_FLOAT(PARAM_TRACK_LAYER3_SERVO,trackingLayerServo[1]); READ_FLOAT(PARAM_TRACK_LAYER3_SHIFT,trackingLayerShift[1]); READ_FLOAT(PARAM_TRACK_LAYER3_WAVE,trackingLayerWave[1]); READ_FLOAT(PARAM_TRACK_LAYER3_TEAR,trackingLayerTear[1]);
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER3_PROFILE,s.trackingLayerProfile[1]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER3_SHAPE,s.trackingLayerShape[1]); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TRACK_LAYER4_AMOUNT,trackingLayerAmount[2]); READ_FLOAT(PARAM_TRACK_LAYER4_X,trackingLayerX[2]); READ_FLOAT(PARAM_TRACK_LAYER4_Y,trackingLayerY[2]);
    READ_FLOAT(PARAM_TRACK_LAYER4_WIDTH,trackingLayerWidth[2]); READ_FLOAT(PARAM_TRACK_LAYER4_HEIGHT,trackingLayerHeight[2]);
    READ_FLOAT(PARAM_TRACK_LAYER4_SERVO,trackingLayerServo[2]); READ_FLOAT(PARAM_TRACK_LAYER4_SHIFT,trackingLayerShift[2]); READ_FLOAT(PARAM_TRACK_LAYER4_WAVE,trackingLayerWave[2]); READ_FLOAT(PARAM_TRACK_LAYER4_TEAR,trackingLayerTear[2]);
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER4_PROFILE,s.trackingLayerProfile[2]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TRACK_LAYER4_SHAPE,s.trackingLayerShape[2]); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_TEAR_CHROMA_MODE,s.tearChromaMode); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_TEAR_BAND2_AMOUNT,tearBandAmount[0]); READ_FLOAT(PARAM_TEAR_BAND2_Y,tearBandY[0]); READ_FLOAT(PARAM_TEAR_BAND2_HEIGHT,tearBandHeightExtra[0]);
    READ_FLOAT(PARAM_TEAR_BAND2_SHIFT,tearBandShift[0]); READ_FLOAT(PARAM_TEAR_BAND2_FRAGMENTATION,tearBandFragmentation[0]);
    READ_FLOAT(PARAM_TEAR_BAND2_CHROMA,tearBandChroma[0]); READ_FLOAT(PARAM_TEAR_BAND2_SMEAR,tearBandSmear[0]); READ_FLOAT(PARAM_TEAR_BAND2_DROPOUT,tearBandDropout[0]);
    READ_FLOAT(PARAM_TEAR_BAND3_AMOUNT,tearBandAmount[1]); READ_FLOAT(PARAM_TEAR_BAND3_Y,tearBandY[1]); READ_FLOAT(PARAM_TEAR_BAND3_HEIGHT,tearBandHeightExtra[1]);
    READ_FLOAT(PARAM_TEAR_BAND3_SHIFT,tearBandShift[1]); READ_FLOAT(PARAM_TEAR_BAND3_FRAGMENTATION,tearBandFragmentation[1]);
    READ_FLOAT(PARAM_TEAR_BAND3_CHROMA,tearBandChroma[1]); READ_FLOAT(PARAM_TEAR_BAND3_SMEAR,tearBandSmear[1]); READ_FLOAT(PARAM_TEAR_BAND3_DROPOUT,tearBandDropout[1]);
    READ_FLOAT(PARAM_TEAR_BAND4_AMOUNT,tearBandAmount[2]); READ_FLOAT(PARAM_TEAR_BAND4_Y,tearBandY[2]); READ_FLOAT(PARAM_TEAR_BAND4_HEIGHT,tearBandHeightExtra[2]);
    READ_FLOAT(PARAM_TEAR_BAND4_SHIFT,tearBandShift[2]); READ_FLOAT(PARAM_TEAR_BAND4_FRAGMENTATION,tearBandFragmentation[2]);
    READ_FLOAT(PARAM_TEAR_BAND4_CHROMA,tearBandChroma[2]); READ_FLOAT(PARAM_TEAR_BAND4_SMEAR,tearBandSmear[2]); READ_FLOAT(PARAM_TEAR_BAND4_DROPOUT,tearBandDropout[2]);
    READ_FLOAT(PARAM_TEAR_BAND5_AMOUNT,tearBandAmount[3]); READ_FLOAT(PARAM_TEAR_BAND5_Y,tearBandY[3]); READ_FLOAT(PARAM_TEAR_BAND5_HEIGHT,tearBandHeightExtra[3]);
    READ_FLOAT(PARAM_TEAR_BAND5_SHIFT,tearBandShift[3]); READ_FLOAT(PARAM_TEAR_BAND5_FRAGMENTATION,tearBandFragmentation[3]);
    READ_FLOAT(PARAM_TEAR_BAND5_CHROMA,tearBandChroma[3]); READ_FLOAT(PARAM_TEAR_BAND5_SMEAR,tearBandSmear[3]); READ_FLOAT(PARAM_TEAR_BAND5_DROPOUT,tearBandDropout[3]);
    READ_FLOAT(PARAM_HEAD_AMOUNT,headAmount); READ_FLOAT(PARAM_HEAD_BOTTOM_Y,headY);
    READ_FLOAT(PARAM_HEAD_HEIGHT,headHeight); READ_FLOAT(PARAM_HEAD_SHIFT,headShift);
    READ_FLOAT(PARAM_HEAD_CONDITION,headCondition);
    READ_FLOAT(PARAM_TAPE_WEAR,tapeWear); READ_FLOAT(PARAM_TAPE_DROPOUTS,tapeDropouts); READ_FLOAT(PARAM_TAPE_SCRATCHES,tapeScratches);
    READ_FLOAT(PARAM_TAPE_CREASES,tapeCreases); READ_FLOAT(PARAM_TAPE_EDGE,tapeEdge); READ_FLOAT(PARAM_TAPE_MOLD,tapeMold);
    READ_FLOAT(PARAM_TAPE_VASELINE,tapeVaseline); READ_FLOAT(PARAM_TAPE_STICKY,tapeSticky); READ_FLOAT(PARAM_TAPE_PINHOLES,tapePinholes); READ_FLOAT(PARAM_TAPE_FLAKES,tapeFlakes); READ_FLOAT(PARAM_TAPE_ROUGHNESS,tapeRoughness); READ_FLOAT(PARAM_TAPE_SEED,tapeSeed); READ_FLOAT(PARAM_TAPE_BACK_TENSION,backTension);
    READ_FLOAT(PARAM_MAINS_HUM,mainsHum);
    READ_FLOAT(PARAM_OSD_AMOUNT,osdAmount); READ_FLOAT(PARAM_OSD_REC,osdRec); READ_FLOAT(PARAM_OSD_PLAY,osdPlay); READ_FLOAT(PARAM_OSD_TIMESTAMP,osdTimestamp);
    READ_FLOAT(PARAM_OSD_OPACITY,osdOpacity); READ_FLOAT(PARAM_OSD_SCALE,osdScale); READ_FLOAT(PARAM_OSD_GLITCH,osdGlitch); READ_FLOAT(PARAM_OSD_GHOST,osdGhost);
    READ_FLOAT(PARAM_OSD_REC_BLINK,osdRecBlink); READ_FLOAT(PARAM_OSD_TIME_SPEED,osdTimeSpeed);
    READ_FLOAT(PARAM_OSD_YEAR,osdYear); READ_FLOAT(PARAM_OSD_MONTH,osdMonth); READ_FLOAT(PARAM_OSD_DAY,osdDay);
    READ_FLOAT(PARAM_OSD_HOUR,osdHour); READ_FLOAT(PARAM_OSD_MINUTE,osdMinute); READ_FLOAT(PARAM_OSD_SECOND,osdSecond);
    err=CheckoutPopup(in_data,PARAM_OSD_DECK_MODE,s.osdDeckMode); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_DATE_FORMAT,s.osdDateFormat); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_OSD_REC_X,osdRecX); READ_FLOAT(PARAM_OSD_REC_Y,osdRecY);
    READ_FLOAT(PARAM_OSD_STATUS_X,osdStatusX); READ_FLOAT(PARAM_OSD_STATUS_Y,osdStatusY);
    READ_FLOAT(PARAM_OSD_TIMESTAMP_X,osdTimestampX); READ_FLOAT(PARAM_OSD_TIMESTAMP_Y,osdTimestampY);
    err=CheckoutPopup(in_data,PARAM_OSD_COUNTER_FORMAT,s.osdCounterFormat); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_OSD_COUNTER_AMOUNT,osdCounterAmount); READ_FLOAT(PARAM_OSD_COUNTER_OFFSET,osdCounterOffset); err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_COUNTER,s.osdShowCounter); if(err!=PF_Err_NONE) return err; READ_FLOAT(PARAM_OSD_COUNTER_X,osdCounterX); READ_FLOAT(PARAM_OSD_COUNTER_Y,osdCounterY);
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
    READ_FLOAT(PARAM_VHS6RIFT_GRAIN,vhs6RiftGrain); READ_FLOAT(PARAM_VHS6RIFT_GRAIN_SIZE,vhs6RiftGrainSize); READ_FLOAT(PARAM_VHS6RIFT_NOISE_DETAIL,vhs6RiftNoiseDetail); READ_FLOAT(PARAM_VHS6RIFT_SHADOW_BIAS,vhs6RiftShadowBias); READ_FLOAT(PARAM_VHS6RIFT_NOISE_SEED,vhs6RiftNoiseSeed);
    PF_ParamDef noiseEnable; AEFX_CLR_STRUCT(noiseEnable); err=PF_CHECKOUT_PARAM(in_data,PARAM_VHS6RIFT_NOISE_ENABLE,in_data->current_time,in_data->time_step,in_data->time_scale,&noiseEnable); if(err!=PF_Err_NONE) return err; s.vhs6RiftNoiseEnable=noiseEnable.u.bd.value!=0; err=PF_CHECKIN_PARAM(in_data,&noiseEnable); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_EDGE_WAVE_FREQUENCY,edgeWaveFrequency); READ_FLOAT(PARAM_EDGE_WAVE_SPEED,edgeWaveSpeed); READ_FLOAT(PARAM_EDGE_WAVE_DETAIL,edgeWaveDetail);
    READ_FLOAT(PARAM_CHROMA_EDGE_ENHANCE,chromaEdgeEnhance);
    err=CheckoutOsdText(in_data,s.osdText); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_LANGUAGE,s.osdLanguage); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_DIGITS,s.osdDigits); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_DATE_LAYOUT,s.osdDateLayout); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_CLOCK_MODE,s.osdClockMode); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_MONTH_STYLE,s.osdMonthStyle); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_CLOCK_RUNS,s.osdClockRuns); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_FONT,s.osdFont); if(err!=PF_Err_NONE) return err;
    err=CheckoutPopup(in_data,PARAM_OSD_EXTRA_LABEL,s.osdExtraLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_TEXT,s.osdShowText); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_REC_LABEL,s.osdShowRecLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_PLAY_LABEL,s.osdShowPlayLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_PAUSE_LABEL,s.osdShowPauseLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_STOP_LABEL,s.osdShowStopLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_FF_LABEL,s.osdShowFfLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_REW_LABEL,s.osdShowRewLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_SPEED_LABEL,s.osdShowSpeedLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_TAPE_LABEL,s.osdShowTapeLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_ERROR_LABEL,s.osdShowErrorLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_PROGRAM_LABEL,s.osdShowProgramLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_RES_LABEL,s.osdShowResLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_FORWARD_LABEL,s.osdShowForwardLabel); if(err!=PF_Err_NONE) return err;
    err=CheckoutCheckbox(in_data,PARAM_OSD_TEXT_GLOW,s.osdTextGlow); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_OSD_TEXT_SIZE,osdTextSize); READ_FLOAT(PARAM_OSD_TEXT_GLOW_AMOUNT,osdTextGlowAmount); READ_FLOAT(PARAM_OSD_TEXT_SEED,osdTextSeed); READ_FLOAT(PARAM_OSD_TEXT_OPACITY,osdTextOpacity);
    err=CheckoutCheckbox(in_data,PARAM_OSD_SHOW_DATE,s.osdShowDate); if(err!=PF_Err_NONE) return err;
    READ_FLOAT(PARAM_OSD_WORDS_X,osdWordsX); READ_FLOAT(PARAM_OSD_WORDS_Y,osdWordsY);
    READ_FLOAT(PARAM_OSD_EXTRA_X,osdExtraX); READ_FLOAT(PARAM_OSD_EXTRA_Y,osdExtraY);
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
        const float masterDriveValue=masterDrive(s.master);
        const float master=clamp01(masterDriveValue);
        const float masterOverdrive=std::clamp(masterDriveValue,0.0f,3.0f);
        const bool compositeStages=(s.vhs6RiftNoiseEnable&&s.vhs6RiftGrain>0.0f)||s.signalHueNoise>0.0f||s.tapeDetail<100||s.trackRead>0||s.headCondition>0||s.chromaShift>0||s.composite>0||s.chromaSubsample>0||s.verticalChroma>0||s.verticalLuma>0||s.lumaNoise>0||s.chromaNoise>0||s.signalHueNoise>0||s.chromaLoss>0||s.compositeNoise>0||s.tapeNoise>0||s.rfDrift>0||s.rfHum>0||s.rfAm>0||s.agcPump>0||s.impulseNoise>0||s.dropouts>0||s.multipathGain>0||s.multipathEnsemble>0||s.signalGhost>0||s.chromaDelay>0||s.afcHunt>0||s.chromaFlutter>0||s.ycCrosstalk>0||s.colorPhaseDrift>0||s.colorDrift>0||s.hsyncNoise>0||s.vsyncNoise>0||s.burstNoise>0||s.edgeEnhance>0||s.tapeFade>0||s.blackLift>0||s.temporalGhost>0||s.ringing>0||s.crtEqualizer>0||s.gradeExposure!=0||s.gradeContrast!=100||s.gradeBrightness!=0||s.gradeSaturation!=100||s.vignette>0||s.flicker>0||s.lumaSmear>0||s.bloom>0||s.analogGlow>0||s.tapeSurface>0||s.tapePinholes>0||s.tapeFlakes>0||s.tapeRoughness>0||s.tapeTransport>0||(s.trackingAmount>0&&s.trackingServo>0)||s.geometry>0||s.interlace>0||s.verticalRoll>0||s.edgeWave>0||s.scanlines>0||s.vhs43Frame!=0||s.roundedOverscan>0||s.lensDistortion!=0||s.digitalZoom>1.0f||s.frameRepeatFrequency>0||s.frameRepeatStrength>0||s.magnetMoment>0||s.groundLoop>0||s.mainsHum>0||s.rfPickup>0||s.ntscRsPhaseNoise>0;
        const bool osdEnabled=s.osdAmount>0.0f;
        const bool any=compositeStages||s.sync>0||s.staticAmount>0||s.tearChromaAmount>0||s.trackingAmount>0||s.trackingLayerAmount[0]>0||s.trackingLayerAmount[1]>0||s.trackingLayerAmount[2]>0||s.headAmount>0||s.redChannelOffset!=0.0f||s.greenChannelOffset!=0.0f||s.blueChannelOffset!=0.0f||s.syncDamageEnable>0||osdEnabled;
        if(!any||(master<=0.0f&&!osdEnabled)) { CopyRegion(input,output,format); return PF_Err_NONE; }
        std::vector<float> source,working,scratch;
        if(!ConvertInputWorld(input,format,source)) return PF_Err_UNRECOGNIZED_PARAM_TYPE;
        std::vector<float> previousPixels;
        if(previousInput && (s.temporalGhost>0||s.interlace>0||(s.frameRepeatFrequency>0&&s.frameRepeatStrength>0)) && previousInput->width==w && previousInput->height==h) {
            if(!ConvertInputWorld(previousInput,format,previousPixels)) previousPixels.clear();
        }
        working=source; scratch.resize(source.size());
        if(osdEnabled) {
            VhsOsdParams op;
            op.amount=clamp01(s.osdAmount/100.0f); op.opacity=clamp01(s.osdOpacity/100.0f);
            op.scale=std::max(0.35f,s.osdScale/100.0f); op.glitch=std::max(0.0f,s.osdGlitch/100.0f);
            op.ghost=clamp01(s.osdGhost/100.0f); op.recAmount=clamp01(s.osdRec/100.0f);
            op.playAmount=clamp01(s.osdPlay/100.0f); op.timestampAmount=clamp01(s.osdTimestamp/100.0f);
            op.batteryAmount=clamp01(s.osdBattery/100.0f);op.showBattery=s.osdBattery>0.0f;
            op.recBlinkHz=std::max(0.0f,s.osdRecBlink); op.timestampSpeed=std::max(0.0f,s.osdTimeSpeed);
            op.timeSeconds=s.timeSeconds; op.framesPerSecond=s.frameRate; op.frame=s.frame;
            op.startYear=static_cast<int>(std::lround(s.osdYear)); op.startMonth=static_cast<int>(std::lround(s.osdMonth));
            op.startDay=static_cast<int>(std::lround(s.osdDay)); op.startHour=static_cast<int>(std::lround(s.osdHour));
            op.startMinute=static_cast<int>(std::lround(s.osdMinute)); op.startSecond=static_cast<int>(std::lround(s.osdSecond));
            op.recColorR=s.osdRecColorR/255.0f; op.recColorG=s.osdRecColorG/255.0f; op.recColorB=s.osdRecColorB/255.0f;
            op.deckMode=static_cast<VhsDeckMode>(std::clamp<A_long>(s.osdDeckMode,1,5));
            op.timestampFormat=static_cast<VhsTimestampFormat>(std::clamp<A_long>(s.osdDateFormat,1,5));
            op.drawRecText=false; op.drawDeckText=false; op.drawTapeSpeedText=false; op.drawTimestampText=false; op.drawTapeCounterText=false;
            op.tapeSpeed=std::clamp<A_long>(s.tapeSpeed,1,3);
            op.recX=s.osdRecX; op.recY=s.osdRecY; op.statusX=s.osdStatusX; op.statusY=s.osdStatusY;
            op.timestampX=s.osdTimestampX; op.timestampY=s.osdTimestampY;
            op.showTapeCounter=s.osdCounterAmount>0.0f && s.osdShowText!=0 && s.osdShowCounter!=0; op.tapeCounterAmount=clamp01(s.osdCounterAmount/100.0f);
            op.tapeCounterFormat=static_cast<VhsTapeCounterFormat>(std::clamp<A_long>(s.osdCounterFormat,1,3));
            op.tapeCounterX=s.osdCounterX; op.tapeCounterY=s.osdCounterY;
            op.seed=0x9050D5u; op.premultipliedAlpha=true;
            ImageView osdView{working.data(),w,h,w*4};
            if(!applyVhsOsd(osdView,op)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            static constexpr const char* kOsdFontFamilies[]={
                "A Massir Ballpoint","A Naskh Tahrir","A-bad kh@t","Guesswhat Exceptional",
                "Arabic Childhood ver1","Arial Narrow","Evogria","K Kamran","Maqroo",
                "Milan Display","Nasma","Real Vhs Font","RooyinFree",
                "YR Sponge Arabic","ZIAN Black"};
            const A_long osdFontIndex=std::clamp<A_long>(s.osdFont-1,0,14);
            const char* osdFont=kOsdFontFamilies[osdFontIndex];
            static constexpr std::uint16_t kOsdFontWeights[]={400,400,400,400,400,700,400,400,400,900,400,400,700,400,900};
            const auto osdFontWeight=kOsdFontWeights[osdFontIndex];
            const auto language=static_cast<VhsOsdLanguage>(std::clamp<A_long>(s.osdLanguage-1,0,2));
            const auto digits=static_cast<VhsOsdDigitSet>(std::clamp<A_long>(s.osdDigits-1,0,2));
            const int osdCell=std::max(1,int(std::lround((float(h)/294.0f)*std::max(0.35f,op.scale))));
            auto drawOsdText=[&](const std::string& text,float x,float y,float size,float alpha,
                                 vhs90::win::TextAlignment alignment,float rightEdge=0.0f) -> PF_Err {
                alpha=clamp01(alpha*clamp01(s.osdTextOpacity/100.0f));
                if(text.empty()||alpha<=0.001f) return PF_Err_NONE;
                vhs90::win::DirectWriteTextOptions options;
                options.utf8Text=text.c_str(); options.utf8FontFamily=osdFont; options.fontWeight=osdFontWeight;
                const double phase=double(s.frame)*0.7548776662466927+double(s.osdTextSeed)*0.000123;
                const float jitterX=std::sin(phase*12.9898)*op.glitch*float(osdCell)*0.45f;
                const float jitterY=std::cos(phase*78.233)*op.glitch*float(osdCell)*0.22f;
                options.x=std::max(0.0f,x+jitterX); options.y=std::max(0.0f,y+jitterY);
                options.fontSize=std::max(8.0f,size);
                options.maxWidth=std::max(1.0f,rightEdge>0.0f?rightEdge-options.x:float(w)-options.x);
                options.maxHeight=std::max(1.0f,float(h)-options.y);
                options.red=1.0f; options.green=1.0f; options.blue=1.0f;
                options.opacity=clamp01(alpha);
                options.direction=language==VhsOsdLanguage::Arabic?
                    vhs90::win::TextDirection::RightToLeft:vhs90::win::TextDirection::Auto;
                options.alignment=alignment; options.premultipliedDestination=true;
                if(op.ghost>0.002f) {
                    const float ghostOffset=std::max(1.0f,options.fontSize*0.12f*op.ghost);
                    auto ghostA=options; ghostA.x=std::max(0.0f,options.x+ghostOffset);
                    ghostA.red=0.15f; ghostA.green=0.78f; ghostA.blue=1.0f;
                    ghostA.opacity=clamp01(alpha*op.ghost*0.28f);
                    const auto ghostResultA=vhs90::win::drawDirectWriteText(osdView,ghostA);
                    if(!ghostResultA.rendered) return PF_Err_INTERNAL_STRUCT_DAMAGED;
                    auto ghostB=options; ghostB.x=std::max(0.0f,options.x-ghostOffset);
                    ghostB.red=1.0f; ghostB.green=0.22f; ghostB.blue=0.45f;
                    ghostB.opacity=clamp01(alpha*op.ghost*0.18f);
                    const auto ghostResultB=vhs90::win::drawDirectWriteText(osdView,ghostB);
                    if(!ghostResultB.rendered) return PF_Err_INTERNAL_STRUCT_DAMAGED;
                }
                if(s.osdTextGlow) {
                    const float glowAlpha=clamp01(alpha*0.48f*clamp01(s.osdTextGlowAmount/100.0f));
                    const float glowRadius=std::max(1.0f,std::round(options.fontSize*0.08f));
                    for(int gy=-1;gy<=1;gy+=2) for(int gx=-1;gx<=1;gx+=2) {
                        auto glow=options;
                        glow.x=std::max(0.0f,options.x+float(gx)*glowRadius);
                        glow.y=std::max(0.0f,options.y+float(gy)*glowRadius);
                        glow.red=0.55f; glow.green=0.82f; glow.blue=1.0f;
                        glow.opacity=glowAlpha;
                        const auto glowResult=vhs90::win::drawDirectWriteText(osdView,glow);
                        if(!glowResult.rendered) return PF_Err_INTERNAL_STRUCT_DAMAGED;
                    }
                }
                const auto result=vhs90::win::drawDirectWriteText(osdView,options);
                if(!result.rendered) return PF_Err_INTERNAL_STRUCT_DAMAGED;
                // Do not silently substitute an unrelated family when a selected
                // font is unavailable. Keep the readable DirectWrite fallback, but
                // visibly flag the missing family in the rendered OSD.
                if(!result.requestedFontFound) {
                    auto warning=options;
                    warning.utf8Text="FONT?";
                    warning.utf8FontFamily="Segoe UI";
                    warning.fontWeight=700;
                    warning.fontSize=std::max(9.0f,std::min(14.0f,options.fontSize*0.42f));
                    warning.x=std::max(0.0f,float(w)-54.0f);
                    warning.y=2.0f;
                    warning.maxWidth=52.0f;
                    warning.maxHeight=20.0f;
                    warning.red=1.0f; warning.green=0.24f; warning.blue=0.08f;
                    warning.opacity=0.95f;
                    warning.direction=vhs90::win::TextDirection::LeftToRight;
                    warning.alignment=vhs90::win::TextAlignment::Trailing;
                    warning.utf8FontFamily="Segoe UI";
                    (void)vhs90::win::drawDirectWriteText(osdView,warning);
                }
                return PF_Err_NONE;
            };
            const float textSize=7.0f*float(osdCell)*std::clamp(s.osdTextSize/100.0f,0.1f,5.0f);
            if(s.osdShowText) {
                if(s.osdRec>0.0f && s.osdShowRecLabel) {
                    const std::string rec=vhsOsdLabel(VhsOsdLabel::Record,language);
                    const float x=float(w)*s.osdRecX/100.0f+5.0f*float(osdCell);
                    const float y=float(h)*s.osdRecY/100.0f;
                    const PF_Err e=drawOsdText(rec,x,y,textSize,op.opacity*op.amount*op.recAmount,
                        vhs90::win::TextAlignment::Leading); if(e!=PF_Err_NONE) return e;
                }
                if(s.osdPlay>0.0f) {
                    bool showDeck=false;
                    VhsOsdLabel deck=VhsOsdLabel::Play;
                    switch(op.deckMode) {
                        case VhsDeckMode::Pause: deck=VhsOsdLabel::Pause; showDeck=s.osdShowPauseLabel!=0; break;
                        case VhsDeckMode::Stop: deck=VhsOsdLabel::Stop; showDeck=s.osdShowStopLabel!=0; break;
                        case VhsDeckMode::FastForward: deck=VhsOsdLabel::FastForward; showDeck=s.osdShowFfLabel!=0; break;
                        case VhsDeckMode::Rewind: deck=VhsOsdLabel::Rewind; showDeck=s.osdShowRewLabel!=0; break;
                        default: showDeck=s.osdShowPlayLabel!=0; break;
                    }
                    const float sx=float(w)*s.osdStatusX/100.0f;
                    const float sy=float(h)*s.osdStatusY/100.0f;
                    if(showDeck) {
                        const std::string status=vhsOsdLabel(deck,language);
                        const PF_Err e=drawOsdText(status,0.0f,sy,textSize,
                            op.opacity*op.amount*op.playAmount,vhs90::win::TextAlignment::Trailing,sx);
                        if(e!=PF_Err_NONE) return e;
                    }
                    if(s.osdShowSpeedLabel) {
                        const std::string speed=vhsOsdLabel(s.tapeSpeed==3?VhsOsdLabel::TapeSpeedEP:
                            (s.tapeSpeed==2?VhsOsdLabel::TapeSpeedLP:VhsOsdLabel::TapeSpeedSP),language);
                        const PF_Err se=drawOsdText(speed,0.0f,sy+textSize*1.25f,textSize*0.72f,
                            op.opacity*op.amount*op.playAmount,vhs90::win::TextAlignment::Trailing,sx);
                        if(se!=PF_Err_NONE) return se;
                    }
                }
                if(s.osdCounterAmount>0.0f && s.osdShowCounter) {
                    const std::string counter=formatVhsOsdCounter(s.frame,s.frameRate,s.osdCounterOffset,
                        static_cast<int>(std::clamp<A_long>(s.osdCounterFormat,1,3)),digits);
                    const float x=float(w)*s.osdCounterX/100.0f;
                    const float y=float(h)*s.osdCounterY/100.0f-textSize*0.72f;
                    const PF_Err e=drawOsdText(counter,x,y,textSize*0.72f,
                        op.opacity*op.amount*clamp01(s.osdCounterAmount/100.0f),
                        vhs90::win::TextAlignment::Leading); if(e!=PF_Err_NONE) return e;
                }
                if(s.osdTimestamp>0.0f && (s.osdShowDate || s.osdClockMode>1)) {
                    VhsOsdDateTime dt;
                    dt.year=static_cast<int>(std::lround(s.osdYear));
                    dt.month=static_cast<int>(std::lround(s.osdMonth));
                    dt.day=static_cast<int>(std::lround(s.osdDay));
                    dt.hour=static_cast<int>(std::lround(s.osdHour));
                    dt.minute=static_cast<int>(std::lround(s.osdMinute));
                    dt.second=static_cast<int>(std::lround(s.osdSecond));
                    dt.elapsedSeconds=s.timeSeconds*std::max(0.0f,s.osdTimeSpeed);
                    dt.clockRuns=s.osdClockRuns!=0;
                    dt.dateFormat=static_cast<VhsOsdDateFormat>(std::clamp<A_long>(s.osdDateLayout,1,4));
                    dt.timeMode=static_cast<VhsOsdTimeMode>(std::clamp<A_long>(s.osdClockMode,1,3)-1);
                    dt.monthStyle=static_cast<VhsOsdMonthStyle>(std::clamp<A_long>(s.osdMonthStyle,1,2)-1);
                    const std::string stamp=s.osdShowDate?formatVhsOsdDateTime(dt,language,digits):formatVhsOsdTime(dt,language,digits);
                    const float x=float(w)*s.osdTimestampX/100.0f;
                    const float y=float(h)*s.osdTimestampY/100.0f-textSize;
                    const PF_Err e=drawOsdText(stamp,0.0f,y,textSize,
                        op.opacity*op.amount*op.timestampAmount,vhs90::win::TextAlignment::Trailing,x);
                    if(e!=PF_Err_NONE) return e;
                }
                float wordY=float(h)*s.osdWordsY/100.0f;
                const float wordX=float(w)*s.osdWordsX/100.0f;
                auto drawWord=[&](bool enabled,VhsOsdLabel label)->PF_Err {
                    if(!enabled) return PF_Err_NONE;
                    const std::string word=vhsOsdLabel(label,language);
                    const PF_Err e=drawOsdText(word,wordX,wordY,textSize,op.opacity*op.amount,
                        vhs90::win::TextAlignment::Leading);
                    if(e==PF_Err_NONE) wordY+=textSize*1.25f;
                    return e;
                };
                PF_Err wordErr=drawWord(s.osdShowTapeLabel!=0,VhsOsdLabel::Tape); if(wordErr!=PF_Err_NONE)return wordErr;
                wordErr=drawWord(s.osdShowErrorLabel!=0,VhsOsdLabel::Error); if(wordErr!=PF_Err_NONE)return wordErr;
                wordErr=drawWord(s.osdShowProgramLabel!=0,VhsOsdLabel::Program01); if(wordErr!=PF_Err_NONE)return wordErr;
                wordErr=drawWord(s.osdShowResLabel!=0,VhsOsdLabel::Resolution); if(wordErr!=PF_Err_NONE)return wordErr;
                wordErr=drawWord(s.osdShowForwardLabel!=0,VhsOsdLabel::Forward); if(wordErr!=PF_Err_NONE)return wordErr;
                if(s.osdExtraLabel>=2&&s.osdExtraLabel<=18) {
                    static constexpr VhsOsdLabel kExtraLabels[]={
                        VhsOsdLabel::Tape,VhsOsdLabel::Error,VhsOsdLabel::Program01,VhsOsdLabel::Resolution,
                        VhsOsdLabel::Forward,VhsOsdLabel::Eject,VhsOsdLabel::Search,VhsOsdLabel::Tracking,
                        VhsOsdLabel::Auto,VhsOsdLabel::Channel,VhsOsdLabel::Camera,VhsOsdLabel::Deck,
                        VhsOsdLabel::Timer,VhsOsdLabel::Stereo,VhsOsdLabel::Slow,VhsOsdLabel::Still,VhsOsdLabel::Memory};
                    const std::string extra=vhsOsdLabel(kExtraLabels[s.osdExtraLabel-2],language);
                    const float x=float(w)*s.osdExtraX/100.0f;
                    const float y=float(h)*s.osdExtraY/100.0f;
                    const PF_Err e=drawOsdText(extra,x,y,textSize,op.opacity*op.amount,
                        vhs90::win::TextAlignment::Leading); if(e!=PF_Err_NONE) return e;
                }
                if(!s.osdText.empty()) {
                    const PF_Err e=drawOsdText(s.osdText,24.0f,24.0f,textSize,op.opacity*op.amount,
                        vhs90::win::TextAlignment::Leading); if(e!=PF_Err_NONE) return e;
                }
            }
        }
        if(compositeStages&&master>0.0f) {
            CompositeParams p;
            p.strength=master;
            // This selector is active only for the raster composite renderer.
            // Full Waveform NTSC uses its own source decoder implementation.
            p.demodulator=s.compositeDecoder==3?CompositeDemodulator::TwoLineComb:(s.compositeDecoder==2?CompositeDemodulator::OneLineComb:CompositeDemodulator::ConsumerNotch);
            p.geometry.roundedOverscan=clamp01(s.roundedOverscan/100.0f);
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
            if(s.composite>0) {
                const float compositeDrive=clamp01(s.composite/100.0f);
                const VHS90TapeSpeed tapeSpeed=(s.tapeSpeed==1)?VHS90TapeSpeed::SP:((s.tapeSpeed==2)?VHS90TapeSpeed::LP:VHS90TapeSpeed::EP);
                const YiqBandwidthParams tapeBandwidth=makeVhsTapeBandwidth(tapeSpeed);
                const auto blendCutoff=[compositeDrive](float clean,float tape){return clean+(tape-clean)*compositeDrive;};
                p.inputBandwidth.lumaCutoffHz=blendCutoff(4200000.0f,tapeBandwidth.lumaCutoffHz);
                p.inputBandwidth.iCutoffHz=blendCutoff(1300000.0f,tapeBandwidth.iCutoffHz);
                p.inputBandwidth.qCutoffHz=blendCutoff(600000.0f,tapeBandwidth.qCutoffHz);
                p.outputBandwidth.lumaCutoffHz=blendCutoff(4200000.0f,tapeBandwidth.lumaCutoffHz*0.90f);
                p.outputBandwidth.iCutoffHz=blendCutoff(1300000.0f,tapeBandwidth.iCutoffHz*0.90f);
                p.outputBandwidth.qCutoffHz=blendCutoff(600000.0f,tapeBandwidth.qCutoffHz*0.90f);
            }
            if(s.composite<=0) { p.outputBandwidth.iCutoffHz=0.0f; p.outputBandwidth.qCutoffHz=0.0f; p.outputBandwidth.lumaCutoffHz=0.0f; }
            p.palMode=(s.colorSystem==2);
            if(s.ringing>0) { p.ringingEnabled=true; p.ringing.strength=clamp01(s.ringing/100.0f); p.ringing.power=s.ringingPower; }
            if(s.crtEqualizer>0) { p.crtEqualizerEnabled=true; p.crtEqualizer.amount=clamp01(s.crtEqualizer/100.0f); }
            // The full raw-waveform simulator currently models NTSC 525/60 only.
            p.waveformCompositeEnabled=(s.signalModel==2 && s.colorSystem==1);
            if(!previousPixels.empty() && (s.temporalGhost>0||s.interlace>0||(s.frameRepeatFrequency>0&&s.frameRepeatStrength>0))) {
                p.temporalGhostEnabled=true;
                p.previousFrame={previousPixels.data(),w,h,w*4};
                p.previousFrameIndex=s.frame-1;
                p.temporalGhost.amount=std::max(clamp01(s.temporalGhost/100.0f)*0.35f,clamp01(s.interlace/100.0f)*0.35f);
                p.temporalGhost.lumaAmount=1.0f;
                p.temporalGhost.chromaAmount=0.8f;
                p.temporalGhost.offsetXPixels=2.0f;
                p.temporalGhost.redFringePixels=0.6f;
                p.temporalGhost.blueFringePixels=-0.6f;
                p.temporalGhost.fieldWeave=clamp01(s.interlace/100.0f);p.temporalGhost.fieldParity=static_cast<int>(p.frame&1);
                p.temporalGhost.frameRepeatFrequency=clamp01(s.frameRepeatFrequency/100.0f);
                p.temporalGhost.frameRepeatStrength=clamp01(s.frameRepeatStrength/100.0f);
                p.temporalGhost.frame=s.frame;p.temporalGhost.seed=static_cast<std::uint32_t>(std::max(1.0f,s.tapeSeed));
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
            if(s.geometry>0||s.interlace>0||s.verticalRoll>0||s.edgeWave>0||s.vhs43Frame!=0||s.lensDistortion!=0||s.digitalZoom>1.0f) {
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
                p.geometry.vhs43Frame=s.vhs43Frame!=0;p.geometry.lensDistortion=s.lensDistortion/1000.0f;
                p.geometry.digitalZoom=std::max(1.0f,s.digitalZoom);
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
                tc.pinholes=s.tapePinholes; tc.flakes=s.tapeFlakes; tc.surfaceRoughness=s.tapeRoughness;
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
            if(s.edgeEnhance>0||s.chromaEdgeEnhance>0||s.tapeFade>0||s.gradeExposure!=0||s.gradeContrast!=100||s.gradeBrightness!=0||s.gradeSaturation!=100||s.chromaIGain!=100||s.chromaQGain!=100||s.chromaIOffset!=0||s.chromaQOffset!=0||s.chromaQuantization>0||s.vignette>0||s.flicker>0) {
                p.imageGradeEnabled=true; p.imageGrade.seed=0x90A11E5Eu; p.imageGrade.frame=s.frame;
                p.imageGrade.edgeEnhancement=clamp01(s.edgeEnhance/100.0f);
                p.imageGrade.chromaEdgeEnhancement=clamp01(s.chromaEdgeEnhance/100.0f);
                p.imageGrade.chromaSharpenCutoffHz=s.tapeSpeed==3?560000.0f:(s.tapeSpeed==2?600000.0f:640000.0f);
                p.imageGrade.fade=clamp01(s.tapeFade/100.0f);
                p.imageGrade.blackLift=clamp01(s.blackLift/100.0f);
                p.imageGrade.exposureEV=s.gradeExposure; p.imageGrade.contrast=s.gradeContrast/100.0f;
                p.imageGrade.brightness=s.gradeBrightness/255.0f;
                p.imageGrade.saturation=s.gradeSaturation/100.0f;
                p.imageGrade.chromaIGain=s.chromaIGain/100.0f;p.imageGrade.chromaQGain=s.chromaQGain/100.0f;
                p.imageGrade.chromaIOffset=s.chromaIOffset/200.0f;p.imageGrade.chromaQOffset=s.chromaQOffset/200.0f;
                p.imageGrade.chromaQuantization=s.chromaQuantization/2000.0f;
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
            if(s.rfDrift>0||s.rfHum>0||s.rfAm>0||s.agcPump>0) {
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
                p.bloomEnabled=true; p.bloom.threshold=0.55f; p.bloom.intensity=clamp01(s.bloom/100.0f);
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
            StaticParams p; p.mode=static_cast<StaticPatternMode>(std::clamp<A_long>(s.staticMode-1,0,12));
            p.angleDegrees=s.staticAngle; p.ribbonWidth=clamp01(s.staticRibbonWidth/100.0f); p.greenLineAmount=clamp01(s.staticGreenLine/100.0f); p.phaseCount=static_cast<int>(std::lround(s.staticPhaseCount));
            p.intensity=clamp01(s.staticAmount/100.0f*master);
            p.sourceMix=1.0f; p.density=clamp01(s.staticDensity/100.0f); p.seed=0x905A71C1u;
            p.frame=s.frame; p.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processStaticPattern(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        
        if(master>0.0f&&s.tearChromaAmount>0.0f) {
            TearBand bands[5]; const int bandCount=std::clamp<A_long>(static_cast<A_long>(std::lround(s.tearBandCount)),1,5);
            TearBand& baseBand=bands[0]; baseBand.centerX=0.5f;
            baseBand.centerY=clamp01((s.tearBandStartY+s.tearBandHeight*0.5f)/100.0f);
            baseBand.width=1.0f; baseBand.height=clamp01(s.tearBandHeight/100.0f); baseBand.intensity=1.0f;
            baseBand.horizontalShiftPx=s.tearDisplacement; baseBand.waveAmplitudePx=std::fabs(s.tearDisplacement)*0.25f;
            baseBand.fragmentation=clamp01(s.tearFragmentation/100.0f); baseBand.chromaShiftPx=s.tearChromaShift;
            baseBand.smearPx=s.tearSmear; baseBand.dropoutProbability=clamp01(s.tearDropout/100.0f); baseBand.flicker=0.12f;
            baseBand.seedOffset=0x9e3779b9u;
            for(int bi=1;bi<bandCount;++bi) {
                const int li=bi-1; TearBand& b=bands[bi]; b.centerX=0.5f; b.centerY=clamp01(s.tearBandY[li]/100.0f);
                b.width=1.0f; b.height=clamp01(s.tearBandHeightExtra[li]/100.0f);
                b.intensity=clamp01(s.tearBandAmount[li]/100.0f);
                b.horizontalShiftPx=s.tearBandShift[li]; b.waveAmplitudePx=std::fabs(s.tearBandShift[li])*0.25f;
                b.fragmentation=clamp01(s.tearBandFragmentation[li]/100.0f); b.chromaShiftPx=s.tearBandChroma[li];
                b.smearPx=s.tearBandSmear[li]; b.dropoutProbability=clamp01(s.tearBandDropout[li]/100.0f);
                b.flicker=0.12f; b.seedOffset=static_cast<std::uint32_t>(bi*0x9e3779b9u);
            }
            TearChromaParams tp; tp.mode=static_cast<TearChromaMode>(std::clamp<A_long>(s.tearChromaMode-1,0,4));
            tp.intensity=clamp01(s.tearChromaAmount/100.0f*master); tp.spectralIntensity=clamp01(s.spectralSmear/100.0f);
            tp.spectralHueSpread=clamp01(s.spectralHueSpread/100.0f); tp.chromaSaturation=s.chromaSaturation/100.0f;
            tp.spectralCenterY=clamp01(s.spectralCenterY/100.0f); tp.spectralCoverage=clamp01(s.spectralCoverage/100.0f);
            tp.spectralSmearX=s.tearSmear*2.0f; tp.bands=bands; tp.bandCount=static_cast<std::size_t>(bandCount);
            tp.seed=static_cast<std::uint32_t>(std::clamp(std::lround(s.tearSeed),1l,32767l)); tp.frame=s.frame; tp.timeSeconds=s.timeSeconds;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processTearChroma(in,out,tp)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&s.sync>0) {
            SyncCollapseParams p; p.strength=clamp01(s.sync/100.0f*master);
            p.noiseAmount=0.0f; // Grain belongs to the dedicated Grain engine, never Sync Collapse.
            p.frame=s.frame; p.timeSeconds=s.timeSeconds; p.seed=0x90C011A5u;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processSyncCollapse(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&s.syncDamageEnable>0) {
            SyncCollapseLabParams p;
            p.enabled=true;
            p.lumaEnabled=s.syncDamageLumaEnable!=0;
            p.dropoutsEnabled=s.syncDamageDropoutsEnable!=0;
            p.chromaEnabled=s.syncDamageChromaEnable!=0;
            p.animationMode=static_cast<SyncDamageAnimationMode>(std::clamp<A_long>(s.syncDamageAnimationMode-1,0,2));
            p.inputMode=static_cast<SyncDamageInputMode>(std::clamp<A_long>(s.syncDamageInputMode-1,0,1));
            p.effectAmount=clamp01(s.syncDamageEffectAmount/100.0f*master);
            p.horizontalTearAmount=clamp01(s.syncDamageTearAmount/100.0f);
            p.bandIntensity=clamp01(s.syncDamageBandIntensity/100.0f);
            p.bandDensity=std::clamp(s.syncDamageBandDensity,1.0f,32.0f);
            p.bandHeight=clamp01(s.syncDamageBandHeight/100.0f);
            p.bandIrregularity=clamp01(s.syncDamageBandIrregularity/100.0f);
            p.horizontalDisplacementPx=std::clamp(s.syncDamageDisplacement,0.0f,160.0f);
            p.boundaryFragmentation=clamp01(s.syncDamageBoundaryFragmentation/100.0f);
            p.fullFrameCoverage=clamp01(s.syncDamageCoverage/100.0f);
            p.lumaInstability=clamp01(s.syncDamageLumaInstability/100.0f);
            p.brightFlashIntensity=clamp01(s.syncDamageBrightFlash/100.0f);
            p.darkCollapseIntensity=clamp01(s.syncDamageDarkCollapse/100.0f);
            p.eventFrequencyHz=std::clamp(s.syncDamageEventFrequency,0.1f,10.0f);
            p.eventDurationSeconds=std::clamp(s.syncDamageEventDuration,0.05f,2.0f);
            p.brightEventProbability=clamp01(s.syncDamageBrightProbability/100.0f);
            p.darkEventProbability=clamp01(s.syncDamageDarkProbability/100.0f);
            p.transitionSharpness=clamp01(s.syncDamageTransitionSharpness/100.0f);
            p.localBandLumaVariation=clamp01(s.syncDamageLocalLuma/100.0f);
            p.manualBrightPulse=clamp01(s.syncDamageManualBright/100.0f);
            p.manualDarkCollapse=clamp01(s.syncDamageManualDark/100.0f);
            p.dropoutStreakDensity=clamp01(s.syncDamageDropoutDensity/100.0f);
            p.streakLengthPx=std::clamp(s.syncDamageStreakLength,2.0f,120.0f);
            p.streakBrightness=clamp01(s.syncDamageStreakBrightness/100.0f);
            p.darkDropoutAmount=clamp01(s.syncDamageDarkDropout/100.0f);
            p.horizontalFragmentation=clamp01(s.syncDamageHorizontalFragmentation/100.0f);
            p.temporalVariation=clamp01(s.syncDamageTemporalVariation/100.0f);
            p.subtleChromaTint=clamp01(s.syncDamageChromaTint/100.0f);
            p.magentaTint=clamp01(s.syncDamageMagentaTint/100.0f);
            p.greenTint=clamp01(s.syncDamageGreenTint/100.0f);
            p.rgbOffsetPx=std::clamp(s.syncDamageRgbOffset,0.0f,8.0f);
            p.chromaBleed=clamp01(s.syncDamageChromaBleed/100.0f);
            p.animationSpeed=std::clamp(s.syncDamageAnimationSpeed/100.0f,0.0f,4.0f);
            p.seed=static_cast<std::uint32_t>(std::clamp<A_long>(static_cast<A_long>(std::lround(s.syncDamageRandomSeed)),1,32767));
            p.blendWithInput=clamp01(s.syncDamageBlendInput/100.0f);
            p.frame=s.frame; p.timeSeconds=s.timeSeconds; p.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processSyncCollapseLab(in,out,p)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        if(master>0.0f&&(s.trackingAmount>0||s.trackingLayerAmount[0]>0||s.trackingLayerAmount[1]>0||s.trackingLayerAmount[2]>0||s.headAmount>0)) {
            DamageRegion regions[5]; std::size_t count=0;
            if(s.trackingAmount>0) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::TrackingNoise;
                r.shape=static_cast<RegionShape>(std::max<A_long>(1,std::min<A_long>(3,s.trackingShape))-1);
                r.centerX=clamp01(s.trackingX/100.0f); r.centerY=clamp01(s.trackingY/100.0f);
                r.angleRadians=s.trackingAngle*0.01745329251994329577f;
                r.width=clamp01(s.trackingWidth/100.0f); r.height=clamp01(s.trackingHeight/100.0f);
                r.intensity=clamp01(s.trackingAmount/100.0f*master);
                r.motionXPerSecond=s.trackingMotionX/100.0f; r.motionYPerSecond=s.trackingMotionY/100.0f;
                r.servoDriftPx=s.trackingServo; r.tearProbability=clamp01(s.trackingTear/100.0f);
                r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingProfile-1,0,4));
                ApplyTrackingRecipe(r,s.trackingPresetBase);
            }
            if(s.trackingLayerAmount[0]>0.0f) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::TrackingNoise;
                r.shape=static_cast<RegionShape>(std::clamp<A_long>(s.trackingLayerShape[0]-1,0,2));
                r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingLayerProfile[0]-1,0,4));
                r.centerX=clamp01(s.trackingLayerX[0]/100.0f); r.centerY=clamp01(s.trackingLayerY[0]/100.0f);
                r.angleRadians=s.trackingLayerAngle[0]*0.01745329251994329577f;
                r.motionXPerSecond=s.trackingLayerMotionX[0]/100.0f; r.motionYPerSecond=s.trackingLayerMotionY[0]/100.0f;
                r.width=clamp01(s.trackingLayerWidth[0]/100.0f); r.height=clamp01(s.trackingLayerHeight[0]/100.0f);
                r.intensity=clamp01(s.trackingLayerAmount[0]/100.0f*master); r.servoDriftPx=s.trackingLayerServo[0];
                r.maxShiftPx=s.trackingLayerShift[0]; r.waveAmplitudePx=s.trackingLayerWave[0];
                r.tearProbability=clamp01(s.trackingLayerTear[0]/100.0f); r.tearShiftPx=s.trackingLayerShift[0]*1.5f;
                r.noiseDensity=0.22f; r.noiseAmount=0.12f; r.seedOffset=0x1000u; ApplyTrackingRecipe(r,s.trackingLayerPreset[0]);
            }
            if(s.trackingLayerAmount[1]>0.0f) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::TrackingNoise;
                r.shape=static_cast<RegionShape>(std::clamp<A_long>(s.trackingLayerShape[1]-1,0,2));
                r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingLayerProfile[1]-1,0,4));
                r.centerX=clamp01(s.trackingLayerX[1]/100.0f); r.centerY=clamp01(s.trackingLayerY[1]/100.0f);
                r.angleRadians=s.trackingLayerAngle[1]*0.01745329251994329577f;
                r.motionXPerSecond=s.trackingLayerMotionX[1]/100.0f; r.motionYPerSecond=s.trackingLayerMotionY[1]/100.0f;
                r.width=clamp01(s.trackingLayerWidth[1]/100.0f); r.height=clamp01(s.trackingLayerHeight[1]/100.0f);
                r.intensity=clamp01(s.trackingLayerAmount[1]/100.0f*master); r.servoDriftPx=s.trackingLayerServo[1];
                r.maxShiftPx=s.trackingLayerShift[1]; r.waveAmplitudePx=s.trackingLayerWave[1];
                r.tearProbability=clamp01(s.trackingLayerTear[1]/100.0f); r.tearShiftPx=s.trackingLayerShift[1]*1.5f;
                r.noiseDensity=0.22f; r.noiseAmount=0.12f; r.seedOffset=0x1100u; ApplyTrackingRecipe(r,s.trackingLayerPreset[1]);
            }
            if(s.trackingLayerAmount[2]>0.0f) {
                DamageRegion& r=regions[count++]; r.kind=RegionKind::TrackingNoise;
                r.shape=static_cast<RegionShape>(std::clamp<A_long>(s.trackingLayerShape[2]-1,0,2));
                r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingLayerProfile[2]-1,0,4));
                r.centerX=clamp01(s.trackingLayerX[2]/100.0f); r.centerY=clamp01(s.trackingLayerY[2]/100.0f);
                r.angleRadians=s.trackingLayerAngle[2]*0.01745329251994329577f;
                r.motionXPerSecond=s.trackingLayerMotionX[2]/100.0f; r.motionYPerSecond=s.trackingLayerMotionY[2]/100.0f;
                r.width=clamp01(s.trackingLayerWidth[2]/100.0f); r.height=clamp01(s.trackingLayerHeight[2]/100.0f);
                r.intensity=clamp01(s.trackingLayerAmount[2]/100.0f*master); r.servoDriftPx=s.trackingLayerServo[2];
                r.maxShiftPx=s.trackingLayerShift[2]; r.waveAmplitudePx=s.trackingLayerWave[2];
                r.tearProbability=clamp01(s.trackingLayerTear[2]/100.0f); r.tearShiftPx=s.trackingLayerShift[2]*1.5f;
                r.noiseDensity=0.22f; r.noiseAmount=0.12f; r.seedOffset=0x1200u; ApplyTrackingRecipe(r,s.trackingLayerPreset[2]);
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
        if(master>0.0f&&(s.redChannelOffset!=0.0f||s.greenChannelOffset!=0.0f||s.blueChannelOffset!=0.0f)) {
            RgbChannelRegistrationParams rp; rp.redOffsetPixels=s.redChannelOffset*master/100.0f;
            rp.greenOffsetPixels=s.greenChannelOffset*master/100.0f; rp.blueOffsetPixels=s.blueChannelOffset*master/100.0f;
            rp.mode=static_cast<RgbRegistrationMode>(std::clamp<A_long>(s.chromaRegMode-1,0,3)); rp.premultipliedAlpha=true;
            ConstImageView in{working.data(),w,h,w*4}; ImageView out{scratch.data(),w,h,w*4};
            if(!processRgbChannelRegistration(in,out,rp)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            working.swap(scratch);
        }
        for(int y=0;y<output->height;++y) for(int x=0;x<output->width;++x) {
            const int sx=output->origin_x+x-input->origin_x, sy=output->origin_y+y-input->origin_y;
            if(sx<0||sx>=w||sy<0||sy>=h) { const float z[4]={0,0,0,0}; WritePixel(output,format,x,y,z); continue; }
            const std::size_t pixelOffset=(std::size_t(sy)*w+sx)*4;
            const float* processed=working.data()+pixelOffset;
            if(masterOverdrive>1.0f) {
                const float* original=source.data()+pixelOffset;
                const float boosted[4]={
                    original[0]+(processed[0]-original[0])*masterOverdrive,
                    original[1]+(processed[1]-original[1])*masterOverdrive,
                    original[2]+(processed[2]-original[2])*masterOverdrive,
                    processed[3]
                };
                WritePixel(output,format,x,y,boosted);
            } else {
                WritePixel(output,format,x,y,processed);
            }
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
    auto unionRect=[](PF_Rect& dst,const PF_Rect& srcRect) {
        if(srcRect.right<=srcRect.left || srcRect.bottom<=srcRect.top) return;
        if(dst.right<=dst.left || dst.bottom<=dst.top) { dst=srcRect; return; }
        dst.left=std::min(dst.left,srcRect.left);
        dst.top=std::min(dst.top,srcRect.top);
        dst.right=std::max(dst.right,srcRect.right);
        dst.bottom=std::max(dst.bottom,srcRect.bottom);
    };
    extra->output->result_rect=result.result_rect;
    extra->output->max_result_rect=result.max_result_rect;
    unionRect(extra->output->max_result_rect,extra->output->result_rect);
    // Checkout the immediately preceding source frame through SmartFX's declared
    // checkout ID; no global frame cache is used, so MFR/out-of-order renders stay safe.
    if((s->temporalGhost>0||s->interlace>0||(s->frameRepeatFrequency>0&&s->frameRepeatStrength>0)) && in_data->time_step>0 && in_data->current_time>=in_data->time_step) {
        PF_CheckoutResult previousResult; AEFX_CLR_STRUCT(previousResult);
        const A_long previousTime=in_data->current_time-in_data->time_step;
        const PF_Err previousErr=extra->cb->checkout_layer(in_data->effect_ref,PARAM_INPUT,kInputCheckoutID+1,&req,
            previousTime,in_data->time_step,in_data->time_scale,&previousResult);
        s->previousFrameAvailable=(previousErr==PF_Err_NONE);
        if(s->previousFrameAvailable) {
            // The temporal checkout is another input dependency. Union its
            // valid rectangles into the output contract as in Adobe's
            // SmartFX SmartyPants sample.
            unionRect(extra->output->result_rect,previousResult.result_rect);
            unionRect(extra->output->max_result_rect,previousResult.max_result_rect);
            unionRect(extra->output->max_result_rect,extra->output->result_rect);
        }
    }
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
    s.compositeDecoder=params[PARAM_DECODER_MODE]->u.pd.value;
    s.frameRepeatFrequency=float(params[PARAM_FRAME_REPEAT_FREQUENCY]->u.fs_d.value);
    s.frameRepeatStrength=float(params[PARAM_FRAME_REPEAT_STRENGTH]->u.fs_d.value);
    s.tapeDetail=float(params[PARAM_TAPE_DETAIL]->u.fs_d.value);
    s.trackRead=float(params[PARAM_TRACK_READ]->u.fs_d.value);
    s.headCondition=float(params[PARAM_HEAD_CONDITION]->u.fs_d.value);
    s.tapeWear=float(params[PARAM_TAPE_WEAR]->u.fs_d.value); s.tapeDropouts=float(params[PARAM_TAPE_DROPOUTS]->u.fs_d.value); s.tapeScratches=float(params[PARAM_TAPE_SCRATCHES]->u.fs_d.value);
    s.tapeCreases=float(params[PARAM_TAPE_CREASES]->u.fs_d.value); s.tapeEdge=float(params[PARAM_TAPE_EDGE]->u.fs_d.value); s.tapeMold=float(params[PARAM_TAPE_MOLD]->u.fs_d.value);
    s.tapeVaseline=float(params[PARAM_TAPE_VASELINE]->u.fs_d.value); s.tapeSticky=float(params[PARAM_TAPE_STICKY]->u.fs_d.value); s.tapePinholes=float(params[PARAM_TAPE_PINHOLES]->u.fs_d.value); s.tapeFlakes=float(params[PARAM_TAPE_FLAKES]->u.fs_d.value); s.tapeRoughness=float(params[PARAM_TAPE_ROUGHNESS]->u.fs_d.value); s.tapeSeed=float(params[PARAM_TAPE_SEED]->u.fs_d.value); s.backTension=float(params[PARAM_TAPE_BACK_TENSION]->u.fs_d.value);
    s.trackMisalignment=float(params[PARAM_TRACK_MISALIGNMENT]->u.fs_d.value);
    s.redChannelOffset=float(params[PARAM_CHROMA_REG_RED]->u.fs_d.value);
    s.greenChannelOffset=float(params[PARAM_CHROMA_REG_GREEN]->u.fs_d.value);
    s.blueChannelOffset=float(params[PARAM_CHROMA_REG_BLUE]->u.fs_d.value);
    s.chromaRegMode=params[PARAM_CHROMA_REG_MODE]->u.pd.value;
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
    s.chromaEdgeEnhance=float(params[PARAM_CHROMA_EDGE_ENHANCE]->u.fs_d.value);
    s.tapeFade=float(params[PARAM_TAPE_FADE]->u.fs_d.value);
    s.blackLift=float(params[PARAM_BLACK_LIFT]->u.fs_d.value);
    s.temporalGhost=float(params[PARAM_TEMPORAL_GHOST]->u.fs_d.value);
    s.ringing=float(params[PARAM_RINGING]->u.fs_d.value);
    s.ringingPower=float(params[PARAM_RINGING_POWER]->u.fs_d.value);
    s.crtEqualizer=float(params[PARAM_CRT_EQUALIZER]->u.fs_d.value);
    s.gradeExposure=float(params[PARAM_GRADE_EXPOSURE]->u.fs_d.value); s.gradeContrast=float(params[PARAM_GRADE_CONTRAST]->u.fs_d.value);
    s.gradeBrightness=float(params[PARAM_GRADE_BRIGHTNESS]->u.fs_d.value);
    s.gradeSaturation=float(params[PARAM_GRADE_SATURATION]->u.fs_d.value);
    s.chromaIGain=float(params[PARAM_GRADE_I_GAIN]->u.fs_d.value);
    s.chromaQGain=float(params[PARAM_GRADE_Q_GAIN]->u.fs_d.value);
    s.chromaIOffset=float(params[PARAM_GRADE_I_OFFSET]->u.fs_d.value);
    s.chromaQOffset=float(params[PARAM_GRADE_Q_OFFSET]->u.fs_d.value);
    s.chromaQuantization=float(params[PARAM_GRADE_CHROMA_QUANTIZATION]->u.fs_d.value);
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
    s.staticMode=params[PARAM_PATTERN_MODE]->u.pd.value; s.staticAngle=float(params[PARAM_PATTERN_ANGLE]->u.fs_d.value);
    s.staticRibbonWidth=float(params[PARAM_PATTERN_WIDTH]->u.fs_d.value); s.staticGreenLine=float(params[PARAM_PATTERN_GREEN]->u.fs_d.value); s.staticPhaseCount=float(params[PARAM_PATTERN_PHASE_COUNT]->u.fs_d.value);
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
    s.trackingProfile=params[PARAM_TRACKING_PROFILE]->u.pd.value;
    s.trackingPresetBase=params[PARAM_TRACKING_PRESET_BASE]->u.pd.value;
    s.trackingLayerPreset[0]=params[PARAM_TRACKING_PRESET_2]->u.pd.value; s.trackingLayerPreset[1]=params[PARAM_TRACKING_PRESET_3]->u.pd.value; s.trackingLayerPreset[2]=params[PARAM_TRACKING_PRESET_4]->u.pd.value;
    s.tearChromaAmount=float(params[PARAM_TEAR_CHROMA_AMOUNT]->u.fs_d.value); s.tearChromaMode=params[PARAM_TEAR_CHROMA_MODE]->u.pd.value;
    s.spectralSmear=float(params[PARAM_SPECTRAL_SMEAR]->u.fs_d.value); s.spectralHueSpread=float(params[PARAM_SPECTRAL_HUE_SPREAD]->u.fs_d.value);
    s.chromaSaturation=float(params[PARAM_CHROMA_SATURATION]->u.fs_d.value); s.spectralCenterY=float(params[PARAM_SPECTRAL_CENTER_Y]->u.fs_d.value);
    s.spectralCoverage=float(params[PARAM_SPECTRAL_COVERAGE]->u.fs_d.value); s.tearBandStartY=float(params[PARAM_TEAR_BAND_START_Y]->u.fs_d.value);
    s.tearBandHeight=float(params[PARAM_TEAR_BAND_HEIGHT]->u.fs_d.value); s.tearBandCount=params[PARAM_TEAR_BAND_COUNT]->u.fs_d.value;
    s.tearDisplacement=float(params[PARAM_TEAR_DISPLACEMENT]->u.fs_d.value); s.tearFragmentation=float(params[PARAM_TEAR_FRAGMENTATION]->u.fs_d.value);
    s.tearChromaShift=float(params[PARAM_TEAR_CHROMA_SHIFT]->u.fs_d.value); s.tearSmear=float(params[PARAM_TEAR_SMEAR]->u.fs_d.value);
    s.tearDropout=float(params[PARAM_TEAR_DROPOUT]->u.fs_d.value); s.tearSeed=float(params[PARAM_TEAR_SEED]->u.fs_d.value);
    s.tearBandAmount[0]=float(params[PARAM_TEAR_BAND2_AMOUNT]->u.fs_d.value); s.tearBandY[0]=float(params[PARAM_TEAR_BAND2_Y]->u.fs_d.value); s.tearBandHeightExtra[0]=float(params[PARAM_TEAR_BAND2_HEIGHT]->u.fs_d.value);
    s.tearBandShift[0]=float(params[PARAM_TEAR_BAND2_SHIFT]->u.fs_d.value); s.tearBandFragmentation[0]=float(params[PARAM_TEAR_BAND2_FRAGMENTATION]->u.fs_d.value);
    s.tearBandChroma[0]=float(params[PARAM_TEAR_BAND2_CHROMA]->u.fs_d.value); s.tearBandSmear[0]=float(params[PARAM_TEAR_BAND2_SMEAR]->u.fs_d.value); s.tearBandDropout[0]=float(params[PARAM_TEAR_BAND2_DROPOUT]->u.fs_d.value);
    s.tearBandAmount[1]=float(params[PARAM_TEAR_BAND3_AMOUNT]->u.fs_d.value); s.tearBandY[1]=float(params[PARAM_TEAR_BAND3_Y]->u.fs_d.value); s.tearBandHeightExtra[1]=float(params[PARAM_TEAR_BAND3_HEIGHT]->u.fs_d.value);
    s.tearBandShift[1]=float(params[PARAM_TEAR_BAND3_SHIFT]->u.fs_d.value); s.tearBandFragmentation[1]=float(params[PARAM_TEAR_BAND3_FRAGMENTATION]->u.fs_d.value);
    s.tearBandChroma[1]=float(params[PARAM_TEAR_BAND3_CHROMA]->u.fs_d.value); s.tearBandSmear[1]=float(params[PARAM_TEAR_BAND3_SMEAR]->u.fs_d.value); s.tearBandDropout[1]=float(params[PARAM_TEAR_BAND3_DROPOUT]->u.fs_d.value);
    s.tearBandAmount[2]=float(params[PARAM_TEAR_BAND4_AMOUNT]->u.fs_d.value); s.tearBandY[2]=float(params[PARAM_TEAR_BAND4_Y]->u.fs_d.value); s.tearBandHeightExtra[2]=float(params[PARAM_TEAR_BAND4_HEIGHT]->u.fs_d.value);
    s.tearBandShift[2]=float(params[PARAM_TEAR_BAND4_SHIFT]->u.fs_d.value); s.tearBandFragmentation[2]=float(params[PARAM_TEAR_BAND4_FRAGMENTATION]->u.fs_d.value);
    s.tearBandChroma[2]=float(params[PARAM_TEAR_BAND4_CHROMA]->u.fs_d.value); s.tearBandSmear[2]=float(params[PARAM_TEAR_BAND4_SMEAR]->u.fs_d.value); s.tearBandDropout[2]=float(params[PARAM_TEAR_BAND4_DROPOUT]->u.fs_d.value);
    s.tearBandAmount[3]=float(params[PARAM_TEAR_BAND5_AMOUNT]->u.fs_d.value); s.tearBandY[3]=float(params[PARAM_TEAR_BAND5_Y]->u.fs_d.value); s.tearBandHeightExtra[3]=float(params[PARAM_TEAR_BAND5_HEIGHT]->u.fs_d.value);
    s.tearBandShift[3]=float(params[PARAM_TEAR_BAND5_SHIFT]->u.fs_d.value); s.tearBandFragmentation[3]=float(params[PARAM_TEAR_BAND5_FRAGMENTATION]->u.fs_d.value);
    s.tearBandChroma[3]=float(params[PARAM_TEAR_BAND5_CHROMA]->u.fs_d.value); s.tearBandSmear[3]=float(params[PARAM_TEAR_BAND5_SMEAR]->u.fs_d.value); s.tearBandDropout[3]=float(params[PARAM_TEAR_BAND5_DROPOUT]->u.fs_d.value);
    s.trackingLayerAmount[0]=float(params[PARAM_TRACK_LAYER2_AMOUNT]->u.fs_d.value);
    s.trackingLayerProfile[0]=params[PARAM_TRACK_LAYER2_PROFILE]->u.pd.value; s.trackingLayerShape[0]=params[PARAM_TRACK_LAYER2_SHAPE]->u.pd.value;
    s.trackingLayerX[0]=float(params[PARAM_TRACK_LAYER2_X]->u.fs_d.value); s.trackingLayerY[0]=float(params[PARAM_TRACK_LAYER2_Y]->u.fs_d.value);
    s.trackingLayerWidth[0]=float(params[PARAM_TRACK_LAYER2_WIDTH]->u.fs_d.value); s.trackingLayerHeight[0]=float(params[PARAM_TRACK_LAYER2_HEIGHT]->u.fs_d.value);
    s.trackingLayerServo[0]=float(params[PARAM_TRACK_LAYER2_SERVO]->u.fs_d.value); s.trackingLayerShift[0]=float(params[PARAM_TRACK_LAYER2_SHIFT]->u.fs_d.value);
    s.trackingLayerWave[0]=float(params[PARAM_TRACK_LAYER2_WAVE]->u.fs_d.value); s.trackingLayerTear[0]=float(params[PARAM_TRACK_LAYER2_TEAR]->u.fs_d.value);
    s.trackingLayerAngle[0]=float(params[PARAM_TRACK_LAYER2_ANGLE]->u.fs_d.value); s.trackingLayerMotionX[0]=float(params[PARAM_TRACK_LAYER2_MOTION_X]->u.fs_d.value); s.trackingLayerMotionY[0]=float(params[PARAM_TRACK_LAYER2_MOTION_Y]->u.fs_d.value);
    s.trackingLayerAmount[1]=float(params[PARAM_TRACK_LAYER3_AMOUNT]->u.fs_d.value);
    s.trackingLayerProfile[1]=params[PARAM_TRACK_LAYER3_PROFILE]->u.pd.value; s.trackingLayerShape[1]=params[PARAM_TRACK_LAYER3_SHAPE]->u.pd.value;
    s.trackingLayerX[1]=float(params[PARAM_TRACK_LAYER3_X]->u.fs_d.value); s.trackingLayerY[1]=float(params[PARAM_TRACK_LAYER3_Y]->u.fs_d.value);
    s.trackingLayerWidth[1]=float(params[PARAM_TRACK_LAYER3_WIDTH]->u.fs_d.value); s.trackingLayerHeight[1]=float(params[PARAM_TRACK_LAYER3_HEIGHT]->u.fs_d.value);
    s.trackingLayerServo[1]=float(params[PARAM_TRACK_LAYER3_SERVO]->u.fs_d.value); s.trackingLayerShift[1]=float(params[PARAM_TRACK_LAYER3_SHIFT]->u.fs_d.value);
    s.trackingLayerWave[1]=float(params[PARAM_TRACK_LAYER3_WAVE]->u.fs_d.value); s.trackingLayerTear[1]=float(params[PARAM_TRACK_LAYER3_TEAR]->u.fs_d.value);
    s.trackingLayerAngle[1]=float(params[PARAM_TRACK_LAYER3_ANGLE]->u.fs_d.value); s.trackingLayerMotionX[1]=float(params[PARAM_TRACK_LAYER3_MOTION_X]->u.fs_d.value); s.trackingLayerMotionY[1]=float(params[PARAM_TRACK_LAYER3_MOTION_Y]->u.fs_d.value);
    s.trackingLayerAmount[2]=float(params[PARAM_TRACK_LAYER4_AMOUNT]->u.fs_d.value);
    s.trackingLayerProfile[2]=params[PARAM_TRACK_LAYER4_PROFILE]->u.pd.value; s.trackingLayerShape[2]=params[PARAM_TRACK_LAYER4_SHAPE]->u.pd.value;
    s.trackingLayerX[2]=float(params[PARAM_TRACK_LAYER4_X]->u.fs_d.value); s.trackingLayerY[2]=float(params[PARAM_TRACK_LAYER4_Y]->u.fs_d.value);
    s.trackingLayerWidth[2]=float(params[PARAM_TRACK_LAYER4_WIDTH]->u.fs_d.value); s.trackingLayerHeight[2]=float(params[PARAM_TRACK_LAYER4_HEIGHT]->u.fs_d.value);
    s.trackingLayerServo[2]=float(params[PARAM_TRACK_LAYER4_SERVO]->u.fs_d.value); s.trackingLayerShift[2]=float(params[PARAM_TRACK_LAYER4_SHIFT]->u.fs_d.value);
    s.trackingLayerWave[2]=float(params[PARAM_TRACK_LAYER4_WAVE]->u.fs_d.value); s.trackingLayerTear[2]=float(params[PARAM_TRACK_LAYER4_TEAR]->u.fs_d.value);
    s.trackingLayerAngle[2]=float(params[PARAM_TRACK_LAYER4_ANGLE]->u.fs_d.value); s.trackingLayerMotionX[2]=float(params[PARAM_TRACK_LAYER4_MOTION_X]->u.fs_d.value); s.trackingLayerMotionY[2]=float(params[PARAM_TRACK_LAYER4_MOTION_Y]->u.fs_d.value);
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
    s.osdDeckMode=params[PARAM_OSD_DECK_MODE]->u.pd.value; s.osdDateFormat=params[PARAM_OSD_DATE_FORMAT]->u.pd.value;
    s.osdLanguage=params[PARAM_OSD_LANGUAGE]->u.pd.value; s.osdDigits=params[PARAM_OSD_DIGITS]->u.pd.value;
    s.osdDateLayout=params[PARAM_OSD_DATE_LAYOUT]->u.pd.value; s.osdClockMode=params[PARAM_OSD_CLOCK_MODE]->u.pd.value;
    s.osdMonthStyle=params[PARAM_OSD_MONTH_STYLE]->u.pd.value; s.osdClockRuns=params[PARAM_OSD_CLOCK_RUNS]->u.bd.value?1:0; s.osdFont=params[PARAM_OSD_FONT]->u.pd.value;
    s.osdExtraLabel=params[PARAM_OSD_EXTRA_LABEL]->u.pd.value; s.osdExtraX=float(params[PARAM_OSD_EXTRA_X]->u.fs_d.value); s.osdExtraY=float(params[PARAM_OSD_EXTRA_Y]->u.fs_d.value);
    s.osdShowText=params[PARAM_OSD_SHOW_TEXT]->u.bd.value?1:0; s.osdShowRecLabel=params[PARAM_OSD_SHOW_REC_LABEL]->u.bd.value?1:0;
    s.osdShowPlayLabel=params[PARAM_OSD_SHOW_PLAY_LABEL]->u.bd.value?1:0; s.osdShowPauseLabel=params[PARAM_OSD_SHOW_PAUSE_LABEL]->u.bd.value?1:0;
    s.osdShowStopLabel=params[PARAM_OSD_SHOW_STOP_LABEL]->u.bd.value?1:0; s.osdShowFfLabel=params[PARAM_OSD_SHOW_FF_LABEL]->u.bd.value?1:0;
    s.osdShowRewLabel=params[PARAM_OSD_SHOW_REW_LABEL]->u.bd.value?1:0; s.osdShowSpeedLabel=params[PARAM_OSD_SHOW_SPEED_LABEL]->u.bd.value?1:0;
    s.osdShowTapeLabel=params[PARAM_OSD_SHOW_TAPE_LABEL]->u.bd.value?1:0; s.osdShowErrorLabel=params[PARAM_OSD_SHOW_ERROR_LABEL]->u.bd.value?1:0;
    s.osdShowProgramLabel=params[PARAM_OSD_SHOW_PROGRAM_LABEL]->u.bd.value?1:0; s.osdShowResLabel=params[PARAM_OSD_SHOW_RES_LABEL]->u.bd.value?1:0;
    s.osdShowForwardLabel=params[PARAM_OSD_SHOW_FORWARD_LABEL]->u.bd.value?1:0; s.osdTextGlow=params[PARAM_OSD_TEXT_GLOW]->u.bd.value?1:0; s.osdShowDate=params[PARAM_OSD_SHOW_DATE]->u.bd.value?1:0;
    s.osdTextSize=float(params[PARAM_OSD_TEXT_SIZE]->u.fs_d.value); s.osdTextGlowAmount=float(params[PARAM_OSD_TEXT_GLOW_AMOUNT]->u.fs_d.value); s.osdTextSeed=float(params[PARAM_OSD_TEXT_SEED]->u.fs_d.value); s.osdTextOpacity=float(params[PARAM_OSD_TEXT_OPACITY]->u.fs_d.value);
    s.osdWordsX=float(params[PARAM_OSD_WORDS_X]->u.fs_d.value); s.osdWordsY=float(params[PARAM_OSD_WORDS_Y]->u.fs_d.value);
    s.osdRecX=float(params[PARAM_OSD_REC_X]->u.fs_d.value); s.osdRecY=float(params[PARAM_OSD_REC_Y]->u.fs_d.value);
    s.osdStatusX=float(params[PARAM_OSD_STATUS_X]->u.fs_d.value); s.osdStatusY=float(params[PARAM_OSD_STATUS_Y]->u.fs_d.value);
    s.osdTimestampX=float(params[PARAM_OSD_TIMESTAMP_X]->u.fs_d.value); s.osdTimestampY=float(params[PARAM_OSD_TIMESTAMP_Y]->u.fs_d.value);
    s.osdCounterFormat=params[PARAM_OSD_COUNTER_FORMAT]->u.pd.value;
    s.osdCounterAmount=float(params[PARAM_OSD_COUNTER_AMOUNT]->u.fs_d.value); s.osdCounterOffset=float(params[PARAM_OSD_COUNTER_OFFSET]->u.fs_d.value); s.osdShowCounter=params[PARAM_OSD_SHOW_COUNTER]->u.bd.value?1:0;
    { const PF_Err textErr=VHS90ReadOsdText(in_data,params[PARAM_OSD_TEXT_DATA]->u.arb_d.value,s.osdText); if(textErr!=PF_Err_NONE) return textErr; }
    s.osdCounterX=float(params[PARAM_OSD_COUNTER_X]->u.fs_d.value); s.osdCounterY=float(params[PARAM_OSD_COUNTER_Y]->u.fs_d.value);
    s.osdRecColorR=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.red); s.osdRecColorG=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.green); s.osdRecColorB=float(params[PARAM_OSD_REC_COLOR]->u.cd.value.blue);
    s.vhsStyle=params[PARAM_VHS_STYLE]->u.pd.value; s.vhsStyleIntensity=float(params[PARAM_VHS_STYLE_INTENSITY]->u.fs_d.value);
    s.cameraProfile=params[PARAM_CAMERA_PROFILE]->u.pd.value; s.cameraIntensity=float(params[PARAM_CAMERA_INTENSITY]->u.fs_d.value);
    s.ntscRsLook=params[PARAM_NTSCRS_LOOK]->u.pd.value; s.ntscRsIntensity=float(params[PARAM_NTSCRS_INTENSITY]->u.fs_d.value);
    s.ntscRsPhaseNoise=float(params[PARAM_NTSCRS_PHASE_NOISE]->u.fs_d.value);
    s.signalHueNoise=float(params[PARAM_SIGNAL_HUE_NOISE]->u.fs_d.value);
    s.mainsHum=float(params[PARAM_MAINS_HUM]->u.fs_d.value);
    s.groundLoop=float(params[PARAM_GROUND_LOOP]->u.fs_d.value);
    s.syncDamageEnable=params[PARAM_SYNC_DAMAGE_ENABLE]->u.bd.value?1:0;
    s.syncDamageTearAmount=float(params[PARAM_SYNC_DAMAGE_TEAR_AMOUNT]->u.fs_d.value);
    s.syncDamageBandIntensity=float(params[PARAM_SYNC_DAMAGE_BAND_INTENSITY]->u.fs_d.value);
    s.syncDamageBandDensity=float(params[PARAM_SYNC_DAMAGE_BAND_DENSITY]->u.fs_d.value);
    s.syncDamageBandHeight=float(params[PARAM_SYNC_DAMAGE_BAND_HEIGHT]->u.fs_d.value);
    s.syncDamageBandIrregularity=float(params[PARAM_SYNC_DAMAGE_BAND_IRREGULARITY]->u.fs_d.value);
    s.syncDamageDisplacement=float(params[PARAM_SYNC_DAMAGE_DISPLACEMENT]->u.fs_d.value);
    s.syncDamageBoundaryFragmentation=float(params[PARAM_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION]->u.fs_d.value);
    s.syncDamageCoverage=float(params[PARAM_SYNC_DAMAGE_COVERAGE]->u.fs_d.value);
    s.syncDamageLumaEnable=params[PARAM_SYNC_DAMAGE_LUMA_ENABLE]->u.bd.value?1:0;
    s.syncDamageLumaInstability=float(params[PARAM_SYNC_DAMAGE_LUMA_INSTABILITY]->u.fs_d.value);
    s.syncDamageBrightFlash=float(params[PARAM_SYNC_DAMAGE_BRIGHT_FLASH]->u.fs_d.value);
    s.syncDamageDarkCollapse=float(params[PARAM_SYNC_DAMAGE_DARK_COLLAPSE]->u.fs_d.value);
    s.syncDamageEventFrequency=float(params[PARAM_SYNC_DAMAGE_EVENT_FREQUENCY]->u.fs_d.value);
    s.syncDamageEventDuration=float(params[PARAM_SYNC_DAMAGE_EVENT_DURATION]->u.fs_d.value);
    s.syncDamageBrightProbability=float(params[PARAM_SYNC_DAMAGE_BRIGHT_PROBABILITY]->u.fs_d.value);
    s.syncDamageDarkProbability=float(params[PARAM_SYNC_DAMAGE_DARK_PROBABILITY]->u.fs_d.value);
    s.syncDamageTransitionSharpness=float(params[PARAM_SYNC_DAMAGE_TRANSITION_SHARPNESS]->u.fs_d.value);
    s.syncDamageLocalLuma=float(params[PARAM_SYNC_DAMAGE_LOCAL_LUMA]->u.fs_d.value);
    s.syncDamageManualBright=float(params[PARAM_SYNC_DAMAGE_MANUAL_BRIGHT]->u.fs_d.value);
    s.syncDamageManualDark=float(params[PARAM_SYNC_DAMAGE_MANUAL_DARK]->u.fs_d.value);
    s.syncDamageDropoutsEnable=params[PARAM_SYNC_DAMAGE_DROPOUTS_ENABLE]->u.bd.value?1:0;
    s.syncDamageDropoutDensity=float(params[PARAM_SYNC_DAMAGE_DROPOUT_DENSITY]->u.fs_d.value);
    s.syncDamageStreakLength=float(params[PARAM_SYNC_DAMAGE_STREAK_LENGTH]->u.fs_d.value);
    s.syncDamageStreakBrightness=float(params[PARAM_SYNC_DAMAGE_STREAK_BRIGHTNESS]->u.fs_d.value);
    s.syncDamageDarkDropout=float(params[PARAM_SYNC_DAMAGE_DARK_DROPOUT]->u.fs_d.value);
    s.syncDamageHorizontalFragmentation=float(params[PARAM_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION]->u.fs_d.value);
    s.syncDamageTemporalVariation=float(params[PARAM_SYNC_DAMAGE_TEMPORAL_VARIATION]->u.fs_d.value);
    s.syncDamageChromaEnable=params[PARAM_SYNC_DAMAGE_CHROMA_ENABLE]->u.bd.value?1:0;
    s.syncDamageChromaTint=float(params[PARAM_SYNC_DAMAGE_CHROMA_TINT]->u.fs_d.value);
    s.syncDamageMagentaTint=float(params[PARAM_SYNC_DAMAGE_MAGENTA_TINT]->u.fs_d.value);
    s.syncDamageGreenTint=float(params[PARAM_SYNC_DAMAGE_GREEN_TINT]->u.fs_d.value);
    s.syncDamageRgbOffset=float(params[PARAM_SYNC_DAMAGE_RGB_OFFSET]->u.fs_d.value);
    s.syncDamageChromaBleed=float(params[PARAM_SYNC_DAMAGE_CHROMA_BLEED]->u.fs_d.value);
    s.syncDamageAnimationMode=params[PARAM_SYNC_DAMAGE_ANIMATION_MODE]->u.pd.value;
    s.syncDamageInputMode=params[PARAM_SYNC_DAMAGE_INPUT_MODE]->u.pd.value;
    s.syncDamageAnimationSpeed=float(params[PARAM_SYNC_DAMAGE_ANIMATION_SPEED]->u.fs_d.value);
    s.syncDamageRandomSeed=float(params[PARAM_SYNC_DAMAGE_RANDOM_SEED]->u.fs_d.value);
    s.syncDamageBlendInput=float(params[PARAM_SYNC_DAMAGE_BLEND_INPUT]->u.fs_d.value);
    s.syncDamageEffectAmount=float(params[PARAM_SYNC_DAMAGE_EFFECT_AMOUNT]->u.fs_d.value);
    s.vhs6RiftGrain=float(params[PARAM_VHS6RIFT_GRAIN]->u.fs_d.value);
    s.vhs6RiftGrainSize=float(params[PARAM_VHS6RIFT_GRAIN_SIZE]->u.fs_d.value);
    s.vhs6RiftNoiseDetail=float(params[PARAM_VHS6RIFT_NOISE_DETAIL]->u.fs_d.value);
    s.vhs6RiftShadowBias=float(params[PARAM_VHS6RIFT_SHADOW_BIAS]->u.fs_d.value);
    s.vhs6RiftNoiseSeed=float(params[PARAM_VHS6RIFT_NOISE_SEED]->u.fs_d.value);
    s.vhs6RiftNoiseEnable=params[PARAM_VHS6RIFT_NOISE_ENABLE]->u.bd.value!=0;
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

static PF_Err UserChangedParam(PF_InData* in_data,PF_ParamDef* params[],void* extra) {
    if(!in_data || !params || !extra) return PF_Err_BAD_CALLBACK_PARAM;
    const auto* changed=static_cast<const PF_UserChangedParamExtra*>(extra);
    if(changed->param_index!=PARAM_OSD_TEXT_EDIT) return PF_Err_NONE;
    if(!params[PARAM_OSD_TEXT_DATA]) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    std::string currentText;
    PF_Err err=VHS90ReadOsdText(in_data,params[PARAM_OSD_TEXT_DATA]->u.arb_d.value,currentText);
    if(err!=PF_Err_NONE) return err;
    std::string editedText;
    if(!VHS90ShowOsdTextDialog(currentText,editedText)) return PF_Err_NONE;
    PF_ArbitraryH replacement=nullptr;
    err=VHS90CreateOsdText(in_data,editedText,&replacement);
    if(err!=PF_Err_NONE) return err;
    if(!replacement) return PF_Err_OUT_OF_MEMORY;
    params[PARAM_OSD_TEXT_DATA]->u.arb_d.value=replacement;
    params[PARAM_OSD_TEXT_DATA]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
    return PF_Err_NONE;
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
            case PF_Cmd_USER_CHANGED_PARAM: err=UserChangedParam(in_data,params,extra); break;
            case PF_Cmd_ARBITRARY_CALLBACK: err=VHS90OsdArbitraryCallback(in_data,static_cast<PF_ArbParamsExtra*>(extra)); break;
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
    PF_Err result=PF_Err_NONE;
    PF_REGISTER_EFFECT_EXT2(inPtr,callback,"VHS-90","com.vhs90.effect",
        "Analog & Tape Effects",AE_RESERVED_INFO,"EffectMain",
        "https://github.com/vamprieieuu/vhs---90");
    return result;
}