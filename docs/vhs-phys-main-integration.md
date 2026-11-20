# vhs-phys-main — VHS-90 integration audit

Source: `vhs/github repos/vhs-phys-main` from the supplied `vhs.zip`.
Inventory: 202 files (94 Rust, 37 Python, 24 Markdown, 7 shell scripts, 6 JSON, 4 TOML, 3 text/no-extension files, 19 JPG, 2 PNG, 1 lockfile, 1 Python spec, 1 workflow).
Core source: 94 Rust files; test suite: 38 Rust test/support files; Python GUI/lab/test tooling is not part of the native AE render path.

## License gate

The source is marked **PolyForm Noncommercial 1.0.0**, not MIT. It grants changes/derivative works for permitted noncommercial purposes and requires license/notice propagation on distribution. It does not grant a general commercial-use right. Therefore no Rust/Python source was copied verbatim into VHS-90. The integration below is an independently authored C++ implementation of general physical relationships and image-domain behaviour, not a translation of the source's code. Any future commercial distribution must not rely on this source's restricted implementation or assume this license permits commercial distribution; obtain a separate commercial license or keep the independent implementation clean-room.

## Architecture findings

The project is a full record/playback simulator rather than a simple shader. Its signal path includes camera pickup, optical/spectral response, recording electronics, FM recording, magnetic tape, playback heads, timebase mechanics, deck electronics, decoding, CRT, and environmental interference. The NTSC/PAL variants share source but compile against separate timing constants. The Python GUI, schematic renderer, CLI, audio I/O, packaging, and gallery scripts are host application/tooling rather than AE render kernels.

## VHS-90 overlap map

| Source subsystem | Existing VHS-90 area | Decision |
|---|---|---|
| Tape defect map, oxide loss, debris, mold, scratches, creases | `VHS90TapeSurfaceEngine` | Existing engine already samples a physical tape surface; retain one authoritative implementation. |
| Capstan/drum wow, flutter, stick-slip, line jitter, control-track servo | `VHS90TapeTransportEngine` | Existing engine already provides raster warp and control-track servo; compare source spectra for future tuning, do not add a second transport pass. |
| Head spacing, azimuth, adjacent-track read, head contamination | `VHS90HeadConditionEngine`, `VHS90TrackReadEngine` | Existing head and track modules already cover these behaviours; no duplicate modules. |
| Dropout, AGC, RF drift, sync, chroma instability | `VHS90DropoutEngine`, `VHS90RfInstabilityEngine`, `VHS90ReceiverInstabilityEngine`, `VHS90CompositeEngine` | Existing controls cover image-domain approximations. Keep unified and avoid stacking duplicate noise. |
| Outside magnetic field, head-core saturation and tape erasure | No dedicated physically parameterized environment module | New independent C++ module added: `VHS90PhysicalInterferenceEngine`. |
| Ground loop / mains coupling and external RF pickup | Existing RF hum is artistic/heuristic; no SI-parameterized external source model | Added to the same new module, designed to complement rather than duplicate the existing RF look controls. |
| Camera tube target physics, spectral response, camera scan/beam bending | No equivalent camera-acquisition subsystem in the VHS-90 effect path | Valuable as a future optional Camera Capture module; not forced into the tape playback engine. |
| Full RF sample-domain record/playback pipeline and audio chain | No sample-domain recorder/audio signal pipeline in VHS-90's RGBA effect core | Not transplanted wholesale: its high-rate buffers and deck/session model do not fit the current frame-domain AE architecture. |
| Python GUI, CLI, schematic editor, gallery, packaging | Not applicable to native AE plugin | Excluded from render engine. |

## C++ integration

Added:
- `Source/VHS90Core/VHS90PhysicalInterferenceEngine.h/.cpp`
- `Tests/VHS90PhysicalInterferenceEngineSmoke.cpp`
- CMake core source and smoke-test registration.

The new module exposes SI-derived dipole field estimation, nonlinear magnetic head-core signal loss, a distributed switching-field estimate for tape erasure, mains/ground-loop hum, and RF pickup. It is deterministic by seed/frame, alpha-preserving, validates parameters, and defaults to a bit-exact pass-through. It is registered in CMake, has a dedicated smoke test, and is exposed as an optional stage inside `CompositeParams` / `processCompositeRoundTrip`. The After Effects panel does not yet expose dedicated controls for the physical source parameters; UI wiring remains pending and is not claimed complete in this audit. The source's camera pickup-tube stage was also assessed: it is a distinct pre-record camera-acquisition simulation (target charge memory, tube lag, beam landing, ACT/DBC, blooming, and spectral response), not a playback/tape degradation stage. It is a valid future optional Camera Capture engine, but was not forced into the current playback chain because it needs persistent frame/field state, pre-roll, and a separate UI contract; the source documentation reports a substantial performance cost for this stage.

## Source inspection status

The archive tree and all 202 paths were inventoried. Primary architecture, physical modelling, interference, camera, tube, mechanics, head, tracking, tape, electronics, and build manifests were inspected at source level for the integration decision. The remaining 202-file review is tracked as a bounded follow-up: per-test assertions, detailed camera submodels, Python GUI internals, CLI options, every design/research document, and packaging scripts/assets. Binary gallery images were classified as reference outputs, not source code.

## Validation

The standalone engine smoke test completed successfully in GitHub Actions run 620. CMake source/test registration passed in run 622. Composite pipeline API and stage wiring passed in run 624 (Linux core build and smoke-test workflow). Do not treat the module as AE-UI complete until controls are added to the parameter model and invoked in the actual render chain. The source's PolyForm Noncommercial license must be respected for any redistribution. Its LICENSE file includes the required notice: `Required Notice: Copyright allthemalware (https://github.com/allthemalware)`. No source file was copied verbatim; the notice is recorded here for provenance and any future licensing review.
