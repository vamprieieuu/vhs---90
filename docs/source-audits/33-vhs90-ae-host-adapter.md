# VHS-90 — Initial After Effects Host Adapter

Date: 2026-11-19
Files added:
- `Source/VHS90Effect.cpp`
- `resources/VHS90PiPL.r`
- `tools/compile_pipl.cmake`
- `build.ps1`
- Optional AEX target in `CMakeLists.txt`
- `docs/AE_BUILD.md`

## Effect registration and flags

- Match name: `com.vhs90.effect`
- Display name: `VHS-90`
- Category: `Analog & Tape Effects`
- Entry point: `EffectMain`
- Plugin version: 1.0.0.1 development.
- Global outflags: `0x02000004` = `PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_NON_PARAM_VARY`.
- Global outflags2: `0x08001400` = `PF_OutFlag2_SUPPORTS_SMART_RENDER | PF_OutFlag2_FLOAT_COLOR_AWARE | PF_OutFlag2_SUPPORTS_THREADED_RENDERING`.
- No `PF_OutFlag_PIX_INDEPENDENT` is advertised because composite, scanline and region kernels sample neighboring pixels.

The PiPL values were checked against the bundled AE SDK enum values. C++ GlobalSetup uses the corresponding SDK symbols rather than duplicating numeric flags.

## Host paths

- SmartFX pre-render checks out the parameter values and requests the full source frame for neighborhood-dependent processing.
- SmartFX render checks out the full input pixels, obtains the actual AE pixel format through `PF_WorldSuite2`, processes a normalized float RGBA frame, then crops the processed frame to the output world's origin and dimensions.
- A legacy `PF_Cmd_RENDER` fallback is included.
- ARGB32, ARGB64 and ARGB128 worlds are converted to/from the core RGBA layout; 16-bit normalization uses `PF_MAX_CHAN16` from the SDK.
- The wrapper preserves source alpha and uses the core's premultiplied-alpha processing paths.
- Per-render settings are held in an immutable pre-render allocation with a delete callback. Pixel buffers are local to the render invocation; no mutable global state is used.

## Initial parameter UI

- Master Strength.
- Signal: Composite Signal, Sync Collapse.
- Static: Static Amount, Static Density.
- Tracking Noise: Amount, Region Shape (Horizontal Band / Vertical Band / Rectangle), Center X/Y, Width, Height, Motion X/Y, Servo Drift and Tear Probability.
- Head Switching: Amount, Bottom Edge Y, Height and Horizontal Shift.

Tracking Noise can be positioned and sized as a horizontal band, vertical band or rectangle. Head Switching uses a bottom-edge anchor: increasing Height moves the upper edge upward; Bottom Edge Y=100 and Height=100 cover the full image.

## Build integration

- `VHS90_BUILD_AE_PLUGIN` is optional and OFF by default so the portable C++ core remains buildable without the Adobe SDK.
- When enabled, CMake requires Windows x64/MSVC, enables C and RC languages, locates the local AE SDK and searches recursively for `PiPLtool.exe`.
- The PiPL helper follows Adobe's preprocessing → PiPLtool → resource-preprocessing sequence.
- `build.ps1` configures Visual Studio 2022 x64, builds the AEX target, runs the six portable tests and expects `build/dist/VHS-90.aex`.

## Verification

- The portable C++ core builds and all six tests pass in Release and Debug ASan/UBSan configurations.
- The host adapter and PiPL resource have been inspected against the supplied SDK declarations for the entry-point signature, SmartFX callback structures, pixel formats, suite name/version, pre-render data deletion callback, and flag values.
- **Not verified:** no MSVC/Windows build was run, the PiPLtool was not executed, and the AEX has not been loaded or rendered in AE 23.2.1.
- Full-frame SmartFX checkout/cropping, downsampled previews, non-zero origins, tile behavior, host pixel formats and MFR still require real-host verification.

This is an initial integration milestone, not a claim that the plugin is test-ready or production-ready.
