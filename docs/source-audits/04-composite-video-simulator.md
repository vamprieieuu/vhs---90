# Source Audit 04 — Composite Video Simulator

**Folder:** `github repos/composite-video-simulator-master`  
**Status:** full file inventory and processing-path review; standalone build unavailable in this environment.

## Scope reviewed
All 36 files were inventoried, including 20 C++ processing/utility programs, Autotools/Make build files, scripts, docs, license, and the bundled reference image. Key processing implementations reviewed include `ffmpeg_to_composite.cpp`, `ffmpeg_ntsc.cpp`, `ffmpeg_vhsled.cpp`, `ffmpeg_scanimate.cpp`, `ffmpeg_raw28ntsc.cpp` and its seven staged variants, `ffmpeg_average_delay.cpp`, `ffmpeg_cassette.cpp`, `ffmpeg_colorkey.cpp`, `ffmpeg_colormap.cpp`, `filmac.cpp`, and `frameblend.cpp`.

## High-value video algorithms
### Composite encode/decode
- Convert YUV chroma into a quadrature NTSC-like subcarrier carried through the luma plane.
- Decode the synthetic carrier back into Y/U/V with a moving four-sample window and phase-aware U/V recovery.
- NTSC phase can vary by 90/180/270-degree line/frame patterns; PAL uses a separate phase progression.
- Optional pre-encode chroma low-pass and post-decode chroma low-pass, including a lighter variant.
- Optional repeated Y/C encode-separate passes to intensify composite artifacts.
- Configurable subcarrier amplitude and diagnostic modes.

### VHS recording/playback model
- SP / LP / EP modes change luma cutoff, chroma cutoff and chroma delay:
  - SP: 2.4 MHz luma, 320 kHz chroma, 4-sample chroma delay.
  - LP: 1.9 MHz luma, 300 kHz chroma, 5-sample delay.
  - EP: 1.4 MHz luma, 280 kHz chroma, 6-sample delay.
- Three cascaded low-pass stages on luma/chroma, followed by a high-frequency luma emphasis/sharpen path.
- Optional previous-line chroma blending, with NTSC phase compensation.
- Chroma-loss events neutralize U/V to 128 on affected scanlines.
- Head switching is driven by tape/head phase, field timing, a bottom-blanking offset, and optional phase noise. It creates a line-by-line decaying displacement profile rather than simply placing a fixed white bar.

### Other useful modules
- Composite luma/chroma noise with correlated line progression.
- Chroma phase noise with per-line phase variation.
- Video pre-emphasis.
- Multipath/average-delay and frame blending utilities.
- Scanimate phosphor-dot raster simulation, beam/dot positioning and scan-time modulation (CRT-oriented, not VHS core).
- VHS LED / transfer characteristic and filmic gamma/tonal processing (separate aesthetic modules).
- Audio cassette emulation is outside the VHS-90 video effect scope.

## Engineering limitations
- These are standalone FFmpeg command-line programs with large duplicated I/O, packet, audio and argument-parsing code. Those host wrappers are not suitable for an AE effect plug-in.
- The useful signal math can be separated from FFmpeg frame structures and ported into shared VHS-90 C++ processing modules.
- Source uses legacy FFmpeg APIs and GNU extensions/VLAs in places; it must not be copied wholesale into the AE SDK build.
- FFmpeg development packages are not installed in this environment, so the original applications could not be built here. No build pass is claimed.
- Several programs duplicate the same composite core; VHS-90 should consolidate the shared algorithm once, not import duplicate copies or create per-source UI sections.

## VHS-90 integration targets
- Composite carrier modulation/demodulation and phase alternation.
- Y/C bandwidth filtering and repeated composite passes.
- SP/LP/EP tape-speed response as parameterized signal profiles.
- VHS chroma vertical blending and chroma delay.
- Phase-driven head-switching geometry with user-positionable and expandable region controls.
- Chroma loss, luma/chroma noise, and multipath/temporal echo where they do not duplicate better shared engines.
- CRT phosphor-dot simulation only if it contributes to an approved VHS look; keep it separate from the analog VHS tape model internally.

All effects remain inside the one VHS-90 processing pipeline; no source-program names are to appear in the UI.
