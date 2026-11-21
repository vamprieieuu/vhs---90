# NTSC-CRT source attribution

Upstream: https://github.com/LMP88959/NTSC-CRT
Author: EMMIR (2018–2023), with NES-mode contributions by Persune.
License: see LICENSE in this directory.

VHS-90 does not embed the upstream application or its global-state CRT runtime. The reusable fixed-point sine/cosine interpolation and three-band YIQ equalizer have been independently adapted into `VHS90CrtEqualizerEngine` with render-local filter state, alpha preservation and a C++ smoke test. Other console-specific profiles, file I/O and app wrappers remain audit references only.
