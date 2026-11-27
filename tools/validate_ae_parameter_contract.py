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
if enum_items[-55:-52] != ["PARAM_GROUND_LOOP_TOPIC", "PARAM_GROUND_LOOP", "PARAM_GROUND_LOOP_END"]:
    fail("Ground Loop parameters must remain appended after all legacy AE parameters.")
if enum_items[-52:-41] != ["PARAM_TRACK_LAYER_MOTION_TOPIC", "PARAM_TRACK_LAYER2_ANGLE", "PARAM_TRACK_LAYER2_MOTION_X", "PARAM_TRACK_LAYER2_MOTION_Y", "PARAM_TRACK_LAYER3_ANGLE", "PARAM_TRACK_LAYER3_MOTION_X", "PARAM_TRACK_LAYER3_MOTION_Y", "PARAM_TRACK_LAYER4_ANGLE", "PARAM_TRACK_LAYER4_MOTION_X", "PARAM_TRACK_LAYER4_MOTION_Y", "PARAM_TRACK_LAYER_MOTION_END"]:
    fail("Tracking layer direction/motion parameters must remain appended after the previous AE controls.")
expected_sync_damage_tail = ["PARAM_SYNC_DAMAGE_TOPIC", "PARAM_SYNC_DAMAGE_ENABLE", "PARAM_SYNC_DAMAGE_TEAR_AMOUNT", "PARAM_SYNC_DAMAGE_BAND_INTENSITY", "PARAM_SYNC_DAMAGE_BAND_DENSITY", "PARAM_SYNC_DAMAGE_BAND_HEIGHT", "PARAM_SYNC_DAMAGE_BAND_IRREGULARITY", "PARAM_SYNC_DAMAGE_DISPLACEMENT", "PARAM_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION", "PARAM_SYNC_DAMAGE_COVERAGE", "PARAM_SYNC_DAMAGE_LUMA_ENABLE", "PARAM_SYNC_DAMAGE_LUMA_INSTABILITY", "PARAM_SYNC_DAMAGE_BRIGHT_FLASH", "PARAM_SYNC_DAMAGE_DARK_COLLAPSE", "PARAM_SYNC_DAMAGE_EVENT_FREQUENCY", "PARAM_SYNC_DAMAGE_EVENT_DURATION", "PARAM_SYNC_DAMAGE_TRANSITION_SHARPNESS", "PARAM_SYNC_DAMAGE_LOCAL_LUMA", "PARAM_SYNC_DAMAGE_MANUAL_BRIGHT", "PARAM_SYNC_DAMAGE_MANUAL_DARK", "PARAM_SYNC_DAMAGE_DROPOUTS_ENABLE", "PARAM_SYNC_DAMAGE_DROPOUT_DENSITY", "PARAM_SYNC_DAMAGE_STREAK_LENGTH", "PARAM_SYNC_DAMAGE_STREAK_BRIGHTNESS", "PARAM_SYNC_DAMAGE_DARK_DROPOUT", "PARAM_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION", "PARAM_SYNC_DAMAGE_TEMPORAL_VARIATION", "PARAM_SYNC_DAMAGE_CHROMA_ENABLE", "PARAM_SYNC_DAMAGE_CHROMA_TINT", "PARAM_SYNC_DAMAGE_MAGENTA_TINT", "PARAM_SYNC_DAMAGE_GREEN_TINT", "PARAM_SYNC_DAMAGE_RGB_OFFSET", "PARAM_SYNC_DAMAGE_CHROMA_BLEED", "PARAM_SYNC_DAMAGE_ANIMATION_MODE", "PARAM_SYNC_DAMAGE_INPUT_MODE", "PARAM_SYNC_DAMAGE_ANIMATION_SPEED", "PARAM_SYNC_DAMAGE_RANDOM_SEED", "PARAM_SYNC_DAMAGE_BLEND_INPUT", "PARAM_SYNC_DAMAGE_EFFECT_AMOUNT", "PARAM_SYNC_DAMAGE_END"]
if enum_items[-41:-1] != expected_sync_damage_tail:
    fail("Sync Collapse & Luma Dropout controls must be appended without renumbering legacy AE parameters.")
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

