#!/usr/bin/env python3
"""Static audit for imported VHS6RIFT, NTSC-RS, CVS and Lost Signal VHS preset/UI wiring."""
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
assert registered_ids == disk_ids, "PF_ADD registration order differs from ParamDiskID order"

# Confirm the new UI controls are read in SmartFX and legacy paths.
required_reads = [
    "CheckoutPopup(in_data,PARAM_VHS_STYLE,s.vhsStyle)",
    "CheckoutPopup(in_data,PARAM_CAMERA_PROFILE,s.cameraProfile)",
    "CheckoutPopup(in_data,PARAM_NTSCRS_LOOK,s.ntscRsLook)",
    "CheckoutPopup(in_data,PARAM_COMPOSITE_DECODER,s.compositeDecoder)",
    "CheckoutCheckbox(in_data,PARAM_VHS_43_FRAME,s.vhs43Frame)",
    "READ_FLOAT(PARAM_ROUNDED_OVERSCAN,roundedOverscan)",
    "READ_FLOAT(PARAM_LENS_DISTORTION,lensDistortion)",
    "READ_FLOAT(PARAM_DIGITAL_ZOOM,digitalZoom)",
    "READ_FLOAT(PARAM_OSD_BATTERY,osdBattery)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_FREQUENCY,frameRepeatFrequency)",
    "READ_FLOAT(PARAM_FRAME_REPEAT_STRENGTH,frameRepeatStrength)",
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
    "b(s.pyVhsGeneration,1.0f+v[28]*0.8f);b(s.tapeSurface,std::max(v[29],std::max(v[34],v[35]))*100.0f);b(s.tapeWear,v[29]*100.0f);",
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
style_choices = re.search(r'kVhsStyleChoices\[\]\s*=\s*"([^"]+)"', source)
assert style_choices and len(style_choices.group(1).split("|")) == 38, "VHS Style popup must expose all 38 entries"
assert camera_cases == set(range(2, 11)), f"Missing VHS6RIFT camera profile cases: {sorted(set(range(2,11))-camera_cases)}"
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
print("PASS: 16 VHS6RIFT + 5 VHS Effect Add-on + 3 Composite Video Simulator VHS speed presets + 13 Lost Signal VHS v1.9 profiles, 9 camera profiles, 10 NTSC-RS-derived VHS looks.")
print("PASS: VHS6RIFT grain controls, Add-on, CVS and Lost Signal VHS preset wiring, NTSC-RS controls, and independent VHS chroma sharpening and Consumer Notch / One-Line Comb / Two-Line Comb decoder and VHS 4:3 frame/rounded overscan/lens/zoom and camcorder battery OSD and deterministic frame-repeat controls are wired.")
