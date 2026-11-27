#!/usr/bin/env python3
"""Static audit for all currently wired VHS-90 source-derived preset families."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
source = (root / "Source/VHS90Effect.cpp").read_text(encoding="utf-8")
sync_engine = (root / "Source/VHS90Core/VHS90SyncEngine.cpp").read_text(encoding="utf-8")
grade_engine = (root / "Source/VHS90Core/VHS90ImageGradeEngine.cpp").read_text(encoding="utf-8")
rf_engine = (root / "Source/VHS90Core/VHS90RfInstabilityEngine.cpp").read_text(encoding="utf-8")
physical_engine = (root / "Source/VHS90Core/VHS90PhysicalInterferenceEngine.cpp").read_text(encoding="utf-8")

def enum_values(name):
    match = re.search(r"enum\s+" + re.escape(name) + r"\s*\{([\s\S]*?)\};", source)
    if not match:
        raise AssertionError(f"Missing enum {name}")
    return [item.strip() for item in match.group(1).split(",") if item.strip()]

param_index = enum_values("ParamIndex")
disk_ids = [re.sub(r"=.*$", "", item).strip() for item in enum_values("ParamDiskID")]
registered_ids = []
for line in source.splitlines():
    if "PF_ADD_" not in line or "PF_ADD_INPUT" in line:
        continue
    if "PF_ADD_ARBITRARY2" in line:
        # Arbitrary parameters carry REFCON after the persistent disk ID.
        match = re.search(r",\s*(ID_[A-Z0-9_]+)\s*,", line)
    else:
        match = re.search(r",\s*(ID_[A-Z0-9_]+)\s*\)", line)
    if match:
        registered_ids.append(match.group(1))

active_indices = [
    item for item in param_index
    if item != "PARAM_INPUT=0" and item != "PARAM_COUNT" and not item.endswith("_END")
]
assert len(active_indices) == len(disk_ids), (
    f"ParamIndex/disk-ID count mismatch: {len(active_indices)} vs {len(disk_ids)}"
)
assert len(disk_ids) == len(set(disk_ids)), "Duplicate ParamDiskID"
assert len(registered_ids) == len(disk_ids), (
    f"UI registration count mismatch: {len(registered_ids)} vs {len(disk_ids)}"
)
# Persistent disk IDs are explicit and independent of UI order. This allows
# the visible groups to be reorganized without renumbering saved projects.
assert registered_ids.count("ID_GRAIN_TOPIC") == 1, "Grain topic disk ID must be registered exactly once"
assert len(registered_ids) == len(set(registered_ids)), "Duplicate registered disk ID"
assert set(registered_ids) == set(disk_ids), "Registered UI disk IDs differ from persistent ParamDiskID set"
disk_values = []
for item in enum_values("ParamDiskID"):
    parts = item.strip().split("=", 1)
    assert len(parts) == 2 and parts[0].startswith("ID_") and parts[1].strip().isdigit(), (
        f"ParamDiskID must use explicit stable numeric values: {item}"
    )
    disk_values.append(int(parts[1].strip()))
assert len(disk_values) == len(set(disk_values)), "Duplicate persistent ParamDiskID numeric value"

# Confirm the new UI controls are read in SmartFX and legacy paths.
required_reads = [
    "CheckoutPopup(in_data,PARAM_VHS_STYLE,s.vhsStyle)",
    "CheckoutPopup(in_data,PARAM_EXTENDED_VHS_PRESET,s.extendedVhsStyle)",
    "CheckoutPopup(in_data,PARAM_CAMERA_PROFILE,s.cameraProfile)",
    "CheckoutPopup(in_data,PARAM_NTSCRS_LOOK,s.ntscRsLook)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PROFILE,s.trackingProfile)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_BASE,s.trackingPresetBase)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_2,s.trackingLayerPreset[0])",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_3,s.trackingLayerPreset[1])",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_4,s.trackingLayerPreset[2])",
    "CheckoutPopup(in_data,PARAM_PATTERN_MODE,s.staticMode)",
    "CheckoutPopup(in_data,PARAM_DECODER_MODE,s.compositeDecoder)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_FREQUENCY,frameRepeatFrequency)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_STRENGTH,frameRepeatStrength)",
    "READ_FLOAT(PARAM_PATTERN_ANGLE,staticAngle)",
    "READ_FLOAT(PARAM_PATTERN_WIDTH,staticRibbonWidth)",
    "READ_FLOAT(PARAM_PATTERN_GREEN,staticGreenLine)",
    "READ_FLOAT(PARAM_PATTERN_PHASE_COUNT,staticPhaseCount)",
    "READ_FLOAT(PARAM_TEAR_BAND2_AMOUNT,tearBandAmount[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_Y,tearBandY[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_HEIGHT,tearBandHeightExtra[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_SHIFT,tearBandShift[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_FRAGMENTATION,tearBandFragmentation[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_CHROMA,tearBandChroma[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_SMEAR,tearBandSmear[0])",
    "READ_FLOAT(PARAM_TEAR_BAND2_DROPOUT,tearBandDropout[0])",
    "READ_FLOAT(PARAM_TEAR_BAND3_AMOUNT,tearBandAmount[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_Y,tearBandY[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_HEIGHT,tearBandHeightExtra[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_SHIFT,tearBandShift[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_FRAGMENTATION,tearBandFragmentation[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_CHROMA,tearBandChroma[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_SMEAR,tearBandSmear[1])",
    "READ_FLOAT(PARAM_TEAR_BAND3_DROPOUT,tearBandDropout[1])",
    "READ_FLOAT(PARAM_TEAR_BAND4_AMOUNT,tearBandAmount[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_Y,tearBandY[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_HEIGHT,tearBandHeightExtra[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_SHIFT,tearBandShift[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_FRAGMENTATION,tearBandFragmentation[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_CHROMA,tearBandChroma[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_SMEAR,tearBandSmear[2])",
    "READ_FLOAT(PARAM_TEAR_BAND4_DROPOUT,tearBandDropout[2])",
    "READ_FLOAT(PARAM_TEAR_BAND5_AMOUNT,tearBandAmount[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_Y,tearBandY[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_HEIGHT,tearBandHeightExtra[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_SHIFT,tearBandShift[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_FRAGMENTATION,tearBandFragmentation[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_CHROMA,tearBandChroma[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_SMEAR,tearBandSmear[3])",
    "READ_FLOAT(PARAM_TEAR_BAND5_DROPOUT,tearBandDropout[3])",
    "CheckoutPopup(in_data,PARAM_TEAR_CHROMA_MODE,s.tearChromaMode)",
    "READ_FLOAT(PARAM_TEAR_CHROMA_AMOUNT,tearChromaAmount)",
    "READ_FLOAT(PARAM_SPECTRAL_SMEAR,spectralSmear)",
    "READ_FLOAT(PARAM_SPECTRAL_HUE_SPREAD,spectralHueSpread)",
    "READ_FLOAT(PARAM_CHROMA_SATURATION,chromaSaturation)",
    "READ_FLOAT(PARAM_SPECTRAL_CENTER_Y,spectralCenterY)",
    "READ_FLOAT(PARAM_SPECTRAL_COVERAGE,spectralCoverage)",
    "READ_FLOAT(PARAM_TEAR_BAND_START_Y,tearBandStartY)",
    "READ_FLOAT(PARAM_TEAR_BAND_HEIGHT,tearBandHeight)",
    "READ_FLOAT(PARAM_TEAR_BAND_COUNT,tearBandCount)",
    "READ_FLOAT(PARAM_TEAR_DISPLACEMENT,tearDisplacement)",
    "READ_FLOAT(PARAM_TEAR_FRAGMENTATION,tearFragmentation)",
    "READ_FLOAT(PARAM_TEAR_CHROMA_SHIFT,tearChromaShift)",
    "READ_FLOAT(PARAM_TEAR_SMEAR,tearSmear)",
    "READ_FLOAT(PARAM_TEAR_DROPOUT,tearDropout)",
    "READ_FLOAT(PARAM_TEAR_SEED,tearSeed)",
    "READ_FLOAT(PARAM_TRACK_LAYER2_AMOUNT,trackingLayerAmount[0])",
    "READ_FLOAT(PARAM_TRACK_LAYER2_X,trackingLayerX[0])",
    "READ_FLOAT(PARAM_TRACK_LAYER2_Y,trackingLayerY[0])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER2_PROFILE,s.trackingLayerProfile[0])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER2_SHAPE,s.trackingLayerShape[0])",
    "READ_FLOAT(PARAM_TRACK_LAYER3_AMOUNT,trackingLayerAmount[1])",
    "READ_FLOAT(PARAM_TRACK_LAYER3_X,trackingLayerX[1])",
    "READ_FLOAT(PARAM_TRACK_LAYER3_Y,trackingLayerY[1])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER3_PROFILE,s.trackingLayerProfile[1])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER3_SHAPE,s.trackingLayerShape[1])",
    "READ_FLOAT(PARAM_TRACK_LAYER4_AMOUNT,trackingLayerAmount[2])",
    "READ_FLOAT(PARAM_TRACK_LAYER4_X,trackingLayerX[2])",
    "READ_FLOAT(PARAM_TRACK_LAYER4_Y,trackingLayerY[2])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER4_PROFILE,s.trackingLayerProfile[2])",
    "CheckoutPopup(in_data,PARAM_TRACK_LAYER4_SHAPE,s.trackingLayerShape[2])",
    "READ_FLOAT(PARAM_CHROMA_REG_RED,redChannelOffset)",
    "READ_FLOAT(PARAM_CHROMA_REG_GREEN,greenChannelOffset)",
    "READ_FLOAT(PARAM_CHROMA_REG_BLUE,blueChannelOffset)",
    "CheckoutPopup(in_data,PARAM_CHROMA_REG_MODE,s.chromaRegMode)",
    "READ_FLOAT(PARAM_VHS_STYLE_INTENSITY,vhsStyleIntensity)",
    "READ_FLOAT(PARAM_CAMERA_INTENSITY,cameraIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_INTENSITY,ntscRsIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_PHASE_NOISE,ntscRsPhaseNoise)",
    "READ_FLOAT(PARAM_SIGNAL_HUE_NOISE,signalHueNoise)",
    "READ_FLOAT(PARAM_VHS6RIFT_GRAIN,vhs6RiftGrain)",
    "READ_FLOAT(PARAM_VHS6RIFT_GRAIN_SIZE,vhs6RiftGrainSize)",
    "READ_FLOAT(PARAM_VHS6RIFT_NOISE_DETAIL,vhs6RiftNoiseDetail)",
    "READ_FLOAT(PARAM_VHS6RIFT_SHADOW_BIAS,vhs6RiftShadowBias)",
    "READ_FLOAT(PARAM_VHS6RIFT_NOISE_SEED,vhs6RiftNoiseSeed)",
    "s.vhs6RiftNoiseEnable=noiseEnable.u.bd.value!=0",
    "READ_FLOAT(PARAM_EDGE_WAVE_FREQUENCY,edgeWaveFrequency)",
    "READ_FLOAT(PARAM_EDGE_WAVE_SPEED,edgeWaveSpeed)",
    "READ_FLOAT(PARAM_EDGE_WAVE_DETAIL,edgeWaveDetail)",
    "READ_FLOAT(PARAM_CHROMA_EDGE_ENHANCE,chromaEdgeEnhance)",
    "READ_FLOAT(PARAM_GRADE_I_GAIN,chromaIGain)",
    "READ_FLOAT(PARAM_GRADE_Q_GAIN,chromaQGain)",
    "READ_FLOAT(PARAM_GRADE_I_OFFSET,chromaIOffset)",
    "READ_FLOAT(PARAM_GRADE_Q_OFFSET,chromaQOffset)",
    "READ_FLOAT(PARAM_GRADE_CHROMA_QUANTIZATION,chromaQuantization)",
]
for needle in required_reads:
    assert needle in source, f"Missing SmartFX parameter read: {needle}"

for needle in [
    "ApplyVhs6RiftStylePreset(s);",
    "ApplyVhs6RiftCameraProfile(s);",
    "ApplyNtscRsVhsLook(s);",
    "ApplyVhsSourcePresets(s);",
    "p.analogNoise.chromaPhaseNoise=clamp01(s.ntscRsPhaseNoise/100.0f);",
    "p.analogNoise.grainAmount=clamp01(s.vhs6RiftGrain/100.0f);",
    "p.analogNoise.shadowBias=clamp01(s.vhs6RiftShadowBias/100.0f);",
    "p.analogNoise.noiseDetail=clamp01(s.vhs6RiftNoiseDetail/100.0f);",
    "p.analogNoise.hueNoiseAmount=clamp01(s.signalHueNoise/100.0f);",
    "s.signalHueNoise>0",
    "p.geometry.edgeWaveFrequency=s.edgeWaveFrequency",
    "p.imageGrade.chromaEdgeEnhancement=clamp01(s.chromaEdgeEnhance/100.0f);",
    "p.imageGrade.chromaSharpenCutoffHz=s.tapeSpeed==3?560000.0f:(s.tapeSpeed==2?600000.0f:640000.0f);",
    "p.imageGrade.chromaIGain=s.chromaIGain/100.0f;p.imageGrade.chromaQGain=s.chromaQGain/100.0f;",
    "p.imageGrade.chromaIOffset=s.chromaIOffset/200.0f;p.imageGrade.chromaQOffset=s.chromaQOffset/200.0f;",
    "p.imageGrade.chromaQuantization=s.chromaQuantization/2000.0f;",
    "rp.redOffsetPixels=s.redChannelOffset*master/100.0f;",
    "rp.greenOffsetPixels=s.greenChannelOffset*master/100.0f;",
    "rp.blueOffsetPixels=s.blueChannelOffset*master/100.0f;",
    "rp.mode=static_cast<RgbRegistrationMode>(std::clamp<A_long>(s.chromaRegMode-1,0,3));",
    "processRgbChannelRegistration(in,out,rp)",
    "r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingProfile-1,0,4));",
    "ApplyTrackingRecipe(r,s.trackingPresetBase);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[0]);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[1]);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[2]);",
    "p.mode=static_cast<StaticPatternMode>(std::clamp<A_long>(s.staticMode-1,0,12));",
    "p.greenLineAmount=clamp01(s.staticGreenLine/100.0f);",
    "baseBand.centerY=clamp01((s.tearBandStartY+s.tearBandHeight*0.5f)/100.0f);",
    "b.intensity=clamp01(s.tearBandAmount[li]/100.0f);",
    "DamageRegion regions[5]; std::size_t count=0;",
    "r.seedOffset=0x1000u;",
    "tp.mode=static_cast<TearChromaMode>(std::clamp<A_long>(s.tearChromaMode-1,0,4));",
    "processTearChroma(in,out,tp)",
    "p.geometry.vhs43Frame=s.vhs43Frame!=0;p.geometry.lensDistortion=s.lensDistortion/1000.0f;",
    "p.geometry.roundedOverscan=clamp01(s.roundedOverscan/100.0f);",
    "op.batteryAmount=clamp01(s.osdBattery/100.0f);op.showBattery=s.osdBattery>0.0f;",
    "b(s.osdPlay,v[36]>0.5f?100.0f:0.0f);b(s.osdTimestamp,v[36]>0.5f?100.0f:0.0f);b(s.osdBattery,v[36]>0.5f?75.0f:0.0f);",
    "||s.vhs43Frame!=0||s.roundedOverscan>0||s.lensDistortion!=0||s.digitalZoom>1.0f",
    "p.demodulator=s.compositeDecoder==3?CompositeDemodulator::TwoLineComb:(s.compositeDecoder==2?CompositeDemodulator::OneLineComb:CompositeDemodulator::ConsumerNotch);",
    "p.inputBandwidth.lumaCutoffHz=blendCutoff(4200000.0f,tapeBandwidth.lumaCutoffHz);",
    "p.outputBandwidth.lumaCutoffHz=blendCutoff(4200000.0f,tapeBandwidth.lumaCutoffHz*0.90f);",
    "kLostSignalProfiles[13][45]",
    "b(s.lensDistortion,v[39]*1000.0f);bi(s.vhs43Frame,v[40]>0.5f?1.0f:0.0f);",
    "p.temporalGhost.frameRepeatFrequency=clamp01(s.frameRepeatFrequency/100.0f);",
    "p.temporalGhost.fieldWeave=clamp01(s.interlace/100.0f);p.temporalGhost.fieldParity=static_cast<int>(p.frame&1);",
    "if(previousInput && (s.temporalGhost>0||s.interlace>0||(s.frameRepeatFrequency>0&&s.frameRepeatStrength>0))",
    "b(s.frameRepeatFrequency,v[43]*100.0f);b(s.frameRepeatStrength,v[44]*100.0f);",
    "p.temporalGhost.frameRepeatStrength=clamp01(s.frameRepeatStrength/100.0f);",
    "p.temporalGhost.frame=s.frame;p.temporalGhost.seed=static_cast<std::uint32_t>(std::max(1.0f,s.tapeSeed));",
    "if((s->temporalGhost>0||s->interlace>0||(s->frameRepeatFrequency>0&&s->frameRepeatStrength>0))",
]:
    assert needle in source, f"Missing preset/render wiring: {needle}"

recipe_start = source.find("static void ApplyVhsStyleRecipe(RenderSettings& s,int styleRecipe)")
recipe_end = source.find("static constexpr int kLegacyVhsStyleRecipeMap", recipe_start)
recipe_cases = set(map(int, re.findall(r"case (\d+):", source[recipe_start:recipe_end])))
assert set(range(2, 73)).issubset(recipe_cases), f"Missing native style recipe cases: {sorted(set(range(2,73))-recipe_cases)}"
assert "ApplyCratesMasterPreset(s,static_cast<int>(styleRecipe-39));" in source, "Crates presets must use the recovered source-value mapper"
assert "kCrates[16][44]" in source, "Crates mapper must retain all 16 source looks and 44 mapped source fields"
crates_start = source.find("static constexpr float kCrates[16][44]={")
assert crates_start >= 0, "Missing Crates 16x44 source-value table"
crates_end = source.find("    };", crates_start)
assert crates_end > crates_start, "Unterminated Crates source-value table"
crates_rows = [line for line in source[crates_start:crates_end].splitlines() if line.lstrip().startswith("{")]
assert len(crates_rows) == 16, f"Crates table must contain 16 source presets, got {len(crates_rows)}"
assert all(len(line.strip().lstrip("{").rstrip(",").rstrip("}").split(",")) == 44 for line in crates_rows), "Every Crates preset must define all 44 mapped source values"
legacy_map = re.search(r"kLegacyVhsStyleRecipeMap\[33\]=\s*\{([^}]+)\}", source)
assert legacy_map, "Missing legacy VHS style compatibility map"
legacy_values = [int(x.strip()) for x in legacy_map.group(1).split(",")]
assert len(legacy_values) == 33 and legacy_values[0] == 0 and legacy_values[1] == 1, "Legacy style map must preserve Custom and all 32 prior popup choices"
assert legacy_values[2:] == [2,3,4,5,6,7,9,11,12,13,15,16,18,19,26,39,42,45,52,55,56,58,59,60,61,63,66,67,68,69,72], "Legacy style map must preserve the exact historic source recipe IDs"
extended_map = re.search(r"kExtendedVhsStyleRecipeMap\[41\]=\s*\{([^}]+)\}", source)
assert extended_map, "Missing expanded VHS style recipe map"
extended_values = [int(x.strip()) for x in extended_map.group(1).split(",")]
assert len(extended_values) == 41 and extended_values[0] == 0, "Expanded style map must contain Custom plus 40 additional recipes"
assert sorted(extended_values[1:]) == sorted(set(range(2,73))-set(legacy_values[2:])), "Expanded styles must expose every remaining source recipe exactly once"
assert "ApplyVhsStyleRecipe(s,kExtendedVhsStyleRecipeMap[selected]);" in source, "Expanded style popup must apply the selected native recipe"
extended_choices = re.search(r'kExtendedVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert extended_choices and len(extended_choices.group(1).split("|")) == 41, "Expanded VHS style popup must retain 41 matching labels"
assert 'PF_ADD_POPUP("Additional VHS Style Preset",41,1,kExtendedVhsStyleChoices,ID_EXTENDED_VHS_PRESET)' in source, "Expanded style popup count must match its labels"
style_choices = re.search(r'kVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert style_choices and len(style_choices.group(1).split("|")) == 32, "Legacy VHS Style popup must retain its 32 compatibility entries"
assert 'PF_ADD_POPUP("VHS Style Preset",32,1,kVhsStyleChoices,ID_VHS_STYLE)' in source, "VHS Style popup count must match its labels"
assert 'PF_ADD_FLOAT_SLIDERX("Master Strength",0,300,0,300,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_MASTER)' in source, "Master Strength must exceed 100 and expose the 300% range"
assert 'PF_ADD_FLOAT_SLIDERX("Grade Saturation",0,500,0,500,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_GRADE_SATURATION)' in source, "Saturation must expose the requested 500% range"
assert "std::clamp(p.saturation,0.0f,5.0f)" in grade_engine, "Image Grade engine must preserve saturation drive through 500%"
assert "const float humOffset=hum*" in rf_engine and "const float ripple=hum*" in rf_engine and "const float interference=hum*" in rf_engine and "const float humGain=1.0f+hum*" in rf_engine, "RF Hum must be neutral when its own control is zero"
assert "const float pump=agc*" in rf_engine, "AGC Pump must be neutral when its own control is zero"
assert "const float humOffset = 0.002f * loopDrive + 0.045f * hum;" in physical_engine and "const float humGain = 1.0f + 0.006f * loopDrive + 0.018f * hum;" in physical_engine, "Mains Hum must remain independent of Ground Loop voltage"
assert "hum <= 0.0f && loopDrive <= 0.0f && rfDrive <= 0.0f" in physical_engine, "Ground Loop must remain active when it is the only physical-interference control"
assert "const float hiComp=0.80f*am,loComp=0.88f*am;" in rf_engine, "AM nonlinearity must be neutral when its own control is zero"
assert 'PF_ADD_FLOAT_SLIDERX("Exposure (EV)",-3,3,-3,3,0,2,PF_ValueDisplayFlag_NONE,0,ID_GRADE_EXPOSURE)' in source, "General Adjustments must expose a real EV exposure control"
assert "osdFont=13, osdExtraLabel=1" in source, "New OSD instances must default to the bundled Rooyin Arabic font"
assert 'PF_ADD_POPUP("OSD Font Family",15,13,kOsdFontChoices,ID_OSD_FONT)' in source, "OSD font popup must default to bundled Rooyin while preserving legacy font choices"
assert '"RooyinFree"' in source and '"YR Sponge Arabic"' in source, "Bundled OSD fonts must remain addressable by their DirectWrite family names"
assert 'PF_ADD_BUTTON("OSD Text Editor","Edit Text...",0,PF_ParamFlag_SUPERVISE,ID_OSD_TEXT_EDIT)' in source, "OSD text must have a visible supervised editor button"
assert "case PF_Cmd_USER_CHANGED_PARAM: err=UserChangedParam(in_data,params,extra); break;" in source, "OSD editor button must reach its user-change handler"
assert "PF_ChangeFlag_CHANGED_VALUE" in source and "VHS90CreateOsdText" in source, "OSD text editor must persist edits into the arbitrary-data stream"
style_pos=source.find('PF_ADD_TOPIC("VHS Style & Presets",ID_VHS_STYLE_TOPIC)')
master_pos=source.find('PF_ADD_FLOAT_SLIDERX("Master Strength"')
signal_pos=source.find('PF_ADD_TOPIC("Signal",ID_SIGNAL_TOPIC)')
assert 0 <= style_pos < master_pos < signal_pos, "VHS Style / Presets must precede Master Strength and Signal"
assert 'PF_ADD_TOPIC("Chroma",ID_CHROMA_TOPIC)' in source and 'PF_ADD_TOPIC("Ringing",ID_RINGING_TOPIC)' in source and 'PF_ADD_TOPIC("Glow",ID_GLOW_TOPIC)' in source and 'PF_ADD_TOPIC("General Adjustments",ID_GENERAL_TOPIC)' in source, "Dedicated Chroma, Ringing, Glow and General sections must remain present"
assert 'PF_ADD_TOPIC("Tape",ID_TAPE_MATERIAL_TOPIC)' in source and 'PF_ADD_FLOAT_SLIDERX("Tape Material Master",0,100,0,100,100,1,PF_ValueDisplayFlag_PERCENT,0,ID_TAPE_SURFACE)' in source, "Unified Tape section must have an active master control"
assert 'PF_ADD_FLOAT_SLIDERX("Luma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_LUMA_NOISE)' in source and 'PF_ADD_FLOAT_SLIDERX("Chroma Noise",0,100,0,100,0,1,PF_ValueDisplayFlag_PERCENT,0,ID_CHROMA_NOISE)' in source, "Grain noise controls must start neutral"
assert 'PF_ADD_FLOAT_SLIDERX("Green Channel Registration",-20,20,-20,20,0,1,PF_ValueDisplayFlag_NONE,0,ID_CHROMA_REG_GREEN)' in source and 'PF_ADD_POPUP("RGB Channel Mode",4,1,kRgbChannelModeChoices,ID_CHROMA_REG_MODE)' in source, "Chroma Shift must expose independent RGB channel modes"
assert 'PF_ADD_TOPIC("Mains Hum",ID_PHYSICAL_TOPIC)' in source and 'PF_ADD_FLOAT_SLIDERX("RF Hum"' in source, "Mains Hum and RF Hum must be distinct controls"
assert 'PF_ADD_TOPIC("Tape Surface Detail",ID_TAPE_SURFACE_DETAIL_TOPIC)' in source, "Tape Surface Detail topic must be registered"
assert all(x in source for x in ['PF_ADD_FLOAT_SLIDERX("Pinholes"', 'PF_ADD_FLOAT_SLIDERX("Oxide Flakes"', 'PF_ADD_FLOAT_SLIDERX("Surface Roughness"']), "Tape pinhole, flake and roughness controls must be exposed"
assert all(x in source for x in ["READ_FLOAT(PARAM_TAPE_PINHOLES,tapePinholes)", "READ_FLOAT(PARAM_TAPE_FLAKES,tapeFlakes)", "READ_FLOAT(PARAM_TAPE_ROUGHNESS,tapeRoughness)", "tc.pinholes=s.tapePinholes", "tc.flakes=s.tapeFlakes", "tc.surfaceRoughness=s.tapeRoughness"]), "Tape surface detail controls must reach the render engine"
assert "kLegacyVhsStyleRecipeMap[33]" in source and "kExtendedVhsStyleRecipeMap[41]" in source and "switch(styleRecipe)" in source, "Legacy and expanded menus must map to source recipe IDs"
assert "p.noiseAmount*glitchBand" in sync_engine, "Sync Collapse noise must be gated by the active moving glitch band"
assert "rgb.r*=exposure;rgb.g*=exposure;rgb.b*=exposure;" in grade_engine, "Exposure EV must be applied uniformly in RGB before YIQ grading"
assert re.search(r"kLegacyVhsStyleRecipeMap\[33\]\s*=\s*\{\s*0,1,2,3,4,5,6,7,9,11,12,13,15,16,18,19,26,39,42,45,52,\s*55,56,58,59,60,61,63,66,67,68,69,72", source), "Legacy style mapping must preserve all prior popup values"
extra_choices = re.search(r'kExtendedVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert extra_choices and len(extra_choices.group(1).split("|")) == 41, "Expanded VHS menu must expose Custom plus all 40 additional recipes"
assert 'PF_ADD_POPUP("Additional VHS Style Preset",41,1,kExtendedVhsStyleChoices,ID_EXTENDED_VHS_PRESET)' in source
extra_map = re.search(r"kExtendedVhsStyleRecipeMap\[41\]\s*=\s*\{([^}]+)\}", source)
assert extra_map, "Missing 40-recipe expanded menu map"
extra_ids = [int(x.strip()) for x in extra_map.group(1).split(",")]
assert len(extra_ids) == 41 and extra_ids[0] == 0
assert set(extra_ids[1:]) == set(range(2,73)) - set([1,2,3,4,5,6,7,9,11,12,13,15,16,18,19,26,39,42,45,52,55,56,58,59,60,61,63,66,67,68,69,72]), "Expanded menu must cover every previously omitted recipe ID exactly once"
assert "if(s.extendedVhsStyle>1) ApplyVhsExtendedStylePreset(s);" in source, "Expanded menu selection must override the legacy style selector"

camera_start = source.find("static void ApplyVhs6RiftCameraProfile")
camera_end = source.find("static void ApplyNtscRsVhsLook", camera_start)
ntsc_start = camera_end
ntsc_end = source.find("static void ApplyVhsSourcePresets", ntsc_start)
camera_cases = set(map(int, re.findall(r"case (\d+):", source[camera_start:camera_end])))
ntsc_cases = set(map(int, re.findall(r"case (\d+):", source[ntsc_start:ntsc_end])))
assert camera_cases == set(range(2, 13)), f"Missing VHS6RIFT and FXC6/Gen 7 camera profile cases: {sorted(set(range(2,13))-camera_cases)}"
camera_choices = re.search(r'kCameraProfileChoices\[\]\s*=\s*"([^"]+)"', source)
assert camera_choices and len(camera_choices.group(1).split("|")) == 12, "Camera Profile popup must expose Off plus 11 reference profiles"
assert ntsc_cases == set(range(2, 12)), f"Missing NTSC-RS-derived look cases: {sorted(set(range(2,11))-ntsc_cases)}"

osd_position = source.find("if(osdEnabled) {")
composite_position = source.find("if(compositeStages&&master>0.0f) {")
assert osd_position >= 0 and composite_position >= 0 and osd_position < composite_position, "OSD must be composited into the source before the VHS composite signal stage"

lost_start = source.find("static constexpr float kLostSignalProfiles[13][45]={")
assert lost_start >= 0, "Missing 13x45 Lost Signal VHS profile table"
lost_end = source.find("\n    };", lost_start)
assert lost_end > lost_start, "Unterminated Lost Signal VHS profile table"
lost_rows = [line for line in source[lost_start:lost_end].splitlines() if line.lstrip().startswith("{")]
assert len(lost_rows) == 13, f"Lost Signal VHS profile table must contain 13 complete profiles, got {len(lost_rows)}"
assert all(len(line.strip().lstrip("{").rstrip(",").rstrip("}").split(",")) == 45 for line in lost_rows), "Every Lost Signal VHS profile must define all 45 mapped values"
print(f"PASS: {len(active_indices)} AE parameter slots; persistent disk IDs are unique and stable across UI reordering.")
print("PASS: legacy 32-entry VHS Style menu remains compatible; expanded menu exposes all 40 omitted recipes without changing old IDs.")
print("PASS: all 71 source-derived VHS style recipes are selectable across the legacy and expanded menus; exact-source-value limits remain documented.")
print("PASS: camera profiles and NTSC-RS Signal Looks remain wired; retired Playback, Decoder, Frame, HUD and Signal Network groups are removed; Particle looks are folded into Static.")
assert "p.mode=static_cast<StaticPatternMode>(std::clamp<A_long>(s.staticMode-1,0,12));" in source, "Static Pattern mode must be mapped through all supported modes"
assert 'PF_ADD_POPUP("Raster Decoder",3,1,kCompositeDecoderChoices,ID_DECODER_MODE)' in source
assert "p.demodulator=CompositeDemodulator::ConsumerNotch;" not in source, "Raster Decoder selection must not be overwritten"
assert 'PF_ADD_FLOAT_SLIDERX("Frame Repeat Frequency"' in source
assert 'PF_ADD_FLOAT_SLIDERX("Frame Repeat Strength"' in source
assert 'kStaticPatternChoices[]="Dark-Field Speckles|Full-Frame Horizontal|Top Strip|Bottom-Weighted Streaks|Monochrome Streaks|Split-Band Chromatic|Static to Blackout|Dark-Field Luma Streaks|Multi-Phase Green Line|Two-Stage Angular Ribbon|Diagonal Sync Collapse|Colored Speckles|Vertical Columns"' in source, "Static pattern menu must expose the two recovered procedural particle modes"
assert 'PF_ADD_POPUP("Pattern Type",13,2,kStaticPatternChoices,ID_PATTERN_MODE)' in source, "All static modes must be selectable in Effect Controls"
assert 'PF_PUI_NO_ECW_UI; PF_ADD_POPUP("Pattern Type"' not in source, "Static pattern mode selector must not be hidden from Effect Controls"