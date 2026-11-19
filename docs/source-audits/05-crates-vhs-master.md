# VHS-90 — Crates VHS Master Binary/Metadata Audit

Date: 2026-11-19
Input: `vhs/plugins/aex/CratesVHSMaster.aex` and `vhs/plugins/c+/CratesVHSMaster.aex.c`
The binary contains an embedded JSON metadata/configuration string. It was extracted and parsed successfully; this is stronger evidence for parameter names and preset values than relying on decompiler variable names.

## Binary facts and implementation shape

- Supplied binary is a Windows PE32+ x86-64 DLL.
- Ghidra output exposes the AE effect entry point and render dispatch; the decompiled code includes recursive render passes, parameter JSON handling and preset application.
- Embedded shader source includes a three-stage OpenCL pipeline: `vhscraft_cl_encode`, `vhscraft_cl_decode`, `vhscraft_cl_finish`, plus pixel load/store kernels.
- The pipeline converts RGB to YUV, separates sharp luma from softened chroma, modulates chroma onto a composite carrier, applies scanline/frame phase, then demodulates and applies spatial/temporal damage.
- The source includes a separate Cinematic VHS pixel path; it is not simply a color preset over the same composite path.
- Shader helpers include bilinear sampling, spiral blur, channel-specific bloom/halation, FBM/value noise, tracking bands, top flagging, head-switch tearing, vertical shake, chroma smear, static/tape-color noise, streak/dropout artifacts, and final sharpening/grade.
- A 33×33×33 color LUT is referenced by the embedded grade system.
- The decompiled source also contains ONNX model identifiers for unrelated image-analysis/deblur tasks; these are not VHS signal kernels and should not be pulled into VHS-90's CPU-first core without a specific requirement.

## Embedded preset list

The metadata contains 17 entries: one undefined/None entry and 16 named looks.

1. Standard
2. Clean
3. Sharp
4. Purple
5. Soft
6. Green
7. Corrupted
8. Streaks
9. Cool
10. Warm
11. Blue Bloom
12. Echo
13. Retro
14. Camcorder
15. Hue Wobble
16. Black and White

All 16 named presets have full per-parameter values in the embedded JSON. The source metadata should be retained as a machine-readable reference for translating these into VHS-90 recipes; names should be renamed in the final VHS-90 UI rather than exposing source-brand names.

## Full parameter map

The embedded metadata defines 66 user controls across seven groups.

### Global
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 1 | All Effects | p_all | 100 |
| 2 | Noise | p_noised | 50 |
| 3 | Distortion | p_distd | 50 |

### Input
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 21 | Base Blur | inBlur | 0 |
| 22 | Chroma Blur | pChromaBlurMult | 100 |
| 23 | Bloom Radius | inHalationRadius | 100 |
| 24 | Bias Red | bmR | 30 |
| 25 | Bias Green | bmG | 50 |
| 26 | Bias Blue | bmB | 20 |
| 27 | Bloom Mix | inHalationMix | 10 |
| 28 | Add Color | pAddColor | (0.75294, 0, 1) |
| 29 | Add Color Strength | pAddColorStrength | 5 |
| 30 | Saturation | pChromaStrength | 50 |
| 31 | Black Borders Horizontal | pBlackBorder | 0 |
| 32 | Black Borders Vertical | pBlackBorderVertical | 0 |
| 33 | Input Raw Signal | pInSignal | off |
| 34 | Output Raw Signal | cOutSignal | off |

### Noise
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 41 | Static Noise | pHissNoise | 50 |
| 42 | Static Frequency | pHissScale | 100 |
| 43 | Tape Color Noise | amtNoise | 30 |
| 44 | Headswitch Size | pHeadswitchSize | 24 |
| 45 | Headswitch Noise | pHeadswitchNoise | 50 |
| 46 | Headswitch Spikes | pHeadSpikeAmount | 100 |
| 47 | Spike Amount | pSpikeAmount | 50 |
| 48 | Spike Intensity | pSpikeIntensity | 30 |
| 49 | Splotch Noise | pSplotchNoise | 50 |
| 50 | Splotch Frequency | pChromaScale | 100 |
| 51 | Tracking Noise | pTrackingInterference | 100 |
| 52 | Tracking Speed | pTrackingSpeed | -5 |
| 53 | Tracking Size | pTrackingSize | 0 |
| 54 | Color Killer | COLOR_KILLER_THRESHOLD | 0 |

