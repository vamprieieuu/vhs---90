#!/usr/bin/env python3
"""Static regression checks for the AE adapter's Legacy/SmartFX/OSD contracts.

This is deliberately not presented as an After Effects host test: Adobe's host
must load and execute the AEX to prove that runtime behavior.
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "Source/VHS90Effect.cpp").read_text(encoding="utf-8")
ARB = (ROOT / "Source/VHS90Windows/VHS90OsdArbitraryData.cpp").read_text(encoding="utf-8")
OSD_TEST = (ROOT / "Tests/VHS90OsdTextDataSmoke.cpp").read_text(encoding="utf-8")
DW_TEST = (ROOT / "Tests/VHS90DirectWriteSmoke.cpp").read_text(encoding="utf-8")

def fail(message: str) -> None:
    raise SystemExit("AE RENDER CONTRACT ERROR: " + message)

def body(name: str) -> str:
    match = re.search(r"\b" + re.escape(name) + r"\s*\([^;]*?\)\s*\{", SOURCE, re.S)
    if not match:
        fail("missing function " + name)
    start = SOURCE.find("{", match.start())
    depth = 0
    for i in range(start, len(SOURCE)):
        if SOURCE[i] == "{":
            depth += 1
        elif SOURCE[i] == "}":
            depth -= 1
            if depth == 0:
                return SOURCE[start + 1:i]
    fail("unbalanced function " + name)

legacy = body("LegacyRender")
pre = body("SmartPreRender")
smart = body("SmartRender")
global_setup = body("GlobalSetup")
render = body("RenderWorld")

if "RenderWorld(s,inWorld,outWorld,inFmt)" not in legacy:
    fail("LegacyRender no longer reaches the shared RenderWorld pipeline")
if "ReadSettings(in_data,*s)" not in pre:
    fail("SmartPreRender no longer checks out the common render settings")
if "req.rect.left=0" not in pre or "req.rect.top=0" not in pre:
    fail("SmartFX source checkout is no longer full-frame")
if "checkout_layer" not in pre or "kInputCheckoutID+1" not in pre:
    fail("SmartFX temporal checkout contract is missing")
if "RenderWorld(*s,inWorld,outWorld,inFmt,previousWorld)" not in smart:
    fail("SmartRender no longer reaches the shared RenderWorld pipeline")
for token in ("checkout_layer_pixels", "checkout_output", "checkin_layer_pixels"):
    if token not in smart:
        fail("SmartRender checkout/checkin contract missing: " + token)
if "PF_OutFlag2_SUPPORTS_THREADED_RENDERING" not in global_setup:
    fail("threaded-rendering flag is missing")

# SmartFX is intentionally not advertised while the known black-frame
# re-render issue remains unverified in the real AE host.
smart_flag = "PF_OutFlag2_SUPPORTS_SMART_RENDER"
smart_advertised = smart_flag in global_setup
if smart_advertised:
    print("SMARTFX_ADVERTISED=YES (requires real AE host render acceptance)")
else:
    print("SMARTFX_ADVERTISED=NO (kept disabled pending AE black-frame regression test)")

osd_at = render.find("if(osdEnabled)")
composite_match = re.search(r"if\s*\(\s*compositeStages\b", render)
composite_at = composite_match.start() if composite_match else -1
if osd_at < 0 or composite_at < 0 or osd_at >= composite_at:
    fail("OSD must be composited before VHS/NTSC signal processing")
if "drawDirectWriteText" not in render or "s.osdText" not in render:
    fail("DirectWrite free-text OSD path is missing")

for token in ("PF_Arbitrary_FLATTEN_FUNC", "PF_Arbitrary_UNFLATTEN_FUNC",
              "PF_Arbitrary_COPY_FUNC", "PF_Arbitrary_COMPARE_FUNC",
              "PF_Arbitrary_INTERP_FUNC"):
    if token not in ARB:
        fail("arbitrary-data persistence callback missing: " + token)
if "unflattenOsdText" not in OSD_TEST or "flattenOsdText" not in OSD_TEST:
    fail("OSD serialization round-trip tests are missing")
for token in ("السَّلَامُ عَلَيْكُمْ", "RooyinFree", "YR Sponge Arabic"):
    if token not in DW_TEST:
        fail("Arabic/font visual smoke coverage missing: " + token)
if "vhs90-osd-arabic-smoke.bmp" not in DW_TEST:
    fail("OSD visual frame is not emitted for CI inspection")

print("LEGACY_SHARED_PIPELINE=PASS")
print("SMARTFX_SOURCE_CONTRACT=PASS (host runtime still required)")
print("THREADED_RENDER_FLAG=PASS")
print("OSD_BEFORE_SIGNAL_PIPELINE=PASS")
print("OSD_ARBITRARY_SAVE_REOPEN_CONTRACT=PASS")
print("ARABIC_DIRECTWRITE_PIXEL_SMOKE=PASS")
