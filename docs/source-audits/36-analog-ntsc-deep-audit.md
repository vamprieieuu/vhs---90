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

The original RF code assumes 525-line NTSC sampled at 910 samples/line, with sync/blank/burst intervals and IRE units. These algorithms cannot be copied line-for-line into an arbitrary-resolution RGB pixel buffer without a waveform domain. VHS-90 now has both raster-space adaptations for fast modular looks and a selectable full waveform path. The full path preserves the upstream 525-line/910-sample signal domain internally while resampling arbitrary AE frame dimensions at the adapter boundary.

## C++ ports now added to VHS-90

New host-independent C++17 kernels:
- `VHS90RfInstabilityEngine.{h,cpp}`: deterministic RF drift, hum, AM-style luma compression and content-driven AGC pumping adapted to raster-space; alpha-aware and frame-seeded.
- `VHS90ImpulseNoiseEngine.{h,cpp}`: sparse horizontal impulse bursts with signed decaying amplitude and correlated walk, deterministic by seed/frame.
- `VHS90MultipathEchoEngine.{h,cpp}`: three delayed, signed echo taps and ensemble gain; separate from previous-frame temporal ghosting.
- `VHS90ReceiverInstabilityEngine.{h,cpp}`: AFC Hunt with sync-edge decay/cycle slips, line-correlated chroma flutter and high-frequency Y/C crosstalk.
- `VHS90WaveformEngine.{h,cpp}`: complete 525-line/910-sample NTSC encode → RF-effects → receiver-decode path, resampled to/from arbitrary AE frame sizes, with deterministic frame seed and premultiplied-alpha preservation. The BSD-2-Clause upstream encoder/effects/decoder/parameter/signal implementation is retained under `ThirdParty/AnalogNTSC/` with its license.

They are registered in CMake, have standalone smoke tests, and are connected as stages of the shared `VHS90CompositeEngine`. A new AE `Signal Model` popup selects the existing fast raster NTSC approximation or the full NTSC waveform model. In full-waveform mode, RF Drift, Mains Hum, AM Nonlinearity, AGC Pump, Impulse Noise, RF Ghost Gain/Delay/Ensemble, AFC Hunt, Chroma Flutter, Y/C Crosstalk, H/V Sync Noise, Burst Noise and Composite Noise are mapped into the signal-domain effect controls.

Previously integrated from this source family or equivalent inspected sources:
- line jitter / geometry and tape transport via `VHS90AnalogGeometryEngine` and `VHS90TapeTransportEngine`;
- deterministic luma/chroma/composite noise via `VHS90AnalogNoiseEngine`;
- shared sync collapse, tracking/head-switch regions and dropouts via the existing VHS-90 sync/region engines;
- YIQ bandwidth and composite encode/decode via the existing IIR/composite engines.

## Remaining Analog-NTSC validation / port considerations

- The waveform path is currently an SDR RGB8 intermediate at 720×480 internally, matching the upstream encoder/decoder contract. AE frame dimensions are resampled at input/output; float/HDR precision is not preserved through this path yet.
- Full waveform mode is NTSC-only. PAL continues to use the raster-space path until a PAL waveform encoder/decoder is implemented and validated.
- Full waveform processing currently allocates per-frame waveform/decoded buffers. Profile CPU cost and memory pressure on HD/4K clips before recommending it for heavy timelines.
- Compare sync-gated DC restoration, group-delay and multipath interactions against known reference renders; smoke tests currently establish determinism, identity bypass, output difference and alpha preservation, not full perceptual equivalence to every upstream sample.

## Validation status

- GitHub Actions run `44599161053` passed 16/16 CTest tests after adding independent horizontal sync, vertical sync and burst-noise controls.
- GitHub Actions run `44598929488` passed 16/16 CTest tests with the content-driven AGC pump refinement.
- GitHub Actions run `44598799885` passed 16/16 CTest tests after integrating AFC Hunt, Chroma Flutter and Y/C Crosstalk into the composite pipeline.
- GitHub Actions run `44598604938` passed 15/15 CTest tests after adding RF Drift, Mains Hum, AM Nonlinearity, Impulse Noise and Multipath Echo, with their corresponding AE controls wired.
- GitHub Actions run `44602045505` passed 20/20 CTest tests, including the full waveform smoke test.
- GitHub Actions run `44601767654` compiled the vendored Analog-NTSC waveform pipeline successfully.
- The AE adapter exposes the new `Signal Model` selector; a follow-up static parameter audit confirmed 66 real parameter slots and 66 `PF_ADD_*` calls.
- No Windows/MSVC AE build or AE runtime test is claimed.
