# NTSCRT 359-file audit ledger

Generated from upstream recursive Git tree on 2026-11-20. Upstream tree reports 359 blobs, 59 directory entries, `truncated=false`.

**Important:** this is a complete path/size disposition ledger, not a completed content audit. Only rows marked `content-reviewed` were read in this audit pass. The 45 JSON preset files were fetched and parsed, but their conversion to VHS-90 presets is pending. Every other row is explicitly left open with a reason; do not interpret inventory status as code review or port completion.

**Current disposition counts:** 76 content-reviewed; 45 preset JSON parsed; 73 platform-review-pending; 58 test/reference-pending; 49 inventory-only; 45 asset-inventory; 13 metadata-inventory. There are currently zero rows in `engine-review-pending`; this does not mean the entire 359-file audit is closed, because platform, test/reference, inventory, asset and preset-semantic reviews remain open.

| Upstream path | Bytes | Status | Disposition |
|---|---:|---|---|
| `.gitattributes` | 73 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `.github/workflows/mcp-registry.yml` | 2764 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `.github/workflows/windows-port.yml` | 7655 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `.gitignore` | 532 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `.gitmodules` | 215 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Assets/AppIcon.icns` | 1419876 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `Assets/icon-1024.png` | 617475 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `Assets/icon-source.png` | 636923 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `CertificateSigningRequest.certSigningRequest` | 964 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `DEVELOPMENT.md` | 80655 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `LICENSE` | 1069 | content-reviewed | Read during this audit pass |
| `NTSCRT july 27.webp` | 80040 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `Package.swift` | 3983 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `README.md` | 19946 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
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
| `Sources/CrtAppBridge/LibrashaderBridge.m` | 7857 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/NtscRsBridge.m` | 4603 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/LibrashaderBridge.h` | 2340 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/NtscRsBridge.h` | 1790 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/librashader.h` | 108825 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/librashader_ld.h` | 70783 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/module.modulemap` | 79 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
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
| `Sources/CrtSmoke/main.swift` | 13860 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtSweep/main.swift` | 11373 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtVcrProbe/main.swift` | 7379 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtVideoSmoke/VideoSmoke.swift` | 3633 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `THIRD_PARTY_NOTICES.md` | 4097 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `TestAssets/flat-gray.png` | 1968 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `TestAssets/game-frame.png` | 3329 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `TestAssets/testpattern.png` | 6117 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `Tests/CrtCoreTests/ChainInputCacheTests.swift` | 3464 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/DigitalVideoTests.swift` | 31342 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/ExportGlitchTests.swift` | 7100 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/ExportShaderToggleTests.swift` | 9748 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/FieldLinesTests.swift` | 2043 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/GlitchStageTests.swift` | 16672 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/HowlaroundTests.swift` | 30180 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/NtscChainTests.swift` | 5639 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/PreviewCompositorTests.swift` | 7115 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/PreviewGeometryTests.swift` | 7005 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/PreviewScalingTests.swift` | 8378 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/ReceiverSimulatorTests.swift` | 22470 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/ShaderBypassTests.swift` | 4692 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/SourceCropTests.swift` | 16449 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/UpdateCheckTests.swift` | 2351 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/VcrTests.swift` | 18869 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Tests/CrtCoreTests/Video1Tests.swift` | 5519 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `Vendor/ntscrs-capi/Cargo.lock` | 5720 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `Vendor/ntscrs-capi/Cargo.toml` | 396 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Vendor/ntscrs-capi/src/lib.rs` | 8906 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `docs/header.webp` | 80040 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `docs/screenshot.webp` | 179882 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `docs/windows-port-plan.md` | 14827 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `licenses/Apache-2.0.txt` | 10140 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `licenses/GPL-2.0.txt` | 15220 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `licenses/GPL-3.0.txt` | 34903 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `licenses/MIT.txt` | 1151 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `licenses/MPL-2.0.txt` | 15383 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `licenses/rust-crates.txt` | 527781 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `mcpb/icon.png` | 249433 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `mcpb/manifest.json` | 4042 | content-reviewed | Reviewed MCP bundle metadata and server entry point; packaging manifest only. |
| `mcpb/server/launch.cmd` | 1823 | content-reviewed | Reviewed Windows MCP launcher and bundled executable resolution; app deployment wrapper only. |
| `mcpb/server/launch.sh` | 1093 | content-reviewed | Reviewed macOS/Linux MCP launcher and bundled executable resolution; app deployment wrapper only. |
| `port/Cargo.lock` | 137943 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `port/Cargo.toml` | 1040 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/README.md` | 5184 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/app/Cargo.toml` | 1357 | content-reviewed | Reviewed eframe/egui app, media and CLI dependencies; platform app manifest only. |
| `port/app/build.rs` | 578 | content-reviewed | Reviewed Windows resource/version embedding; build metadata only. |
| `port/app/icon-256.png` | 92660 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/app/ntscrt.ico` | 146065 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
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
| `port/engine/tests/digital.rs` | 7713 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/dub.args` | 69 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/dub.json` | 128086 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/healthy.args` | 5 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/healthy.json` | 17601 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/hhold.args` | 24 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/hhold.json` | 25908 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/search.args` | 55 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/search.json` | 49912 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/slack.args` | 74 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/slack.json` | 58700 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/tape.args` | 97 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/tape.json` | 49920 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/transport.args` | 148 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/transport.json` | 54287 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/vcr.args` | 131 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/vcr.json` | 71258 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/vhold.args` | 22 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/vhold.json` | 17571 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/weak.args` | 25 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/glitch/weak.json` | 29833 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/fixtures/mac-launch-look.json` | 5128 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/glitch_parity.rs` | 5194 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/howl.rs` | 3637 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/mac_parity.rs` | 3393 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/engine/tests/preview_paths.rs` | 5457 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/fetch-librashader.sh` | 1053 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/librashader-wgpu-push-constants.patch` | 1610 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/licenses/rust-crates-windows.txt` | 665879 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `port/media/Cargo.toml` | 580 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/media/src/bin/probe.rs` | 2674 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/media/src/lib.rs` | 8312 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/media/src/mf.rs` | 10917 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/media/src/mf_writer.rs` | 9166 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/package-readme.txt` | 1291 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/cli-check.py` | 13257 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/gen-licenses.py` | 4005 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `port/scripts/glitch-ref-render/main.swift` | 2499 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/glitch-ref/main.swift` | 2204 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/make-glitch-refs.sh` | 2924 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/make-look-refs.py` | 3946 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/scripts/sync-mcp-text.py` | 2280 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/spike/Cargo.toml` | 298 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/examples/preview-bench.rs` | 3552 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/crt-aperture-ntsc.png` | 1821846 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-aperture-plain.png` | 235316 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-easymode-ntsc.png` | 1713966 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-easymode-plain.png` | 183313 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-hyllian-ntsc.png` | 1955324 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-hyllian-plain.png` | 496217 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-royale-ntsc.png` | 2500166 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crt-royale-plain.png` | 1757231 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtglow_gauss-ntsc.png` | 1168724 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtglow_gauss-plain.png` | 198064 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtglow_lanczos-ntsc.png` | 1172160 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtglow_lanczos-plain.png` | 180824 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtsim-ntsc.png` | 1116783 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/crtsim-plain.png` | 703898 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/ghost.args` | 55 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/ghost.png` | 201184 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/in.png` | 1202 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/roll.args` | 37 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/roll.png` | 2903 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/tape.args` | 97 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/tape.png` | 184028 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-clean.args` | 10 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-clean.png` | 162789 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-dub.args` | 75 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-dub.png` | 190144 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-ep.args` | 109 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-ep.png` | 183305 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-search.args` | 47 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-search.png` | 186445 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-used.args` | 62 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-used.png` | 186843 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/glitch/vcr-wrecked.args` | 149 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/ref-mac/glitch/vcr-wrecked.png` | 150399 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Clean CRT.png` | 378823 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Gentle waves loop.png` | 1645706 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/High Chroma static.png` | 2082781 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Low Res.png` | 1732391 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Medium VHS.png` | 2079926 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Mild VHS.png` | 543787 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Obliterated.png` | 1632189 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Readable Text.png` | 1745703 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/ref-mac/looks/Wavy loop.png` | 1447185 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/spike/src/bin/compare.rs` | 2165 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/src/bin/glitch-dump.rs` | 1811 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/src/bin/glitch-render.rs` | 1138 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `port/spike/src/main.rs` | 4775 | test-reference-pending | Tests, fixtures or reference images; parity/asset review pending |
| `presets/2 second dirty VCR loop.json` | 84498 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/2 second glitch.json` | 40234 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/2 second glitches.json` | 108663 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/2 second mild glitch.json` | 63227 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/5 second blips.json` | 68286 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/90s VHS bootleg.json` | 5364 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/CD-ROM (low quality).json` | 4782 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Cheap camcorder.json` | 4754 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Chunky webcam.json` | 4698 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Clean CRT.json` | 3574 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Clean VHS.json` | 3554 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Damaged VHS - strong.json` | 5144 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Dirty VCR + CRT (vertical).json` | 5400 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Dirty VCR.json` | 5221 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Gentle waves loop.json` | 3555 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Glitch 1.json` | 34917 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Glitch 2.json` | 38121 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Glitch 3.json` | 38354 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Glitch loop 1.json` | 27441 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Glitch pop.json` | 41200 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/High Chroma static.json` | 3618 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Low Res.json` | 3549 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Medium VHS.json` | 3614 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Mild VHS.json` | 3510 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Noisy TV broadcast on VHS.json` | 5128 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Obliterated.json` | 3548 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Readable Text.json` | 3582 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Screen Loop/12 copies luma key.json` | 643 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Screen Loop/15 copies color drift.json` | 673 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Screen Loop/22 copies bottom left.json` | 633 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Screen Loop/7 copies slow luma.json` | 659 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Screen Loop/Bottom to top.json` | 632 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Static loop 1.json` | 30966 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Tracking Noise up.json` | 37932 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Two blips loop.json` | 31401 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/V Hold glitch.json` | 36381 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/VHS with color loss.json` | 5390 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Vertical Freak Out.json` | 28180 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Vertical spike loop.json` | 24283 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Very wavy.json` | 20635 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Wavy loop.json` | 3569 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/Webcam (low quality).json` | 4821 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/glitchy block video.json` | 5268 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/glitchy streaming video with noise.json` | 5295 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `presets/glitchy streaming video.json` | 5185 | parsed | Fetched and JSON-parsed; semantic migration still pending |
| `scripts/build-ntscrs.sh` | 1375 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/check-control-strength.py` | 13381 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/check-export-toggles.sh` | 2353 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/check-zoom-scanlines.sh` | 3540 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/control-strength-baseline.json` | 10753 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/gen-rust-licenses.py` | 4716 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `scripts/make-mcpb.sh` | 1037 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/make-release.sh` | 8984 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/test.sh` | 1018 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/win-vm.sh` | 1486 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `scripts/wrap-app.sh` | 5626 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `server.json` | 756 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |

## Closure rule

A row may be marked complete only after the file's content has been reviewed and assigned one of: ported, adapted, retained as reference, platform-only/not applicable (with reason), test-only (with result), binary asset not required (with reason), or generated/vendor data (with provenance/license). The NTSCRT audit remains open until all 359 rows have a final disposition and all required VHS-90 engine ports are integrated and tested.
