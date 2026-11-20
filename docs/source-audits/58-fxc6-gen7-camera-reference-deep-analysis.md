# VHS-90 — FXC6 and Gen 7 camera reference deep analysis

Date: 2026-11-20  
Source archive: vhs.zip, folder vhs/Vhs cams/  
All six supplied files were re-opened; all four videos were probed and sampled in sequence. The two text notes were read in Arabic.

## 1. FXC6 — two split-screen before/after recordings

### Media
- Clip A: ScreenRecording_10-14-2026 18-22-43_1.mov — HEVC, 828×1050, 60/approximately 38.32 fps reported by container metadata, 258 frames, 6.733 s, no audio stream.
- Clip B: ScreenRecording_10-14-2026 18-23-02_1.mov — HEVC, 828×1068, 60/approximately 37.06 fps reported by container metadata, 252 frames, 6.800 s, no audio stream.
- Both show a vertical split with Original Capture on the left and RF Direct VHS on the right. The note asks to infer the color transformation and says two clips were supplied to make the behavior clearer.

### Sampled moments
Both clips were sampled at approximately 0.2, 0.8, 1.5, 2.2, 3.0, 4.0, 5.0 and 6.2 seconds.

### Visual identity on the RF Direct VHS side
- The strongest recurring signature is a cooler blue/cyan-to-violet cast, especially in smooth sky and shadow regions.
- Fine, dense grain/noise is visible across the processed side, including relatively smooth sky; it is not a sparse tracking tear or a large moving dropout band.
- Fine foliage and road texture appear softened/less clean, while grain increases local high-frequency activity. Do not interpret a higher Laplacian/high-pass measurement as proof of increased source detail; noise itself raises those metrics.
- Clip A's processed side appears comparatively pale/cool in the sky with dark green foliage and a blue-gray road. Clip B's processed side is darker and more strongly blue/violet in the sky and shadow regions. The difference is partly scene/exposure dependent and should not be collapsed into a fixed flat blue overlay.
- The right side does not show a reliable repeated lower head-switch bar, major horizontal tear, or full-frame static collapse. Keep this camera look restrained and separate from Gen 7's severe damage.

### Alignment and measurement limitation
The split-screen halves are not independently exported, aligned source/output frames. They show different spatial portions and apparent timing/crop differences, so direct pixel-by-pixel comparison or claims of exact luma/detail reduction are not justified. Aggregate half-frame RGB/noise statistics are confounded by scene content and capture layout. Use the visual appearance as a camera-reference target and refine using controlled AE source/render pairs.

### Recommended native profile
Add a distinct Camera Profile named **FXC6 Direct Capture**. It should use a restrained cool chroma bias, mild-to-moderate fine grain, subtle luma softness, controlled chroma compression/bleed and no default severe tracking bands. Noise and color response should remain independently adjustable.

## 2. Gen 7 — clean source and processed recording

### Media
- Before: IMG_3621.MP4 — HEVC, 828×1086, 30 fps, 45 frames, 1.517 s, no audio stream.
- After: ScreenRecording_10-14-2026 17-30-46_1.mov — HEVC, 828×1078, 299 frames, 8.960 s, actual average frame rate approximately 33.43 fps; AAC stereo 44.1 kHz audio.
- The before clip is a clean person against a green background. The after recording contains the processed output and no source audio counterpart.

### Visual sequence
The after recording was sampled at approximately 0.2, 0.8, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5 and 8.6 seconds; a denser frame-metric pass was also made at 0.25-second intervals.

The visible behavior is not a simple static filter:
- Colorized state: a strong teal/green base, yellow-green skin/highlights, cyan/blue edge fringing, and retained red/yellow/blue shirt stripes.
- Interference state: dark, nearly monochrome high-density static with wavy/horizontal interference. The subject remains partly recognizable through the interference.
- Recovery/low-damage state: the image returns toward a cleaner green-background frame, though the color response remains altered.
- The clip cycles between colorized, interference-heavy and cleaner states; the later section contains longer dark static periods. It does not behave as a single monotonically increasing amount slider in the supplied recording. Preserve this temporal variation as a deterministic, seedable signal-state process rather than baking in the exact reference sequence.

### Color and signal characteristics
- Strong green/cyan color response with yellow-biased highlights/skin.
- Noticeable RGB/chroma misregistration around hair, face, hand and shirt edges.
- Horizontal line breaks, wavy interference and dense correlated noise rather than uniform independent white grain.
- Luma detail and chroma detail degrade differently; the subject silhouette remains partially readable during static phases.
- Several moments approach near-full-frame monochrome static with darkened overall level, followed by partial recovery.

### Audio evidence and limitation
- The after file has an AAC stereo stream at 44.1 kHz; the before file has no audio stream.
- Measured after-file global RMS is approximately 0.0706 normalized amplitude.
- Across one-second windows, energy is generally concentrated in 150–500 Hz and 500–2,000 Hz; energy above 5 kHz is usually small, with a few variable windows. The final second has a strong low-frequency component.
- This is consistent with a bandwidth-limited/muffled captured signal, but the original has no audio. Therefore no before/after attenuation curve, noise-rise delta or precise muffling response can be measured. The Arabic note's description of rising background noise and progressive muffling is a user-supplied design requirement, not a measured before/after fact.
- VHS-90's current native effect is video-only. Do not claim that Camera Profile changes audio; a separate linked audio processing module would be required to implement noise rise, low-pass/muffling and progressive audio degradation.

### Recommended native profile
Add a distinct Camera Profile named **Gen 7 Reference**. It should provide a stronger teal/yellow camcorder color response, more aggressive chroma displacement/bleed, correlated horizontal noise, independent static and tracking damage, and optional signal collapse/blackout behavior. Use existing Tracking, Static, Tear & Chroma and Composite engines rather than creating one monolithic Gen 7 renderer. The camera profile should be a starting recipe, with each engine remaining independently editable.

## 3. Native profile implementation

Added two entries to the existing Camera Profile menu:
- FXC6 Direct Capture
- Gen 7 Reference

FXC6 maps to a restrained cool capture look with fine grain, softened detail, controlled chroma subsampling and independent I/Q color bias. Gen 7 maps to a stronger teal/yellow look with increased chroma instability, horizontal noise, moderate static/tracking and Tear & Chroma participation.

These are source-inspired camera reference recipes, not claims of exact camera hardware emulation or exact pixel parity. Their visual calibration must be completed by rendering controlled before/after pairs in After Effects.

## 4. Files and source notes
- Fxc6/ScreenRecording_10-14-2026 18-22-43_1.mov
- Fxc6/ScreenRecording_10-14-2026 18-23-02_1.mov
- Fxc6/نص.txt
- Gen 7/before vhs/IMG_3621.MP4
- Gen 7/After vhs_/ScreenRecording_10-14-2026 17-30-46_1.mov
- Gen 7/After vhs_/Notes on after vhs .txt

## 5. Verification boundary
- All six files were re-opened and inventoried.
- All four video files were re-probed and sampled; the two Fxc6 notes and Gen 7 note were read.
- The processed Gen 7 audio was measured, but no original audio exists for comparison.
- The AE profile wiring requires CI confirmation; Windows/MSVC Adobe SDK, PiPL, AEX loading, actual AE visual comparison and MFR remain unverified.

**Disposition:** FXC6 and Gen 7 have separate reference analyses and separate native Camera Profile recipes. The next validation is a controlled AE render comparison against the supplied clips, followed by any required profile tuning.
