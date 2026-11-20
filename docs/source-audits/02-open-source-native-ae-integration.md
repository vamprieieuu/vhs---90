# Native open-source AE sources — joint audit and VHS-90 integration

## Scope and source inventory

Both requested archive folders were examined together.

### VHS6RIFT
- 1,051 extracted non-bytecode files (the archive also carried two Python bytecode cache files).
- 894 files are Adobe After Effects SDK examples; 90 more belong to the bundled SDK headers/utilities/resources. These are SDK reference/vendor content, not VHS6RIFT effect source, and were not copied into VHS-90.
- The project itself contributes 24 root/native/tool/asset files and 43 documentation files.
- The active native effect is concentrated in `include/VHS6RIFT.h`, `src/VHS6RIFT.cpp`, `resources/VHS6RIFTPiPL.r`, CMake/build scripts, and validation tools.
- Source validators report 83 parameter slots, 9 balanced topic groups, 64/64 active controls wired, and 14/14 functional gates passed.

### ntsc-rs 0.9.0
- 93 files total: 14 NTSC processing-core files, 40 GUI/GStreamer files, 13 plugin adapters, plus workspace/build/docs/assets.
- The portable Rust core contains the YIQ/composite pipeline, IIR transfer-function filters, field selection, chroma demodulation, luma smear, FBM noise, chroma phase noise, head switching, tracking noise, snow, chroma delay/loss, vertical chroma blending, and VHS edge-wave processing.
- The Rust GUI, GStreamer application, and separate OpenFX/AE host wrappers are not needed in the existing native VHS-90 host architecture.

## Important source-level discrepancies

The VHS6RIFT documentation contains claims that do not match the active native C++ renderer:
- Its CMake file explicitly states that the current native source does not call the optional `ntscrs_c` bridge; the bundled header is not a functioning linked Rust implementation.
- Several documents describe overlays, CRT, additional camera engines, or later parameter totals that are not present in the active 83-slot native parameter table.
- The README's 16 style presets and ten camera entries (Off plus nine real camera profiles) are present in the active source and verified by its own tests.

Integration therefore follows the active native source and executable tests, not stale feature-matrix claims or a prebuilt `.aex.rzdb` artifact.

## VHS6RIFT functionality transferred

### 16 style presets
The preset recipes were adapted to VHS-90's existing engine controls rather than copying VHS6RIFT's monolithic per-pixel renderer. The AE panel now exposes:
1. Custom
2. Clean VHS
3. Vintage VHS
4. Heavy VHS Degradation
5. Bad Tracking
6. Very Bad Tracking
7. Unstable VCR Playback
8. Moving Band Noise
9. Tape Damage
10. Bottom Tape Damage
11. Static Interference
12. Color Bleed
13. Color Shift
14. Ghosting
15. Signal Loss
16. Severe Analog Glitch
17. Damaged Tape Playback

Each preset blends toward its source-derived target values using Style Intensity.

### Nine hardware profiles
The camera selector includes Off plus:
- JVC GR-C7U
- JVC GR-AX220U
- JVC GR-AX920U
- JVC GR-SXM37US
- JVC GR-SXM740U
- Sony CCD-TR55
- JVC GR-S55
- Panasonic NV-M1
- RCA CMR300

Camera Intensity blends the profile into the current VHS-90 controls. These are source-derived engineering profiles, not claims of pixel-calibrated camera captures.

### AE build foundation
VHS6RIFT's more flexible SDK discovery/build logic was adapted to VHS-90 without replacing its host ABI:
- Added `tools/find_ae_sdk.ps1`.
- Updated `build.ps1` to discover and validate the SDK before invoking CMake.
- Updated CMake to accept `AE_SDK_ROOT` or `AE_SDK_PATH`, discover flat `Headers/`, `Examples/Headers/`, and nested extraction layouts, validate required AE headers and `Util/Param_Utils.h`, and locate `PiPLtool.exe`.
- VHS-90 keeps its existing SmartFX pre-render/render path, float/deep-color handling, threaded/MFR-safe render-local buffers, PiPL identity and current parameter IDs. VHS6RIFT's older direct `PF_Cmd_RENDER` monolith was not substituted for the existing SmartFX host adapter.

The separate VHS6RIFT audio DSP callback was not wired into this video effect: it requires an AE audio-effect callback/host contract and a distinct audio test path, whereas VHS-90's current render product is a video effect. The audio code remains an identified optional follow-up, not silently claimed as integrated.

## NTSC-RS functionality transferred

### Native algorithm ports
- **Scanline chroma phase noise:** adapted the NTSC-RS per-line I/Q phase rotation into `VHS90AnalogNoiseEngine`. It uses the VHS-90 deterministic hash/frame seed, rotates I/Q per scanline, preserves luma and alpha, and is independently controllable.
- **Fractal-Brownian edge wave:** replaced the old short AR-noise edge-wave profile in `VHS90AnalogGeometryEngine` with deterministic 2D gradient FBM. It exposes frequency, speed and detail controls and applies smooth subpixel row displacement to the image.
- The Rust algorithm's filter/field/chroma/tracking functions were audited against the existing VHS-90 C++ pipeline. The current project already has IIR filtering, YIQ composite encode/decode, box/one-line/two-line demodulation, interlace/field weave, luma smear, snow/static, chroma loss, vertical chroma blend, adjustable tracking/head-switch regions, and chroma delay. These were retained as shared VHS-90 modules rather than duplicated.

### Renamed VHS-90 looks
The AE panel now offers ten source-derived looks, all branded as VHS-90:
- Natural NTSC
- Home SP
- LP Tape
- EP Tape
- Tracking Snow
- Chroma Loss
- Edge Wave
- Field Blend
- Composite Smear
- Chroma Instability

Look Intensity blends the preset values into the current controls. The panel does not expose a second NTSC-RS plugin or duplicate the Rust GUI.

## Validation

- VHS6RIFT source checks: `tools/validate_project.py` PASS; `tools/audit_parameter_wiring.py` PASS (64/64); `tools/test_vhs6rift.py` PASS (14/14).
- VHS-90 Linux core CI for the new NTSC-RS FBM and phase-noise code: run #44801124916 PASS (core build and smoke tests).
- VHS-90 Linux core CI including the imported-preset wiring audit: run #44801789097 PASS.
- The new static validator checks 137 AE parameter registrations against disk-ID order, all 16 style cases, all nine camera profiles, all ten VHS-90 signal-look cases, and SmartFX parameter/render wiring.
- The Windows/MSVC AE SDK build and visual verification inside After Effects have not been run here. The CI job validates the host-independent core and static AE wiring, not a finished Windows `.aex`.

## Main integration commits

- Style/camera profiles and first VHS-90 signal looks: `f52d14909c08b9018b314e1c6c6af75ac2456f93`
- AE SDK discovery/build foundation: `dd57675107ac670a7d08779ade40f6f3ff7ef657`
- NTSC-RS scanline phase noise + FBM edge-wave: `d44448ee1b56554ed8d71b95cdcce4bd4f524171`
- Ten complete VHS-90 looks: `0fea2f797803263ecb66821df9d9e9401f3b8fa3`
