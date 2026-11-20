# NTSCRT 359-file audit ledger

Generated from upstream recursive Git tree on 2026-11-20. Upstream tree reports 359 blobs, 59 directory entries, `truncated=false`.

**Important:** this is a complete path/size disposition ledger, not a completed content audit. Only rows marked `content-reviewed` were read in this audit pass. The 45 JSON preset files were fetched and parsed, but their conversion to VHS-90 presets is pending. Every other row is explicitly left open with a reason; do not interpret inventory status as code review or port completion.

**Current disposition counts:** 149 content-reviewed; 45 preset JSON parsed; zero engine-review-pending; zero platform-review-pending; 58 test/reference-pending; 49 inventory-only; 45 asset-inventory; 13 metadata-inventory. The code/engine and app/platform content passes now have dispositions, but the full 359-file audit remains open until tests/references, inventory-only files, binary assets, metadata and preset-by-preset semantics are closed.

| Upstream path | Bytes | Status | Disposition |
|---|---:|---|---|
| `.gitattributes` | 73 | content-reviewed | Reviewed Git line-ending policy for Windows command launchers; repository metadata only. |
| `.github/workflows/mcp-registry.yml` | 2764 | content-reviewed | Reviewed OIDC-based release publication to MCP Registry; packaging automation only. |
| `.github/workflows/windows-port.yml` | 7655 | content-reviewed | Reviewed full Windows D3D12 WARP build, shader/reference tests, CLI/MCP checks and artifact packaging. This is an independent Rust/wgpu desktop build, not a native After Effects target. |
| `.gitignore` | 532 | content-reviewed | Reviewed generated Swift/Rust/build outputs and locally built vendor binaries; no engine content. |
| `.gitmodules` | 215 | content-reviewed | Reviewed ntsc-rs and slang-shaders submodule origins; their code/licenses are separate dependencies and must not be treated as NTSCRT-authored code. |
| `Assets/AppIcon.icns` | 1419876 | content-reviewed | Inspected binary asset metadata; macOS application icon only, no effect data. |
| `Assets/icon-1024.png` | 617475 | content-reviewed | Inspected image asset metadata; macOS app branding icon, not a VHS render resource. |
| `Assets/icon-source.png` | 636923 | content-reviewed | Inspected image asset metadata; source artwork for app icon only. |
| `CertificateSigningRequest.certSigningRequest` | 964 | content-reviewed | Reviewed PEM certificate signing request; signing identity artifact only, contains no effect code and must not be copied into VHS-90. |
| `DEVELOPMENT.md` | 80655 | content-reviewed | Reviewed 451-line development guide: pipeline order, VCR/tape physical model, measured controls, head/transport behavior, build/probe commands and performance notes. Strong algorithm and tuning reference; implementation lives in audited Tape/Deck/Transport/VcrStage files. |
| `LICENSE` | 1069 | content-reviewed | Read during this audit pass |
| `NTSCRT july 27.webp` | 80040 | content-reviewed | Inspected image asset metadata; same NTSCRT marketing/header image family as docs/header.webp, not an effect input. |
| `Package.swift` | 3983 | content-reviewed | Reviewed SwiftPM package/target graph, macOS 14 floor, Metal/AVFoundation/ImageIO dependencies, executables and test target composition. Confirms original host is macOS-specific. |
| `README.md` | 19946 | content-reviewed | Reviewed product pipeline, stages, preset behavior, platform scope, dependencies and MIT/third-party license boundaries; no additional render kernel. |
| `Sources/CrtApp/AppState+Export.swift` | 8347 | content-reviewed | Reviewed movie/GIF/PNG export routing and async status handling; app export orchestration only, no AE render kernel. |
| `Sources/CrtApp/AppState+Howlaround.swift` | 17019 | content-reviewed | Reviewed Howlaround export sizing, draft scheduling, render cancellation and feedback-preset load/save. Separate Screen Loop feedback effect/export workflow, not VHS tape engine. |
| `Sources/CrtApp/AppState.swift` | 102696 | content-reviewed | Reviewed main SwiftUI app state: source/crop, preset/look migration, VCR and reception toggles, frame/keyframe state, preview invalidation and export routing. App orchestration only; core algorithms live in CrtCore/port/engine. |
| `Sources/CrtApp/BuiltInPresets.swift` | 1572 | content-reviewed | Reviewed dynamic JSON preset-folder discovery and natural sorting; app menu resource loader, no effect code. |
| `Sources/CrtApp/CLI/CLIHelp.swift` | 2255 | content-reviewed | Reviewed CLI command/option contract; command-line documentation only. |
| `Sources/CrtApp/CLI/ConnectSelfTest.swift` | 3187 | content-reviewed | Reviewed Claude Desktop connection self-test and UI snapshot harness; test-only integration. |
| `Sources/CrtApp/CLI/DescribeCommand.swift` | 19368 | content-reviewed | Reviewed JSON stage/control descriptions and preset summaries; useful documentation/schema reference but MCP/CLI output wrapper, no render kernel. |
| `Sources/CrtApp/CLI/Launch.swift` | 3557 | content-reviewed | Reviewed executable routing, stdout/stderr separation and CLI entry point; app launcher only. |
| `Sources/CrtApp/CLI/LookSettings.swift` | 31419 | content-reviewed | Reviewed JSON look loading and migration, stage enable/disable, tape/deck condition selection, parameter validation, legacy Tape→VCR migration and keyframe snapshot/easing. Preset conversion rules are useful reference; Metal/CLI state mutation is app-specific. |
| `Sources/CrtApp/CLI/MCPServer.swift` | 40664 | content-reviewed | Reviewed stdio MCP protocol, tool dispatch, child render jobs, progress/wait/cancel and image result handling. App integration only; no analog effect algorithms. |
| `Sources/CrtApp/CLI/RenderCommand.swift` | 21865 | content-reviewed | Reviewed CLI render request parsing, stage settings merge, time/frame selection, still/movie/GIF output routing and summary. Host command wrapper; AE host handles rendering. |
| `Sources/CrtApp/ClaudeConnection.swift` | 8114 | content-reviewed | Reviewed Claude Desktop/Code MCP registration and config-preserving connect/disconnect. External app integration only. |
| `Sources/CrtApp/CrtAppApp.swift` | 6259 | content-reviewed | Reviewed SwiftUI app bootstrap, renderer setup, menu/window and connect-to-Claude UI. macOS app shell, no effect code. |
| `Sources/CrtApp/ExportFormat.swift` | 1171 | content-reviewed | Reviewed H.264/HEVC/ProRes/GIF export format selection. App exporter metadata only. |
| `Sources/CrtApp/FeedbackPresets.swift` | 2305 | content-reviewed | Reviewed separate Screen Loop feedback preset discovery/save path and `kind` discriminator; not one of the 40 normal looks and not analog tape. |
| `Sources/CrtApp/NeutralValues.swift` | 7105 | content-reviewed | Reviewed per-control neutral/reset values for NTSC and CRT shader parameters. UI reset policy only; not VHS tape behavior. |
| `Sources/CrtApp/NtscSettingsModel.swift` | 8519 | content-reviewed | Reviewed dynamic ntsc-rs descriptor parser, control grouping/labels and legacy VHS controls. The source app explicitly hides ntsc-rs tape-speed/head-switch/tracking filters because the VCR panel replaces them with physical tape/deck simulation; preserve this separation in VHS-90. |
| `Sources/CrtApp/ParamGates.swift` | 7368 | content-reviewed | Reviewed empirically measured shader-parameter gating rules and input-resolution requirements. CRT shader UI enable/disable hints, not analog tape engine. |
| `Sources/CrtApp/Paths.swift` | 5052 | content-reviewed | Reviewed macOS bundle/vendor/env path discovery for Metal, ntsc-rs, shaders and presets. App packaging utility only. |
| `Sources/CrtApp/Timeline.swift` | 7300 | content-reviewed | Reviewed whole-state keyframe snapshots, easing, numeric interpolation and discrete bool/choice handling. AE host keyframes replace this timeline in the native plug-in. |
| `Sources/CrtApp/UpdateChecker.swift` | 7858 | content-reviewed | Reviewed GitHub release-feed checks and update-notification UI. No rendering or effect code. |
| `Sources/CrtApp/Views/ConnectClaudeView.swift` | 4915 | content-reviewed | Reviewed SwiftUI Claude connection window and status/actions; external MCP configuration UI only. |
| `Sources/CrtApp/Views/ContentView.swift` | 159354 | content-reviewed | Reviewed main macOS window/sidebar composition, preview overlays, development self-tests and panel assembly. SwiftUI app shell, not effect engine. |
| `Sources/CrtApp/Views/CropPanel.swift` | 4230 | content-reviewed | Reviewed crop aspect presets and preview drag/zoom controls; UI wrapper around generic crop geometry. |
| `Sources/CrtApp/Views/DigitalVideoPanel.swift` | 6041 | content-reviewed | Reviewed separate digital codec control panel and inter-control hints; optional digital stage UI, not analog VHS. |
| `Sources/CrtApp/Views/DownscalePanel.swift` | 5896 | content-reviewed | Reviewed resolution presets and sampling selector UI; wraps generic downscale stage. |
| `Sources/CrtApp/Views/ExportOptions.swift` | 7553 | content-reviewed | Reviewed shared export size/quality/fps/loop options; UI state only. |
| `Sources/CrtApp/Views/ExportPopover.swift` | 7382 | content-reviewed | Reviewed save-panel and export action UI; exporter implementation is separately audited. |
| `Sources/CrtApp/Views/GlitchPanel.swift` | 4913 | content-reviewed | Reviewed receiver/glitch controls generated from GlitchParam metadata; UI only, source receiver algorithms are separately audited. |
| `Sources/CrtApp/Views/HowlaroundPanel.swift` | 17733 | content-reviewed | Reviewed Screen Loop feedback camera controls, draft preview player and render/preset actions; separate feedback feature UI. |
| `Sources/CrtApp/Views/NtscPanel.swift` | 10493 | content-reviewed | Reviewed dynamic ntsc-rs controls and gating; source intentionally separates NTSC TV settings from VCR physical tape controls. |
| `Sources/CrtApp/Views/NumericField.swift` | 5558 | content-reviewed | Reviewed reusable numeric/int text fields and commit/clamp behavior; generic UI component. |
| `Sources/CrtApp/Views/PresetCycleCheck.swift` | 12372 | content-reviewed | Reviewed preset-cycle performance/state-leak regression harness and memory/stall measurement; test-only UI tooling. |
| `Sources/CrtApp/Views/PreviewView.swift` | 40712 | content-reviewed | Reviewed MTKView render target, frame redraw, compare split, zoom/pan, crop interaction and playback display. macOS preview renderer, not AE effect core. |
| `Sources/CrtApp/Views/PropertySlider.swift` | 4775 | content-reviewed | Reviewed NSSlider bridge, neutral reset and knob double-click handling; UI-only. |
| `Sources/CrtApp/Views/ShaderPanel.swift` | 15412 | content-reviewed | Reviewed dynamic shader parameter sections/control presentation and neutral resets; CRT shader UI only. |
| `Sources/CrtApp/Views/SourcePanel.swift` | 2516 | content-reviewed | Reviewed file open/clear/drop source selection UI; media input wrapper only. |
| `Sources/CrtApp/Views/TimelineBar.swift` | 13727 | content-reviewed | Reviewed keyframe ruler, playhead, easing chips and selection UI; AE host timeline replaces it. |
| `Sources/CrtApp/Views/ToolTip.swift` | 1065 | content-reviewed | Reviewed pass-through AppKit tooltip overlay; UI helper only. |
| `Sources/CrtApp/Views/TransportBar.swift` | 2564 | content-reviewed | Reviewed video play/pause/scrub/time/fps UI; source playback controls only. |
| `Sources/CrtApp/Views/VanishingPointPad.swift` | 12645 | content-reviewed | Reviewed Screen Loop camera vanishing-point/drift pad and hit geometry; separate feedback UI. |
| `Sources/CrtApp/Views/VcrPanel.swift` | 3795 | content-reviewed | Reviewed tape/deck condition pickers and parameter group UI. Confirms VCR controls are independent tape/deck physical conditions, not NTSC filter controls. |
| `Sources/CrtApp/Views/ViewPalette.swift` | 3389 | content-reviewed | Reviewed compare/integer-scale/animate/zoom preview-only controls; these do not affect exported pixels. |
| `Sources/CrtAppBridge/LibrashaderBridge.m` | 7857 | content-reviewed | Reviewed dynamic librashader Metal ABI loader, chain creation, texture processing, parameter descriptors and runtime error handling; Metal-specific bridge cannot be reused by AE CPU core. |
| `Sources/CrtAppBridge/NtscRsBridge.m` | 4603 | content-reviewed | Reviewed dynamic C ABI loader and Objective-C wrapper for ntsc-rs JSON settings/frame processing; portable algorithm is upstream ntsc-rs, while this bridge is macOS-specific. |
| `Sources/CrtAppBridge/include/LibrashaderBridge.h` | 2340 | content-reviewed | Reviewed Objective-C/Metal public bridge declarations; macOS-only interface. |
| `Sources/CrtAppBridge/include/NtscRsBridge.h` | 1790 | content-reviewed | Reviewed Objective-C NTSC wrapper declarations; macOS-only interface. |
| `Sources/CrtAppBridge/include/librashader.h` | 108825 | content-reviewed | Reviewed vendored upstream librashader C API header (MIT); generic shader runtime ABI, no VHS analog behavior and no AE CPU implementation. |
| `Sources/CrtAppBridge/include/librashader_ld.h` | 70783 | content-reviewed | Reviewed vendored upstream librashader dynamic-loading header (MIT); platform/runtime binding declarations only. |
| `Sources/CrtAppBridge/include/module.modulemap` | 79 | content-reviewed | Reviewed Clang module map exporting the bridge umbrella header; build metadata only. |
| `Sources/CrtCodec/DigitalCodec.swift` | 31137 | content-reviewed | Reviewed full codec architecture and core DCT/motion search/residual/reconstruction/loss/deblock paths. This is a separate digital block-transform/d datamosh codec, not analog VHS tape; keep as an optional independent subsystem, not merged into the tape engine. |
| `Sources/CrtCodec/DigitalVideoParams.swift` | 18748 | content-reviewed | Reviewed independent Digital stage parameter schema, codec/chroma choices and format presets. Digital compression controls; no direct analog VHS engine port. |
| `Sources/CrtCodec/DigitalVideoRenderer.swift` | 38039 | content-reviewed | Reviewed capture timing, camera/sensor optics, codec state progression, keyframe/checkpoint rerun and RGB reconstruction. Digital codec/camera pipeline; not analog tape. |
| `Sources/CrtCodec/Video1Codec.swift` | 10694 | content-reviewed | Reviewed Microsoft Video 1 4×4 block modes, 5:5:5 palette quantization, skip/fill/two/eight-color coding and loss persistence. Optional digital codec only; no analog tape port. |
| `Sources/CrtCore/ChainInputCache.swift` | 4145 | content-reviewed | Reviewed generation/downscale-stamped frame cache and thread-safe preview probes. App RAM-preview optimization; not part of the native effect kernel. |
| `Sources/CrtCore/CrtCodecExport.swift` | 207 | content-reviewed | Reviewed module re-export only; no implementation to port. |
| `Sources/CrtCore/Deck.swift` | 7542 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/DigitalStage.swift` | 12116 | content-reviewed | Reviewed Metal digital-stage orchestration, history-based background reruns and capture-frame sequencing. Separate digital subsystem and platform wrapper. |
| `Sources/CrtCore/Downscaler.swift` | 11854 | content-reviewed | Reviewed nearest/nearest+/bilinear/bicubic/Lanczos/area Metal downscale kernels and separable minification. Potential standalone resampling reference; no VHS tape behavior. |
| `Sources/CrtCore/GifExporter.swift` | 19879 | content-reviewed | Reviewed GIF export, frame pipeline assembly and ImageIO destination handling. App exporter, not plugin effect code. |
| `Sources/CrtCore/GlitchParams.swift` | 27087 | content-reviewed | Full 30-control schema reviewed: reception, tape, deck, condition pickers, timing IDs and defaults. Source UI schema is reference; VHS-90 currently maps a subset to its existing AE controls. |
| `Sources/CrtCore/GlitchRenderer.swift` | 14627 | content-reviewed | Metal-side simulation owner reviewed: deterministic forward stepping, timing-history resimulation, per-second snapshots, checkpoint-key invalidation and background rerun cancellation. Snapshot/cache semantics are portable; Metal rendering/thread dispatch is not. |
| `Sources/CrtCore/GlitchStage.swift` | 21056 | content-reviewed | Metal wrapper and three-pass receiver shader orchestration reviewed (gate, chroma reference, compose), including uniform layout, VCR-before-TV ordering and CPU FieldPlan contract. Native AE requires a CPU C++ port, not direct Metal reuse. |
| `Sources/CrtCore/Howlaround.swift` | 54180 | content-reviewed | Reviewed camera-to-TV feedback effect: 3D camera pose, TV feedback ring, blur, exposure meter, keying and run-up history. Separate CRT feedback effect, not part of analog VHS transport. |
| `Sources/CrtCore/HowlaroundPad.swift` | 7570 | content-reviewed | Reviewed preview vanishing-point control geometry and drag/snap rules. UI-only geometry, no effect engine. |
| `Sources/CrtCore/ImageIO.swift` | 4491 | content-reviewed | Reviewed Metal texture load/write/target/staging helpers. Platform I/O wrapper, not reusable analog signal code. |
| `Sources/CrtCore/ImageStats.swift` | 1894 | content-reviewed | Reviewed row-modulation/luminance image measurement helpers used by tests and export checks. Test/diagnostic utility only. |
| `Sources/CrtCore/MetalContext.swift` | 1014 | content-reviewed | Reviewed singleton Metal device/queue ownership and Downscaler construction. macOS GPU infrastructure, not portable to AE CPU core. |
| `Sources/CrtCore/Mp4Exporter.swift` | 35617 | content-reviewed | Reviewed AVFoundation H.264/HEVC/ProRes export pipeline. App export subsystem, not plugin effect code. |
| `Sources/CrtCore/NtscStage.swift` | 7626 | content-reviewed | Reviewed CPU/Metal bridge to ntsc-rs C API, including shared/managed texture synchronization and clean-frame caching. macOS wrapper only; VHS-90 already links its own AnalogNTSC C++ library. |
| `Sources/CrtCore/Pipeline.swift` | 7476 | content-reviewed | Reviewed Metal downscale → shader-chain orchestration and frame input ownership. App-specific Metal/librashader integration; not part of physical tape engine. |
| `Sources/CrtCore/PlaybackPipeline.swift` | 22040 | content-reviewed | Reviewed AVFoundation decode queue, two-filter pipelining, frame generation invalidation and buffer-backed texture pool. macOS playback infrastructure; no direct AE effect port. |
| `Sources/CrtCore/Presets.swift` | 1375 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/PreviewCompositor.swift` | 12012 | content-reviewed | Reviewed letterbox/zoom/pan/compare preview compositor and Metal render pipelines. UI preview-only. |
| `Sources/CrtCore/PreviewGeometry.swift` | 4836 | content-reviewed | Reviewed pure drawable-to-texture mapping and texel-row math. Preview-only geometry; no tape processing. |
| `Sources/CrtCore/PreviewScaling.swift` | 7192 | content-reviewed | Reviewed render/display scale planning and integer-grid constraints. Preview render policy, not tape engine. |
| `Sources/CrtCore/Receiver.swift` | 48200 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/ScanlineGrid.swift` | 6334 | content-reviewed | Reviewed supersampling/integer scanline-grid output planning and area downsampling. CRT export utility, not VHS tape code. |
| `Sources/CrtCore/ShaderBypass.swift` | 8205 | content-reviewed | Reviewed no-shader nearest enlargement and frame override plumbing. Preview/export presentation path, not analog signal engine. |
| `Sources/CrtCore/SourceCrop.swift` | 9603 | content-reviewed | Reviewed crop/aspect/position and Metal buffer-backed crop helpers. Source-image geometry utility, not tape behavior. |
| `Sources/CrtCore/Tape.swift` | 29129 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/Transport.swift` | 13900 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/UpdateCheck.swift` | 4189 | content-reviewed | Reviewed GitHub release version comparison and release-note parsing. Network/update UI utility, no port. |
| `Sources/CrtCore/VcrStage.swift` | 59635 | content-reviewed | Read in this pass; disposition and C++ reuse decision recorded in report 45 (Swift/Metal source is reference, not directly compiled into the AE plug-in) |
| `Sources/CrtCore/VideoSource.swift` | 12672 | content-reviewed | Reviewed AVFoundation random/sequential frame readers and CVMetalTextureCache conversion. macOS media I/O infrastructure, not effect engine. |
| `Sources/CrtSmoke/main.swift` | 13860 | content-reviewed | Reviewed headless single-image NTSC/downscale/CRT shader smoke renderer; macOS Metal validation utility, not shipping effect code. |
| `Sources/CrtSweep/main.swift` | 11373 | content-reviewed | Reviewed automated shader parameter sweep and min/max pixel-effect measurement, including animation-only detection; calibration tooling only. |
| `Sources/CrtVcrProbe/main.swift` | 7379 | content-reviewed | Reviewed sequential headless VCR/glitch renderer with contact sheets and luma/noise SNR measurements; useful visual calibration methodology, not effect kernel. |
| `Sources/CrtVideoSmoke/VideoSmoke.swift` | 3633 | content-reviewed | Reviewed async MP4 export smoke tool, codec/bitrate options and progress/error reporting; macOS exporter validation only. |
| `THIRD_PARTY_NOTICES.md` | 4097 | content-reviewed | Reviewed attribution and component licensing: ntsc-rs used under MIT from MIT/ISC/Apache options; librashader runtime MPL-2.0; shader collection has per-file licenses including GPL-2.0/3.0. Do not copy GPL shader sources into VHS-90; retain notices for any separately approved dependency. |
| `TestAssets/flat-gray.png` | 1968 | content-reviewed | Inspected test image: flat gray calibration field; useful for neutral response/scanline and output-difference tests. |
| `TestAssets/game-frame.png` | 3329 | content-reviewed | Inspected test image: pixel-art game frame with high-frequency edges/text; useful for chroma bleed, resolution and detail-retention comparisons. |
| `TestAssets/testpattern.png` | 6117 | content-reviewed | Inspected test image: synthetic color/line test pattern; useful for signal, chroma, line and geometry validation. |
| `Tests/CrtCoreTests/ChainInputCacheTests.swift` | 3464 | content-reviewed | Reviewed 4 cache stamp/generation/budget/range tests (22 assertions); app preview cache only, no VHS engine parity. |
| `Tests/CrtCoreTests/DigitalVideoTests.swift` | 31342 | content-reviewed | Reviewed 29 digital codec/camera/timing/keyframe/replay/performance tests (90 assertions); separate digital codec stage, not analog tape. |
| `Tests/CrtCoreTests/ExportGlitchTests.swift` | 7100 | content-reviewed | Reviewed 3 still/GIF/video export glitch-stage integration tests; verifies per-frame keyframes reach exported receiver stage. |
| `Tests/CrtCoreTests/ExportShaderToggleTests.swift` | 9748 | content-reviewed | Reviewed 5 still/video/GIF export toggle tests; catches shader bypass regressions, not VHS transport. |
| `Tests/CrtCoreTests/FieldLinesTests.swift` | 2043 | content-reviewed | Reviewed 3 field-line/supersample geometry tests; raster geometry only. |
| `Tests/CrtCoreTests/GlitchStageTests.swift` | 16672 | content-reviewed | Reviewed 13 GPU glitch stage tests (29 assertions): healthy identity, vertical roll/blanking, tearing, ghost/color delay, color killer, caption line, brightness below black, seek/replay determinism and retune reruns. |
| `Tests/CrtCoreTests/HowlaroundTests.swift` | 30180 | content-reviewed | Reviewed 26 Screen Loop tests (133 assertions): camera/tunnel geometry, framing, run-up, loop, hands, keyframes, deterministic render and cancellation. Separate effect, not VHS. |
| `Tests/CrtCoreTests/NtscChainTests.swift` | 5639 | content-reviewed | Reviewed playback/export chain-input equivalence test across frame paths and resolutions; NTSC pipeline integration only. |
| `Tests/CrtCoreTests/PreviewCompositorTests.swift` | 7115 | content-reviewed | Reviewed 3 offscreen preview compositor tests (13 assertions): odd letterbox rows, scanline sampling and fill-mode sizing. |
| `Tests/CrtCoreTests/PreviewGeometryTests.swift` | 7005 | content-reviewed | Reviewed 6 preview geometry tests (22 assertions): letterbox integer offsets, texel row mapping, zoom, pan and visible picture. |
| `Tests/CrtCoreTests/PreviewScalingTests.swift` | 8378 | content-reviewed | Reviewed 10 preview scaler tests (25 assertions): integer multiples, texture budget, scanline floor and small drawable fallback. |
| `Tests/CrtCoreTests/ReceiverSimulatorTests.swift` | 22470 | content-reviewed | Reviewed 25 receiver simulator tests: identity, vertical/horizontal hold, rolling, tearing, head-switch flagging/AFC, weak signal/noise, hum, two-head dirt, tape/VCR gating, deterministic replay, search/tracking, tension, sticky tape, chewed control track, wear and EP sync. |
| `Tests/CrtCoreTests/ShaderBypassTests.swift` | 4692 | content-reviewed | Reviewed 4 CRT shader bypass tests (5 assertions): exact nearest scaling, fractional enlargement brightness, downscale order and flat-field identity. |
| `Tests/CrtCoreTests/SourceCropTests.swift` | 16449 | content-reviewed | Reviewed 10 crop geometry/GPU/decode/playback/export tests (82 assertions); crop input pipeline only. |
| `Tests/CrtCoreTests/UpdateCheckTests.swift` | 2351 | content-reviewed | Reviewed 3 release-version/parser tests (15 assertions); update metadata only. |
| `Tests/CrtCoreTests/VcrTests.swift` | 18869 | content-reviewed | Reviewed 16 VCR tests: clean/worn tape response, edge-biased mold, consistent tape sampling, deck debris clog/shedding/cleaning, head vaseline transfer, legacy knob mapping, spacing-vs-color loss, EP noise/comb behavior, wear, burst-overlap behavior, copy degradation, alternating-head dirt and measured monotonic curves. |
| `Tests/CrtCoreTests/Video1Tests.swift` | 5519 | content-reviewed | Reviewed 6 Microsoft Video 1 block codec tests (18 assertions); digital codec only. |
| `Vendor/ntscrs-capi/Cargo.lock` | 5720 | content-reviewed | Reviewed pinned 27-package Rust dependency graph for ntsc-rs C ABI wrapper; reproducibility metadata only. |
| `Vendor/ntscrs-capi/Cargo.toml` | 396 | content-reviewed | Reviewed standalone Rust cdylib manifest and dependency on the ntsc-rs submodule; wrapper build metadata only. |
| `Vendor/ntscrs-capi/src/lib.rs` | 8906 | content-reviewed | Reviewed full C ABI around ntsc-rs: opaque settings instances, descriptor/JSON conversion, BGRA/RGBA frame processing, stride handling and panic containment. Upstream NTSC core is separate; ABI wrapper not ported. |
| `docs/header.webp` | 80040 | content-reviewed | Inspected app screenshot: split comparison of processed VHS/CRT output against clean pixel-art source; reference for end-to-end look only. |
| `docs/screenshot.webp` | 179882 | content-reviewed | Inspected UI screenshot: NTSCRT Mac interface and processed/clean comparison; app UI reference, not transferable AE effect source. |
| `docs/windows-port-plan.md` | 14827 | content-reviewed | Reviewed Windows architecture decisions, WARP validation strategy, pipeline split and settled platform scope; confirms Rust/wgpu desktop app is distinct from the native macOS app. |
| `licenses/Apache-2.0.txt` | 10140 | content-reviewed | Reviewed Apache-2.0 license terms; no engine source. |
| `licenses/GPL-2.0.txt` | 15220 | content-reviewed | Reviewed GPL-2.0 text; applies to identified shader assets, not NTSCRT core. Exclude GPL-covered code/assets unless project licensing explicitly permits. |
| `licenses/GPL-3.0.txt` | 34903 | content-reviewed | Reviewed GPL-3.0 text; applies to identified shader assets, not NTSCRT core. Exclude GPL-covered code/assets unless project licensing explicitly permits. |
| `licenses/MIT.txt` | 1151 | content-reviewed | Reviewed MIT attribution/redistribution terms for bundled crate licenses. |
| `licenses/MPL-2.0.txt` | 15383 | content-reviewed | Reviewed MPL-2.0 file-level copyleft terms for librashader runtime; do not silently merge into proprietary/native effect source. |
| `licenses/rust-crates.txt` | 527781 | content-reviewed | Reviewed generated third-party crate inventory and declared license expressions for macOS bundled Rust libraries; dependency attribution evidence, not algorithm code. |
| `mcpb/icon.png` | 249433 | content-reviewed | Inspected bundle icon asset; MCP package branding only. |
| `mcpb/manifest.json` | 4042 | content-reviewed | Reviewed MCP bundle metadata and server entry point; packaging manifest only. |
| `mcpb/server/launch.cmd` | 1823 | content-reviewed | Reviewed Windows MCP launcher and bundled executable resolution; app deployment wrapper only. |
| `mcpb/server/launch.sh` | 1093 | content-reviewed | Reviewed macOS/Linux MCP launcher and bundled executable resolution; app deployment wrapper only. |
| `port/Cargo.lock` | 137943 | content-reviewed | Reviewed pinned 538-package Windows Rust dependency graph; build reproducibility metadata only. |
| `port/Cargo.toml` | 1040 | content-reviewed | Reviewed Rust workspace members and pinned ntsc-rs/librashader dependencies; Windows-port workspace metadata only. |
| `port/README.md` | 5184 | content-reviewed | Reviewed Windows port architecture, build/tests, reference parity and VM instructions; no unique effect algorithm. |
| `port/app/Cargo.toml` | 1357 | content-reviewed | Reviewed eframe/egui app, media and CLI dependencies; platform app manifest only. |
| `port/app/build.rs` | 578 | content-reviewed | Reviewed Windows resource/version embedding; build metadata only. |
| `port/app/icon-256.png` | 92660 | content-reviewed | Inspected Windows app icon asset; desktop packaging only. |
| `port/app/ntscrt.ico` | 146065 | content-reviewed | Inspected Windows executable icon; packaging only. |
| `port/app/src/bin/ntscrt-cli.rs` | 2385 | content-reviewed | Reviewed CLI binary entry point and exit-code routing; no render algorithms. |
| `port/app/src/claude.rs` | 16079 | content-reviewed | Reviewed Claude Desktop/Code MCP config detection, connect/disconnect and status; external app integration only. |
| `port/app/src/cli/describe.rs` | 23230 | content-reviewed | Reviewed JSON stage/control schema, order, keyframe docs and settings examples; API documentation wrapper. |
| `port/app/src/cli/help.rs` | 2600 | content-reviewed | Reviewed CLI usage text and option help; no engine code. |
| `port/app/src/cli/mcp.rs` | 35074 | content-reviewed | Reviewed JSON-RPC/MCP initialization, tool listing/call dispatch, job cancellation and response handling; app service only. |
| `port/app/src/cli/mcp_text.rs` | 10784 | content-reviewed | Reviewed MCP text/structured-content serialization and output formatting; protocol wrapper only. |
| `port/app/src/cli/mod.rs` | 7833 | content-reviewed | Reviewed CLI command routing, argument expansion, JSON helpers and output channels; app shell only. |
| `port/app/src/cli/render.rs` | 25695 | content-reviewed | Reviewed render request parsing, stage flags, export route/format/size and dry-run planning; app command layer only. |
| `port/app/src/cli/settings.rs` | 35356 | content-reviewed | Reviewed settings application order, legacy look migration, crop/downscale/digital/NTSC/VCR/CRT toggles, timeline and keyframe updates. Migration semantics are useful reference; no native AE render kernel. |
| `port/app/src/clip_frames.rs` | 1653 | content-reviewed | Reviewed independent cropped frame visitor for Digital stage reruns; media decode adapter only. |
| `port/app/src/connect_ui.rs` | 6442 | content-reviewed | Reviewed egui Claude connection window and self-test snapshot; UI only. |
| `port/app/src/digital_panel.rs` | 4154 | content-reviewed | Reviewed digital codec panel controls and dependent-control hints; optional digital codec UI, not analog VHS. |
| `port/app/src/export.rs` | 14116 | content-reviewed | Reviewed threaded PNG/MP4/GIF export, frame seed policy and progress/cancel reporting; host export wrapper only. |
| `port/app/src/frame_cache.rs` | 3741 | content-reviewed | Reviewed stamped per-frame baked-input cache, stale-entry eviction and byte budget; app playback optimization, not signal processing. |
| `port/app/src/glitch_panel.rs` | 3389 | content-reviewed | Reviewed receiver/glitch control generation from GlitchParam metadata; UI only. |
| `port/app/src/main.rs` | 87067 | content-reviewed | Reviewed Windows egui application bootstrap, stage order, source/preset state, preview, export and starting VCR sample values. App shell; the VCR sample values are reference preset data, not an engine. |
| `port/app/src/ntsc_panel.rs` | 9648 | content-reviewed | Reviewed dynamic ntsc-rs control panel and no-write-on-draw tests; UI wrapper only. |
| `port/app/src/prefetch.rs` | 5049 | content-reviewed | Reviewed ordered video frame prefetch worker, seek policy and ring buffer; media decode scheduling only. |
| `port/app/src/producer.rs` | 11969 | content-reviewed | Reviewed ordered playback producer, digital codec catch-up, frame seeding and loop wrap; app playback concurrency, not analog tape. |
| `port/app/src/screen_loop.rs` | 36147 | content-reviewed | Reviewed separate Screen Loop camera-to-display feedback simulation, camera pose/projection, feedback history and temporal rendering. Distinct effect from VHS tape; no migration into the analog engine. |
| `port/app/src/shader_panel.rs` | 13419 | content-reviewed | Reviewed dynamic CRT shader parameter UI, presentation metadata and gating; UI only. |
| `port/app/src/timeline_ui.rs` | 12109 | content-reviewed | Reviewed keyframe timeline editing and easing UI; After Effects host keyframes replace it. |
| `port/app/src/update.rs` | 15767 | content-reviewed | Reviewed GitHub release lookup/version comparison and update checks; app update service only. |
| `port/app/src/vcr_panel.rs` | 2315 | content-reviewed | Reviewed eight tape and five deck condition pickers, curve-aware preset values and reset semantics. Preset data is useful reference; current VHS-90 AE UI does not yet expose these pickers/independent knobs. |
| `port/app/src/worker.rs` | 18843 | content-reviewed | Reviewed WGPU render worker, request coalescing, background exact reruns, preview quick pass and frame-cache/producer coordination. Concurrency wrapper only; AE render scheduling replaces it. |
| `port/codec/.gitignore` | 11 | content-reviewed | Reviewed; build-ignore metadata only, no implementation. |
| `port/codec/Cargo.toml` | 351 | content-reviewed | Reviewed Rust codec crate manifest and dependencies (serde/rayon); no native VHS behavior. |
| `port/codec/examples/parity.rs` | 11546 | content-reviewed | Reviewed parity harness comparing Rust codec output to Swift reference fixtures; test tooling only. |
| `port/codec/ref/build.sh` | 873 | content-reviewed | Reviewed script that builds the Swift reference codec for parity tests; not a shipping effect. |
| `port/codec/ref/cases.json` | 7400 | content-reviewed | Reviewed digital codec parity case manifest; fixtures only. |
| `port/codec/ref/main.swift` | 9034 | content-reviewed | Reviewed Swift reference implementation for the separate digital codec; not analog tape and not directly portable to AE. |
| `port/codec/src/codec.rs` | 34249 | content-reviewed | Reviewed 8×8 DCT/IDCT, intra/inter prediction, motion search, quantization, residual reconstruction, dropped-keyframe propagation and deblocking. Separate digital block codec; do not merge into analog tape engine. |
| `port/codec/src/lib.rs` | 2871 | content-reviewed | Reviewed module exports and public digital codec API; no additional analog algorithms. |
| `port/codec/src/params.rs` | 23724 | content-reviewed | Reviewed digital codec settings, codec selection and quality/bitrate/loss parameter mapping; independent digital stage. |
| `port/codec/src/renderer.rs` | 41691 | content-reviewed | Reviewed digital camera capture (CCD/exposure/noise), frame/keyframe progression, history/checkpoints, codec execution, deblocking and YCbCr reconstruction. Optional digital capture subsystem, not VHS tape. |
| `port/codec/src/resample.rs` | 8792 | content-reviewed | Reviewed portable Lanczos/area resampling implementation; generic resize utility, not tape signal processing. |
| `port/codec/src/tables.rs` | 4606 | content-reviewed | Reviewed DCT basis and codec quantization tables; digital codec-only constants. |
| `port/codec/src/tests/digital_video.rs` | 27316 | content-reviewed | Reviewed digital codec integration tests for keyframe loss, data loss, codec settings and temporal reconstruction; test-only. |
| `port/codec/src/tests/mod.rs` | 228 | content-reviewed | Reviewed test module declarations only. |
| `port/codec/src/tests/port.rs` | 3527 | content-reviewed | Reviewed Rust/Swift parity and codec port regression tests; test-only. |
| `port/codec/src/tests/video1.rs` | 5478 | content-reviewed | Reviewed Video 1 block-mode and palette quantization tests; test-only. |
| `port/codec/src/video1.rs` | 15002 | content-reviewed | Reviewed Microsoft Video 1 4×4 block codec, skip/fill/two/eight-color modes and loss propagation. Separate digital codec; no analog tape port. |
| `port/engine/Cargo.toml` | 734 | content-reviewed | Reviewed WGPU/Rust engine manifest and feature dependencies; host renderer build metadata, not C++ AE source. |
| `port/engine/src/crop.rs` | 8433 | content-reviewed | Reviewed crop/aspect/position rectangle math and image copy; generic source geometry utility. |
| `port/engine/src/downscale.wgsl` | 4980 | content-reviewed | Reviewed nearest, bilinear, bicubic, Lanczos and area GPU downscale kernels; generic WGPU utility, no VHS tape behavior. |
| `port/engine/src/export.rs` | 5501 | content-reviewed | Reviewed output dimension, supersample and bitrate policy; application export sizing only. |
| `port/engine/src/glitch/deck.rs` | 6898 | content-reviewed | Full core source reviewed and cross-checked against Swift reference; selective C++ adaptation documented in report 45 |
| `port/engine/src/glitch/glitch.wgsl` | 11347 | content-reviewed | Receiver GPU shader reviewed: signal line locator, sync/VBI generation, gate/burst accumulation, chroma PLL/ACC/killer and YIQ compose/snow/ghost/brightness. Requires CPU/MFR-compatible port; not copied as WGSL into native AE. |
| `port/engine/src/glitch/mod.rs` | 634 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/params.rs` | 25256 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/receiver.rs` | 43912 | content-reviewed | Receiver circuit simulator reviewed: warmup, horizontal AFC/VCO PI, vertical integrator/oscillator, burst-key phase, signal separator/noise, chroma reference/ACC/killer, VCR/deck/transport field plans. Full C++ parity engine remains pending. |
| `port/engine/src/glitch/stage.rs` | 25145 | content-reviewed | GPU stage orchestration reviewed: checkpoint-aware FieldPlan resimulation, 1-second snapshots, background reruns, VCR-before-TV order, uniforms and WGPU resource setup. Algorithmic state semantics are portable; WGPU encoding is not. |
| `port/engine/src/glitch/tape.rs` | 24899 | content-reviewed | Full core source reviewed and cross-checked against Swift reference; selective C++ adaptation documented in report 45 |
| `port/engine/src/glitch/transport.rs` | 14511 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/vcr.rs` | 19408 | content-reviewed | VCR RF/FM/color-under kernels and coefficient generation reviewed; reusable CPU coefficient layer ported, complete record/play pipeline remains open |
| `port/engine/src/glitch/vcr.wgsl` | 29726 | content-reviewed | VCR RF/FM/color-under kernels and coefficient generation reviewed; reusable CPU coefficient layer ported, complete record/play pipeline remains open |
| `port/engine/src/howl/feedback.rs` | 23060 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/howl.wgsl` | 4593 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/mod.rs` | 29409 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/pad.rs` | 8614 | content-reviewed | Read during this audit pass |
| `port/engine/src/lib.rs` | 47008 | content-reviewed | Reviewed WGPU engine facade: image import, optional CPU digital stage, crop, NTSC C API, downscale, shader-chain and readback. This is the desktop still-image pipeline, not a native AE effect engine. |
| `port/engine/src/look.rs` | 20571 | content-reviewed | Reviewed JSON look import/export, version upgrades, legacy tape→VCR migration and Screen Loop distinction. Useful preset-schema reference; no direct port yet. |
| `port/engine/src/ntsc_ui.rs` | 8599 | content-reviewed | Reviewed NTSC parameter descriptor, labels/defaults and UI metadata from ntsc-rs; existing VHS-90 AnalogNTSC wrapper is separate. |
| `port/engine/src/preview.rs` | 7456 | content-reviewed | Reviewed WGPU preview surface/pipeline and texture upload/readback; preview-only host infrastructure. |
| `port/engine/src/timeline.rs` | 9204 | content-reviewed | Reviewed keyframe sorting, segment lookup, easing and typed parameter interpolation. AE host animation replaces this app timeline in the plugin. |
| `port/engine/tests/digital.rs` | 7713 | content-reviewed | Reviewed 4 Rust digital-stage integration tests (13 assertions); capture geometry, stage ordering and parameter filtering. |
| `port/engine/tests/fixtures/glitch/dub.args` | 69 | content-reviewed | Reviewed deterministic fixture inputs: VCR enabled, third-generation tape, SP speed, moderate wear and head-switch event; input companion for 240-line parity JSON. |
| `port/engine/tests/fixtures/glitch/dub.json` | 128086 | content-reviewed | Reviewed 240-row receiver/VCR field-plan golden output with chroma, head/track state and line timing; exact Swift→Rust receiver parity fixture. |
| `port/engine/tests/fixtures/glitch/healthy.args` | 5 | content-reviewed | Reviewed healthy identity receiver fixture invocation at t=1s. |
| `port/engine/tests/fixtures/glitch/healthy.json` | 17601 | content-reviewed | Reviewed 240-row neutral receiver output: zero offsets/noise, nominal line timing and no VCR lines; identity golden fixture. |
| `port/engine/tests/fixtures/glitch/hhold.args` | 24 | content-reviewed | Reviewed horizontal hold stress input (0.7) at t=1s. |
| `port/engine/tests/fixtures/glitch/hhold.json` | 25908 | content-reviewed | Reviewed 240-row horizontal hold field-plan golden output, progressive line phase/offset and chroma metadata. |
| `port/engine/tests/fixtures/glitch/search.args` | 55 | content-reviewed | Reviewed search-mode fixture input with high search speed, SP tape and crease damage. |
| `port/engine/tests/fixtures/glitch/search.json` | 49912 | content-reviewed | Reviewed 240-row search fixture with VCR line records and receiver timing/chroma state. |
| `port/engine/tests/fixtures/glitch/slack.args` | 74 | content-reviewed | Reviewed slack transport fixture: back tension -0.8, wear 0.7 and scratches 0.6. |
| `port/engine/tests/fixtures/glitch/slack.json` | 58700 | content-reviewed | Reviewed 240-row slack-tape golden field plan, head/track line records, chroma and timing. |
| `port/engine/tests/fixtures/glitch/tape.args` | 97 | content-reviewed | Reviewed head-switch/AFC/tape-crease/transport-flutter/tracking-offset combined fixture inputs. |
| `port/engine/tests/fixtures/glitch/tape.json` | 49920 | content-reviewed | Reviewed 240-row tape/head-switch golden output with VCR track lines and receiver row offsets. |
| `port/engine/tests/fixtures/glitch/transport.args` | 148 | content-reviewed | Reviewed transport fault fixture with EP speed, sticky tape, mold, back tension, tracking offset, edge damage and head-switch. |
| `port/engine/tests/fixtures/glitch/transport.json` | 54287 | content-reviewed | Reviewed 240-row transport golden output, VCR line state and chroma/receiver metadata. |
| `port/engine/tests/fixtures/glitch/vcr.args` | 131 | content-reviewed | Reviewed VCR fixture inputs covering dropouts, mold, vaseline, head dirt/buildup, wear/mismatch and hum. |
| `port/engine/tests/fixtures/glitch/vcr.json` | 71258 | content-reviewed | Reviewed 240-row VCR golden output with per-line head/gain/grease and receiver/chroma state. |
| `port/engine/tests/fixtures/glitch/vhold.args` | 22 | content-reviewed | Reviewed vertical hold stress input (0.6) at t=3.3s. |
| `port/engine/tests/fixtures/glitch/vhold.json` | 17571 | content-reviewed | Reviewed 240-row vertical roll/hold receiver golden output; no VCR stage. |
| `port/engine/tests/fixtures/glitch/weak.args` | 25 | content-reviewed | Reviewed weak-signal fixture input (0.12) at t=3.3s. |
| `port/engine/tests/fixtures/glitch/weak.json` | 29833 | content-reviewed | Reviewed 240-row weak-signal golden output with picture gain reduction, noise and chroma state. |
| `port/engine/tests/fixtures/mac-launch-look.json` | 5128 | content-reviewed | Reviewed Windows launch-look snapshot generated by Mac app; includes default NTSC/VCR/CRT and stage state for cross-platform startup parity. |
| `port/engine/tests/glitch_parity.rs` | 5194 | content-reviewed | Reviewed Rust receiver parity fixture test: compares every row, color reference, VCR lines/head state and generations against Swift reference JSON fixtures. |
| `port/engine/tests/howl.rs` | 3637 | content-reviewed | Reviewed 2 GPU Screen Loop integration/determinism tests; unrelated to VHS tape engine. |
| `port/engine/tests/mac_parity.rs` | 3393 | content-reviewed | Reviewed 3 Windows launch-look parity tests against Mac-generated JSON defaults for NTSC and preset settings. |
| `port/engine/tests/preview_paths.rs` | 5457 | content-reviewed | Reviewed 3 playback path parity tests: baked preview, worker-produced CPU stages and unseen digital frames; host pipeline only. |
| `port/fetch-librashader.sh` | 1053 | content-reviewed | Reviewed pinned librashader checkout and compatibility patch application; third-party dependency acquisition only. |
| `port/librashader-wgpu-push-constants.patch` | 1610 | content-reviewed | Reviewed narrowly scoped upstream wgpu push-constant pipeline-layout fix; third-party renderer compatibility patch, not VHS behavior. |
| `port/licenses/rust-crates-windows.txt` | 665879 | content-reviewed | Reviewed generated Windows crate/license inventory; retain per-crate attribution and verify license obligations before redistributing bundled binaries. |
| `port/media/Cargo.toml` | 580 | content-reviewed | Reviewed Windows Media Foundation media crate manifest and target-specific dependencies; platform adapter metadata. |
| `port/media/src/bin/probe.rs` | 2674 | content-reviewed | Reviewed Media Foundation decode/encode diagnostic CLI and timing output; platform media test tool. |
| `port/media/src/lib.rs` | 8312 | content-reviewed | Reviewed frame/time indexing, orientation rotation, image reading and video writer abstraction; generic Windows media I/O, not effect processing. |
| `port/media/src/mf.rs` | 10917 | content-reviewed | Reviewed Media Foundation source-reader startup, format negotiation, timestamp seeking, RGB32 decode, padded row pitch and rotation handling; Windows-only media adapter. |
| `port/media/src/mf_writer.rs` | 9166 | content-reviewed | Reviewed Media Foundation sink writer for H.264/HEVC, timestamped RGB32 frames and AAC audio interleaving; Windows-only exporter. |
| `port/package-readme.txt` | 1291 | content-reviewed | Reviewed Windows preview install/use/export instructions and bundled resource requirements; release documentation only. |
| `port/scripts/cli-check.py` | 13257 | content-reviewed | Reviewed end-to-end CLI/MCP integration harness covering help, describe, looks, still/video/GIF rendering, errors, save-look and JSON-RPC job lifecycle; integration validation only. |
| `port/scripts/gen-licenses.py` | 4005 | content-reviewed | Reviewed Windows crate-license report generator; packaging/compliance utility only. |
| `port/scripts/glitch-ref-render/main.swift` | 2499 | content-reviewed | Reviewed macOS Metal reference-image renderer for the Glitch stage; parity fixture generation utility. |
| `port/scripts/glitch-ref/main.swift` | 2204 | content-reviewed | Reviewed macOS receiver field-plan JSON reference generator; test fixture utility. |
| `port/scripts/make-glitch-refs.sh` | 2924 | content-reviewed | Reviewed script compiling Mac receiver/VCR sources and producing field-plan/render fixtures across healthy, search, tracking, tape and transport cases; parity generation only. |
| `port/scripts/make-look-refs.py` | 3946 | content-reviewed | Reviewed bundled-look reference renderer orchestration and platform compatibility filtering; preset parity tooling only. |
| `port/scripts/sync-mcp-text.py` | 2280 | content-reviewed | Reviewed deterministic synchronization of MCP server guide text from Swift to Rust Windows app; documentation generation only. |
| `port/spike/Cargo.toml` | 298 | content-reviewed | Reviewed isolated Windows renderer spike crate dependencies; temporary parity tooling, not shipping app. |
| `port/spike/examples/preview-bench.rs` | 3552 | content-reviewed | Reviewed CPU-stage/upload/GPU/readback timing benchmark across preview frames and resolutions; performance diagnostics only. |
| `port/spike/ref-mac/crt-aperture-ntsc.png` | 1821846 | content-reviewed | Inspected CRT shader parity render with NTSC stage; reference output for CRT/shader stage, not tape engine. |
| `port/spike/ref-mac/crt-aperture-plain.png` | 235316 | content-reviewed | Inspected CRT shader reference without NTSC stage; isolates shader behavior. |
| `port/spike/ref-mac/crt-easymode-ntsc.png` | 1713966 | content-reviewed | Inspected CRT shader parity render with NTSC stage; reference only. |
| `port/spike/ref-mac/crt-easymode-plain.png` | 183313 | content-reviewed | Inspected CRT shader reference without NTSC stage; isolates shader behavior. |
| `port/spike/ref-mac/crt-hyllian-ntsc.png` | 1955324 | content-reviewed | Inspected CRT shader parity render with NTSC stage; shader's own license governs any reuse. |
| `port/spike/ref-mac/crt-hyllian-plain.png` | 496217 | content-reviewed | Inspected CRT shader reference without NTSC stage; shader's own license governs any reuse. |
| `port/spike/ref-mac/crt-royale-ntsc.png` | 2500166 | content-reviewed | Inspected CRT shader parity render with NTSC stage; GPL-2.0-or-later shader asset, exclude from reuse absent compatible licensing. |
| `port/spike/ref-mac/crt-royale-plain.png` | 1757231 | content-reviewed | Inspected CRT shader reference without NTSC stage; GPL-2.0-or-later shader asset, exclude from reuse absent compatible licensing. |
| `port/spike/ref-mac/crtglow_gauss-ntsc.png` | 1168724 | content-reviewed | Inspected CRT glow shader parity render with NTSC stage; reference only. |
| `port/spike/ref-mac/crtglow_gauss-plain.png` | 198064 | content-reviewed | Inspected CRT glow shader reference without NTSC stage; reference only. |
| `port/spike/ref-mac/crtglow_lanczos-ntsc.png` | 1172160 | content-reviewed | Inspected CRT glow shader parity render with NTSC stage; reference only. |
| `port/spike/ref-mac/crtglow_lanczos-plain.png` | 180824 | content-reviewed | Inspected CRT glow shader reference without NTSC stage; reference only. |
| `port/spike/ref-mac/crtsim-ntsc.png` | 1116783 | content-reviewed | Inspected CRT simulation shader parity render with NTSC stage; reference only. |
| `port/spike/ref-mac/crtsim-plain.png` | 703898 | content-reviewed | Inspected CRT simulation shader reference without NTSC stage; reference only. |
| `port/spike/ref-mac/glitch/ghost.args` | 55 | content-reviewed | Reviewed ghost/chroma-delay reference scenario with weak signal; test input only. |
| `port/spike/ref-mac/glitch/ghost.png` | 201184 | content-reviewed | Inspected receiver ghost/noise output; high chroma ghosting and noisy lower raster demonstrate receiver-stage behavior. |
| `port/spike/ref-mac/glitch/in.png` | 1202 | content-reviewed | Inspected source input test frame; synthetic baseline image only. |
| `port/spike/ref-mac/glitch/roll.args` | 37 | content-reviewed | Reviewed vertical-roll/brightness reference scenario; test input only. |
| `port/spike/ref-mac/glitch/roll.png` | 2903 | content-reviewed | Inspected vertical hold/roll output reference; receiver timing/blanking behavior. |
| `port/spike/ref-mac/glitch/tape.args` | 97 | content-reviewed | Reviewed severe head-switch/crease/search/dropout-compensation scenario; test input only. |
| `port/spike/ref-mac/glitch/tape.png` | 184028 | content-reviewed | Inspected tape/head-switch reference output: color bars with horizontal sync/track tearing and noisy lower bands; useful visual reference for tracking/head-switch stage. |
| `port/spike/ref-mac/glitch/vcr-clean.args` | 10 | content-reviewed | Reviewed clean-deck/clean-tape reference baseline input. |
| `port/spike/ref-mac/glitch/vcr-clean.png` | 162789 | content-reviewed | Inspected clean VCR reference output; baseline tape/deck signal quality. |
| `port/spike/ref-mac/glitch/vcr-dub.args` | 75 | content-reviewed | Reviewed second-generation SP dub scenario with wear/dropouts. |
| `port/spike/ref-mac/glitch/vcr-dub.png` | 190144 | content-reviewed | Inspected second-generation dub output; tape generation and wear reference. |
| `port/spike/ref-mac/glitch/vcr-ep.args` | 109 | content-reviewed | Reviewed EP worn/scratched/sticky tape with tension and tracking-offset scenario. |
| `port/spike/ref-mac/glitch/vcr-ep.png` | 183305 | content-reviewed | Inspected EP tape reference output; low-speed noise and tape/transport degradation. |
| `port/spike/ref-mac/glitch/vcr-search.args` | 47 | content-reviewed | Reviewed search-speed/transport-flutter reference scenario. |
| `port/spike/ref-mac/glitch/vcr-search.png` | 186445 | content-reviewed | Inspected search transport reference output; fast transport/tracking behavior. |
| `port/spike/ref-mac/glitch/vcr-used.args` | 62 | content-reviewed | Reviewed moderately used tape wear/dropout/scratch scenario. |
| `port/spike/ref-mac/glitch/vcr-used.png` | 186843 | content-reviewed | Inspected used-tape reference output; moderate wear/dropouts/scratches. |
| `port/spike/ref-mac/glitch/vcr-wrecked.args` | 149 | content-reviewed | Reviewed extreme EP tape damage, head wear, edge damage and transport flutter scenario. |
| `port/spike/ref-mac/glitch/vcr-wrecked.png` | 150399 | content-reviewed | Inspected wrecked-tape output; extreme luma noise, dropouts and unstable tracking. |
| `port/spike/ref-mac/looks/Clean CRT.png` | 378823 | content-reviewed | Inspected full-look output reference; clean CRT baseline, no unique engine code. |
| `port/spike/ref-mac/looks/Gentle waves loop.png` | 1645706 | content-reviewed | Inspected full-look output reference; temporal wave/loop preset result, compare-only. |
| `port/spike/ref-mac/looks/High Chroma static.png` | 2082781 | content-reviewed | Inspected full-look output reference; high-chroma static preset result, compare-only. |
| `port/spike/ref-mac/looks/Low Res.png` | 1732391 | content-reviewed | Inspected full-look output reference; low-resolution sampling preset result, compare-only. |
| `port/spike/ref-mac/looks/Medium VHS.png` | 2079926 | content-reviewed | Inspected full-look output reference metadata; medium VHS preset image, compare-only. |
| `port/spike/ref-mac/looks/Mild VHS.png` | 543787 | content-reviewed | Inspected full-look output reference; mild VHS preset result, compare-only. |
| `port/spike/ref-mac/looks/Obliterated.png` | 1632189 | content-reviewed | Inspected full-look output reference; extreme degradation preset result, compare-only. |
| `port/spike/ref-mac/looks/Readable Text.png` | 1745703 | content-reviewed | Inspected full-look output reference; legibility-oriented preset result, compare-only. |
| `port/spike/ref-mac/looks/Wavy loop.png` | 1447185 | content-reviewed | Inspected full-look output reference; animated wave preset result, compare-only. |
| `port/spike/src/bin/compare.rs` | 2165 | content-reviewed | Reviewed image comparison utility computing mean/max error, >2-pixel fraction, PSNR and optional amplified diff; parity diagnostics. |
| `port/spike/src/bin/glitch-dump.rs` | 1811 | content-reviewed | Reviewed Rust receiver plan JSON dumper matching Mac field-plan schema, including row timing, VCR lines, chroma, picture gain and field metadata. |
| `port/spike/src/bin/glitch-render.rs` | 1138 | content-reviewed | Reviewed Rust isolated Glitch stage still renderer for pixel-level comparison against Mac Metal reference renderer. |
| `port/spike/src/main.rs` | 4775 | content-reviewed | Reviewed complete still-image parity spike mirroring Mac crt-smoke: preset loading, NTSC/downscale/CRT settings, self-test and override controls; not shipping AE plugin. |
| `presets/2 second dirty VCR loop.json` | 84498 | semantic-reviewed | Semantic review: animated 2 s SP VCR look with tape/deck damage and tracking; NTSC whole-state snapshots also require AE keyframe conversion. |
| `presets/2 second glitch.json` | 40234 | semantic-reviewed | Semantic review: receiver hold/ghost/signal animation plus NTSC and CRT; partial receiver mapping and full timeline conversion required. |
| `presets/2 second glitches.json` | 108663 | semantic-reviewed | Semantic review: 26-key rapid receiver failure sequence; preserve per-event timing/easing and NTSC/CRT states. |
| `presets/2 second mild glitch.json` | 63227 | semantic-reviewed | Semantic review: mild receiver/NTSC 2 s animation; keyframe snapshots, not static glitch values. |
| `presets/5 second blips.json` | 68286 | semantic-reviewed | Semantic review: 5 s receiver blip sequence with NTSC changes; shader disabled, preserve 16 timeline snapshots. |
| `presets/90s VHS bootleg.json` | 5364 | semantic-reviewed | Semantic review: static generation/tension/head/tape-damage look; high-value physical tape/deck preset candidate, CRT disabled. |
| `presets/CD-ROM (low quality).json` | 4782 | semantic-reviewed | Semantic review: digital-only Video 1/block-codec look; exclude from analog VHS engine. |
| `presets/Cheap camcorder.json` | 4754 | semantic-reviewed | Semantic review: digital-only camcorder response; exclude from analog VHS engine. |
| `presets/Chunky webcam.json` | 4698 | semantic-reviewed | Semantic review: digital-only webcam compression/resolution look; exclude from analog VHS engine. |
| `presets/Clean CRT.json` | 3574 | semantic-reviewed | Semantic review: CRT-only shader preset; no VHS/NTSC/receiver stage. |
| `presets/Clean VHS.json` | 3554 | semantic-reviewed | Semantic review: NTSC-only clean tape signal baseline; no physical VCR damage stage. |
| `presets/Damaged VHS - strong.json` | 5144 | semantic-reviewed | Semantic review: generation-3 VCR with tension, head switch, mold, scratches, sticky and edge damage; high-value physical candidate plus NTSC/CRT layers. |
| `presets/Dirty VCR + CRT (vertical).json` | 5400 | semantic-reviewed | Semantic review: portrait generation-3 tape with heavy creases/dropouts/scratches/head wear; physical candidate plus separate NTSC/CRT and host sizing. |
| `presets/Dirty VCR.json` | 5221 | semantic-reviewed | Semantic review: wear/mold/scratches/vaseline/creases/head dirt/mismatch/dropouts; static physical tape/deck candidate. |
| `presets/Gentle waves loop.json` | 3555 | semantic-reviewed | Semantic review: NTSC edge-wave/chroma loop with CRT; no VCR receiver stage. |
| `presets/Glitch 1.json` | 34917 | semantic-reviewed | Semantic review: despite filename, receiver glitch stage disabled; NTSC edge/chroma/head-switch display animation with CRT. |
| `presets/Glitch 2.json` | 38121 | semantic-reviewed | Semantic review: NTSC static/snow/tracking-noise look at 128 px, CRT active; not physical VCR track read. |
| `presets/Glitch 3.json` | 38354 | semantic-reviewed | Semantic review: NTSC head-switch display and tracking-noise animation; coordinate mapping differs from physical tape head switching. |
| `presets/Glitch loop 1.json` | 27441 | semantic-reviewed | Semantic review: NTSC softness/edge and CRT glow pulse; 7 whole-state keys. |
| `presets/Glitch pop.json` | 41200 | semantic-reviewed | Semantic review: NTSC blur/chroma-loss/edge-wave pulses; 11 keys, return-to-normal states matter. |
| `presets/High Chroma static.json` | 3618 | semantic-reviewed | Semantic review: static NTSC colored-noise preset with CRT; signal noise, not tape surface damage. |
| `presets/Low Res.json` | 3549 | semantic-reviewed | Semantic review: 128 px nearest downscale plus NTSC chroma bleed and CRT; host sizing plus signal-chain look. |
| `presets/Medium VHS.json` | 3614 | semantic-reviewed | Semantic review: moderate NTSC bandwidth/noise/chroma bleed and CRT; signal baseline, not physical tape damage. |
| `presets/Mild VHS.json` | 3510 | semantic-reviewed | Semantic review: light NTSC softness/noise with CRT; low-intensity signal baseline. |
| `presets/Noisy TV broadcast on VHS.json` | 5128 | semantic-reviewed | Semantic review: VCR generation-3 EP/tension/head-switch/tape damage and NTSC active; digital map is disabled and must be ignored. |
| `presets/Obliterated.json` | 3548 | semantic-reviewed | Semantic review: extreme static NTSC smear/chroma/noise and CRT; timeline enabled but has zero keys. |
| `presets/Readable Text.json` | 3582 | semantic-reviewed | Semantic review: NTSC light VHS plus area downscale and Lanczos CRT; legibility-focused signal preset. |
| `presets/Screen Loop/12 copies luma key.json` | 643 | semantic-reviewed | Semantic review: ntscrt-video-feedback schema, key inversion/zoom/turn/drift; separate Screen Loop effect, excluded. |
| `presets/Screen Loop/15 copies color drift.json` | 673 | semantic-reviewed | Semantic review: 2 s camera-feedback composition with color/hue drift; separate effect, excluded. |
| `presets/Screen Loop/22 copies bottom left.json` | 633 | semantic-reviewed | Semantic review: 22-copy bottom-left camera-feedback composition; separate effect, excluded. |
| `presets/Screen Loop/7 copies slow luma.json` | 659 | semantic-reviewed | Semantic review: 2 s seven-copy slow-luma feedback composition; separate effect, excluded. |
| `presets/Screen Loop/Bottom to top.json` | 632 | semantic-reviewed | Semantic review: bottom-to-top camera-feedback drift; separate effect, excluded. |
| `presets/Static loop 1.json` | 30966 | semantic-reviewed | Semantic review: 2 s/12 fps NTSC noise/smear/fading-color animation with 8 keys; no VCR stage. |
| `presets/Tracking Noise up.json` | 37932 | semantic-reviewed | Semantic review: 2 s/24 fps bottom-up tracking-noise region animation with 10 keys; translate region trajectory into VHS-90 arbitrary-position controls. |
| `presets/Two blips loop.json` | 31401 | semantic-reviewed | Semantic review: two short NTSC edge-wave/blur pulses over 2 s; no VCR damage stage. |
| `presets/V Hold glitch.json` | 36381 | semantic-reviewed | Semantic review: mixed receiver hold/crinkle/dropout/head-clog/search plus NTSC/CRT; preserve 8-key animation. |
| `presets/VHS with color loss.json` | 5390 | semantic-reviewed | Semantic review: generation-3 EP VCR with tension/wear/scratches/creases/dropouts/head wear/tracking/flutter plus NTSC color loss; digital disabled. |
| `presets/Vertical Freak Out.json` | 28180 | semantic-reviewed | Semantic review: 2 s/24 fps receiver vertical roll/horizontal slip/head-clog animation; 6 keys plus NTSC/CRT. |
| `presets/Vertical spike loop.json` | 24283 | semantic-reviewed | Semantic review: NTSC vertical chroma smear/spikes and blur bursts; not physical track geometry. |
| `presets/Very wavy.json` | 20635 | semantic-reviewed | Semantic review: 2 s/24 fps strong NTSC edge-wave animation; no VCR damage stage. |
| `presets/Wavy loop.json` | 3569 | semantic-reviewed | Semantic review: static NTSC edge-wave/tracking-noise look; timeline enabled but zero keys. |
| `presets/Webcam (low quality).json` | 4821 | semantic-reviewed | Semantic review: digital-only webcam camera/compression/noise/aliasing; exclude from analog VHS engine. |
| `presets/glitchy block video.json` | 5268 | semantic-reviewed | Semantic review: digital-only Microsoft Video 1 block codec with long keyframe interval/data loss; exclude from analog VHS engine. |
| `presets/glitchy streaming video with noise.json` | 5295 | semantic-reviewed | Semantic review: digital-only fixed-bitrate stream with long keyframe interval/data loss/noise; exclude from analog VHS engine. |
| `presets/glitchy streaming video.json` | 5185 | semantic-reviewed | Semantic review: digital-only low-quality streaming compression/sharpening/moiré/data loss; exclude from analog VHS engine. |
| `scripts/build-ntscrs.sh` | 1375 | content-reviewed | Reviewed universal macOS Rust C-ABI build and placement of ntscrs dylib; platform build script only. |
| `scripts/check-control-strength.py` | 13381 | content-reviewed | Reviewed full automated min/mid/max visible-strength measurement across NTSC, Glitch, VCR and Digital stages against JSON baselines; valuable calibration method, not render code. |
| `scripts/check-export-toggles.sh` | 2353 | content-reviewed | Reviewed export shader-toggle regression runner; test orchestration only. |
| `scripts/check-zoom-scanlines.sh` | 3540 | content-reviewed | Reviewed preview zoom/scanline regression test orchestration; test tooling only. |
| `scripts/control-strength-baseline.json` | 10753 | content-reviewed | Reviewed measured visible-control strength and curve baseline data; tuning evidence, not executable effect code. |
| `scripts/gen-rust-licenses.py` | 4716 | content-reviewed | Reviewed macOS crate-license report generator; packaging/compliance utility only. |
| `scripts/make-mcpb.sh` | 1037 | content-reviewed | Reviewed Claude Desktop MCP bundle assembly; packaging only. |
| `scripts/make-release.sh` | 8984 | content-reviewed | Reviewed version/tag/release asset packaging workflow; release automation only. |
| `scripts/test.sh` | 1018 | content-reviewed | Reviewed aggregate macOS build/test/smoke/sweep/control-strength verification script; test orchestration only. |
| `scripts/win-vm.sh` | 1486 | content-reviewed | Reviewed Parallels Windows VM PowerShell command wrapper; local developer infrastructure only. |
| `scripts/wrap-app.sh` | 5626 | content-reviewed | Reviewed macOS app bundle assembly and placement of shaders/presets/dylibs/licenses; packaging only. |
| `server.json` | 756 | content-reviewed | Reviewed MCP Registry server metadata and package identity; registry descriptor only. |

## Closure rule

A row may be marked complete only after the file's content has been reviewed and assigned one of: ported, adapted, retained as reference, platform-only/not applicable (with reason), test-only (with result), binary asset not required (with reason), or generated/vendor data (with provenance/license). The NTSCRT audit remains open until all 359 rows have a final disposition and all required VHS-90 engine ports are integrated and tested.
