# NTSCRT source attribution and port notes

Upstream: https://github.com/finnmckenty/NTSCRT  
License: MIT — see [LICENSE](LICENSE).

VHS-90 selectively adapts physical VHS transport and tape-read concepts from the upstream Rust/WGSL engine into host-independent C++17. It does not embed the upstream Swift/macOS application, Rust runtime, WGPU renderer, CLI, MCP server, exporters, or preset UI.

Adapted areas currently present in VHS-90:
- Track-read adjacent-track weighting and opposite-azimuth sinc attenuation, including distinct FM and color-under carrier frequencies: `Source/VHS90Core/VHS90TrackReadEngine.cpp`.
- Deterministic control-track PI servo behavior (pulse threshold, false/missing pulse controls, bounded integrator, search/relock approximation): `Source/VHS90Core/VHS90TapeTransportEngine.cpp`.
- Two-head field-alternating debris/grease recurrence with deterministic pickup/lodging/shedding events, mapped to raster head conditions: `Source/VHS90Core/VHS90HeadConditionEngine.cpp`.

These are selective adaptations, not a full source-equivalent port. In particular:
- The tape material map (`tape_at`) is not yet ported at its full physical-coordinate resolution.
- Head debris recurrence currently uses deterministic proxy shedding/film values; it does not yet sample 12 physical positions along each recorded track.
- Transport servo uses render-local deterministic replay and proxy pulse quality; it does not yet use the exact tape-edge material sample, motor noise value interpolation, or full phase history from upstream.
- The RF/FM/color-under chain (`vcr_record`, `vcr_tape`, `vcr_magnet`, `vcr_play`, `vcr_fmband`, `vcr_line`, `vcr_comb`, `vcr_acc`, `vcr_out`) is not yet ported as a sample-domain engine.
- Receiver line/field synchronization remains a separate partial implementation and has not been parity-ported from `receiver.rs`.

The upstream preset inventory is reviewed separately in `docs/source-audits/43-ntscrt-preset-inventory-and-transport-findings.md`. Preserve this attribution with any substantial copied or adapted portions.
