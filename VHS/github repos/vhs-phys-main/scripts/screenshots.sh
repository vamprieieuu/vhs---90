#!/usr/bin/env bash
# Take the pictures of the desktop front end in docs/img: the settings window and the camera and circuit pages of the schematic.
# usage: scripts/screenshots.sh        (needs ffmpeg and the python venv: see README)
# The window is run without a display (Qt's offscreen platform) and renders one preview frame.
set -euo pipefail
cd "$(dirname "$0")/.."
cargo build --release -q
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
ffmpeg -v error -y -f lavfi -i "mandelbrot=size=960x720:start_scale=0.4:end_scale=0.05" -frames:v 1 "$T/fractal.png"
ffmpeg -v error -y -loop 1 -framerate 30000/1001 -i "$T/fractal.png" -t 4 -pix_fmt yuv420p "$T/fractal.mp4"
QT_QPA_PLATFORM=offscreen PYTHONPATH=python .venv/bin/python scripts/screenshots.py docs/img "$T/fractal.mp4"
