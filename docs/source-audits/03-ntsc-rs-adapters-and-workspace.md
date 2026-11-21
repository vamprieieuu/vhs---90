# VHS-90 — ntsc-rs Adapter and Workspace Review

Date: 2026-11-19
Input: `vhs/open source native plugin for ae/ntsc-rs-0.9.0/` from supplied `vhs.zip`
This supplements `02-ntsc-rs-engine.md` with the host adapters, workspace, build workflow and support crates.

## After Effects adapter

The Rust AE adapter is a useful reference for host integration patterns, not code that can be directly linked into the VHS-90 C++ target.

- Uses the `after-effects` Rust binding crate and the shared `ntscrs` engine.
- Registers settings dynamically from the engine's settings descriptors, assigns stable AE parameter IDs, and maps nested optional setting blocks to grouped controls.
- Implements both legacy render and SmartFX pre-render/smart-render paths.
- Smart pre-render explicitly requests the full source rectangle because the processing model samples across the frame.
- Converts AE ARGB32, ARGB64 and ARGB128 formats to the engine's RGB channel layouts, including a dedicated AE 16-bit normalizer whose nominal maximum is 32768.
- Includes Premiere pixel-format registration for BGRA 8/16/32f.
- Includes custom preset load/save controls and parameter UI enable/disable handling.
- Uses a per-effect settings object and converts to/from a full settings representation so disabled optional modules can retain their previous values.

VHS-90 should reproduce the proven host-side concepts in C++ using the actual AE SDK: correct SmartFX checkout rectangles, stable parameter IDs, proper world pixel-format detection, deep-color normalization, full-frame sampling requests, and deterministic per-render settings snapshots.

## OpenFX adapter

- Implements the OFX plugin ABI with `OfxGetNumberOfPlugins`, `OfxGetPlugin`, load/describe/context/render actions, clip preferences, region-of-interest handling, and time-dependent parameter reads.
- Maps the same shared `NtscEffect` settings to OFX parameters and applies the same engine to RGB/RGBA 8-bit, 16-bit and float data.
- Has explicit sRGB gamma conversion helpers and rowbytes/bounds-aware source and destination buffer handling.
- Depends on an OpenFX Git submodule at `crates/openfx-plugin/vendor/openfx`; the archive includes the submodule declaration but not its contents. The supplied workflow checks out submodules recursively.

## GUI and CLI

- The GUI is a separate standalone application built with egui/eframe and GStreamer; it handles playback, render jobs, export settings, timeline/split-screen UI, file input/output and CLI launch modes.
- This application is not part of VHS-90's native AE effect runtime. Its relevant reusable ideas are the split-screen comparison, render progress/error reporting, settings persistence and preset workflow; its GStreamer GUI code should not be imported into the plugin.
- The GUI crate has a GPL-3.0 package license, while the core engine crate is separately declared MIT OR ISC OR Apache-2.0. VHS-90 only needs the core algorithm concepts and should not import GUI code.

## Build/workspace structure

- Rust workspace contains `ntscrs`, `ae-plugin`, `openfx-plugin`, `gui`, `macros` and `xtask`.
- The GitHub Actions workflow builds Linux, Windows and macOS; Windows installs GStreamer, builds the GUI, OpenFX and AE plugin, and copies the AE DLL to an `.aex` filename. This is a Rust cdylib-based plugin, not a native C++ AE SDK project.
- The OpenFX build uses a recursive submodule checkout.
- The workspace depends on external crates and Git revisions; it is not a self-contained C++ source package.
- The proc-macro crate generates persistent/full settings structures for nested optional controls. This is a settings-model convenience, not a signal-processing requirement.

## Integration decision

For VHS-90, port only the image-processing algorithms and the required data-layout/host-integration patterns into C++17. Do not port the Rust GUI, GStreamer stack, CLI, OpenFX wrapper, proc-macro infrastructure, or Rust ABI. Preserve the core engine's independent Y/I/Q planes, filter behavior, deterministic event seeds, frame/field handling, and host-safe buffer concepts. Replace the source's bottom-anchored Tracking Noise and Head Switching geometry with VHS-90's user-required arbitrary region engine.

## Verification status

- Workspace manifests, AE/OpenFX adapter entry points, build workflow, GUI crate manifest/README, render-job and preset entry points, GStreamer pipeline entry, and settings proc-macro implementation were inspected.
- Cargo is not installed in this environment, so no Rust compile or unit-test run was possible.
- No claim is made that the adapters were built or loaded in After Effects here.
