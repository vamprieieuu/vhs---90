# NTSC-CRT v2.3.2 — complete repository audit and VHS-90 port record

Source: `LMP88959/NTSC-CRT`, default branch `main`.
Recursive inventory: 32 files, 1 directory (complete tree).
License: permissive custom license; author says use is unrestricted and attribution is appreciated but not mandatory. License and attribution note retained under `ThirdParty/NTSC-CRT/`.

## File-by-file inventory (32/32)

| File | Review / disposition |
|---|---|
| `CMakeLists.txt` | Build variants, flags, external PL3D-KC dependency and source lists reviewed. LIVE and VIDEO wrappers are optional; CMake accepts `CRT_SYSTEM` 0–5 only. `CRT_SYSTEM_NESRGB=6` exists in the headers but is not wired into CMake/source lists. |
| `LICENSE` | Permissive custom terms reviewed and copied to `ThirdParty/NTSC-CRT/LICENSE`. |
| `README.md` | Architecture, constraints, feature list, image/video CLI, integration API, emulator variants and attribution reviewed. |
| `controls.txt` | Interactive app key controls; UI-only. |
| `crt_core.h` | Shared CRT state, pixel formats, system selection, sync windows, public API and fixed-point trig declaration reviewed. Notes no alpha support. |
| `crt_core.c` | Full 673-line demodulator: interpolated 14-bit sine/cosine, selectable three-band equalizer / convolution alternative, sync detection, burst carrier convergence, I/Q demodulation, output scaling, field blending, scanlines, optional beam-energy bloom and pixel-format packing reviewed. |
| `crt_ntsc.h` | Standard NTSC 227.5/228 carrier pattern, 525/60 line geometry, sync/burst timing, bandwidth constants and settings reviewed. |
| `crt_ntsc.c` | Full 332-line standard NTSC modulator reviewed: fixed-point exponential, one-pole Y/I/Q band-limiting, YIQ-to-composite carrier modulation, burst phase, line/field phase, RGB input sampling and signal generation. |
| `crt_ntscvhs.h` | VHS SP/LP/EP bandwidth profiles, 525/60 timings and optional aberration switch reviewed. |
| `crt_ntscvhs.c` | Full 339-line VHS variant reviewed. In addition to the NTSC path, it can omit horizontal sync in a random-height group of bottom lines, reset receiver H-sync at each frame, and inject a line-shaped bottom noise/aberration region. Uses global `rand()`. |
| `crt_nes.h` | NES PPU line/pulse timing, 227.3 chroma pattern, 9-bit pixel input and field rules reviewed. |
| `crt_nes.c` | NES-specific 9-bit waveform generation, emphasis/brightness levels, dot skipping, border and field timing reviewed. Not a general RGB VHS kernel. |
| `crt_nesrgb.h` | RGB-to-NES-artifact mode interface and PPU timing reviewed. |
| `crt_nesrgb.c` | RGB-input NES artifact mode and precomputed field/carrier setup reviewed. Experimental/unwired in upstream CMake (system 6 is rejected by its 0–5 range). |
| `crt_pv1k.h` | Casio PV-1000 timing and five-sample chroma cycle profile reviewed. |
| `crt_pv1k.c` | PV-1000-specific modulation/demodulation and five-sample carrier phase reviewed; console-specific. |
| `crt_snes.h` | SNES PPU timing, 227.3 carrier, phase offsets and sync pulse regions reviewed. |
| `crt_snes.c` | SNES-specific NTSC modulation profile reviewed; uses a separate system timing and color phase model. |
| `crt_template.h` | Template timing/configuration and extension instructions reviewed. |
| `crt_template.c` | Full 338-line template modem reviewed; provides a configurable system profile rather than a separate VHS effect. |
| `crt_main.c` | Both command-line and PL3D-KC live application entry points, controls, image loading, phosphor persistence and UI event handling reviewed; host app only. |
| `bmp_rw.h`, `bmp_rw.c` | BMP 24-bit image I/O reviewed. Header credits deqmega/DEQ2000-cyber; not a render kernel. |
| `ppm_rw.h`, `ppm_rw.c` | P6 PPM image I/O reviewed; not a render kernel. |
| `extra/README.md` | States folder is for miscellaneous extras. |
| `extra/video_convert.c` | Frame-sequence CLI, FFmpeg workflow, optional VHS aberration flag, frame loop and file I/O reviewed; not part of AE runtime. |
| `extra/test_output_images.zip` | Supplied output/reference image archive inventoried as a test asset; no executable source. |
| `artifactcolor.png`, `kc.png`, `rainbow.png`, `scube.png` | README demonstration images reviewed as visual references, not runtime overlays. |

