# VHS-90 — ntscQT Composite / VHS Algorithm Audit

Date: 2026-11-19
Input: `vhs/github repos/ntscqt-1.5.0/ntscqt-1.5.0/` from supplied `vhs.zip`.
Inventory: 23 files, including 11 Python modules, two Qt UI descriptions, preset JSON, a NumPy ring pattern, localization data, app packaging and a demo GIF.

## Project shape

- Python 3 / NumPy / SciPy / OpenCV / PyQt desktop tool.
- Its `Ntsc` class is a field-oriented composite NTSC and VHS processor.
- It uses planar int32 YIQ and processes alternate raster fields independently.
- The README explicitly warns that it is very slow and prioritizes fidelity over speed.
- No native C++ or AE/OFX host adapter is supplied.

## YIQ and composite pipeline

- Converts BGR to planar integer YIQ using explicit NTSC coefficients.
- I and Q are independently filtered by cascaded first-order low-pass filters.
- Composite encode mixes I/Q into luma using a four-phase carrier determined by line/field phase.
- Composite decode estimates luma from adjacent samples and recovers I/Q from the residual.
- Optional pre-emphasis modifies luma before encoding.
- Chroma low-pass can be applied before and after composite encode/decode.
- Separate 2.6 MHz lightweight chroma filtering approximates a CRT/consumer decode path.
- The source's chroma encode/decode functions are a useful reference for the composite carrier and quadrature recovery.

## VHS signal controls and algorithms

### VHS speed
- SP: luma cutoff 2.4 MHz, chroma cutoff 320 kHz, chroma delay 9 samples.
- LP: luma cutoff 1.9 MHz, chroma cutoff 300 kHz, chroma delay 12 samples.
- EP: luma cutoff 1.4 MHz, chroma cutoff 280 kHz, chroma delay 14 samples.

### VHS processing
- Three cascaded low-pass stages on luma, followed by high-frequency pre-emphasis/sharpening.
- Three cascaded low-pass stages on I and Q, with independent horizontal delay.
- Optional vertical chroma blend with the previous line in the same field.
- Optional composite recombination at VCR output (or S-Video output).
- Optional chroma loss on individual lines.
- Edge wave uses random per-line displacement smoothed by a low-pass filter.
- Color bleed shifts I/Q spatially with independent horizontal and vertical offsets.
- Ringing can use frequency-domain spectral shaping or a second power/shift variant.

### Noise
- Luma noise is a smoothed random walk along scanlines.
- Chroma noise has separate I and Q random walks.
- Chroma phase noise accumulates line-by-line and rotates the I/Q vector.
- A frequency-noise pattern and configurable amplitude feed the ringing stage.
- Random seed can drive repeatable base processing, but see the head-switch note below.

## Head Switching: useful arbitrary vertical start, but incomplete region control

The source exposes:
- Enable.
- Normalized vertical point (`_vhs_head_switching_point`), default about 4.5 scanlines above NTSC vertical sync.
- Horizontal phase (`_vhs_head_switching_phase`).
- Phase noise.
- A speed value that advances the vertical point over time.

The algorithm maps the normalized point into a raster line and horizontal offset, then shifts luma from that line downward with a 7/8 per-line decay. This can move the onset upward so the disturbance covers much more of the image, which is a useful reference for VHS-90's adjustable Head Switching start position.

Limitations:
- No independent region height/end coordinate.
- Only luma is shifted in the head-switch routine; chroma is not shifted there.
- The phase-noise sample uses Python's module-level `random.randint`, not the instance's seeded `NumpyRandom`. This breaks full reproducibility when head switching is enabled.
- The vertical point is mutable state advanced per call, so random-access frame rendering needs explicit frame-index reconstruction.

VHS-90 should keep the normalized vertical start/horizontal phase idea but add region height, width, intensity, direction and arbitrary X/Y placement.

## Other recovered controls

- Composite pre-emphasis amount and cutoff.
- VHS output sharpening.
- VHS edge wave.
- Head-switch speed/point/phase/phase-noise.
- Horizontal/vertical color bleed and processing order (before/after other degradations).
- Ringing mode, power, shift, noise size and amplitude.
- Luma noise.
- Chroma noise, phase noise and chroma loss.
- Subcarrier amplitude in/out.
- NTSC output, scanline phase shift (90/180/270/other) and phase offset.
- Composite input/output chroma low-pass toggles.
- VHS SP/LP/EP.
- Vertical chroma blend.
- S-Video output.
- Seed-driven random parameter generation.

The built-in template JSON includes one full preset record, while the application also supports randomized parameter sets and user templates.

## VHS-90 reuse opportunities

- Port the YIQ integer matrix and four-phase carrier encode/decode.
- Use the SP/LP/EP bandwidth and chroma-delay curves.
- Preserve luma sharpness while smearing I/Q independently.
- Port the vertical chroma delay-line blend.
- Use the line-by-line chroma phase random walk.
- Port the frequency-domain ringing and per-line edge-wave displacement.
- Extend the head-switch vertical point into the user's required arbitrary region system.

## Limitations

- No alpha channel; the API uses BGR frames and YIQ arrays.
- Field processing uses stateful mutable random generators.
- Head-switch phase noise uses global Python random, not the seeded instance.
- Head-switch position advances as mutable state instead of being derived directly from absolute frame index.
- Fixed pixel-space signal processing and Python loops make it unsuitable as a direct AE renderer.
- No OSD, font engine or Arabic text support.
- Dependencies include NumPy, SciPy, OpenCV, PyQt and PyInstaller.

## Local verification

- A synthetic 320×240 BGR frame was processed through the core `composite_layer` path with VHS enabled and non-zero luma noise.
- Output shape was 240×320×3; the core returned a floating-point array.
- No full video export, Qt GUI run or performance benchmark was performed.
- No C++ port is claimed.

## VHS-90 integration decision

Port only the useful signal math into the native C++ core: YIQ transforms, composite carrier modulation/demodulation, cascaded low-pass filters, pre-emphasis, SP/LP/EP bandwidth curves, chroma delay/vertical blend, line noise, phase rotation, ringing and edge wave. Replace mutable/global random state with frame-indexed deterministic hashes, preserve alpha and extend head switching to arbitrary region geometry.
