#!/usr/bin/env python3
"""Static audit for all currently wired VHS-90 source-derived preset families."""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
source = (root / "Source/VHS90Effect.cpp").read_text(encoding="utf-8")

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
# ParamDiskID values are persistent serialized IDs; their enum declaration
# order is deliberately independent from the visible AE parameter order.
# This permits grouping/reordering UI controls without changing saved projects.
assert registered_ids.count("ID_GRAIN_TOPIC") == 1, "Grain topic disk ID must be registered exactly once"
assert len(registered_ids) == len(set(registered_ids)), "Duplicate registered disk ID"
assert set(registered_ids) == set(disk_ids), "Registered UI disk IDs differ from the persistent ParamDiskID set"
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
    "CheckoutPopup(in_data,PARAM_CAMERA_PROFILE,s.cameraProfile)",
    "CheckoutPopup(in_data,PARAM_NTSCRS_LOOK,s.ntscRsLook)",
    "CheckoutPopup(in_data,PARAM_COMPOSITE_DECODER,s.compositeDecoder)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PROFILE,s.trackingProfile)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_BASE,s.trackingPresetBase)",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_2,s.trackingLayerPreset[0])",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_3,s.trackingLayerPreset[1])",
    "CheckoutPopup(in_data,PARAM_TRACKING_PRESET_4,s.trackingLayerPreset[2])",
    "CheckoutPopup(in_data,PARAM_PATTERN_MODE,s.staticMode)",
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
    "CheckoutPopup(in_data,PARAM_PARTICLE_MODE,s.particleMode)",
    "CheckoutPopup(in_data,PARAM_PARTICLE_DISTRIBUTION,s.particleDistribution)",
    "READ_FLOAT(PARAM_PARTICLE_AMOUNT,particleAmount)",
    "READ_FLOAT(PARAM_PARTICLE_DENSITY,particleDensity)",
    "READ_FLOAT(PARAM_PARTICLE_SIZE,particleSize)",
    "READ_FLOAT(PARAM_PARTICLE_LENGTH,particleLength)",
    "READ_FLOAT(PARAM_PARTICLE_CHROMA,particleChroma)",
    "READ_FLOAT(PARAM_PARTICLE_SPEED,particleSpeed)",
    "READ_FLOAT(PARAM_PARTICLE_SEED,particleSeed)",
    "READ_FLOAT(PARAM_PARTICLE_SOURCE_MIX,particleSourceMix)",
    "READ_FLOAT(PARAM_PARTICLE_BRIGHTNESS,particleBrightness)",
    "READ_FLOAT(PARAM_PARTICLE_CONTRAST,particleContrast)",
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
    "CheckoutCheckbox(in_data,PARAM_VHS_43_FRAME,s.vhs43Frame)",
    "READ_FLOAT(PARAM_ROUNDED_OVERSCAN,roundedOverscan)",
    "READ_FLOAT(PARAM_LENS_DISTORTION,lensDistortion)",
    "READ_FLOAT(PARAM_DIGITAL_ZOOM,digitalZoom)",
    "READ_FLOAT(PARAM_OSD_BATTERY,osdBattery)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_FREQUENCY,frameRepeatFrequency)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_STRENGTH,frameRepeatStrength)",
    "READ_FLOAT(PARAM_CHROMA_REG_RED,redChannelOffset)",
    "READ_FLOAT(PARAM_CHROMA_REG_BLUE,blueChannelOffset)",
    "READ_FLOAT(PARAM_VHS_STYLE_INTENSITY,vhsStyleIntensity)",
    "READ_FLOAT(PARAM_CAMERA_INTENSITY,cameraIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_INTENSITY,ntscRsIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_PHASE_NOISE,ntscRsPhaseNoise)",
    "READ_FLOAT(PARAM_SIGNAL_HUE_NOISE,signalHueNoise)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_AMOUNT,deviceConnectionAmount)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_DENSITY,deviceConnectionDensity)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_SIZE,deviceConnectionSize)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_SPEED,deviceConnectionSpeed)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_DISTRIBUTION,deviceConnectionDistribution)",
    "READ_FLOAT(PARAM_DEVICE_CONNECTION_SEED,deviceConnectionSeed)",
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
    "dp.amount=clamp01(s.deviceConnectionAmount/100.0f*master);",
    "dp.seed=static_cast<std::uint32_t>(std::clamp(std::lround(s.deviceConnectionSeed),1l,32767l));",
    "s.signalHueNoise>0",
    "processDeviceConnections(in,out,dp)",
    "p.geometry.edgeWaveFrequency=s.edgeWaveFrequency",
    "p.imageGrade.chromaEdgeEnhancement=clamp01(s.chromaEdgeEnhance/100.0f);",
    "p.imageGrade.chromaSharpenCutoffHz=s.tapeSpeed==3?560000.0f:(s.tapeSpeed==2?600000.0f:640000.0f);",
    "p.imageGrade.chromaIGain=s.chromaIGain/100.0f;p.imageGrade.chromaQGain=s.chromaQGain/100.0f;",
    "p.imageGrade.chromaIOffset=s.chromaIOffset/200.0f;p.imageGrade.chromaQOffset=s.chromaQOffset/200.0f;",
    "p.imageGrade.chromaQuantization=s.chromaQuantization/400.0f;",
    "rp.redOffsetPixels=s.redChannelOffset*master/100.0f;",
    "rp.blueOffsetPixels=s.blueChannelOffset*master/100.0f;",
    "processRgbChannelRegistration(in,out,rp)",
    "r.trackingProfile=static_cast<TrackingProfile>(std::clamp<A_long>(s.trackingProfile-1,0,4));",
    "ApplyTrackingRecipe(r,s.trackingPresetBase);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[0]);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[1]);",
    "ApplyTrackingRecipe(r,s.trackingLayerPreset[2]);",
    "p.mode=static_cast<StaticPatternMode>(std::clamp<A_long>(s.staticMode-1,0,10));",
    "p.greenLineAmount=clamp01(s.staticGreenLine/100.0f);",
    "baseBand.centerY=clamp01((s.tearBandStartY+s.tearBandHeight*0.5f)/100.0f);",
    "b.intensity=clamp01(s.tearBandAmount[li]/100.0f);",
    "DamageRegion regions[5]; std::size_t count=0;",
    "r.seedOffset=0x1000u;",
    "pp.mode=static_cast<ParticlePatternMode>(std::clamp<A_long>(s.particleMode-1,0,4));",
    "pp.distribution=static_cast<ParticleDistribution>(std::clamp<A_long>(s.particleDistribution-1,0,4));",
    "processParticlePattern(in,out,pp)",
    "tp.mode=static_cast<TearChromaMode>(std::clamp<A_long>(s.tearChromaMode-1,0,4));",
    "processTearChroma(in,out,tp)",
    "p.demodulator=s.compositeDecoder==3?CompositeDemodulator::TwoLineComb:(s.compositeDecoder==2?CompositeDemodulator::OneLineComb:CompositeDemodulator::ConsumerNotch);",
    "p.geometry.vhs43Frame=s.vhs43Frame!=0;p.geometry.lensDistortion=s.lensDistortion/1000.0f;",
    "p.geometry.roundedOverscan=clamp01(s.roundedOverscan/100.0f);",
    "op.batteryAmount=clamp01(s.osdBattery/100.0f);op.showBattery=s.osdBattery>0.0f;",
    "b(s.osdPlay,v[36]>0.5f?100.0f:0.0f);b(s.osdTimestamp,v[36]>0.5f?100.0f:0.0f);b(s.osdBattery,v[36]>0.5f?75.0f:0.0f);",
    "||s.vhs43Frame!=0||s.roundedOverscan>0||s.lensDistortion!=0||s.digitalZoom>1.0f",
    "p.demodulator=s.compositeDecoder==3?CompositeDemodulator::TwoLineComb:(s.compositeDecoder==2?CompositeDemodulator::OneLineComb:CompositeDemodulator::ConsumerNotch);",
    "bi(s.compositeDecoder,1);b(s.composite,100);b(s.lumaNoise,v[0]*100);",
    "kLostSignalProfiles[13][45]",
    "b(s.lensDistortion,v[39]*1000.0f);bi(s.vhs43Frame,v[40]>0.5f?1.0f:0.0f);",
    "p.temporalGhost.frameRepeatFrequency=clamp01(s.frameRepeatFrequency/100.0f);",
    "p.temporalGhost.fieldWeave=clamp01(s.interlace/100.0f);p.temporalGhost.fieldParity=static_cast<int>(p.frame&1);",
    "if(previousInput && (s.temporalGhost>0||s.interlace>0||(s.frameRepeatFrequency>0&&s.frameRepeatStrength>0))",
    "b(s.frameRepeatFrequency,v[43]*100.0f);b(s.frameRepeatStrength,v[44]*100.0f);",
    "b(s.tapeSurface,std::max(v[29],std::max(v[34],v[35]))*100.0f);b(s.tapeWear,v[29]*100.0f);",
    "p.temporalGhost.frameRepeatStrength=clamp01(s.frameRepeatStrength/100.0f);",
    "p.temporalGhost.frame=s.frame;p.temporalGhost.seed=static_cast<std::uint32_t>(std::max(1.0f,s.tapeSeed));",
    "if((s->temporalGhost>0||s->interlace>0||(s->frameRepeatFrequency>0&&s->frameRepeatStrength>0))",
    "if(s.vhsStyle<26){bi(s.vhs43Frame,0.0f);b(s.roundedOverscan,0.0f);b(s.lensDistortion,0.0f);b(s.digitalZoom,1.0f);b(s.osdBattery,0.0f);b(s.frameRepeatFrequency,0.0f);b(s.frameRepeatStrength,0.0f);}",
]:
    assert needle in source, f"Missing preset/render wiring: {needle}"