## Reusable algorithm findings

1. **Fixed-point sine/cosine interpolation:** 14-bit phase input, quarter-wave table and linear interpolation. Ported into the new CRT EQ coefficient generator.
2. **Three-band YIQ equalizer:** four cascaded low-pass poles for low and high cutoffs, then low/mid/high band separation and independent fixed-point gains. Ported as a render-local, row-reset C++ engine.
3. **Y/I/Q band-limiting:** one-pole fixed-point low-pass with coefficient derived from a fixed-point exponential approximation. VHS-90 already has configurable YIQ IIR bandwidth and tape-speed SP/LP/EP profiles; no duplicate filter was added.
4. **Composite carrier and burst phase:** phase-alternating carrier, color burst convergence and sync recovery. Overlaps the existing full-waveform NTSC engine and is retained as a reference rather than copied as a second modem.
5. **VHS bottom sync collapse / aberration:** random-height loss of horizontal sync in the bottom lines plus a shaped noise band. VHS-90 already has independently positioned Tracking Noise, Head Switching and Sync Collapse modules with adjustable region masks; the original fixed-bottom-only variant is not duplicated.
6. **Beam-energy bloom:** optional scanline-energy-dependent horizontal beam width and black border. Upstream compile flag `CRT_DO_BLOOM` is disabled by default; the shared VHS-90 bloom engine is a glow kernel, not a beam-width geometry equivalent. Keep as a future optional CRT monitor profile, not required for VHS signal migration.
7. **NES/PV1K/SNES profiles:** specialized console/PPU timings and source formats; not applicable to a general RGB-input VHS effect and excluded from VHS-90 runtime.
8. **Global state / determinism caveats:** upstream keeps EQ histories and receiver state in file-static variables, does not process alpha, uses 8-bit signed signal buffers and `rand()` for noise/aberration. Those patterns cannot be used directly in MFR rendering.

## C++ port in this pass

- `Source/VHS90Core/VHS90CrtEqualizerEngine.h/.cpp`: adapts the NTSC-CRT fixed-point three-band YIQ equalizer. It uses the source's 14-bit interpolated sine table to derive filter coefficients, Q16 gain multipliers, four-pole low/high cascades, low/mid/high band recombination and source-like default Y/I/Q gains.
- The original file-static filter state is replaced with stack-local per-render/per-scanline state. Each row resets the filters to match upstream demodulator behavior while remaining thread/MFR-safe.
- Input/output remain float RGBA; YIQ values are quantized to the upstream 8-bit-equivalent scale for the filter math, and premultiplied alpha is restored unchanged.
- Integrated after composite demodulation in `VHS90CompositeEngine`; exposed as the AE `CRT 3-Band EQ` control.
- `Tests/VHS90CrtEqualizerEngineSmoke.cpp` and composite integration coverage verify identity bypass, deterministic output, visible filtering and alpha preservation.
- No upstream application, image reader/writer, FFmpeg converter, PL3D-KC dependency or console-specific source was copied into the plugin.

## Verification boundary

- GitHub Actions run `44606767936` passed **25/25 CTest tests**, including the fixed-point equalizer and composite integration after the coefficient-overflow correction.
- Static AE parameter audit after this addition: **79 real parameter slots and 79 `PF_ADD_*` calls**. This is not a Windows/MSVC AE SDK build or a live After Effects runtime test.
- The 32-file NTSC-CRT source inventory is complete. The portable three-band EQ is integrated; bottom-only VHS aberration is intentionally represented by VHS-90's more flexible tracking/sync region modules rather than a second hard-coded bottom effect.
