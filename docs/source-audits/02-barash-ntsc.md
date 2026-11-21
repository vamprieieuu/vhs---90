# Source Audit 02 — BarashNTSC

**Folder:** `github repos/BarashNTSC-0.1.2`  
**Status:** complete review of all 9 project files; C++ port candidates identified.

## Files reviewed
- `BarashNTSC.py` (full processor, threading/export/preview/UI, presets).
- `hook_ffmpeg.py`.
- `requirements.txt`.
- Both JSON presets.
- README and bundled icon asset.

## Confirmed image-processing algorithms
The main processor works in OpenCV/NumPy BGR/YCrCb space and contains:
- Horizontal luma resolution reduction and reconstruction.
- Luma edge enhancement.
- Tape fade with luma lift and chroma desaturation.
- Separate Cr/Cb downsample-upsample for color bleed.
- Chroma noise, opposite-direction Cr/Cb shift, and frame-phase composite carrier modulation.
- Frame-index-driven chroma phase rotation.
- Contrast, brightness and saturation controls.
- Low-resolution luma noise.
- Random short bright/dim tape-wear segments.
- Multi-frequency line jitter and smooth random per-row offsets.
- A moving tracking band with localized horizontal displacement, vertical jitter and noise.
- Interlace field phase offsets alternating by frame.
- Optional previous-frame ghost blending.
- Scanline attenuation, radial vignette and flicker.

## Preset data
Two supplied JSON examples were read completely:
- `example_ANALOG`: detail 39, noise 74, color bleed 20, composite 18, contrast 129, plus the supplied tracking, jitter, ghosting, interlace and wear values.
- `example_worn_tape`: detail 35, noise 25, color bleed 15, chroma shift 3, color phase 10, composite 25, tracking 30, tape wear 35 and the other supplied values.

## Porting decisions for VHS-90
- The processing algorithms are portable conceptually to C++/AE, but the Python/OpenCV/Tkinter app and multiprocessing/UI layer are not an AE plug-in architecture.
- Candidate shared modules: luma/chroma resolution engine, chroma downsample/bleed, phase rotation, moving tracking-band generator, scanline/row jitter, field alternation, temporal ghost buffer, tape-wear streak generator, flicker and vignette.
- The moving tracking band is a useful independent pattern source; VHS-90's shared Tracking Noise controls must extend its location/size/axis beyond a fixed band, per the approved requirement.
- The source processor strips alpha by converting BGRA to BGR. VHS-90 must not repeat this behavior; alpha is preserved independently.
- Random state is mutable and frame-processing order dependent. VHS-90 must replace this with deterministic per-frame/per-module seeds for MFR-safe output.
- The UI ranges are not copied blindly; VHS-90 will use its shared master-strength architecture and independent controls.

## Verification
Static source review completed. No Python/OpenCV runtime test was performed in this environment, so no runtime pass is claimed. No source code has yet been copied into the VHS-90 C++ engine in this commit.
