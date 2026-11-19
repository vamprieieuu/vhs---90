# VHS-90 — NTSC-VHS Renderer / Bloom Pipeline Audit

Date: 2026-11-19
Input: `vhs/github repos/NTSC-VHS-Renderer-1.3.0/` from supplied `vhs.zip`.
Inventory: 23 files (about 2.5 MB), including Python GUI/frame orchestration, two Cython bloom implementations, a Cython-generated C source file, config, prebuilt Windows helper executables and build metadata.

## Project architecture

This is a desktop video-rendering wrapper around the separate NTSC-CRT command-line engines, plus a Cython/Pygame bloom implementation. It is not itself a native After Effects plugin or a standalone C++ VHS signal engine.

- Extracts video frames with FFmpeg.
- Runs either the standard NTSC executable or the separately compiled NTSC-VHS executable.
- Streams rendered frames through preview/export management.
- Applies optional BloomEffect processing and image-level brightness/contrast/sharpness/saturation/vignette controls.
- GUI and rendering process communicate through multiprocessing managers, queues and a worker thread.
- The bundled `bloom.c` is generated Cython output; the editable algorithm source is `bloom.pyx`.

## Signal-processing reuse

The signal engine is not reimplemented here. It relies on the external NTSC-CRT project already audited in `16-ntsc-crt-c89.md`.

The wrapper exposes:
- Standard NTSC versus VHS NTSC system selection.
- Monochrome mode.
- Progressive/interlaced scan selection.
- Fill between scan lines.
- Optional bottom-line corruption flag.
- Noise amount.
- Output width/height and match-input-size option.

The `mess_bottom_line` flag is only appended for the non-standard system branch in the wrapper. This is a limited bottom-region option, not a configurable arbitrary-position tracking/head-switch region.

## BloomEffect algorithm

The current `bloom.pyx` contains separate 24-bit RGB and 32-bit RGBA implementations.

### Main multi-scale bloom

1. Bright-pass filter extracts pixels above a configurable threshold.
2. The bright-pass image is downscaled by ×2, ×4, ×8 and ×16.
3. Each reduced image is blurred with a separable 5×5 Gaussian kernel:
   `[1, 4, 6, 4, 1]` horizontally and vertically, normalized by 256.
4. The reduced blurred images are scaled back to original dimensions using smooth/bilinear scaling.
5. The multi-scale bloom layers are additively combined with the source image.

A fast mode uses only the ×16 layer and an optimized/downscaled bright pass. A separate in-place path exists for RGB and RGBA. The source also contains mask-related parameters, but comments mark several mask arguments as unused; the mask-generation and filtering helpers should be inspected individually before assuming full mask support.

### Additional helpers
- 5×5 separable Gaussian blur for RGB and RGBA.
- Bright-pass extraction based on luminance.
- Surface/array conversions for Pygame.
- Multi-resolution bloom layers and rescaling.
- In-place and copy-returning variants.

## Alpha considerations

- The bloom implementation has separate RGBA functions, which is useful as a reference.
- The documentation states the RGBA variant returns alpha values, but the implementation must be verified for whether alpha is blurred, preserved or used in bright-pass extraction before porting.
- VHS-90 should perform bloom in premultiplied-safe form and preserve the original alpha channel rather than assume the RGBA path is correct.

## VHS-90 integration decision

- Do not import the Python GUI, FFmpeg frame orchestration, multiprocessing manager, Pygame surface layer or prebuilt NTSC executables into VHS-90.
- Treat NTSC-CRT as the signal source (already audited separately).
- Port the multi-scale bright-pass bloom concept and its separable Gaussian kernel into the shared C++ image-processing module, with a CPU-first implementation.
- Add explicit threshold, radius/scale count, intensity and color-channel weighting controls; preserve alpha and test transparent edges.
- Do not conflate bloom with the VHS signal's luma smear or ringing; they are distinct stages and should remain independently controllable.

## Verification status

- README, config defaults, frame processor's executable selection/argument construction and bloom Cython public functions/core algorithm were inspected.
- The prebuilt Windows executables were inventoried but not executed.
- No Cython extension build or GUI run was performed.
- No claim is made that the project itself can be linked into an AE plugin.