# The additional Damage Lab tracking layers are intended to be independently
# editable in Effect Controls. Hiding their registrations would leave the
# render controls wired but inaccessible to users.
tracking_layers_start = setup_body.find('PF_ADD_TOPIC("Tracking Layer 2"')
tracking_layers_end = setup_body.find("PF_END_TOPIC(ID_TRACK_LAYER4_TOPIC+100)", tracking_layers_start)
if tracking_layers_start < 0 or tracking_layers_end < 0:
    fail("Tracking Layers 2–4 UI registration block is missing.")
tracking_layers_ui = setup_body[tracking_layers_start:tracking_layers_end]
if "PF_PUI_NO_ECW_UI" in tracking_layers_ui:
    fail("Tracking Layers 2–4 controls must remain visible/editable in Effect Controls.")
if 'PF_ADD_POPUP("Tracking Type",5,1,kTrackingProfileChoices,ID_TRACKING_PROFILE)' not in setup_body:
    fail("The primary Tracking Type selector must remain visible/editable in Effect Controls.")

# Multi-band Tear is a five-region engine. Bands 2–5 must not be registered as
# empty Effect Controls topics: their independent geometry/signal controls are
# part of the editable Damage Lab workflow.
tear_bands_start = setup_body.find('PF_ADD_TOPIC("Tear Band 2"')
tear_bands_end = setup_body.find("PF_END_TOPIC(ID_TEAR_BAND5_TOPIC+100)", tear_bands_start)
if tear_bands_start < 0 or tear_bands_end < 0:
    fail("Multi-band Tear controls (Bands 2–5) are missing.")
tear_bands_ui = setup_body[tear_bands_start:tear_bands_end]
if "PF_PUI_NO_ECW_UI" in tear_bands_ui:
    fail("Tear Bands 2–5 must expose their independent controls in Effect Controls.")

# The Damage Lab's primary spatial, static, tape-surface and head-switching
# controls are user-facing controls, not hidden SmartFX-only parameters.
def topic_body(topic_name: str) -> str:
    start = setup_body.find('PF_ADD_TOPIC("' + topic_name + '"')
    if start < 0:
        fail("Missing Damage Lab topic: " + topic_name)
    end = setup_body.find("PF_END_TOPIC(", start)
    if end < 0:
        fail("Unclosed Damage Lab topic: " + topic_name)
    return setup_body[start:end]

for topic_name in ("Waves / Static", "Grain", "Tracking Noise", "Head Switching", "Tape", "Mains Hum"):
    body = topic_body(topic_name)
    if "PF_PUI_NO_ECW_UI" in body:
        fail("Damage Lab topic must remain visible/editable: " + topic_name)

if "READ_FLOAT(PARAM_GROUND_LOOP,groundLoop)" not in SOURCE:
    fail("Ground Loop must be read from its AE parameter into RenderSettings.")
for token in (
    "PARAM_SYNC_DAMAGE_TEAR_AMOUNT", "PARAM_SYNC_DAMAGE_BAND_INTENSITY", "PARAM_SYNC_DAMAGE_BAND_DENSITY",
    "PARAM_SYNC_DAMAGE_BAND_HEIGHT", "PARAM_SYNC_DAMAGE_BAND_IRREGULARITY", "PARAM_SYNC_DAMAGE_DISPLACEMENT",
    "PARAM_SYNC_DAMAGE_BOUNDARY_FRAGMENTATION", "PARAM_SYNC_DAMAGE_COVERAGE", "PARAM_SYNC_DAMAGE_LUMA_INSTABILITY",
    "PARAM_SYNC_DAMAGE_BRIGHT_FLASH", "PARAM_SYNC_DAMAGE_DARK_COLLAPSE", "PARAM_SYNC_DAMAGE_EVENT_FREQUENCY",
    "PARAM_SYNC_DAMAGE_EVENT_DURATION", "PARAM_SYNC_DAMAGE_TRANSITION_SHARPNESS", "PARAM_SYNC_DAMAGE_LOCAL_LUMA",
    "PARAM_SYNC_DAMAGE_MANUAL_BRIGHT", "PARAM_SYNC_DAMAGE_MANUAL_DARK", "PARAM_SYNC_DAMAGE_DROPOUT_DENSITY",
    "PARAM_SYNC_DAMAGE_STREAK_LENGTH", "PARAM_SYNC_DAMAGE_STREAK_BRIGHTNESS", "PARAM_SYNC_DAMAGE_DARK_DROPOUT",
    "PARAM_SYNC_DAMAGE_HORIZONTAL_FRAGMENTATION", "PARAM_SYNC_DAMAGE_TEMPORAL_VARIATION", "PARAM_SYNC_DAMAGE_CHROMA_TINT",
    "PARAM_SYNC_DAMAGE_MAGENTA_TINT", "PARAM_SYNC_DAMAGE_GREEN_TINT", "PARAM_SYNC_DAMAGE_RGB_OFFSET",
    "PARAM_SYNC_DAMAGE_CHROMA_BLEED", "PARAM_SYNC_DAMAGE_ANIMATION_SPEED", "PARAM_SYNC_DAMAGE_RANDOM_SEED",
    "PARAM_SYNC_DAMAGE_BLEND_INPUT", "PARAM_SYNC_DAMAGE_EFFECT_AMOUNT",
):
    if f"READ_FLOAT({token}," not in SOURCE:
        fail("Sync Collapse & Luma Dropout float control is not read: " + token)
