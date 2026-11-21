# VHS

A real-time analog videotape emulation plugin for DaVinci Resolve (OFX). Reproduces the characteristic artifacts of consumer and broadcast analog tape formats — VHS, Betacam SP, Hi8 — with GPU-accelerated signal processing.

The plugin simulates the full analog video chain: composite encoding/decoding, NTSC/PAL bandwidth limits, magnetic tape noise, head-switching artifacts, tracking errors, dropouts, geometric distortion, and CRT display characteristics. 

## Features

- **Format presets** — Presents that generate nostalgic vibes: VHS Camcorder, Betacam SP, Hi8 Camcorder, and Damaged VHS profiles with appropriate noise, bandwidth, and artifact characteristics. Save and load your own presets.
- **Composite encoding** — NTSC or PAL with adjustable composite strength, dot crawl, cross-colour artifacts, and composite-domain noise
- **Analog signal degradation** — head-switching noise band, tracking instability with vertical drift, and configurable dropouts
- **Magnetic noise** — frequency-modulated luma and chroma noise with hue bias control, characteristic of magnetic tape playback
- **Chroma decimation** — reduced colour bandwidth with PAL phase-switch and interlace combing artifacts
- **Geometric wobble** — per-line horizontal jitter, top-of-frame skew with exponential decay, and edge pincushion distortion
- **Channel registration** — independent red and blue channel offsets to simulate CRT misconvergence
- **CRT bloom (mist)** — highlight bloom with veiling glare, emulating the analog display of a CRT monitor
- **Color fade & CRT trim** — adjustable colour fade, hue tint, black lift, white knee, and dynamic range to simulate aged tapes and monitor calibration
- **Built-in looks** — packaged 3D LUTs for period-accurate colour rendering
- **Camcorder OSD** — on-screen date, time, REC indicator, play mode, counter, and tape speed labels with selectable 7-segment or bitmap glyph sets
- **Input/Output colour space transforms** — optional CST pipeline to convert from and to DaVinci Wide Gamut / intermediate spaces

## Download

[Download v0.9 beta (macOS)](https://github.com/lofi-fx/vhs/releases/tag/v0.9-beta)

## Installing the plugin

The plugin is unsigned, so macOS will block it the normal way. To install:

1. **Extract the zip** — double-click the downloaded file. You'll see `LofiFxVhs.ofx.bundle`.
2. **Move it into place** — in Finder, press `Cmd+Shift+G`, type `/Library/OFX/Plugins/`, and hit Go. Drag `LofiFxVhs.ofx.bundle` into that folder.
3. **Bypass Gatekeeper** — open **System Settings > Privacy & Security**. Scroll down. Next to *"LofiFxVhs was blocked from loading"* click **Allow Anyway**.
4. If you don't see the message, open **Terminal** (from Applications/Utilities), paste this, and press Enter:
   ```
   sudo xattr -rd com.apple.quarantine /Library/OFX/Plugins/LofiFxVhs.ofx.bundle
   ```
   You'll be asked for your password. Type it (nothing will show as you type) and press Enter.
5. Restart DaVinci Resolve. The plugin appears under **LoFi FX VHS** in the OFX effects list.

## Building from source
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
```

Requires: CMake 3.20+, macOS 11+ (Metal GPU), Xcode Command Line Tools. Internet connection on first build (fetches OpenFX SDK headers).

Output: `build/LofiFxVhs.ofx.bundle`

## Install from source

```
sudo cmake --build build --target install_local
```

Or copy `build/LofiFxVhs.ofx.bundle` to `/Library/OFX/Plugins/`. Restart Resolve — the plugin appears under **LoFi FX VHS** in the OFX effects list.

## License

AGPL-3.0-or-later.
