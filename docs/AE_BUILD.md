# Native After Effects build

## Requirements

- Windows 11 x64.
- Visual Studio 2022 with the Desktop development with C++ workload (MSVC v143 and Windows SDK).
- CMake 3.20 or newer.
- Adobe After Effects SDK compatible with the intended AE 2023 / 23.2.1 host. The SDK must contain `Headers/AE_Effect.h` and the Adobe `PiPLtool.exe` executable somewhere inside the SDK tree (the CMake configuration searches recursively).

## Configure the SDK path

Set `AE_SDK_ROOT` to the extracted SDK root (not its Headers subfolder):

```powershell
$env:AE_SDK_ROOT = "C:\SDKs\AfterEffectsSDK"
.\build.ps1
```

Or pass the path directly:

```powershell
.\build.ps1 -AeSdkRoot "C:\SDKs\AfterEffectsSDK"
```

The build script configures Visual Studio 2022 x64, builds the native module and CPU tests, runs CTest, and copies the resulting plugin to:

`build/dist/VHS-90.aex`

## CPU-only core tests

No Adobe SDK is needed to build the host-independent processing core:

```powershell
cmake -S . -B build-core -DVHS90_BUILD_AE_PLUGIN=OFF
cmake --build build-core --config Release
ctest --test-dir build-core -C Release --output-on-failure
```

## Host adapter design

The initial AE adapter uses the SmartFX pre-render/render path and retains a legacy render fallback. It requests a full input frame because composite demodulation and scanline damage need neighboring samples, then crops the processed frame to the output world's origin and dimensions. It converts AE ARGB32, ARGB64 and ARGB128 worlds to the core's normalized float RGBA buffers and converts back while preserving alpha.

Global flags in C++ and PiPL are paired deliberately:

- Global outflags: `0x02000004` (deep color aware + non-param-vary).
- Global outflags2: `0x08001400` (SmartFX + float color aware + threaded rendering).

The per-render settings snapshot is immutable and all processing buffers are local to the render call; no mutable global state is used.

## Initial host controls

The first UI shell exposes Master Strength plus Signal (Composite Signal, Sync Collapse), Static (Amount, Density), Tracking Noise (Amount, region shape, Center X/Y, Width/Height, Motion X/Y, Servo Drift, Tear Probability), and Head Switching (Amount, Bottom Edge Y, Height, Horizontal Shift). Tracking regions can be horizontal bands, vertical bands or rectangles. Head Switching uses a bottom-edge anchor so Height=100% and Bottom Edge Y=100% spans the full frame.

## Validation status

The host adapter, PiPL resource and Windows build script are present in source. They have **not** been compiled with MSVC against a local AE SDK or loaded in After Effects in this environment. The six portable C++ tests validate the CPU core only. The first Windows build and in-host test are still required before calling the AEX production-ready.
