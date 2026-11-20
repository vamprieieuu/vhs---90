# VHS-90 — ntscQT 1.5.0 Expanded Full Audit & Native Integration Disposition

Date: 2026-11-20  
Source archive: `vhs.zip` → `vhs/github repos/ntscqt-1.5.0/ntscqt-1.5.0/`  
Inventory: **28 archive entries: 23 files + 5 directories**  
Core source: `app/ntsc.py` (745 lines); GUI/application: `app/NtscApp.py` (578 lines), `app/Renderer.py` (render loop)  
License: Apache License 2.0

## 1. Architecture and source provenance

ntscQT 1.5.0 is a Python 3 / NumPy / SciPy / OpenCV / PyQt5 desktop video processor. Its signal model is derived from the composite-video-simulator project. It is not a native C++ library and its GUI, FFmpeg orchestration, OpenCV frame I/O and mutable application state are not suitable for direct inclusion in the AE plugin.

The core converts BGR frames to planar int32 YIQ, processes alternating fields, modulates/demodulates chroma through a four-phase subcarrier, then converts back to BGR. It has a deliberately accurate but slow frame-processing path. The GUI's Seed reproduces the random parameter/noise configuration; Preview Height and Render Height trade quality for speed.

The included LICENSE is Apache-2.0. Keep the license and attribution with any source-derived implementation. No Python, PyQt, SciPy, OpenCV or FFmpeg runtime was added to VHS-90.

## 2. File and folder disposition

All 28 archive entries were inventoried. The 23 files are:

| Path | Disposition |
|---|---|
| `.gitignore` | Python/PyInstaller/build exclusions only. |
| `app/config_dialog.py` | Qt configuration-copy dialog; host UI only. |
| `app/funcs.py` | Resize, file picker and width-multiple-of-four workarounds; host/video I/O only. |
| `app/logs.py` | Loguru logger setup; host utility only. |
| `app/ntsc.py` | Main 745-line YIQ/composite/VHS signal engine; all signal stages reviewed. |
| `app/NtscApp.py` | 578-line Qt application, parameter widgets, Seed, templates, preview, image/video and config flow; host UI only. |
| `app/Renderer.py` | FFmpeg/OpenCV threaded video render, pause/live-preview and audio remux; AE host owns frame timing/rendering, so not portable. |
| `app/ringPattern.npy` | Parsed as a 720-element float32 frequency-response curve, range 0..1; consumed by the source's optional frequency-domain ringing2 path. |
| `builtin_templates.json` | One community template dictionary; configuration data only, not a source algorithm. Do not expose source username as a VHS-90 preset label. |
| `demo.gif` | Demonstration media; not executable source. |
| `LICENSE` | Apache-2.0; preserve attribution and license notice. |
| `make.sh` | PyInstaller packaging script; not part of the effect engine. |
| `ntscQT.py` | PyQt application entry point and locale bootstrap; host-only. |
| `ntscQT.spec` | PyInstaller packaging configuration; host-only. |
| `README.md` | User documentation, artifact descriptions, speed/seed/render notes and upstream provenance. |
| `requirements.txt` | Python GUI/video dependency pins; do not add these dependencies to the native AE plugin. |
| `translate/ru_RU.qm` | Compiled Russian Qt translation resource; GUI-only binary. |
| `ui/configExportDialog.py` | Generated Qt form wrapper; GUI-only. |
| `ui/configExportDialog.ui` | Qt Designer source; GUI-only. |
| `ui/CustomSlider.py` | Qt double-slider/spin-box widget; GUI-only. |
| `ui/DoubleSlider.py` | Qt floating-point slider wrapper; GUI-only. |
| `ui/mainWindow.py` | Generated Qt main-window wrapper; GUI-only. |
| `ui/mainWindow.ui` | Qt Designer main-window layout; GUI-only. |

The five directory entries are the project root, nested project root, `app/`, `translate/` and `ui/`. No additional hidden source directory was present in this archive path.

## 3. Signal algorithms and parameter behavior

The core source contains the following substantive stages:

- BGR↔YIQ conversion and field-aware output reconstruction.
- Three-stage first-order I/Q low-pass filtering for composite input/output, including a lighter TV output filter.
- Composite pre-emphasis using a high-pass component of luma before modulation.
- Four-phase chroma modulation/demodulation, selectable scanline phase and phase offset.
- Luma noise, separate I/Q chroma noise and per-scanline I/Q phase rotation noise.
- VHS head switching based on a raster phase point, line displacement and decaying horizontal shift.
- Horizontal/vertical chroma bleed, chroma loss by scanline, chroma vertical delay-line blending and chroma down/up-sampling.
- SP/LP/EP tape bandwidth profiles:
  - SP: luma 2.4 MHz, chroma 320 kHz, chroma delay 9 samples.
  - LP: luma 1.9 MHz, chroma 300 kHz, chroma delay 12 samples.
  - EP: luma 1.4 MHz, chroma 280 kHz, chroma delay 14 samples.
