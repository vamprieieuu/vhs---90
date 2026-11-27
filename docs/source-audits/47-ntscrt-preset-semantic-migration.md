# NTSCRT preset semantic migration audit (39/39 supplied repository snapshot)

Repository-tree verification on 2026-11-27 found 39 JSON files beneath `VHS/github repos/NTSCRT-0.26.0/presets/` in this checked-in source snapshot: 34 top-level preset files and 5 Screen Loop feedback presets. The earlier 45/45 count is not supported by this snapshot and must not be repeated as a verified count. The 39 files are semantically classified below; the five Screen Loop files are a separate video-feedback family, not analog VHS presets. This pass distinguishes enabled stages from serialized defaults and inspects active values, descriptions, downscale/shader choices, timeline metadata and the separate Screen Loop schema. Parsed JSON is not automatically a port candidate.

## Schema and migration rules

Normal looks contain description, digital, downscale, glitch, ntsc, shader, timeline, version and view sections. Stage enabled flags are authoritative: many looks serialize complete default parameter maps for disabled stages. Never activate those maps during migration.

- glitch.values contains receiver/VCR controls: tape generation/speed/wear/dropouts/scratches/creases/edge damage/mold/sticky/vaseline, deck/head dirt/buildup/wear/mismatch, back tension, tracking offset, head switch, AFC/search and transport flutter. This is the closest source for VHS-90 tape transport, surface, track-read, head-condition and receiver engines.
- ntsc.settings is a separate NTSC/analog shader-stage parameter set: composite/chroma/luma noise, bandwidth, chroma delays, phase, pre-emphasis, VHS edge wave/sharpen, tracking-noise region and head-switching display controls. These are not the VCR physical tape model. Only map controls with equivalent VHS-90 DSP behavior; do not claim a full ntsc-rs port.
- timeline.keys stores complete time-stamped snapshots of NTSC and shader parameter dictionaries with easing, not merely glitch.values keyframes. Migration requires flattening each snapshot into independent After Effects parameter keyframes at the same times/easing.
- shader selects a third-party CRT shader and uniforms. Several bundled shaders have GPL terms; do not copy those shader sources into VHS-90 unless licensing is deliberately made compatible. Prefer native original implementations.
- digital is a separate digital-camera/codec stage (CCD, frame rate, Video 1/DCT, keyframes, data loss, datamosh); it is not analog VHS.
- downscale, view, version, export and timeline presentation are host/preset metadata. AE layer dimensions, render scheduling and host keyframes replace app-specific controls.
- presets/Screen Loop uses kind=ntscrt-video-feedback and a separate camera-feedback model. It is not VHS and is excluded.

## Per-preset disposition

| Preset | Semantic content | VHS-90 migration disposition |
|---|---|---|
| 2 second dirty VCR loop | 2 s/24 fps, 15 whole-state keys; VCR enabled, SP tape, wear/dropouts/scratches/creases, head contamination/mismatch and tracking offset; NTSC active, CRT off | High-value tape/deck/transport candidate. Convert NTSC snapshots only where matching DSP controls exist. |
| 5 second glitch | Timeline glitch preset; inspect enabled receiver/NTSC/CRT stages and preserve all source key times/easing during any AE conversion | Timeline migration candidate; do not assume the stage values are identical to other glitch presets. |
| 15 second glitches | Long-form timeline glitch preset; retain its own duration, enabled-stage map and keyframe timing rather than reusing a shorter loop recipe | Timeline migration candidate; source-specific snapshot conversion remains required. |
| 5 seconds mild glitch | Mild timeline glitch preset; preserve the source frame rate, enabled stages, easing and full-state snapshots | Timeline migration candidate; convert only supported receiver/NTSC controls. |
| 5 second blips | 5 s/30 fps, 16 keys; short receiver blips and NTSC softness/chroma/edge changes; shader disabled | Receiver and NTSC snapshot migration; keep shader disabled. |
| 90s VHS bootleg | Static look; VCR generation, tension, head dirt/buildup, creases, dropout, scratches and transport; NTSC active; CRT off | High-value physical tape/deck/transport preset candidate. |
| CD-ROM (low quality) | Digital-only Video 1/15-bit block codec, 240 px nearest downscale; no tape/NTSC/CRT | Exclude from analog VHS engine. |
| Cheap camcorder | Digital-only camera response/noise/low-light/saturation; no tape/NTSC/CRT | Exclude from analog VHS engine. |
| Chunky webcam | Digital-only low-resolution compression/blur/chroma; no tape/NTSC/CRT | Exclude from analog VHS engine. |
| Clean CRT | CRT shader only with 256 px area downscale; no tape/NTSC/receiver | CRT reference only; shader license applies. |
| Clean VHS | NTSC stage only with moderate bandwidth/chroma/luma response; no VCR/receiver damage | Signal-chain reference, not tape/deck. Port only matching composite/chroma/luma kernels. |

