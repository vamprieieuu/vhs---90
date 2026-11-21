#!/usr/bin/env python3
"""Validate the native AE adapter's parameter order/count and PiPL UI contract."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/VHS90Effect.cpp").read_text(encoding="utf-8")
PIPL = (ROOT / "resources/VHS90PiPL.r").read_text(encoding="utf-8")

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
calls = re.findall(r"\b(PF_ADD_[A-Z0-9_]+|PF_END_TOPIC)\s*\(([^;\n]*)\)\s*;", setup_body)
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
    if macro == "PF_END_TOPIC":
        match = re.search(r"(ID_[A-Z0-9_]+)\s*\+\s*100", args)
        if not match or match.group(1) not in end_map:
            fail(f"Unknown topic-end parameter registration: {args.strip()}")
        actual_params.append(end_map[match.group(1)])
        continue
    ids = re.findall(r"\bID_[A-Z0-9_]+\b", args)
    if not ids:
        fail(f"Parameter registration has no disk ID: {macro}({args.strip()})")
    disk_id = ids[0] if macro == "PF_ADD_ARBITRARY2" else ids[-1]
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


# SmartFX rectangle contract: output bounds must be sourced from the layer
# checkout, not copied from an arbitrary output request ROI.
pre_render_match = re.search(r"static\s+PF_Err\s+SmartPreRender\s*\([^)]*\)\s*\{", SOURCE)
if not pre_render_match:
    fail("SmartPreRender function was not found.")
start = SOURCE.find("{", pre_render_match.start())
depth = 0
end = None
for pos in range(start, len(SOURCE)):
    if SOURCE[pos] == "{":
        depth += 1
    elif SOURCE[pos] == "}":
        depth -= 1
        if depth == 0:
            end = pos + 1
            break
pre_render_body = SOURCE[start:end] if end else ""
if "extra->output->result_rect=result.result_rect" not in pre_render_body:
    fail("SmartPreRender must publish the checked-out input result_rect.")
if "extra->output->max_result_rect=result.max_result_rect" not in pre_render_body:
    fail("SmartPreRender must publish the checked-out input max_result_rect.")
if "extra->output->result_rect=extra->input->output_request.rect" in pre_render_body:
    fail("SmartPreRender must not report the raw requested ROI as result_rect.")
if "unionRect(extra->output->max_result_rect,extra->output->result_rect)" not in pre_render_body:
    fail("SmartPreRender must guarantee max_result_rect encloses result_rect.")

print(f"AE host contract OK: {len(actual_params)} registered parameters + implicit input, exact enum order, PARAM_COUNT, GlobalSetup/PiPL CUSTOM_UI flags.")
