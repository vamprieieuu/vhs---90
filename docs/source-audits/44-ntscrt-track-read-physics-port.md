# NTSCRT track-read physics port — implementation checkpoint

Date: 2026-11-20  
Upstream reference: `finnmckenty/NTSCRT/port/engine/src/glitch/tape.rs`  
Target: `vamprieieuu/vhs---90`

## Source equations verified

The upstream VHS tape read model computes the overlap of the head aperture with three adjacent recorded tracks, normalized by the 58 µm SP track width. Tracks with the opposite head azimuth receive frequency-dependent cancellation:

`lambda = 5.8 / frequency_MHz` µm

`x = pi * overlap_width_um * tan(12 degrees) / lambda`

`azimuth_loss = sin(x) / x` (with the zero-frequency limit equal to 1).

The source uses 4.0 MHz for the FM carrier and `40 / 63.556 MHz` for the color-under carrier. These are distinct channels; using a generic 0.55 MHz value is not equivalent to the source's color-under timing.

## Target changes

Updated `Source/VHS90Core/VHS90TrackReadEngine.cpp` in two commits:

- `bb3a56ddfcff801391f6dbee25486faedf9b609b`: replaced the approximate opposite-azimuth attenuation with the source's physical sinc equation and channel-specific frequencies.
- `7d8a636905f2f1c0e360a65c1aed68ebf674fefe`: corrected the adjacent-track sample coordinate so the three tracks are sampled relative to the active/center track (`-1, 0, +1`) rather than using absolute track indices as image-row offsets.

The existing TrackRead stage remains integrated in the composite pipeline and exposes the adjacent-track crosstalk and track-misalignment controls. This is a targeted physics correction, not a claim that the entire NTSCRT track/deck simulation has been ported.

## Validation

GitHub Actions Core CI was triggered for both commits:
- Run 439: https://github.com/vamprieieuu/vhs---90/actions/runs/44619031646
- Run 440: https://github.com/vamprieieuu/vhs---90/actions/runs/44619050942

At the time of this checkpoint both runs were still in progress. Do not report them as passing until their conclusions are available.

## Remaining NTSCRT work

- Complete a file-by-file audit ledger for all 359 upstream files; the recursive inventory and 45/45 preset JSON parse are complete, but the full source audit is not.
- Port the control-track pulse detector and PI capstan servo with missing/fake pulse behavior and search/relock semantics.
- Replace the head-condition approximation with the two-head, per-field deck recurrence and history/snapshot behavior.
- Port the RF/FM/color-under record/playback chain and its spacing/gap filters.
- Add behavioral parity tests, then run Windows/MSVC + AE SDK build and real After Effects/MFR tests. Linux core CI alone is not an AE runtime validation.