| Dirty VCR + CRT (vertical) | Portrait-oriented 160 px downscale, generation 3, heavy creases/dropouts/scratches and deck/head wear; NTSC/CRT active | VCR damage maps to physical engines; portrait/downscale belongs to host; NTSC/CRT separate. |
| Dirty VCR | VCR wear, mold, scratches, vaseline, creases, head dirt/mismatch and dropouts; NTSC/CRT active | High-value static deck/tape candidate; use measured damage curves. |
| Gentle waves loop | 2 s/24 fps; NTSC edge-wave/chroma behavior plus CRT; no VCR receiver stage | NTSC shader-style animation, not physical tape transport. |
| Glitch 1 | 2 s/24 fps, 9 keys; NTSC edge wave/chroma loss/phase and head-switching changes; CRT bloom | Despite its name, glitch stage is disabled. NTSC/CRT animated look, not VCR. |
| Glitch 2 | 2 s/24 fps, 10 keys; 128 px nearest downscale, heavy static/snow/tracking-noise NTSC parameters and CRT | NTSC noise/tracking animation, not VCR track reading. |
| Gltch 3 | NTSC head-switch display position/height, tracking noise and color-noise preset; preserve its exact source spelling and inspect the JSON timeline before porting | NTSC display/edge tracking candidate; translate coordinate model before mapping. |
| Glitch loop 1 | 2 s/12 fps, 7 keys; NTSC softness/edge and CRT glow pulse | NTSC parameter animation; convert snapshots to AE keyframes. |
| Glitch pop | 2 s/12 fps, 11 keys; short NTSC blur/chroma-loss/edge-wave pulses; CRT | NTSC animation only; preserve pulses and return-to-normal states. |
| High Chroma static | Static NTSC colored static/noise on 256 px downscale with CRT | Signal/noise preset, not tape surface damage. |
| Low Res | Static 128 px nearest downscale with NTSC chroma bleed and CRT | Host downscale plus NTSC signal reference; no VCR model. |
| Medium VHS | Static moderate NTSC bandwidth/noise/chroma bleed and CRT | Signal-chain baseline, not physical tape/deck damage. |
| Mild VHS | Static light NTSC softness/noise with CRT | Low-intensity signal-chain baseline. |

| Obliterated | Static extreme NTSC smear/chroma bleed/noise with CRT; 5 s timeline enabled but zero keys | Static signal-chain preset; enabled timeline without keys has no animation data. |
| Readable Text | Light NTSC VHS plus 384 px area downscale and sharper Lanczos CRT shader | Signal-chain + host downscale + shader; legibility-focused preset. |
| Screen Loop/12 copies luma key | ntscrt-video-feedback camera-feedback look with key inversion, zoom/turn/drift | Exclude: separate Screen Loop renderer. |
| Screen Loop/15 copies color drift | ntscrt-video-feedback, 2 s, color/hue drift and 15-copy feedback composition | Exclude: separate camera-feedback effect. |
| Screen Loop/22 copies bottom left | ntscrt-video-feedback, 22-copy/bottom-left composition and drift | Exclude: separate camera-feedback effect. |
| Screen Loop/7 copies slow luma | ntscrt-video-feedback, 2 s, 7-copy slow-luma/key composition | Exclude: separate camera-feedback effect. |
| Screen Loop/Bottom to top | ntscrt-video-feedback, vertical drift from bottom to top with zoom/turn | Exclude: separate camera-feedback effect. |
| Static loop 1 | 2 s/12 fps, 8 NTSC/CRT keys; waves of static, smear and fading color | NTSC noise/edge/chroma keyframe sequence; no VCR stage. |
| Tracking Noise up | 2 s/24 fps, 10 NTSC/CRT keys; tracking-noise band grows from bottom with changing height/intensity/edge-wave/chroma | Region-based NTSC tracking-noise animation. VHS-90 arbitrary-position tracking region is conceptually relevant; translate its bottom-up motion curve explicitly. |
| Two blips loop | 2 s/12 fps, 8 NTSC/CRT keys; two short edge-wave/blur pulses | NTSC parameter animation, not VCR damage. |
| V Hold glitch | 2 s/12 fps, 8 keys; receiver vertical/horizontal hold, crinkle/dropouts/head clog/head-switch and fast search; NTSC/CRT active | Mixed receiver + NTSC animation; map receiver controls partially and preserve keyframes/easing. |

