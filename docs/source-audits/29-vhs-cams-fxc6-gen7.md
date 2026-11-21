# VHS-90 — Vhs Cams / Fxc6 and Gen 7 Reference Audit

Date: 2026-11-19
Input: `vhs/Vhs cams/` from supplied `vhs.zip`.
Inventory: 6 files: two Fxc6 split-comparison screen recordings, one Fxc6 note, one Gen 7 clean/before clip, one Gen 7 processed/after clip and one Gen 7 note.

## Media metadata

| Clip | Resolution | Frame rate | Frames | Duration | Audio |
|---|---:|---:|---:|---:|---|
| Fxc6 — 18-22-43 | 828×1050 | 38.3168 fps | 258 | 6.733 s | None |
| Fxc6 — 18-23-02 | 828×1068 | 37.0588 fps | 252 | 6.800 s | None |
| Gen 7 — before | 828×1086 | 29.6703 fps | 45 | 1.517 s | None |
| Gen 7 — after | 828×1078 | 33.4265 fps | 299 | 8.960 s | AAC stereo, 44.1 kHz |

## 1. Fxc6 split-screen comparison

Both Fxc6 videos show a vertical split comparison within the same recording:
- Left side is labeled “Original Capture”.
- Right side is labeled “RF Direct VHS”.
- The center divider is part of the comparison UI, not a VHS artifact.
- The screen-recording control-center overlay at the very end is not part of the camera look.

The right/processed side is visibly more saturated and has more horizontal texture/noise than the original side. The color response is more vivid, with blue/purple sky and green foliage, while the original side is comparatively muted.

### Measured side-by-side color/texture metrics

The measurements below average frames before the final phone control-center overlay and exclude the center divider and bottom UI strip:

| Clip | Side | Mean luma | Mean saturation | Mean horizontal edge change |
|---|---|---:|---:|---:|
| Fxc6 18-22-43 | Original (left) | 93.2 | 32.3 | 1.2 |
| Fxc6 18-22-43 | RF Direct VHS (right) | 103.0 | 48.7 | 2.8 |
| Fxc6 18-23-02 | Original (left) | 138.7 | 34.0 | 1.0 |
| Fxc6 18-23-02 | RF Direct VHS (right) | 108.7 | 64.8 | 1.3 |

The processed side has higher saturation in both examples. Luma does not move in one fixed direction: it rises in the first scene and falls in the second, so VHS-90 should not implement the look as a universal brightness increase. The right-side image also shows more line texture and signal noise.

The supplied Fxc6 note explicitly asks to analyze color changes, added lines, background noise and audio degradation as VHS intensity/rate rises. However, neither Fxc6 clip contains an audio stream, so audio behavior cannot be measured from these two examples.

## 2. Gen 7 before/after visual comparison

### Before
- 828×1086, 45 frames, 29.67 fps, 1.517 s.
- Clean, stable source footage with natural skin, vivid green background and red/yellow/blue clothing.
- No audio track.

### After
- 828×1078, 299 frames, 33.43 fps, 8.96 s.
- Strong analog color response with green/teal background, warm skin and saturated red/yellow/blue clothing in the less-damaged states.
- Severe interference states become largely monochrome/low-saturation, with dense horizontal/diagonal luma noise, moiré-like line structures and image breakup.
- The processed clip alternates between cleaner saturated states and heavy monochrome static states. It is not a simple monotonic increase in damage throughout the entire 8.96 seconds.

### Observed temporal phases

Frame sampling at 0.5-second intervals shows:
- 0.0–0.5 s: saturated color image with moderate distortion.
- 0.5–1.5 s: strong monochrome/low-saturation interference and dense line noise.
- 1.5–3.5 s: mostly cleaner, saturated color image.
- 3.5–4.5 s: another noisy/darker interval.
- 4.5–6.0 s: mostly saturated color states.
- 6.0–8.96 s: progressively more persistent heavy noise, low saturation and dense line breakup.

Per-half-second metrics support these distinct states: clean intervals have mean saturation around 156–175 and horizontal edge-change around 1–3; severe static intervals fall to saturation around 46–75 and edge-change around 7–11. This indicates the effect is switching between different signal states, not merely applying a constant noise overlay.

## 3. Gen 7 audio analysis

Only the Gen 7 “after” clip contains audio. It is AAC stereo at 44.1 kHz. The Gen 7 “before” clip and both Fxc6 clips have no audio streams, so there is no direct clean-audio baseline for comparison.

The available after-audio track was measured in 0.5-second windows:
- At 4.0–5.0 s, spectral centroid is about 1.56–1.87 kHz and energy above 4 kHz is about 7.4–9.6%.
- At 6.5–8.0 s, spectral centroid rises to about 2.85–3.31 kHz and energy above 4 kHz reaches about 16.7–20%.
- At 8.0–9.0 s, spectral centroid drops sharply to about 343–404 Hz; energy above 4 kHz falls to about 0.9–1.6%, while the share below 300 Hz rises to about 85–91%.

The final second therefore has a strongly low-frequency-dominated / muffled character. The earlier 6.5–8.0 s section has more high-frequency content, consistent with a noisy/glitchy interval. These are measurements of the supplied processed audio only; without a clean reference track, they do not prove the exact before/after noise increase or the original filter cutoff.

## 4. VHS-90 audio-system implications

The user's written note specifically requests:
- Background noise increasing as VHS strength/rate increases.
- Progressive muffling / high-frequency loss.
- More audible distortion and degradation.
- Glitch/dropout behavior in sync with the picture.
- A final severely degraded / strange dark signal state.

These should be independent audio modules rather than one volume slider:
1. Tape hiss / RF noise layer with controllable level and band-limited spectrum.
2. Progressive low-pass / muffling, controlled by tape condition and VHS strength.
3. Soft saturation / tape compression and harmonic distortion.
4. Wow/flutter and small pitch instability.
5. Short audio dropouts and glitch bursts synchronized to visual signal events.
6. Optional stereo narrowing / mono collapse at severe degradation.
7. Separate dry/wet mix and output gain so adding hiss does not simply make the entire signal louder.

Important host limitation: the current VHS-90 target is a native After Effects video effect. Audio processing must be treated as a separate supported audio workflow/plugin capability; do not claim that the video AEX alone processes the composition's audio.

## 5. Integration decisions

- Preserve the Fxc6 split comparison as a color/noise reference, not as a runtime overlay.
- Use the processed-side saturation and line texture as tunable color-response targets; do not force a fixed brightness lift.
- Preserve Gen 7's alternating clean/color and severe-static states as a temporal state model, rather than forcing a single monotonic ramp.
- Keep severe monochrome line noise separate from the saturated color state.
- Treat the final audio low-pass behavior as a distinct muffled state; do not infer exact cutoff values from the processed-only clip.
- Do not reproduce screen-recording UI, split-divider labels or other capture interface elements.