for token in ("PARAM_SYNC_DAMAGE_ENABLE", "PARAM_SYNC_DAMAGE_LUMA_ENABLE", "PARAM_SYNC_DAMAGE_DROPOUTS_ENABLE", "PARAM_SYNC_DAMAGE_CHROMA_ENABLE"):
    if f"CheckoutCheckbox(in_data,{token}," not in SOURCE:
        fail("Sync Collapse & Luma Dropout checkbox is not read: " + token)
for token in ("PARAM_SYNC_DAMAGE_ANIMATION_MODE", "PARAM_SYNC_DAMAGE_INPUT_MODE"):
    if f"CheckoutPopup(in_data,{token}," not in SOURCE:
        fail("Sync Collapse & Luma Dropout popup is not read: " + token)
for layer in (2, 3, 4):
    for field in ("ANGLE", "MOTION_X", "MOTION_Y"):
        token = f"READ_FLOAT(PARAM_TRACK_LAYER{layer}_{field},"
        if token not in SOURCE:
            fail("Tracking layer direction/motion control is not wired: " + token)

# Keep the visible mode menus synchronized with the native engine enums.
tear_choices = re.search(r'kTearChromaModeChoices\[\]\s*=\s*"([^"]+)"', SOURCE)
static_choices = re.search(r'kStaticPatternChoices\[\]\s*=\s*"([^"]+)"', SOURCE)
if not tear_choices or len(tear_choices.group(1).split("|")) != 5:
    fail("Tear & Chroma must expose exactly five native engine modes.")
if not static_choices or len(static_choices.group(1).split("|")) != 13:
    fail("Procedural Static must expose exactly thirteen native engine modes.")
if 'PF_ADD_POPUP("Tear & Chroma Mode",5,5,kTearChromaModeChoices,ID_TEAR_CHROMA_MODE)' not in setup_body:
    fail("The five Tear & Chroma modes must remain visible in Effect Controls.")
if 'PF_ADD_POPUP("Pattern Type",13,2,kStaticPatternChoices,ID_PATTERN_MODE)' not in setup_body:
    fail("All thirteen procedural static modes must remain visible in Effect Controls.")

