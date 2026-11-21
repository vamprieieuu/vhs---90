# Source Audit 07 — NTSC-VHS Renderer 1.3.0

**Folder:** github repos/NTSC-VHS-Renderer-1.3.0  
**Status:** all project files inventoried; signal processor dependency and bloom source reviewed.

## Project structure and processing path
- The application is a Python/PyQt desktop renderer, not an AE plug-in.
- It extracts video frames with FFmpeg, invokes two external NTSC-CRT command-line builds (standard NTSC and VHS), monitors generated BMP frames, then applies optional post-processing and writes the result through FFmpeg.
- The core analog signal processing is delegated to the NTSC-CRT library already audited separately; this repository does not provide a new independent VHS signal encoder.
- GUI, multiprocessing queues, timeline preview, config persistence and export code are host application infrastructure and are not transferred to VHS-90.

## Additional effect code: BloomEffect
The bundled Cython source bloom.pyx (plus deprecated.pyx and the pxd declarations) contains a CPU bloom implementation:
- Bright-pass extraction controlled by a threshold.
- Four downsample scales: 1/2, 1/4, 1/8 and 1/16.
- A 5x5 Gaussian blur on each reduced image, repeatable by a smooth-pass count.
- Bilinear upscaling to original dimensions.
- Additive blending of each blurred bright-pass layer onto the source.
- A fast mode keeps only the coarsest scale and uses an optimized bright pass.
- Optional masks and separate 24-bit/32-bit pathways are declared; the 32-bit path has alpha-aware storage but its blend semantics need independent verification before use.
- Additional source contains threshold/bloom mask building, filtering and array rescaling helpers.

## Other image controls in the renderer
After the external NTSC/VHS render, FramesProcessor optionally applies brightness, contrast, saturation, sharpness, vignette and bloom. These are simple post-stage operations, not part of the analog tape model. The config contains separate enabled flags and values.

## VHS-90 implementation decisions
- Reuse the bloom algorithmic design as a candidate for the shared Glow/Halation module: multi-scale bright pass, reduced-resolution blur, upsample and additive compositing. Rebuild it in native C++ with AE-safe memory handling; do not bring Cython, Python, Pygame, Pillow or NumPy into the plug-in.
- Keep the signal pipeline itself sourced from the already-audited NTSC-CRT core rather than duplicating it.
- Implement glow as a shared engine stage with threshold, radius/scale levels, intensity and alpha-aware compositing; compare it against the exact VHS Craft glow reference before claiming visual equivalence.
- The renderer's UI, external executable orchestration, and BMP/FFmpeg pipeline are not reusable as AE architecture.
- Its current output path is RGB-focused; VHS-90 must preserve AE alpha and premultiplication.

No new VHS signal algorithms or complete preset library were found beyond the delegated NTSC-CRT engine. No Cython extension was built in this environment, and no bloom runtime pass is claimed.