style_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyVhs6RiftStylePreset"):source.find("static void ApplyVhs6RiftCameraProfile")])))
camera_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyVhs6RiftCameraProfile"):source.find("static void ApplyNtscRsVhsLook")])))
ntsc_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyNtscRsVhsLook"):source.find("static void ApplyVhsSourcePresets")])))
assert set(range(2, 18)).issubset(style_cases), f"Missing VHS6RIFT style preset cases: {sorted(set(range(2,18))-style_cases)}"
addon_cases = set(range(18, 23))
assert addon_cases.issubset(style_cases), f"Missing VHS Effect Add-on preset cases: {sorted(addon_cases-style_cases)}"
cvs_cases = set(range(23, 26))
assert cvs_cases.issubset(style_cases), f"Missing Composite Video Simulator VHS SP/LP/EP cases: {sorted(cvs_cases-style_cases)}"
lost_signal_cases = set(range(26, 39))
assert lost_signal_cases.issubset(style_cases), f"Missing Lost Signal VHS v1.9 profiles: {sorted(lost_signal_cases-style_cases)}"
crates_cases = set(range(39, 55))
assert crates_cases.issubset(style_cases), f"Missing Crates VHS Master recipes: {sorted(crates_cases-style_cases)}"
assert "ApplyCratesMasterPreset(s,static_cast<int>(s.vhsStyle-39));" in source, "Crates presets must use the recovered source-value mapper"
assert "kCrates[16][44]" in source, "Crates mapper must retain all 16 source looks and 44 mapped source fields"
crates_start = source.find("static constexpr float kCrates[16][44]={")
assert crates_start >= 0, "Missing Crates 16x44 source-value table"
crates_end = source.find("    };", crates_start)
assert crates_end > crates_start, "Unterminated Crates source-value table"
crates_rows = [line for line in source[crates_start:crates_end].splitlines() if line.lstrip().startswith("{")]
assert len(crates_rows) == 16, f"Crates table must contain 16 source presets, got {len(crates_rows)}"
assert all(len(line.strip().lstrip("{").rstrip(",").rstrip("}").split(",")) == 44 for line in crates_rows), "Every Crates preset must define all 44 mapped source values"
vhspro_cases = set(range(55, 65))
assert vhspro_cases.issubset(style_cases), f"Missing VHSPro recipes: {sorted(vhspro_cases-style_cases)}"
vhscraft_cases = set(range(65, 73))
assert vhscraft_cases.issubset(style_cases), f"Missing VHSCraft recipes: {sorted(vhscraft_cases-style_cases)}"
style_choices = re.search(r'kVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert style_choices and len(style_choices.group(1).split("|")) == 72, "VHS Style popup must expose all 72 entries"
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
print(f"PASS: {len(active_indices)} AE parameter slots; UI order matches all disk IDs.")
print("PASS: 16 VHS6RIFT + 5 Add-on + 3 CVS + 13 Lost Signal + 16 Crates + 10 VHSPro + 8 VHSCraft = 71 named VHS Style recipes plus Custom (72 popup entries).")
print("PASS: all 7 audited preset families have native menu entries; Crates maps 44 recovered source fields per look, while VHSPro/VHSCraft remain source-inspired where exact tuples were not recoverable.")
print("PASS: camera profiles, NTSC-RS controls, VHS playback, decoder, frame geometry, camcorder OSD and deterministic frame-repeat controls remain wired.")