for required in [
    'PF_ADD_CHECKBOX("Enable Sync Collapse","Enable",0,0,ID_SYNC_DAMAGE_ENABLE)',
    'PF_ADD_FLOAT_SLIDERX("Horizontal Tear Amount",0,100',
    'PF_ADD_FLOAT_SLIDERX("Band Intensity",0,100',
    'PF_ADD_FLOAT_SLIDERX("Band Density",1,32',
    'PF_ADD_FLOAT_SLIDERX("Band Height",1,100',
    'PF_ADD_FLOAT_SLIDERX("Band Irregularity",0,100',
    'PF_ADD_FLOAT_SLIDERX("Horizontal Displacement (px)",0,160',
    'PF_ADD_FLOAT_SLIDERX("Boundary Fragmentation",0,100',
    'PF_ADD_FLOAT_SLIDERX("Full-Frame Coverage",0,100',
    'PF_ADD_CHECKBOX("Enable Luma Instability","Enable",1,0,ID_SYNC_DAMAGE_LUMA_ENABLE)',
    'PF_ADD_FLOAT_SLIDERX("Luma Instability",0,100',
    'PF_ADD_FLOAT_SLIDERX("Bright Flash Intensity",0,100',
    'PF_ADD_FLOAT_SLIDERX("Dark Collapse Intensity",0,100',
    'PF_ADD_FLOAT_SLIDERX("Event Frequency (Hz)",0.1,10',
    'PF_ADD_FLOAT_SLIDERX("Event Duration (s)",0.05,2',
    'PF_ADD_FLOAT_SLIDERX("Transition Sharpness",0,100',
    'PF_ADD_FLOAT_SLIDERX("Local Band Luma Variation",0,100',
    'PF_ADD_FLOAT_SLIDERX("Manual Bright Pulse",0,100',
    'PF_ADD_FLOAT_SLIDERX("Manual Dark Collapse",0,100',
    'PF_ADD_CHECKBOX("Enable Fine Dropouts","Enable",1,0,ID_SYNC_DAMAGE_DROPOUTS_ENABLE)',
    'PF_ADD_FLOAT_SLIDERX("Dropout Streak Density",0,100',
    'PF_ADD_FLOAT_SLIDERX("Streak Length (px)",2,120',
    'PF_ADD_FLOAT_SLIDERX("Streak Brightness",0,100',
    'PF_ADD_FLOAT_SLIDERX("Dark Dropout Amount",0,100',
    'PF_ADD_FLOAT_SLIDERX("Horizontal Fragmentation",0,100',
    'PF_ADD_FLOAT_SLIDERX("Temporal Variation",0,100',
    'PF_ADD_CHECKBOX("Enable Chromatic Variation","Enable",1,0,ID_SYNC_DAMAGE_CHROMA_ENABLE)',
    'PF_ADD_FLOAT_SLIDERX("Subtle Chroma Tint",0,100',
    'PF_ADD_FLOAT_SLIDERX("Magenta Tint",0,100',
    'PF_ADD_FLOAT_SLIDERX("Green Tint",0,100',
    'PF_ADD_FLOAT_SLIDERX("RGB Offset (px)",0,8',
    'PF_ADD_FLOAT_SLIDERX("Chroma Bleed",0,100',
    'PF_ADD_POPUP("Animation Mode",3,1,kSyncDamageAnimationChoices',
    'PF_ADD_POPUP("Input Mode",2,1,kSyncDamageInputChoices',
    'PF_ADD_FLOAT_SLIDERX("Animation Speed",0,400',
    'PF_ADD_FLOAT_SLIDERX("Random Seed",1,32767',
    'PF_ADD_FLOAT_SLIDERX("Blend With Input",0,100',
    'PF_ADD_FLOAT_SLIDERX("Effect Amount",0,100',
]:
    if required not in setup_body:
        fail("Missing Sync Collapse & Luma Dropout control: " + required)
sync_topic = topic_body("Sync Collapse & Luma Dropout")
if "PF_PUI_NO_ECW_UI" in sync_topic:
    fail("Sync Collapse & Luma Dropout controls must remain visible in Effect Controls.")

for required in [
    'PF_ADD_FLOAT_SLIDERX("Master Strength",0,300,0,300,100',
    'PF_ADD_FLOAT_SLIDERX("Bottom Edge Y",0,100,0,100,100',
    'PF_ADD_FLOAT_SLIDERX("Height",0,100,0,100,4,1,PF_ValueDisplayFlag_PERCENT,0,ID_HEAD_HEIGHT)',
    'PF_ADD_FLOAT_SLIDERX("Horizontal Shift",0,100,0,100,24',
    'PF_ADD_FLOAT_SLIDERX("Region Angle",-180,180',
    'PF_ADD_FLOAT_SLIDERX("Motion X",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Motion Y",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Static Amount",0,100',
    'PF_ADD_FLOAT_SLIDERX("Tape Material Master",0,100',
    'PF_ADD_FLOAT_SLIDERX("Mains Hum",0,100',
    'PF_ADD_FLOAT_SLIDERX("Ground Loop (mV)",0,500,0,500,0,1,PF_ValueDisplayFlag_NONE,0,ID_GROUND_LOOP)',
    'PF_ADD_FLOAT_SLIDERX("Layer 2 Angle",-180,180',
    'PF_ADD_FLOAT_SLIDERX("Layer 2 Motion X",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Layer 2 Motion Y",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Layer 3 Angle",-180,180',
    'PF_ADD_FLOAT_SLIDERX("Layer 3 Motion X",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Layer 3 Motion Y",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Layer 4 Angle",-180,180',
    'PF_ADD_FLOAT_SLIDERX("Layer 4 Motion X",-100,100',
    'PF_ADD_FLOAT_SLIDERX("Layer 4 Motion Y",-100,100',
]:
    if required not in setup_body:
        fail("Missing required visible Damage Lab control: " + required)

