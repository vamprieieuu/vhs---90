# VHS-90 — VHS Camera Reference Clips Audit

Date: 2026-11-19
Input: vhs/Vhs cams/
Scope: all 6 files inventoried; all 4 video files probed; five timestamps sampled from each video; Fxc6 split-screen comparison frames inspected; Gen 7 before/after frames and notes inspected; Gen 7 audio stream measured.

## Fxc6 — two before/after comparison clips

Both files are HEVC 828-pixel-wide portrait screen recordings at 60 fps, about 6.73–6.80 seconds long. Each displays a vertical split: the left side is labelled Original Capture and the right side RF Direct Capture / VHS. The clips show road, trees, sky and palm foliage filmed from a moving vehicle.

### Visual changes on the processed side

- A strong blue/cyan cast appears in the sky and dark foliage; the scene is generally cooler and more muted in selected regions.
- Fine image detail is reduced, particularly foliage and road texture. The processed half has a softer, lower-detail appearance.
- A visible fine-grain/noise floor is present across the processed half, including smooth sky areas.
- Color and luma are not affected identically: chroma appears less stable and the blue channel is more prominent in the sky.
- The lower windshield/dashboard/wiper area remains visible and can carry high-contrast edge artifacts; these should not be mistaken for the VHS signal effect itself.

Approximate image statistics were sampled from several split-screen frames, excluding the lower label region. These are not calibrated source-vs-output measurements because the two sides are screen-captured halves, not independently exported aligned source frames. Across the sampled frames, the processed half generally had lower local edge energy and lower luma variation in detailed areas, while its color balance shifted toward cooler blue/cyan.

## Gen 7 — original and processed clip

Original: IMG_3621.MP4, HEVC 828×1086, 30 fps, 45 frames, 1.517 seconds. The source shows a person against a green background.

Processed: ScreenRecording_10-14-2026 17-30-46_1.mov, HEVC 828×1078, 60 fps, 299 video frames, about 8.96 seconds. It contains the VHS-processed result, with screen-recorded playback/interface context. Five points were sampled across the processed clip.

### Visible processing characteristics

- A pronounced yellow-green/teal color cast over the face and green background.
- Strong horizontal luma/chroma instability: thin lines, broken scanline-like bands, local horizontal tears and color-edge displacement.
- Grain/static is not uniform white noise; it includes dense correlated horizontal and wavy interference patterns. Some moments approach near-full-frame monochrome static while retaining partial subject structure.
- Chroma is heavily displaced at high-contrast edges (hair, face outline, hand and shirt stripes), with cyan/yellow fringing.
- The image has lower crisp detail and stronger analog-like smear than the original.
- The damaged state changes over time: the amount and organization of visible interference vary from frame to frame rather than remaining as a fixed overlay.

### Audio evidence and limitation

The processed screen recording has an AAC stereo audio stream at 44.1 kHz. Its measured overall RMS is about 0.071 in normalized amplitude; the spectrum is concentrated mainly in the 150–500 Hz and 500–2000 Hz bands, with little sustained high-frequency energy above 5 kHz. This is consistent with a bandwidth-limited/muffled recording in the captured output, but the supplied original IMG_3621.MP4 has no audio stream. Therefore the before/after audio change and any precise muffling curve cannot be measured from this pair; only the processed audio can be described.

The supplied note explicitly asks to study the increased noise/background sound, muffling, progressive glitching and eventual strange dark/black color. Those are requirements from the user's note, not a verified parameter sweep in the clip. A VHS-90 audio path would need a separate linked audio module if that behavior is required; the current native video effect should not pretend to alter audio unless implemented.

## VHS-90 implementation guidance

1. Treat Fxc6 as a restrained color/noise/detail-loss reference rather than a severe tracking-damage look.
2. Treat Gen 7 as a high-intensity camcorder/tape failure look: green/yellow color response, correlated line interference, dense horizontal static, chroma displacement and temporally changing signal collapse.
3. Keep luma degradation and chroma degradation independent so the image can lose detail while retaining strong chroma fringing.
4. Build static as a time-varying signal/noise process, not a full-frame still image overlay.
5. Keep progressive collapse/blackout as a separate signal-loss stage; do not reduce it to simply lowering brightness.
6. The camera reference pair does not define exact numerical controls or a reliable source/output alignment for a pixel-exact match. Use it as a visual target and refine with side-by-side AE renders.

## Verification

- All four video files were probed and sampled at five timestamps each.
- Fxc6 visual comparisons and Gen 7 before/after stills were inspected.
- The processed Gen 7 audio stream was extracted and measured; original audio is absent, so no before/after audio comparison is claimed.
- No source camera clip was altered.