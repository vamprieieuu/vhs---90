# NTSCRT source attribution and port notes

Upstream: https://github.com/finnmckenty/NTSCRT  
License: MIT — see [LICENSE](LICENSE).

VHS-90 selectively adapts physical VHS transport, tape, deck and VCR signal concepts from the upstream Rust/WGSL and Swift/Metal implementations into host-independent C++17. It does not embed the upstream Swift/macOS application, Rust runtime, WGPU/Metal renderer, CLI, MCP server, exporters, or preset UI.

## Adapted C++ areas

- **Physical tape map** — `Source/VHS90Core/VHS90TapeSurfaceEngine.cpp`: physical millimetre coordinates (source TRACK_START = 1.0 mm), helical track/active-line/VBI mapping, SP/LP/EP field advance, four samples across head width, NTSCRT hash/value noise, remanence, pinholes, flakes/shed, scratches, creases, edge lift, mold and Vaseline film. The AE Tape Surface adapter uses the measured wear/dropout/scratch/edge curves, crease top-end transform and tape-seed transform.
- **Track read** — `Source/VHS90Core/VHS90TrackReadEngine.cpp`: absolute three-track overlap shares, source round-away-from-zero track selection, separate 4.0 MHz FM and 40/63.556 MHz color-under azimuth sinc, and VBI-scaled neighboring raster-line offsets.
- **Transport/control track** — `Source/VHS90Core/VHS90TapeTransportEngine.cpp`: deterministic PI servo adaptation, physical control-head tape sampling at the upstream control-track location, material-dependent pulse loss and crease-induced false pulses. It remains a simplified frame-domain replay, not the full upstream line-domain capstan/guide simulation.
- **Deck/head condition** — `Source/VHS90Core/VHS90HeadConditionEngine.cpp`: alternating two-head field recurrence, 12 physical tape samples per active field, detail-off/flake-on sampling, debris pickup/lodging/shedding/clean-off, grease transfer/decay, head efficiency and source-formula color boost.
- **VCR coefficient layer** — `Source/VHS90Core/VHS90VcrSignalEngine.cpp`: 28.636 MHz sample rate, 1820 samples/line, channel gain/CNR, 64×81 spacing bank, 95-tap analytic FM bandpass, Butterworth low-pass, emphasis/de-emphasis shelves, input trap and chroma delay.

## Known gaps — not a full source-equivalent port

- VHS-90 still exposes one Tape Surface master control rather than the upstream independent tape knobs and user tape-seed setting. Sticky-binder/transport stick-slip coupling and exact source parameter defaults/curves for every tape control remain incomplete.
- The deck recurrence replays from field zero and does not yet reproduce the upstream rolling 12-field state/snapshot cache.
- The transport PI model is frame-domain and does not yet reproduce the complete upstream line-domain capstan, guide lag, tension, flutter, search phase and history model.
- The nine-stage sample-domain RF/FM/color-under record/playback chain (`vcr_record`, `vcr_tape`, `vcr_magnet`, `vcr_play`, `vcr_fmband`, `vcr_line`, `vcr_comb`, `vcr_acc`, `vcr_out`) is not yet implemented; only its coefficient/helper layer is ported.
- Receiver line/field synchronization, AFC/VCO, burst phase, chroma PLL, ACC/color-killer and rolling history remain a separate partial implementation and have not reached parity with `Receiver.swift` / `receiver.rs` / `glitch.wgsl`.
- All 45 preset JSON files have been semantically reviewed; see docs/source-audits/47-ntscrt-preset-semantic-migration.md. Their stage boundaries and unsupported digital/Screen Loop dispositions are documented. Native AE preset conversion and visual parity are still pending.
- The 359-file source content audit is closed with 314 content-reviewed and 45 semantic-reviewed rows; see `docs/source-audits/46-ntscrt-359-file-disposition-ledger.md`. Port implementation and host validation remain open; see `docs/source-audits/45-ntscrt-same-session-port-status.md`.

The upstream preset inventory is also summarized in `docs/source-audits/43-ntscrt-preset-inventory-and-transport-findings.md`. Preserve this attribution with any substantial copied or adapted portions.