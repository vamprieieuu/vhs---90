# Analog-NTSC repository — deep audit and VHS-90 port record

Date: 2026-11-19
Source: `kdkd/Analog-NTSC` (archive folder: `Analog-NTSC-master`)
Recursive Git tree: 42 entries, 31 files, 11 directories; complete tree response (`truncated=false`).

## Repository identity and build boundaries

This is an Adobe Premiere / After Effects NTSC composite plugin plus a platform-independent C++20 signal library, NTRF encoder/player tools and a macOS application. The signal library is the valuable reusable part. The original host adapter is coupled to a fixed 720×480 RGB24/8-bit path and the original Makefile uses SDK-specific build glue; neither is copied into VHS-90. VHS-90 remains its own C++17 CPU-first AE 23.2.1 Windows x64 plugin.

License: BSD-2-Clause. Preserve attribution and license notice if any original code is copied. The new VHS-90 kernels are adaptations, not wholesale copies.

## Complete file inventory (31/31)

| File | Role / decision |
|---|---|
| `.gitignore` | Build/SDK ignores; no runtime code |
| `3rdparty/macos/README.txt` | SDK placement note; not imported |
| `3rdparty/windows/README.txt` | SDK placement note; not imported |
| `LICENSE.txt` | BSD-2-Clause; attribution recorded |
| `Makefile` | Original macOS/MinGW build, SDK and PiPL rules; not reused |
| `README.md` | Architecture, parameter, presets, installation and build guide reviewed |
| `docs/NTRF.md` | NTRF binary stream/container spec; encoder/player infrastructure, not an AE effect kernel |
| `docs/NTSC_NOTES.md` | NTSC timing/IRE reference reviewed |
| `docs/api.md` | Public API, signal flow, parameter metadata, NTRF and integration guide reviewed |
| `include/rf2/ntrf_container.h` | NTRF file/container API; not imported |
| `include/rf2/ntsc_decode.h` | Legacy wrapper; compatibility only |
| `include/rf2/ntsc_decoder.h` | Decoder controls/API reviewed; RGB24 fixed-raster interface not reused |
| `include/rf2/ntsc_effects.h` | Effects parameter surface reviewed and used to identify port candidates |
| `include/rf2/ntsc_encoder.h` | Composite encoder API reviewed; raw 720×480 interface not reused |
| `include/rf2/ntsc_params.h` | Parameter IDs/types/categories reviewed |
| `include/rf2/ntsc_processor.h` | Encode → effects → decode processor reviewed; host-neutral concept retained |
| `include/rf2/ntsc_signal.h` | NTSC timing, IRE levels, line map, bandwidth config and encode APIs reviewed |
| `src/app/main.mm` | 1,915-line macOS AVFoundation/Cocoa NTRF player, TCP/NDI/export UI; not portable to AE and not imported |
| `src/common/ntrf_container.cpp` | 451-line NTRF reader/writer, LZFSE compression and frame index; container-only, not imported |
| `src/common/ntsc_decoder.cpp` | 684-line composite decoder reviewed in prior source pass |
| `src/common/ntsc_effects.cpp` | 1,360-line signal-domain effects implementation reviewed in prior source pass and this RF-effects pass |
| `src/common/ntsc_encoder.cpp` | Composite encoder implementation reviewed in prior source pass |
| `src/common/ntsc_params.cpp` | Parameter metadata table reviewed in prior source pass |
| `src/common/ntsc_processor.cpp` | Frame orchestration reviewed in prior source pass |
| `src/common/ntsc_signal.cpp` | NTSC waveform/timing implementation reviewed in prior source pass |
| `src/encoder/main.cpp` | 615-line command-line FFmpeg/input/audio/NTRF streaming tool; not an AE effect kernel |
| `src/plugin/Info.plist` | macOS bundle metadata; not imported |
| `src/plugin/installer.nsi` | Premiere MediaCore NSIS installer; not imported |
| `src/plugin/rf2_ntsc.cpp` | 720-line AE/Premiere adapter, input scaling, 8 preset recipes, parameter ranges and render wiring reviewed |
| `src/plugin/rf2_ntsc.h` | Host parameter index/order and preset enum reviewed |
| `src/plugin/rf2_ntsc.r` | PiPL metadata reviewed; not copied into VHS-90 |

