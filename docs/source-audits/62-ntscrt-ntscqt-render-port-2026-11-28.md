# NTSCRT VCR / NTSC-QT CPU port checkpoint — 2026-11-28

## Implemented

### NTSCRT VCR line path

- Added \`VcrSignalParams\` and \`processVcrSignalLine\` in \`Source/VHS90Core/VHS90VcrSignalEngine.{h,cpp}\`.
- The image adapter resamples each active image row to the source line's 1,820-sample working grid, processes it, and maps the result back to the frame width.
- The line chain now includes luma pre-emphasis, FM record modulation, physical head-spacing/gap attenuation, RF noise, analytic FM bandpass/demodulation, luma low-pass/de-emphasis, color-under modulation/recovery, two-stage chroma low-pass and a bounded short dropout hold.
- \`VHS90CompositeEngine\` runs this stage after adjacent-track read and head-condition processing, before the NTSC composite encoder. The AE panel exposes enable, amount, spacing, gap, RF/head noise, dropout threshold and compensation controls.
- The previous NTSCRT adjacent-track engine remains a separate upstream stage; it is not folded into the new FM line chain.

### NTSC-QT spectral path

- Added \`VHS90NtscQtEngine.{h,cpp}\` and integrated it into \`VHS90CompositeEngine\` before composite modulation.
- Spectral mask A uses the horizontal-frequency mask and supports frequency-noise masks over the full two-dimensional field spectrum. Because the base mask is invariant over vertical frequency, the no-noise case is evaluated as equivalent independent horizontal transforms.
- Spectral mask B uses the original 720-sample \`ringPattern.npy\` response. A build-time Python standard-library converter validates the NPY shape/type and generates a C++ constexpr header; no runtime file dependency is introduced.
- The separate Y-only composite pre-emphasis control uses an adjustable low-pass cutoff. Amount blending and premultiplied-alpha preservation are covered by tests.
- AE controls are appended after the prior parameter tail; existing parameter IDs are unchanged.

## Verification

- Windows x64 / MSVC 2022: 37/37 CTest tests pass at plugin-source commit \`a4954f7\`.
- Linux: 36/36 CTest tests pass.
- AE parameter validator: 403 stable parameter slots, unique persistent disk IDs and topic-end mappings.
- Render contract validator: Legacy shared-pipeline path, SmartFX source checkout/checkin contract, threaded-render flag, OSD ordering, OSD arbitrary-data serialization and DirectWrite Arabic smoke all pass.
- The Windows OSD visual smoke frame verifies Arabic decoding after adding MSVC \`/utf-8\`; it is a DirectWrite unit-test frame, not an AE host render.

## Explicit remaining limits

- SmartFX remains unadvertised until the known black-frame issue is tested and fixed in an actual After Effects host. The source callbacks exist, but source-contract validation is not a SmartFX runtime test.
- MFR/threaded rendering has the required flag and render-local processing state, but actual AE MFR/out-of-order execution has not been run.
- The VCR line chain is a CPU record/playback port, not a claim of complete NTSCRT parity. Remaining source-specific work includes exact transport/timeline state, per-line/field servo and phase memory, full ACC/APC/burst recovery, detailed multi-track overlap coupling and source-matched temporal curves.
- NTSC-QT still needs source-by-source numerical comparison for its RNG sequence, field-edge handling and rendered response. The spectral modes and pre-emphasis are implemented; exact pixel parity is not claimed.
- No After Effects-generated VHSart output has yet been rendered against the supplied camera footage. Therefore no visual-match score or final look acceptance is claimed.
