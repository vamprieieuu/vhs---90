# VHS-90 — Supplied AEX Plugin Bundle Inventory

Date: 2026-11-19
Input: `vhs/plugins/` from the supplied `vhs.zip`.

## Inventory

### Native binaries (`aex/`)

Seven Windows After Effects effect binaries are present:
- `CratesVHSMaster.aex`
- `HEISEI VHS.aex`
- `signal.aex`
- `Universe_Transitions_VHS_Transition_AE_Fx.aex`
- `VHSCraft.aex`
- `VHSPro.aex`
- `VHSTape.aex`

### Decompiled C/C++-style material (`c+/`)

- `CratesVHSMaster.aex.c` + `.h`
- `HEISEI VHS.aex.c` + `.h`
- `signal.aex.c` + `.h`
- `Universe_Transitions_VHS_Transition_AE_Fx.aex.c` + `.h`
- `VHSCraft.aex.c` + `.h`
- `VHSPro.aex.c` only (no matching header file in the archive)
- `VHSTape.aex.h` only (no matching C decompilation in the archive)

The decompiled `.c/.h` files are Ghidra-style reverse-engineering output, not vendor source code that can be assumed to compile directly.

## Audit checkpoints recorded

- `05-crates-vhs-master.md`: embedded metadata, preset list, 66-control map and three-stage compute pipeline.
- `06-vhscraft.md`: parameter map and separate Classic/Cinematic processing paths.
- `07-signal-plugin.md`: recovered control labels and reconstruction limits.
- `08-heisei-vhs.md`: recovered parameter vocabulary.
- `09-universe-vhs-transition.md`: transition/noise modes and slider ranges.
- `10-vhspro.md`: Rust/wgpu GPU architecture, parameter vocabulary and preset menu.
- `11-vhstape.md`: NTSC/PAL, SP/LP/EP, playback, damage and OSD vocabulary.

## Important coverage limits

- The seven binary files have been inventoried, but the source audit depth differs by plugin depending on the decompilation material supplied.
- VHSTape has only a Ghidra header; VHSPro has only a Ghidra C file.
- No supplied AEX has been loaded in After Effects during these static audits.
- This inventory does not imply that all seven effects have been reproduced or integrated into VHS-90 yet.
