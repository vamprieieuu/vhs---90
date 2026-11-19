# Imported NTSC/VHS composite core

This directory contains the C89 NTSC-VHS composite core from the supplied NTSC-CRT v2.3.2 source set. The two generic core files are pinned to the exact v2.3.2 Git blob contents; crt_ntscvhs.c and crt_ntscvhs.h match the same source revision. The related system headers are retained because crt_core.h includes their declarations.

The VHS-90 build compiles only crt_core.c and crt_ntscvhs.c with CRT_SYSTEM=5. Unrelated console emulation source files are not part of this target.

The core is a composite signal engine, not the AE host adapter. The VHS-90 adapter must:
- keep the core instance-local and avoid sharing mutable state across MFR renders;
- replace process-global rand() behavior with explicit frame/module seeds;
- preserve AE alpha and handle premultiplied pixels;
- adapt AE rowbytes, dimensions and pixel formats;
- expose VHS speed and other compile-time options through runtime-safe configuration where appropriate;
- disable or replace the source's fixed lower-frame VHS noise with the shared arbitrary-region Tracking Noise and expandable Head Switching modules.

Local verification of the supplied v2.3.2 core:
- crt_core.c and crt_ntscvhs.c compile separately with GCC in strict C89 mode.
- A synthetic 320x240 BGRA frame was modulated and demodulated to a 640x480 BGRA output successfully.
- The sample video-converter target in the upstream CMake project fails due to incompatible int* / unsigned char* types in extra/video_convert.c; this is a demo/CLI integration issue, not a failure of the two core translation units.

The AE plug-in build still needs to run the imported source as part of the full VHS-90 build before final release.
