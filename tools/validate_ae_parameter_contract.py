#!/usr/bin/env python3
"""Validate the native AE adapter's parameter order/count and PiPL UI contract."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/VHS90Effect.cpp").read_text(encoding="utf-8")
PIPL = (ROOT / "resources/VHS90PiPL.r").read_text(encoding="utf-8")
COMPOSITE = (ROOT / "Source/VHS90Core/VHS90CompositeEngine.cpp").read_text(encoding="utf-8")
REGION = (ROOT / "Source/VHS90Core/VHS90RegionEngine.cpp").read_text(encoding="utf-8")
REGION_HEADER = (ROOT / "Source/VHS90Core/VHS90RegionEngine.h").read_text(encoding="utf-8")
SYNC_ENGINE = (ROOT / "Source/VHS90Core/VHS90SyncEngine.cpp").read_text(encoding="utf-8")
IMAGE_GRADE = (ROOT / "Source/VHS90Core/VHS90ImageGradeEngine.cpp").read_text(encoding="utf-8")
RF_ENGINE = (ROOT / "Source/VHS90Core/VHS90RfInstabilityEngine.cpp").read_text(encoding="utf-8")

def fail(message: str) -> None:
    raise SystemExit(f"AE CONTRACT ERROR: {message}")

enum_match = re.search(r"enum\s+ParamIndex\s*\{(.*?)\};", SOURCE, re.S)
if not enum_match:
    fail("ParamIndex enum was not found.")
enum_items = []
for raw in enum_match.group(1).split(","):
    raw = raw.split("//", 1)[0].strip()
    match = re.match(r"(PARAM_[A-Z0-9_]+)", raw)
    if match:
        enum_items.append(match.group(1))
if not enum_items or enum_items[0] != "PARAM_INPUT" or enum_items[-1] != "PARAM_COUNT":
    fail("ParamIndex must start with PARAM_INPUT and end with PARAM_COUNT.")
expected_params = enum_items[1:-1]

setup_match = re.search(r"static\s+PF_Err\s+ParamsSetup\s*\([^)]*\)\s*\{", SOURCE)
if not setup_match:
    fail("ParamsSetup function was not found.")
brace_start = SOURCE.find("{", setup_match.start())
depth = 0
brace_end = None
for pos in range(brace_start, len(SOURCE)):
    if SOURCE[pos] == "{":
        depth += 1
    elif SOURCE[pos] == "}":
        depth -= 1
        if depth == 0:
            brace_end = pos + 1
            break
if brace_end is None:
    fail("ParamsSetup braces are unbalanced.")
setup_body = SOURCE[brace_start:brace_end]
calls = re.findall(r"\b(PF_ADD_[A-Z0-9_]+|PF_END_TOPIC|VHS90_ADD_HIDDEN_[A-Z0-9_]+|VHS90_END_HIDDEN_TOPIC)\s*\(([^;\n]*)\)\s*;", setup_body)
if any(macro == "PF_ADD_LAYER" for macro, _ in calls):
    fail("Do not add a second Source layer: AE already supplies PARAM_INPUT at index 0.")

end_map = {
    "ID_SIGNAL_TOPIC": "PARAM_SIGNAL_END", "ID_STATIC_TOPIC": "PARAM_STATIC_END",
    "ID_TRACKING_TOPIC": "PARAM_TRACKING_END", "ID_HEAD_TOPIC": "PARAM_HEAD_END",
    "ID_TAPE_MATERIAL_TOPIC": "PARAM_TAPE_MATERIAL_END", "ID_PHYSICAL_TOPIC": "PARAM_PHYSICAL_END",
    "ID_PYVHS_TOPIC": "PARAM_PYVHS_END", "ID_OSD_TOPIC": "PARAM_OSD_END",
    "ID_VHS_STYLE_TOPIC": "PARAM_VHS_STYLE_END", "ID_NTSCRS_TOPIC": "PARAM_NTSCRS_END",
    "ID_DEVICE_CONNECTION_TOPIC": "PARAM_DEVICE_CONNECTION_END", "ID_VHS_PLAYBACK_TOPIC": "PARAM_VHS_PLAYBACK_END",
    "ID_COMPOSITE_DECODER_TOPIC": "PARAM_COMPOSITE_DECODER_END", "ID_VHS_FRAME_TOPIC": "PARAM_VHS_FRAME_END",
    "ID_CAMCORDER_HUD_TOPIC": "PARAM_CAMCORDER_HUD_END", "ID_FRAME_REPEAT_TOPIC": "PARAM_FRAME_REPEAT_END",
    "ID_PARTICLE_TOPIC": "PARAM_PARTICLE_END", "ID_TEAR_CHROMA_TOPIC": "PARAM_TEAR_CHROMA_END",
    "ID_TRACK_LAYER2_TOPIC": "PARAM_TRACK_LAYER2_END", "ID_TRACK_LAYER3_TOPIC": "PARAM_TRACK_LAYER3_END",
    "ID_TRACK_LAYER4_TOPIC": "PARAM_TRACK_LAYER4_END", "ID_TRACKING_PRESETS_TOPIC": "PARAM_TRACKING_PRESETS_END",
    "ID_PATTERN_TOPIC": "PARAM_PATTERN_END", "ID_TEAR_BAND2_TOPIC": "PARAM_TEAR_BAND2_END",
    "ID_TEAR_BAND3_TOPIC": "PARAM_TEAR_BAND3_END", "ID_TEAR_BAND4_TOPIC": "PARAM_TEAR_BAND4_END",
    "ID_TEAR_BAND5_TOPIC": "PARAM_TEAR_BAND5_END", "ID_OSD_LAYOUT_TOPIC": "PARAM_OSD_LAYOUT_END",
    "ID_OSD_COUNTER_TOPIC": "PARAM_OSD_COUNTER_END", "ID_OSD_LOCALIZATION_TOPIC": "PARAM_OSD_LOCALIZATION_END",
    "ID_OSD_WORDS_TOPIC": "PARAM_OSD_WORDS_END",
}
aliases = {
    "ID_COMPOSITE": "PARAM_COMPOSITE_AMOUNT", "ID_SYNC": "PARAM_SYNC_AMOUNT",
    "ID_TRACKING_X": "PARAM_TRACKING_CENTER_X", "ID_TRACKING_Y": "PARAM_TRACKING_CENTER_Y",
    "ID_HEAD_Y": "PARAM_HEAD_BOTTOM_Y",
}
actual_params = []
for macro, args in calls:
    if macro in ("PF_END_TOPIC", "VHS90_END_HIDDEN_TOPIC"):
        match = re.search(r"(ID_[A-Z0-9_]+)\s*\+\s*100", args)
        if not match or match.group(1) not in end_map:
            fail(f"Unknown topic-end parameter registration: {args.strip()}")
        actual_params.append(end_map[match.group(1)])
        continue
    ids = re.findall(r"\bID_[A-Z0-9_]+\b", args)
    if not ids:
        fail(f"Parameter registration has no disk ID: {macro}({args.strip()})")
    disk_id = ids[0] if macro in ("PF_ADD_ARBITRARY2", "VHS90_ADD_HIDDEN_ARBITRARY2") else ids[-1]
    actual_params.append(aliases.get(disk_id, "PARAM_" + disk_id[3:]))

if actual_params != expected_params:
    mismatch = next((i for i, pair in enumerate(zip(actual_params, expected_params), 1) if pair[0] != pair[1]), None)
    fail(f"Parameter registration order/count differs from ParamIndex (registered={len(actual_params)}, enum={len(expected_params)}, first_mismatch={mismatch}).")
if len(enum_items) - 1 != len(calls) + 1:
    fail(f"PARAM_COUNT must include the implicit input layer (enum={len(enum_items)-1}, registered={len(calls)}).")
if not re.search(r"out_data->num_params\s*=\s*PARAM_COUNT\s*;", SOURCE):
    fail("ParamsSetup must publish out_data->num_params = PARAM_COUNT.")

global_setup = re.search(r"static\s+PF_Err\s+GlobalSetup[\s\S]*?\n\}", SOURCE)
if not global_setup or "PF_OutFlag_CUSTOM_UI" not in global_setup.group(0):
    fail("GlobalSetup must advertise PF_OutFlag_CUSTOM_UI for the arbitrary-data UI dimensions.")
flags = re.search(r"AE_Effect_Global_OutFlags\s*\{\s*(0x[0-9A-Fa-f]+)\s*\}", PIPL)
if not flags or (int(flags.group(1), 16) & (1 << 15)) == 0:
    fail("PiPL AE_Effect_Global_OutFlags must include PF_OutFlag_CUSTOM_UI (0x00008000).")


# Rendering contract: keep the host on the VHS6RIFT-proven legacy render path.
global_setup = re.search(r"static\s+PF_Err\s+GlobalSetup[\s\S]*?\n\}", SOURCE)
if not global_setup:
    fail("GlobalSetup function was not found.")
global_body = global_setup.group(0)
if "PF_OutFlag2_SUPPORTS_SMART_RENDER" in global_body or "PF_OutFlag2_FLOAT_COLOR_AWARE" in global_body:
    fail("Do not advertise SmartFX/float-color flags until SmartRender has AE runtime coverage.")
if "out_data->out_flags2=PF_OutFlag2_SUPPORTS_THREADED_RENDERING" not in global_body:
    fail("Legacy render mode must retain threaded-rendering support.")
flags2 = re.search(r"AE_Effect_Global_OutFlags_2\s*\{\s*(0x[0-9A-Fa-f]+)\s*\}", PIPL)
if not flags2:
    fail("PiPL Global_OutFlags_2 was not found.")
pipl_flags2 = int(flags2.group(1), 16)
if (pipl_flags2 & (1 << 10)) or (pipl_flags2 & (1 << 12)):
    fail("PiPL must not advertise Smart Render or float-color-aware without SmartFX.")
if (pipl_flags2 & (1 << 27)) == 0:
    fail("PiPL must retain threaded-rendering support.")
if not re.search(r"case\s+PF_Cmd_RENDER\s*:\s*err=LegacyRender", SOURCE):
    fail("EffectMain must route PF_Cmd_RENDER through LegacyRender.")

# Every parameter consumed by the previous SmartFX reader must also be
# consumed by the active legacy renderer. This prevents UI controls from
# silently becoming inert when using the VHS6RIFT-compatible host path.
read_start = SOURCE.find("static PF_Err ReadSettings")
read_end = SOURCE.find("static bool ConvertInputWorld", read_start)
legacy_start = SOURCE.find("static PF_Err LegacyRender")
legacy_end = SOURCE.find("} // namespace", legacy_start)
if min(read_start, read_end, legacy_start, legacy_end) < 0:
    fail("ReadSettings or LegacyRender function was not found.")
read_params = set(re.findall(r"\bPARAM_[A-Z0-9_]+\b", SOURCE[read_start:read_end]))
legacy_params = set(re.findall(r"\bPARAM_[A-Z0-9_]+\b", SOURCE[legacy_start:legacy_end]))
unwired = sorted(read_params - legacy_params)
if unwired:
    fail("LegacyRender does not read these registered render controls: " + ", ".join(unwired))

# Regression guards for previously confirmed inert/limited controls.
if 'PF_ADD_FLOAT_SLIDERX("Master Strength",0,500,0,500,100' not in SOURCE:
    fail("Master Strength must expose a 0..500 range.")
if "std::clamp(s.master/100.0f,0.0f,5.0f)" not in SOURCE:
    fail("Master Strength must preserve values above 100 percent in the render adapter.")
if "std::min(5.0f,p.strength)" not in COMPOSITE:
    fail("Composite engine must support the extended Master Strength blend range.")
if "if(s.rfDrift>0||s.rfHum>0||s.rfAm>0||s.agcPump>0)" not in SOURCE:
    fail("AGC Pump alone must activate the RF instability stage.")
if 'PF_ADD_FLOAT_SLIDERX("Amount",0,100,0,100,22,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_AMOUNT)' not in SOURCE:
    fail("New instances must start with visible, enabled Tracking Noise.")
if 'PF_ADD_FLOAT_SLIDERX("Motion Y",-100,100,-100,100,12,1,PF_ValueDisplayFlag_PERCENT,0,ID_TRACKING_MOTION_Y)' not in SOURCE:
    fail("Default Tracking Noise must move vertically.")
if "wrapMovingCenter" not in REGION:
    fail("Tracking regions must wrap through the frame instead of drifting off-screen permanently.")

# The visible AE panel is intentionally curated; legacy parameters remain hidden
# to preserve saved-project disk IDs and render compatibility.
visible_topics = re.findall(r'PF_ADD_TOPIC\("([^"]+)"', setup_body)
expected_topics = [
    "VHS Style & Camera Profiles", "Chroma Offset",
    "Grain & Tracking Noise", "Signal", "Head Switching", "Ringing", "Waves & Static",
    "Static", "Color & Image", "Glow", "Tape", "Text / OSD",
]
if visible_topics != expected_topics:
    fail("Visible VHS-90 topic order/layout differs from the approved compact panel.")
if len(re.findall(r'kVhsStyleChoices\[\]="([^"]+)"', SOURCE)[0].split("|")) != 20:
    fail("VHS Style list must remain curated to 20 distinct presets.")
if len(re.findall(r'kTrackingPresetChoices\[\]="([^"]+)"', SOURCE)[0].split("|")) != 19:
    fail("Tracking presets must include the source-backed plugin and repository profiles.")
if 'PF_ADD_POPUP("Tracking Preset",19,1,kTrackingPresetChoices,ID_TRACKING_PRESET_BASE)' not in SOURCE:
    fail("Tracking Noise preset popup must expose all 19 source recipes.")
if "Vhs6RiftBand = 5" not in REGION_HEADER or "NtscRTracking = 6" not in REGION_HEADER or "VhsMainTracking = 7" not in REGION_HEADER:
    fail("VHS6RIFT and NTSC-R need dedicated tracking render profiles.")
if "Vhs6RiftBand" not in REGION or "NtscRTracking" not in REGION:
    fail("Source-backed tracking render profiles are not implemented.")
if "lineNoise" in SYNC_ENGINE or "const float noise=bandDamage*" not in SYNC_ENGINE:
    fail("Sync Collapse must not add full-frame static grain.")
if "chromaMagnitude" not in IMAGE_GRADE or "quantizedMagnitude" not in IMAGE_GRADE:
    fail("Chroma Quantization must preserve hue by quantizing chroma magnitude.")
if "std::clamp(p.saturation,0.0f,5.0f)" not in IMAGE_GRADE:
    fail("Image Grade must support the extended saturation range.")
if "sceneDrive" not in RF_ENGINE or "randomDrift" not in RF_ENGINE:
    fail("AGC Pump and RF Drift must use the rebuilt scene/time-base models.")

print(f"AE host contract OK: {len(actual_params)} registered parameters + implicit input, exact enum order, PARAM_COUNT, CUSTOM_UI flags, and VHS6RIFT-compatible legacy render flags.")
