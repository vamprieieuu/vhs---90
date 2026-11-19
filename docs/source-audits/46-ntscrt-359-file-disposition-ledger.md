# NTSCRT 359-file audit ledger

Generated from upstream recursive Git tree on 2026-11-20. Upstream tree reports 359 blobs, 59 directory entries, `truncated=false`.

**Important:** this is a complete path/size disposition ledger, not a completed content audit. Only rows marked `content-reviewed` were read in this audit pass. The 45 JSON preset files were fetched and parsed, but their conversion to VHS-90 presets is pending. Every other row is explicitly left open with a reason; do not interpret inventory status as code review or port completion.

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
| `Sources/CrtApp/AppState+Export.swift` | 8347 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/AppState+Howlaround.swift` | 17019 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/AppState.swift` | 102696 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/BuiltInPresets.swift` | 1572 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/CLIHelp.swift` | 2255 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/ConnectSelfTest.swift` | 3187 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/DescribeCommand.swift` | 19368 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/Launch.swift` | 3557 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/LookSettings.swift` | 31419 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/MCPServer.swift` | 40664 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CLI/RenderCommand.swift` | 21865 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/ClaudeConnection.swift` | 8114 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/CrtAppApp.swift` | 6259 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/ExportFormat.swift` | 1171 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/FeedbackPresets.swift` | 2305 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/NeutralValues.swift` | 7105 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/NtscSettingsModel.swift` | 8519 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/ParamGates.swift` | 7368 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Paths.swift` | 5052 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Timeline.swift` | 7300 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/UpdateChecker.swift` | 7858 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ConnectClaudeView.swift` | 4915 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ContentView.swift` | 159354 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/CropPanel.swift` | 4230 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/DigitalVideoPanel.swift` | 6041 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/DownscalePanel.swift` | 5896 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ExportOptions.swift` | 7553 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ExportPopover.swift` | 7382 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/GlitchPanel.swift` | 4913 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/HowlaroundPanel.swift` | 17733 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/NtscPanel.swift` | 10493 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/NumericField.swift` | 5558 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/PresetCycleCheck.swift` | 12372 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/PreviewView.swift` | 40712 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/PropertySlider.swift` | 4775 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ShaderPanel.swift` | 15412 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/SourcePanel.swift` | 2516 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/TimelineBar.swift` | 13727 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ToolTip.swift` | 1065 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/TransportBar.swift` | 2564 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/VanishingPointPad.swift` | 12645 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/VcrPanel.swift` | 3795 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtApp/Views/ViewPalette.swift` | 3389 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `Sources/CrtAppBridge/LibrashaderBridge.m` | 7857 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/NtscRsBridge.m` | 4603 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/LibrashaderBridge.h` | 2340 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/NtscRsBridge.h` | 1790 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/librashader.h` | 108825 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/librashader_ld.h` | 70783 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtAppBridge/include/module.modulemap` | 79 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `Sources/CrtCodec/DigitalCodec.swift` | 31137 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCodec/DigitalVideoParams.swift` | 18748 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCodec/DigitalVideoRenderer.swift` | 38039 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCodec/Video1Codec.swift` | 10694 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/ChainInputCache.swift` | 4145 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/CrtCodecExport.swift` | 207 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Deck.swift` | 7542 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/DigitalStage.swift` | 12116 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Downscaler.swift` | 11854 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/GifExporter.swift` | 19879 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/GlitchParams.swift` | 27087 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/GlitchRenderer.swift` | 14627 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/GlitchStage.swift` | 21056 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Howlaround.swift` | 54180 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/HowlaroundPad.swift` | 7570 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/ImageIO.swift` | 4491 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/ImageStats.swift` | 1894 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/MetalContext.swift` | 1014 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Mp4Exporter.swift` | 35617 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/NtscStage.swift` | 7626 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Pipeline.swift` | 7476 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/PlaybackPipeline.swift` | 22040 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Presets.swift` | 1375 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/PreviewCompositor.swift` | 12012 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/PreviewGeometry.swift` | 4836 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/PreviewScaling.swift` | 7192 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Receiver.swift` | 48200 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/ScanlineGrid.swift` | 6334 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/ShaderBypass.swift` | 8205 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/SourceCrop.swift` | 9603 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Tape.swift` | 29129 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/Transport.swift` | 13900 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/UpdateCheck.swift` | 4189 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/VcrStage.swift` | 59635 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `Sources/CrtCore/VideoSource.swift` | 12672 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
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
| `mcpb/manifest.json` | 4042 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `mcpb/server/launch.cmd` | 1823 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `mcpb/server/launch.sh` | 1093 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/Cargo.lock` | 137943 | metadata-inventory | Dependency/legal metadata inventoried; detailed dependency/license review pending |
| `port/Cargo.toml` | 1040 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/README.md` | 5184 | inventory-only | Build/docs/scripts/vendor/support file; inspect and disposition before closing audit |
| `port/app/Cargo.toml` | 1357 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/build.rs` | 578 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/icon-256.png` | 92660 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/app/ntscrt.ico` | 146065 | asset-inventory | Binary visual asset; metadata inventoried, visual inspection/port decision pending |
| `port/app/src/bin/ntscrt-cli.rs` | 2385 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/claude.rs` | 16079 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/describe.rs` | 23230 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/help.rs` | 2600 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/mcp.rs` | 35074 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/mcp_text.rs` | 10784 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/mod.rs` | 7833 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/render.rs` | 25695 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/cli/settings.rs` | 35356 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/clip_frames.rs` | 1653 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/connect_ui.rs` | 6442 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/digital_panel.rs` | 4154 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/export.rs` | 14116 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/frame_cache.rs` | 3741 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/glitch_panel.rs` | 3389 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/main.rs` | 87067 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/ntsc_panel.rs` | 9648 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/prefetch.rs` | 5049 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/producer.rs` | 11969 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/screen_loop.rs` | 36147 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/shader_panel.rs` | 13419 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/timeline_ui.rs` | 12109 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/update.rs` | 15767 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/vcr_panel.rs` | 2315 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/app/src/worker.rs` | 18843 | platform-review-pending | App/UI/CLI/MCP wrapper; native AE reuse decision pending |
| `port/codec/.gitignore` | 11 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/Cargo.toml` | 351 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/examples/parity.rs` | 11546 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/ref/build.sh` | 873 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/ref/cases.json` | 7400 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/ref/main.swift` | 9034 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/codec.rs` | 34249 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/lib.rs` | 2871 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/params.rs` | 23724 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/renderer.rs` | 41691 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/resample.rs` | 8792 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/tables.rs` | 4606 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/tests/digital_video.rs` | 27316 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/tests/mod.rs` | 228 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/tests/port.rs` | 3527 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/tests/video1.rs` | 5478 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/codec/src/video1.rs` | 15002 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/Cargo.toml` | 734 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/crop.rs` | 8433 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/downscale.wgsl` | 4980 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/export.rs` | 5501 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/glitch/deck.rs` | 6898 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/glitch.wgsl` | 11347 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/mod.rs` | 634 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/params.rs` | 25256 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/receiver.rs` | 43912 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/stage.rs` | 25145 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/glitch/tape.rs` | 24899 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/transport.rs` | 14511 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/vcr.rs` | 19408 | content-reviewed | Read during this audit pass |
| `port/engine/src/glitch/vcr.wgsl` | 29726 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/feedback.rs` | 23060 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/howl.wgsl` | 4593 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/mod.rs` | 29409 | content-reviewed | Read during this audit pass |
| `port/engine/src/howl/pad.rs` | 8614 | content-reviewed | Read during this audit pass |
| `port/engine/src/lib.rs` | 47008 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/look.rs` | 20571 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/ntsc_ui.rs` | 8599 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/preview.rs` | 7456 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
| `port/engine/src/timeline.rs` | 9204 | engine-review-pending | Potentially reusable signal/codec/engine code; content audit and port decision pending |
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
