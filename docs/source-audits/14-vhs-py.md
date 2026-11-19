# Source Audit 14 — vhs.py Analog Video Tape Emulator

**Folder:** github repos/vhs.py-main  
**Status:** all 9 project files inventoried; full VHS.py processing code, video runner and UI control wiring reviewed.

## Actual frame pipeline
1. Convert BGR to YCrCb.
2. Create a luma-side image by resizing the full YCrCb frame horizontally down by lumaCompressionRate with Lanczos4, add Gaussian noise, resize back, and blacken a narrow right border.
3. Create a chroma-side image by resizing the full YCrCb frame horizontally down by chromaCompressionRate with Lanczos4, add independent per-channel integer noise, resize back, and blacken the same border.
4. Apply two row-dependent horizontal wave remaps to the chroma-side image, then take Cr/Cb from that result and Y from the luma-side result.
5. Convert back to BGR, zero the green channel in a configurable right-edge stripe, then apply additional whole-image wave remaps, box blur and unsharp sharpening.
6. Before the Y/C processing, the frame is passed through a soft-light/high-pass sharpening operation and a stronger switch-noise horizontal displacement.

## Additional algorithms
- Gaussian RGB noise with mean/sigma.
- Independent integer noise per B/G/R channel.
- Horizontal resolution loss through Lanczos downsample/upscale, separately controlled for luma and chroma.
- Row-index sine displacement (two wave profiles) and a separate high-amplitude switch-noise displacement.
- Horizontal/vertical box blur and Gaussian/unsharp sharpening.
- An attempted frequency-domain DFT ringing routine exists but is not correctly wired: applyRinging calls the method with an unsupported clip keyword and is not part of the working frame pipeline.

## Parameters and preset behavior
- Luma compression 1..3, luma noise sigma 1..10, luma noise mean -100..100.
- Horizontal and vertical blur 1..15.
- Chroma compression 1..100, chroma noise intensity 1..100.
- Border size 1..3 percent; generations 2..10.
- Generation multiplies luma noise sigma, luma noise mean and chroma noise intensity; it does not actually re-record through repeated physical tape generations.
- UI includes a chroma saturation slider but it is not passed to the backend processor.

## Limitations / porting decisions
- This is a basic image-space approximation, not composite signal encoding or physical tape simulation.
- It processes fixed 640x480 output in its video writer and uses 8-bit OpenCV buffers.
- Randomness uses NumPy and Python global RNG without a frame seed; output is not deterministic or MFR-safe.
- Alpha is not preserved by the BGR/YCrCb pipeline.
- The opened noise-preprocess.avi is not actually composited in the visible processing path.
- The `medianBlur` call uses kernel size 1 and therefore has no effective median filtering.
- Several operations use mutable instance settings and do not restore them safely if an exception occurs.
- Border operations are image-specific and should be optional, not built into the shared VHS pipeline.

## VHS-90 integration targets
Use only the useful algorithms as optional shared modules: independent luma/chroma resolution reduction, luma and chroma noise, row-synchronous wave displacement, switch-noise displacement, soft-light high-pass sharpening and horizontal/vertical blur. Replace global random with deterministic per-frame seeds, implement arbitrary dimensions/rowbytes and AE alpha preservation, and make tracking/switch noise use configurable regions. Do not copy the Python app, browser UI, OpenCV dependency or fixed 640x480 writer.
