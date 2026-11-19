NTSC-VHS-Renderer 1.3.0 source attribution

Upstream: https://github.com/F33RNI/NTSC-VHS-Renderer/tree/1.3.0
Author: Fern Lane (2023).
Main project license: Apache-2.0 (LICENSE-APACHE-2.0).
Bundled BloomEffect license: MIT, Copyright (c) 2019 Yoann Berenguer (bloom-LICENSE-MIT).
The application wraps LMP88959/NTSC-CRT, FFmpeg, Pillow, NumPy, PyQt5 and BloomEffect. VHS-90 does not import its GUI, subprocess frame scheduler, executable binaries, FFmpeg bundle, Cython/Python runtime or compiled extension. Its post-grade and bloom algorithms have already been independently adapted into VHS-90's native C++ engines; this audit records the exact source behavior and compatibility limits.
