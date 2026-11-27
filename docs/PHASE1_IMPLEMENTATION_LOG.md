# VHS-90 Phase 1 — Implementation Log
Date: 2026-11-27
Branch: `phase1-ui-signal-rebuild-2026-11-27`

## UI and preset architecture
- Reorganized the top-level hierarchy so **VHS Style & Presets** comes first, followed by **Master Strength**, then **Signal**.
- Increased Master Strength to a 0–300% UI range and retained a separate bounded overdrive path above 100%.
- Curated the visible VHS Style menu to 32 entries grouped by source family: VHS6RIFT, VHS Add-on, Lost Signal, Crates VHS Master, VHS Pro and VHS Craft.
- Added a compact selection-to-source-recipe map. Existing stored style values above 32 continue to map to their legacy recipe IDs.
- Preserved 11 camera profile entries and expanded the source-specific parameter signatures, including dedicated FXC6 and Gen 7 reference profiles.
- Kept NTSCRT Signal Looks within Signal and renamed them to make their source family explicit.
- Reorganized Grain controls next to Static/Tracking Noise, moved Luma Noise and Chroma Noise into Grain, and made those noise controls neutral by default.
- Kept separate Chroma, Ringing, Glow, General Adjustments, Tape and Mains Hum sections.
- Added RGB channel registration modes (RGB Split, Red/Cyan, Green/Magenta, Blue/Yellow) with independent red, green and blue offsets.

## Removed or retired UI/build paths
- Removed VHS Playback, Composite Decoder, VHS Playback Frame, Camcorder HUD, Temporal Frame, Particle Pattern, Signal Network and External Physical Interference controls.
- Removed Particle Pattern and Device Connection engine sources from the plugin build.
- Removed Python VHS generation from the Composite API/render pass and from the plugin build.
- Kept persistent disk IDs for surviving parameters unchanged; new RGB parameters use appended IDs 9056 and 9057.

## Signal and image-processing repairs
- Composite Signal now drives both luma and chroma bandwidth, using the selected SP/LP/EP tape bandwidth as the source model.
- Adjacent Track Crosstalk normalizes overlapping track shares so the effect changes track mixture rather than reducing frame opacity.
- RF Drift bias and line-wander values were corrected from 8-bit-like magnitudes to normalized image-space magnitudes.
- AGC Pump and AM Nonlinearity response curves were adjusted to react to ordinary footage levels.
- Tape Dropouts were rebuilt as sparse, tapered, animated RF-loss streaks and pinholes rather than broad solid rectangles.
- Analog Glow threshold was lowered and Bloom threshold reduced so the effect responds to ordinary highlight levels.
- Chroma Quantization range was reduced to avoid severe, unstable color changes.
- Tape Material Master now defaults to 100%, while individual tape-damage controls default to zero.
- RF Hum and Mains Hum have distinct labels and controls; RF Ghost Delay was reduced to a practical range.

## Validation
- Core unit/smoke tests passed for the current phase-1 baseline commit.
- After Effects parameter/host-contract validation passed.
- Persistent disk-ID values were compared against `main`: all surviving IDs retained their original numeric values; new IDs were appended.
- Parameter enum order, ParamsSetup registration order, disk-ID set equality and uniqueness were checked.
- Windows x64/MSVC build and direct AEX artifact publication are the final build gate for this phase.
- An actual After Effects GUI visual comparison is not available in the CI environment; do not treat a successful compile as proof of host visual fidelity.

## Remaining work (later phases)
- **Phase 2:** source-specific Tracking Noise presets, Head Switching presets, full-frame/edge placement controls, and reconstruction of Tear Chroma from VHS Lap references.
- **Phase 3:** procedural moving Static, Waves, one unified OSD/Text section, Arabic/RTL shaping, and final visual comparisons.
