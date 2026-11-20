#!/usr/bin/env python3
"""Static audit for the imported VHS6RIFT/NTSC-RS preset UI wiring."""
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
    "READ_FLOAT(PARAM_VHS_STYLE_INTENSITY,vhsStyleIntensity)",
    "READ_FLOAT(PARAM_CAMERA_INTENSITY,cameraIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_INTENSITY,ntscRsIntensity)",
    "READ_FLOAT(PARAM_NTSCRS_PHASE_NOISE,ntscRsPhaseNoise)",
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
    "dp.amount=clamp01(s.deviceConnectionAmount/100.0f*master);",
    "dp.seed=static_cast<std::uint32_t>(std::clamp(std::lround(s.deviceConnectionSeed),1l,32767l));",
    "processDeviceConnections(in,out,dp)",
    "p.geometry.edgeWaveFrequency=s.edgeWaveFrequency",
]:
    assert needle in source, f"Missing preset/render wiring: {needle}"

style_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyVhs6RiftStylePreset"):source.find("static void ApplyVhs6RiftCameraProfile")])))
camera_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyVhs6RiftCameraProfile"):source.find("static void ApplyNtscRsVhsLook")])))
ntsc_cases = set(map(int, re.findall(r"case (\d+):", source[source.find("static void ApplyNtscRsVhsLook"):source.find("static void ApplyVhsSourcePresets")])))
assert style_cases == set(range(2, 18)), f"Missing VHS6RIFT style preset cases: {sorted(set(range(2,18))-style_cases)}"
assert camera_cases == set(range(2, 11)), f"Missing VHS6RIFT camera profile cases: {sorted(set(range(2,11))-camera_cases)}"
assert ntsc_cases == set(range(2, 12)), f"Missing NTSC-RS-derived look cases: {sorted(set(range(2,11))-ntsc_cases)}"

print(f"PASS: {len(active_indices)} AE parameter slots; UI order matches all disk IDs.")
print("PASS: 16 VHS6RIFT style presets, 9 camera profiles, 10 NTSC-RS-derived VHS looks.")
print("PASS: VHS6RIFT procedural grain/luma/chroma controls, Signal device-connection overlay, and NTSC-RS engine controls are wired.")