- Luma low-pass plus high-frequency emphasis/sharpening, and optional S-video versus composite re-encoding.
- Two frequency-domain ringing variants. The first applies a horizontal DFT mask with optional seeded frequency noise. The second applies the bundled 720-value `ringPattern.npy` response raised to a configurable power and shifted/scaled in frequency.
- Seeded random profile generation with triangular distributions, including noise, VHS speed, ringing, phase, bleed and edge-wave settings.
- A low-pass-correlated row-wise VHS edge-wave displacement.

The GUI exposes the corresponding controls, advanced toggles, config import/export, Seed, image/video input, preview controls and video render controls. The GUI is not ported: AE parameters must remain native effect parameters, and AE frame requests must remain deterministic and render-local.

## 4. Comparison with the canonical VHS-90 engine

The useful VHS signal behavior is already represented by the canonical native modules. Reuse those implementations; do not create a second ntscQT engine or duplicate signal controls:

| ntscQT behavior | VHS-90 canonical implementation / disposition |
|---|---|
| YIQ conversion and composite encode/decode | `VHS90CompositeEngine`, `VHS90WaveformEngine`, shared YIQ kernels. |
| Composite I/Q bandwidth filters | `VHS90IIRFilter` / YIQ bandwidth path. |
| SP/LP/EP tape bandwidth and playback characteristics | Existing tape-speed and image-grade/bandwidth mapping; CVS SP/LP/EP profiles already expose source-derived operating modes. |
| Vertical chroma delay-line blend | `processVerticalChromaBlend`; native implementation retains luma and alpha. |
| Luma smear, luma sharpening and chroma sharpening | Existing luma-smear, image-grade and independent chroma-edge-enhancement paths. |
| Luma noise and chroma noise | Analog-noise and composite-noise controls. |
| I/Q phase noise / chroma flutter | Receiver-instability and RF-instability engines. |
| Chroma loss and dropouts | Dropout and tape-surface engines; preserve their independent semantics. |
| VHS edge wave / row-correlated displacement | Analog-noise/transport and deterministic correlated-noise kernels. |
| Head switching | Shared region/head-condition engines, with arbitrary normalized placement/size/angle rather than ntscQT's fixed raster phase. |
| VHS composite re-encoding and output bandwidth | Composite/waveform engine and input/output bandwidth stages. |
| Seeded random behavior | Render-local deterministic frame/seed hashing; do not import mutable global RNG state into AE/MFR. |
| Qt/FFmpeg render and audio remux | Not portable; AE owns frame scheduling, video/audio I/O, preview and rendering. |

The earlier report `20-ntscqt-vhs-engine.md` already records the initial 23-file inventory and the core migration decisions. This expanded audit closes the remaining GUI, packaging, asset and file-disposition review for the archived 1.5.0 folder.

## 5. Deliberate non-duplication and remaining candidates

Two source details are not identical to the current VHS-90 implementation and should remain explicit follow-up candidates rather than being falsely reported as exact ports:

1. **Composite pre-emphasis placement.** ntscQT applies a configurable high-pass-derived luma pre-emphasis before composite modulation. VHS-90 has image-grade/edge-enhancement and composite bandwidth controls, but those are not automatically identical in signal order. A future native implementation should be a dedicated pre-modulation luma stage, independently controlled and tested, not another post-decode sharpen slider.
2. **Frequency-domain ringing response.** ntscQT's `ringing2` uses the exact 720-sample `ringPattern.npy` spectral curve. VHS-90's existing `VHS90RingingEngine` is a spatial luma-edge/ringing implementation, not a claim of exact spectral-mask parity. Porting the curve meaningfully requires a tested efficient row-frequency transform or a mathematically validated equivalent; a raw table copy or an expensive naive DFT is not a valid integration.

The source's 2D FFT ringing, exact random-number stream and mutable per-frame Python state were therefore not copied into the native engine in this audit. Their behavior is documented for a deliberate future port. Existing noise/phase/head-switching/chroma/transport modules were reused rather than multiplied.

## 6. Verification boundary

- Archive inventory: 28 entries (23 files, 5 directories).
- Core source reviewed: `app/ntsc.py`, 745 lines; methods and processing order mapped.
- GUI/application and render pipeline inventoried; Qt widgets and FFmpeg orchestration classified as host-specific.
- `ringPattern.npy` successfully parsed: 720 float32 samples, min 0, max 1.
- Existing VHS-90 Linux core CI remains the relevant native-core evidence; this audit did not run the original Python GUI or claim pixel-identical visual parity.
- Windows/MSVC Adobe SDK compilation, PiPL, AEX loading, After Effects render, Multi-Frame Rendering and visual comparison remain global release gates.

## 7. Folder disposition

**Archive/file inventory: complete (28/28 entries).**  
**Core signal review and native-engine mapping: complete.**  
**Reusable behavior: mapped to existing VHS-90 engines; no duplicate engine added.**  
**Apache-2.0 attribution: recorded.**  
**Pre-emphasis signal-order parity and spectral ringing: documented as specific future candidates, not falsely marked as exact ports.**  
**Windows/AE host validation: still pending globally.**

Next source folder: continue with the next folder in the user's supplied eight-folder sequence after `ntscqt-1.5.0`; verify its exact name from the archive/order list before opening it.