### Distortion
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 61 | Micro Warps | pMicroWarps | 5 |
| 62 | Large Warps | pMacroWarps | 19 |
| 63 | Flagging Intensity | pTopWarp | 50 |
| 64 | Flagging Width | pTopWarpWidth | 294 |
| 65 | Headswitch Strength | pHeadswitchDistStr | 22 |
| 66 | Headswitch Width | pHeadswitchDistortionWidth | 40 |
| 67 | Tracking Distortion | pTrackingDistortion | 100 |
| 68 | Vertical Shake | pVerticalShake | 50 |

### Signal
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 81 | Scanlines NTSC | NTSC_SCANLINES | 540.2 |
| 82 | Samples | SAMPLES | 80 |
| 83 | Signal Blur | pTimeBlur | 70 |
| 84 | Attack | pAttack | 50 |
| 85 | Decay | pDecay | 50 |
| 86 | Echo | pEcho | 7 |
| 87 | Chroma Delay | pChromaDelay | -10 |
| 88 | Phase Wobble | pFrequencyWobble | 10 |
| 89 | Phase Wobble Speed | pFrequencyWobbleSpeed | 100 |
| 90 | Magnetic Lock | amtMagLock | 15 |
| 91 | Magnetic Lock Width | amtMagLockWidth | 11 |
| 92 | Magnetic Lock Frequency | amtMagLockFreq | 56 |
| 93 | SubcarrierFreq | SubcarrierFreq | 227.5 |

### Modulation
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 101 | Strength | pNoiseModulateStrength | 50 |
| 102 | Speed | pNoiseModulateSpeed | 20 |
| 103 | Scale | pNoiseModulateScale | 100 |
| 104 | Mask bias | pNoiseModulateContrast | 100 |
| 105 | Mask Bias | pNoiseModulateOffset | -20 |
| 106 | Mask Base | pNoiseModularBase | 50 |
| 107 | Affect Noise | pModNoise | 100 |
| 108 | Affect Distortion | pModDist | 50 |
| 109 | Affect Phase | pModPhase | 20 |

### Post FX
| Index | Label | Code field | Default |
|---:|---|---|---:|
| 121 | Blur Radius | Bstrength | 75 |
| 122 | Sharp Radius | sharpRadius | 75 |
| 123 | Sharpness | sharpAmt | 75 |
| 124 | Brightness | pBrightness | 100 |
| 125 | Contrast | pContrast | 100 |

Most sliders use metadata table [0,100] with divide-by-100 flags, but several defaults exceed that nominal range (for example Flagging Width 294, Scanlines NTSC 540.2, SubcarrierFreq 227.5) and Tracking Speed/Chroma Delay/Mask Bias are negative. VHS-90 must preserve meaningful signed/extended ranges rather than blindly clamp every parameter to 0–100.

## Distinct signal-stage findings

- The encode stage uses separate sharp and blurred samples, retaining luma detail while blurring chroma.
- Bloom/halation is channel-specific (red/green/blue radii and biases), not one generic glow.
- The composite carrier phase depends on scanline index, horizontal coordinate, time and alternating-line phase.
- The decode stage includes top-of-frame flagging, PLL instability, micro warps, large warps, head-switch tear, tracking displacement, noise, phase and modulation masks.
- The finish stage includes vertical shake, blur-radius sampling and four-neighbour sharpening.
- Cinematic mode instead uses lens curvature, scanline drift, a narrow moving tracking band, RGB-channel bleed, echo, block-residual damage, highlight scatter, grain, bottom-edge noise and final color controls. It explicitly restores source alpha and zeros RGB under fully transparent pixels.
- The source's head-switch and bottom-noise logic is bottom-anchored. VHS-90 must replace that geometry with its arbitrary-position region engine.

## VHS-90 reuse decision

Port the signal math, FBM/noise, YUV carrier, multi-pass sampling, channel-specific bloom and distinct cinematic path into the shared native engine. Keep the CPU implementation authoritative; GPU kernels may be an optional acceleration path. Translate the 16 named looks into VHS-90 recipe data, but do not expose the original source brand or labels in VHS-90's UI. Preserve all out-of-range and negative defaults where they are intentional.

## Validation

- Extracted and parsed the embedded JSON metadata from the supplied AEX successfully.
- Parsed all 66 user controls and 17 preset entries.
- Inspected the decompiled AE dispatch/render path and extracted shader function names/stage structure.
- No VHS-90 C++ implementation or runtime build is claimed from this audit alone.
