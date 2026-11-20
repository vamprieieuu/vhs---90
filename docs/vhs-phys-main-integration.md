# vhs-phys-main — VHS-90 integration audit

Source: `vhs/github repos/vhs-phys-main` from the supplied `vhs.zip`.
Inventory: 202 files (94 Rust, 37 Python, 24 Markdown, 7 shell scripts, 6 JSON, 4 TOML, 3 text/no-extension files, 19 JPG, 2 PNG, 1 lockfile, 1 Python spec, 1 workflow).
Core source: 94 Rust files; test suite: 38 Rust test/support files; Python GUI/lab/test tooling is not part of the native AE render path.

## License gate

The source repository includes its own license and attribution terms, which are retained here for provenance. The project owner has explicitly confirmed that they hold additional authorization covering direct reuse and adaptation of the supplied source code for VHS-90. That authorization is accepted as the project's permission basis for source reuse; the public repository license is not being used as a reason to block the requested integration. Preserve upstream notices and source provenance when transferring or adapting code.

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
| Camera tube target physics, spectral response, camera scan/beam bending | No equivalent camera-acquisition subsystem in the VHS-90 effect path | Distinct optional Camera Capture subsystem; not mixed into the tape-playback stages. Its temporal target-memory model needs a separate history-aware implementation and remains a clearly scoped follow-on inside this source family. |
| Full RF sample-domain record/playback pipeline and audio chain | No sample-domain recorder/audio signal pipeline in VHS-90's RGBA effect core | Not transplanted wholesale: its high-rate buffers and deck/session model do not fit the current frame-domain AE architecture. |
| Python GUI, CLI, schematic editor, gallery, packaging | Not applicable to native AE plugin | Excluded from render engine. |

## C++ integration

Added:
- `Source/VHS90Core/VHS90PhysicalInterferenceEngine.h/.cpp`
- `Tests/VHS90PhysicalInterferenceEngineSmoke.cpp`
- CMake core source and smoke-test registration.

The new module exposes SI-derived dipole field estimation, nonlinear magnetic head-core signal loss, a distributed switching-field estimate for tape erasure, mains/ground-loop hum, and RF pickup. It is deterministic by seed/frame, alpha-preserving, validates parameters, and defaults to a bit-exact pass-through. It is an optional stage inside `CompositeParams` / `processCompositeRoundTrip`. The After Effects panel now exposes an `External Physical Interference` topic with controls for magnet moment, magnet distance, ground-loop voltage, mains hum amount, RF field strength, and RF pickup amount. Mains frequency follows the selected video system (50 Hz PAL / 60 Hz NTSC). The effect settings are read into `RenderSettings`, used to enable the composite stage, and mapped into the physical engine parameters. CMake now explicitly registers the dedicated physical-interference smoke test. The source's camera pickup-tube stage was also assessed: it is a distinct pre-record camera-acquisition simulation (target charge memory, tube lag, beam landing, ACT/DBC, blooming, and spectral response), not a playback/tape degradation stage. It is a valid future optional Camera Capture engine, but was not forced into the current playback chain because it needs persistent frame/field state, pre-roll, and a separate UI contract; the source documentation reports a substantial performance cost for this stage.

## Source inspection status

The original archive contains 202 files: 94 Rust files (43 core modules, 45 Rust integration tests, 2 CLI files, 3 examples, and 1 shared test helper), 37 Python files, 24 Markdown documents, 7 shell scripts, 6 JSON files, 4 TOML files, 3 text files, 19 JPG reference images, 2 PNG images, and the remaining manifest/workflow/spec/license files. All Rust source files and module responsibilities have been inventoried; the Rust integration-test functions and their assertion/coverage conditions have been indexed by file. Python modules and tests, design/research documents, CLI/GUI code, packaging scripts, build manifests, and reference assets have been reviewed and classified. The Python GUI, schematic editor, CLI wrapper, plotting/measurement lab, audio I/O, and packaging utilities are companion tooling rather than native AE render kernels. The camera-tube acquisition model is the one distinct high-value subsystem that is not a direct tape-playback stage; it is documented as a separate history-aware camera module rather than being incorrectly stacked into playback.

## Validation

Python test suite: `214 passed, 72 skipped`. The skipped cases require a locally built `vhsphys` Rust CLI, which could not be built in this container because Cargo is unavailable; therefore those CLI-backed integration cases are not claimed as executed. `python -m compileall -q python packaging scripts` passed. Earlier VHS-90 C++ core and composite integration workflow runs 620, 622, and 624 passed. After registering the physical engine's dedicated smoke test, the first run exposed a small non-zero default erasure tail at zero magnetic field; this was corrected so a zero field is exactly inert. The corrected C++ core and all 29 smoke tests then passed in GitHub Actions runs 631 and 632. The six AE controls and CMake registration are present in the committed source. The AE UI wiring has been added to `Source/VHS90Effect.cpp`; the native Windows AE SDK build remains distinct from the Linux core CI and is not claimed as completed here. The source Rust crate's own `cargo test --workspace --offline` could not run because Cargo is not installed in this environment. Preserve the upstream license/notice and the project owner's confirmed separate authorization in any redistribution.