The repository has no separate unit-test directory in this 31-file tree. Its parameter metadata, encoder/decoder/effects APIs and preset adapter were used as behavioral references. Full upstream AE runtime behavior was not executed in this environment.

## Source algorithm findings

The original engine processes raw composite samples (IRE float waveform) in this order: encode RGB to composite → apply signal-domain effects → decode composite to RGB. Useful effects identified include:
- delayed RF multipath with up to three delayed echoes and alternating positive/negative gains;
- DC restoration and sync-referenced AGC following multipath;
- line-time jitter and RF drift (line-paired gain/bias, within-line wander and subpixel sample shift);
- AM nonlinearity with upper/lower knees on composite waveform;
- impulse bursts with signed amplitude decay and correlated random walk;
- mains hum with field-paired line phase, low-frequency line offset/gain, within-line ripple and interference;
- AFC hunt with edge-weighted horizontal drift, occasional cycle slips and micro-jitter;
- AGC pumping driven by bright energy, transient energy and peak level;
- chroma flutter (line-dependent phase/amplitude instability and correlated noise);
- Y/C crosstalk (high-frequency edge energy modulated by carrier);
- sync/burst noise, VHS tracking/wrinkle/head-switch/dropout and group delay.

The original RF code assumes 525-line NTSC sampled at 910 samples/line, with sync/blank/burst intervals and IRE units. These algorithms cannot be copied line-for-line into an arbitrary-resolution RGB pixel buffer without a waveform domain. The VHS-90 adaptations below preserve the visible behavior where feasible and keep the raw-sample-only behaviors as explicit remaining work.

## C++ ports now added to VHS-90

New host-independent C++17 kernels:
- `VHS90RfInstabilityEngine.{h,cpp}`: deterministic RF drift, hum and AM-style luma compression adapted to raster-space; alpha-aware and frame-seeded.
- `VHS90ImpulseNoiseEngine.{h,cpp}`: sparse horizontal impulse bursts with signed decaying amplitude and correlated walk, deterministic by seed/frame.
- `VHS90MultipathEchoEngine.{h,cpp}`: three delayed, signed echo taps and ensemble gain; separate from previous-frame temporal ghosting.

They are registered in CMake, have standalone smoke tests, and are connected as stages of the shared `VHS90CompositeEngine`. The AE adapter now exposes RF Drift, Mains Hum, AM Nonlinearity, Impulse Noise, RF Ghost Gain, RF Ghost Delay and RF Ghost Ensemble controls.

Previously integrated from this source family or equivalent inspected sources:
- line jitter / geometry and tape transport via `VHS90AnalogGeometryEngine` and `VHS90TapeTransportEngine`;
- deterministic luma/chroma/composite noise via `VHS90AnalogNoiseEngine`;
- shared sync collapse, tracking/head-switch regions and dropouts via the existing VHS-90 sync/region engines;
- YIQ bandwidth and composite encode/decode via the existing IIR/composite engines.

## Remaining Analog-NTSC candidates — not marked complete

These require separate adaptation and regression tests before this repository can be marked fully ported:
1. AFC hunt's decaying sync-edge shift and occasional cycle slips.
2. Chroma flutter's line-dependent phase/amplitude jitter and correlated chroma injection.
3. Y/C crosstalk's high-frequency edge/carrier modulation.
4. Sync and burst noise coupling into receiver lock behavior.
5. Sync-gated DC restoration and receiver AGC response around weak/overlapping sync.
6. Group-delay and multipath interactions should be compared against the raw composite reference, not only the current raster approximation.

The 525/910 sample waveform encoder/decoder itself remains a separate, potentially valuable engine-level upgrade, but must be designed for arbitrary AE frame dimensions, float/HDR, premultiplied alpha, MFR determinism and CPU performance rather than copying the fixed 720×480 RGB24 implementation.

## Validation status

- The new RF-instability and impulse-noise standalone tests passed their individual GitHub Actions runs.
- Multipath engine source and standalone smoke test were added; its integration run is in progress/awaiting the latest pipeline CI result.
- Composite pipeline integration and AE parameter wiring are committed; the most recent complete run must be checked before claiming the whole combined set passes.
- No Windows/MSVC AE build or AE runtime test is claimed.