| Vertical Freak Out | 2 s/24 fps, 6 keys; receiver vertical roll, horizontal slip and head clog; NTSC/CRT active | Receiver/sync animation; map hold controls and translate keyframes. |
| Vertical spike loop | 2 s/12 fps, 6 NTSC/CRT keys; vertical chroma smear/spikes and blur bursts | NTSC vertical chroma/edge animation, not VCR track geometry. |
| Very wavy | 2 s/24 fps, 5 NTSC/CRT keys; strong slow edge waves, NTSC filter/noise and vertical scaling | NTSC edge-wave animation; port only if corresponding DSP stage exists. |
| Wavy loop | 2 s/24 fps, NTSC edge-wave with tracking noise and CRT; timeline enabled with zero keys | Static NTSC wave look; no timeline keys. |
| Webcam (low quality) | Digital-only 256 px webcam camera/compression/noise/aliasing; tape/NTSC/CRT disabled | Exclude from analog VHS engine. |

## Related source-engine families (not preset JSON files)

The following rows are feature-family dispositions and are not included in the 39-file preset inventory above.

| Source family / engine | Semantic content | VHS-90 migration disposition |
|---|---|---|
| glitchy block video | Digital-only Microsoft Video 1 block codec, heavy data loss, ~4 s keyframe interval; tape/NTSC/CRT disabled | Exclude from analog VHS engine; separate digital codec. |
| glitchy streaming video with noise | Digital-only fixed-bitrate stream, long keyframe interval, data loss/noise/sharpening; tape/NTSC/CRT disabled | Exclude from analog VHS engine. |
| glitchy streaming video | Digital-only low-quality streaming compression, sharpening/moiré/data loss; tape/NTSC/CRT disabled | Exclude from analog VHS engine. |

## Mapping into the current VHS-90 architecture

| NTSCRT concept | VHS-90 destination | Port status / caveat |
|---|---|---|
| Tape physical damage: wear, pinholes, flakes, scratches, creases, edge damage, mold, vaseline, sticky shed | VHS90TapeSurfaceEngine | Selective physical-coordinate surface port exists, including deterministic cell/noise sampling and damage curves. Track origin must remain 1.0 mm; NTSCRT TapeGeometry::TRACK_START is 1.0 mm, not 1.52 mm. |
| Tape transport, search, tension, flutter and control-track servo | VHS90TapeTransportEngine | Control-track PI approximation and deterministic transport exist; not full physical TapePosition/field-history parity. |
| Two rotating heads, dirt transfer, wear, mismatch and clogging | VHS90HeadConditionEngine | Selective two-head recurrence exists; full 12-point physical track sampling and original per-head history remain incomplete. |
| Track overlap, azimuth, FM/color-under read | VHS90TrackReadEngine | Adjacent-track weighting and azimuth attenuation exist; full RF/FM/color-under signal chain remains incomplete. |
| Receiver sync/hold/roll/ghost/snow/color reference | VHS90SyncEngine / VHS90CompositeEngine | Related effects exist, but NTSCRT row-plan parity and color-reference behavior are not a complete one-to-one port. |
| NTSC composite/chroma/luma shader controls | New/extended analog signal kernels | Not equivalent to physical tape damage; don't mark ported based on tape-surface controls. |
| CRT shaders | Separate original VHS-90 CRT stage if desired | Third-party shader licensing applies; GPL shader source excluded by default. |
| Digital camera/codec | Separate future digital engine | Out of scope for this analog VHS port. |
| Screen Loop | Separate feedback effect | Explicitly out of scope. |

## Migration conclusions

1. Highest-value preset families for this analog VHS-90 task: 90s VHS bootleg, Damaged VHS - strong, Dirty VCR, Dirty VCR + CRT (vertical), Noisy TV broadcast on VHS, VHS with color loss and 2 second dirty VCR loop. They exercise tape generation/speed, physical damage, deck/head condition, transport and tracking.
2. Receiver-oriented looks (2 second glitch(es), 2 second mild glitch, 5 second blips, V Hold glitch and Vertical Freak Out) require receiver mapping and host keyframes, not merely copying base parameter values.
3. Presets with glitch.enabled=false are not evidence that VCR controls were exercised. Their visible effects are carried by ntsc.settings and whole-state timeline snapshots.
4. Digital-only and Screen Loop presets are classified and intentionally not ported into the analog tape engine.
5. Preset JSON values are source settings, not proof of visual parity. A final preset port requires native AE parameter mapping, animation conversion, deterministic render comparisons and AE-host validation.
