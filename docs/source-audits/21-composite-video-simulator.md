# VHS-90 — Composite Video Simulator C++ Audit

Date: 2026-11-19
Input: `vhs/github repos/composite-video-simulator-master/` from supplied `vhs.zip`.
Inventory: 36 files, including 20 C++ sources and the Autotools build files. The README describes the project as an intentionally slow, accuracy-oriented composite-video/VHS emulator rather than a simple blur/noise overlay.

## Relevant C++ programs

- `ffmpeg_ntsc.cpp`: the most useful YIQ-domain composite/VHS implementation.
- `ffmpeg_to_composite.cpp`: an alternate YUV-domain composite carrier simulation with a simpler VHS stage.
- `ffmpeg_cassette.cpp`: audio cassette / VHS linear-audio simulation, not a video-pixel effect.
- `ffmpeg_vhsled.cpp`: a separate video/LED display-oriented utility, not the core VHS signal processor.
- Other programs implement posterization, color keying/maps, scanimation, film grain and raw NTSC step-by-step experiments.

## YIQ composite engine in ffmpeg_ntsc.cpp

### Color representation
- RGB is converted to integer Y/I/Q using explicit NTSC matrix coefficients.
- I and Q have independent low-pass stages: approximately 1.3 MHz and 0.6 MHz.
- A lighter CRT-style path uses a higher 2.6 MHz cutoff.
- Composite chroma is modulated into luma with a four-phase quadrature carrier.
- Scanline phase supports 90°, 180° and 270° modes plus a phase offset.
- The decoder applies a four-sample horizontal box separation, reconstructs chroma and returns RGB.

This is a useful native C++ reference for a simple composite carrier encode/decode stage, but it is a four-phase pixel approximation, not a sample-accurate waveform encoder.

### Signal degradation
- Composite pre-emphasis / high-pass boost.
- Luma noise as a smoothed random walk along each scanline.
- Independent I/Q chroma noise.
- Chroma phase noise by rotating the I/Q vector.
- Random whole-scanline chroma loss.
- Additional Y/C recombination passes.
- Optional chroma low-pass on composite input/output.

### VHS playback response
Tape speed selects different signal bandwidth and chroma delay:
- SP: luma cutoff 2.4 MHz, chroma cutoff 320 kHz, delay 9 samples.
- LP: luma cutoff 1.9 MHz, chroma cutoff 300 kHz, delay 12 samples.
- EP: luma cutoff 1.4 MHz, chroma cutoff 280 kHz, delay 14 samples.

The VHS stage also includes:
- Three cascaded low-pass filters for luma.
- Pre-emphasis/high-frequency recovery.
- Three cascaded low-pass filters for I and Q.
- Horizontal chroma delay.
- Optional vertical chroma blending with the previous field line.
- Playback sharpening.
- Optional S-Video output or composite recombination.

These are useful distinct signal stages and should not be collapsed into a single VHS blur control.

## Head Switching: the strongest placement reference found so far

The newer `ffmpeg_ntsc.cpp` exposes three independent head-switch parameters:
- Enable/disable.
- `vhs_head_switching_point`: normalized vertical start point (default near the bottom, about 4.5 NTSC scanlines above vertical sync).
- `vhs_head_switching_phase`: horizontal start/phase within the raster.
- `vhs_head_switching_phase_noise`: small random timing variation.

The algorithm maps the normalized point into a raster line and the phase into a horizontal offset. It then shifts luma horizontally from that line downward, with the initial shift decaying by 7/8 on each subsequent line. This is more flexible than a fixed bottom strip: moving the point upward can make the disturbance cover much more of the image.

However:
- It does not expose a separate region-height/end control.
- It affects luma only in this implementation; chroma is not shifted in the same operation.
- The source comment explicitly says the implementation is not fully accurate to the real head-switch line, which would show the shifted line and following line contents after horizontal sync.
- The random phase variation uses global `rand()`.

VHS-90 can reuse the normalized vertical start and horizontal phase concepts, but should add independent region height/end, width, intensity, direction and custom positioning so the user can expand the region to the whole frame and move it anywhere.

## Tracking Noise coverage

The README/comment describes tracking noise as a static bar at the bottom of the screen, but the reviewed `ffmpeg_ntsc.cpp` does not implement a separate arbitrary-position Tracking Noise region system. The available code should not be mistaken for the VHS-90 Tracking Noise requirement. VHS-90 needs its own general region engine.

## Alternate ffmpeg_to_composite.cpp

This version works in YUV rather than YIQ and includes:
- Composite chroma encoded into luma using a four-phase carrier.
- Box-filter separation and chroma reconstruction.
- VHS head-switching noise with a phase control.
- VHS SP/LP/EP bandwidth, chroma delay and vertical chroma blending.
- Luma/chroma noise and scanline chroma dropout.

The source itself comments that it should use proper YIQ rather than YUV for composite processing. Prefer the newer `ffmpeg_ntsc.cpp` YIQ path as the signal reference.

## Host/build limitations

- Both programs are standalone FFmpeg command-line transcoders, not AE effects.
- They depend on FFmpeg 3.x-era APIs and global process state.
- `Makefile.am` places `ffmpeg_ntsc`, `ffmpeg_to_composite` and most other filters in `bin_DISABLED`; only `ffmpeg_vhsled`, `frameblend` and `filmac` are enabled by default.
- The source uses global `rand()`, global settings and mutable static/filter state, so it is not suitable for concurrent AE MFR renders without refactoring.
- The pixel pipeline is 8-bit ARGB/YIQ and does not preserve alpha through its RGB writeback.
- The fixed NTSC/PAL output modes are 720×480 and 720×576.
- FFmpeg development packages were not available in this environment (`pkg-config libavcodec` returned no version), so no build was run.

## VHS-90 integration decision

Port the useful signal-domain math into the native C++17 core without importing FFmpeg:
- YIQ transform and I/Q bandwidth filters.
- Four-phase composite carrier modulation/demodulation as a lightweight mode.
- Separate luma/chroma noise and chroma phase rotation.
- SP/LP/EP luma/chroma bandwidth curves and chroma delay.
- Vertical chroma blend and playback sharpening.
- Head-switch normalized vertical start + horizontal phase + decaying line shift, extended with user-controlled region height/end and intensity.

Replace global RNG and global filter state with explicit per-render state, preserve alpha, support arbitrary AE dimensions and keep the user's more advanced region-placement requirements.