for band in range(2, 6):
    if f'PF_ADD_TOPIC("Tear Band {band}"' not in tear_bands_ui:
        fail(f"Missing Tear Band {band} topic.")


tracking_presets_start = setup_body.find('PF_ADD_TOPIC("Tracking Presets"')
tracking_presets_end = setup_body.find("PF_END_TOPIC(ID_TRACKING_PRESETS_TOPIC+100)", tracking_presets_start)
if tracking_presets_start < 0 or tracking_presets_end < 0:
    fail("Tracking recipe selector topic is missing.")
tracking_presets_ui = setup_body[tracking_presets_start:tracking_presets_end]
if "PF_PUI_NO_ECW_UI" in tracking_presets_ui:
    fail("All four VHS Damage Lab tracking recipe selectors must remain visible/editable.")
for selector in ("Base Tracking Preset", "Layer 2 Preset", "Layer 3 Preset", "Layer 4 Preset"):
    if 'PF_ADD_POPUP("' + selector + '"' not in tracking_presets_ui:
        fail("Missing visible tracking recipe selector: " + selector)


calls = re.findall(r"\b(PF_ADD_[A-Z0-9_]+|PF_END_TOPIC)\s*\(([^;\n]*)\)\s*;", setup_body)
if any(macro == "PF_ADD_LAYER" for macro, _ in calls):
    fail("Do not add a second Source layer: AE already supplies PARAM_INPUT at index 0.")

end_map = {
    "ID_SIGNAL_TOPIC": "PARAM_SIGNAL_END", "ID_RINGING_TOPIC": "PARAM_RINGING_END",
    "ID_GLOW_TOPIC": "PARAM_GLOW_END", "ID_GENERAL_TOPIC": "PARAM_GENERAL_END",
    "ID_CHROMA_TOPIC": "PARAM_CHROMA_END", "ID_STATIC_TOPIC": "PARAM_STATIC_END",
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
    "ID_OSD_WORDS_TOPIC": "PARAM_OSD_WORDS_END", "ID_GRAIN_TOPIC": "PARAM_GRAIN_END",
    "ID_TAPE_SURFACE_DETAIL_TOPIC": "PARAM_TAPE_SURFACE_DETAIL_END",
    "ID_OSD_TEXT_EDIT_TOPIC": "PARAM_OSD_TEXT_EDIT_END",
    "ID_PLAYBACK_TOPIC": "PARAM_PLAYBACK_END", "ID_GROUND_LOOP_TOPIC": "PARAM_GROUND_LOOP_END", "ID_TRACK_LAYER_MOTION_TOPIC": "PARAM_TRACK_LAYER_MOTION_END", "ID_SYNC_DAMAGE_TOPIC": "PARAM_SYNC_DAMAGE_END",
    "ID_EXTENDED_VHS_PRESET_TOPIC": "PARAM_EXTENDED_VHS_PRESET_END",
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

if "if(s.rfDrift>0||s.rfHum>0||s.rfAm>0||s.agcPump>0)" not in SOURCE:
    fail("AGC Pump must activate the standalone RF instability stage even when used alone.")
if "p.demodulator=s.compositeDecoder==3?CompositeDemodulator::TwoLineComb" not in SOURCE:
    fail("Raster Decoder popup must reach the composite demodulator.")
if "p.demodulator=CompositeDemodulator::ConsumerNotch;" in SOURCE:
    fail("A hard-coded Consumer Notch assignment must not override the Raster Decoder popup.")
for required in [
    'PF_ADD_POPUP("Raster Decoder",3,1,kCompositeDecoderChoices,ID_DECODER_MODE)',
    'PF_ADD_FLOAT_SLIDERX("Frame Repeat Frequency"',
    'PF_ADD_FLOAT_SLIDERX("Frame Repeat Strength"',
    'READ_FLOAT(PARAM_FRAME_REPEAT_FREQUENCY,frameRepeatFrequency)',
    'READ_FLOAT(PARAM_FRAME_REPEAT_STRENGTH,frameRepeatStrength)',
]:
    if required not in SOURCE:
        fail("Missing visible Playback/Decoder control or render mapping: " + required)

print(f"AE host contract OK: {len(actual_params)} registered parameters + implicit input, exact enum order, PARAM_COUNT, CUSTOM_UI flags, and VHS6RIFT-compatible legacy render flags.")
