#!/usr/bin/env bash
# Render the camera pickup tubes on a moving highlight and on colour bars and build contact sheets in docs/img.
# usage: scripts/camera.sh        (needs ffmpeg and the python venv: see README)
# The camera runs at about two frames per second, so the clips are short and each sheet takes a few minutes.
set -euo pipefail
cd "$(dirname "$0")/.."
cargo build --release -q
V=target/release/vhsphys
PY=.venv/bin/python
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT

# The tape is pristine in every picture, so what differs between the panels is the camera.
COMMON=(--preset pristine --seed 5 --audio none)

# lag: a white box and a dim grey box cross a dark scene at 420 px/s. At +3 stops the white box is
# eight times the light that gives peak white, four times what the beam can take off in one pass, so
# it leaves a comet tail; the grey box stays below peak white and leaves only lag.
CLIP_S=1.5     # length of the clip: 45 frames
SHOT_S=1.4     # the frame of every panel
IRIS=3
ffmpeg -v error -y -f lavfi -i "color=c=0x1c2433:s=960x720:r=30000/1001" \
  -f lavfi -i "color=c=white:s=150x150:r=30000/1001" \
  -f lavfi -i "color=c=0x505050:s=150x150:r=30000/1001" \
  -filter_complex "[0][1]overlay=x='40+t*420':y=170:eval=frame[a];[a][2]overlay=x='40+t*420':y=420:eval=frame,format=yuv420p" \
  -t "$CLIP_S" "$T/boxes.mp4"
ffmpeg -v error -y -ss "$SHOT_S" -i "$T/boxes.mp4" -frames:v 1 "$T/boxes_source.png"
items=("source=$T/boxes_source.png")
for tube in plumbicon plumbicon-act plumbicon-dbc vidicon saticon; do
  $V run "$T/boxes.mp4" "$T/boxes_$tube.mp4" "${COMMON[@]}" --camera-tube "$tube" --iris-stops "$IRIS" 2>/dev/null
  ffmpeg -v error -y -ss "$SHOT_S" -i "$T/boxes_$tube.mp4" -frames:v 1 "$T/boxes_$tube.png"
  items+=("$tube=$T/boxes_$tube.png")
done
$PY python/vhslab/sheet.py docs/img/features_camera_lag.jpg "${items[@]}"

# materials: colour bars taken through the prism, the infrared filter and each material's spectral
# response with the consumer electronics, which have no colour matrix to undo the mixing. The still
# is the last of 12 frames, so the tube has settled.
FRAMES=12
ffmpeg -v error -y -f lavfi -i "smptebars=size=960x720:rate=30000/1001" -t 1 -pix_fmt yuv420p "$T/bars.mp4"
ffmpeg -v error -y -ss 0.4 -i "$T/bars.mp4" -frames:v 1 "$T/bars_source.png"
items=("source=$T/bars_source.png")
for tube in plumbicon vidicon newvicon saticon chalnicon; do
  $V run "$T/bars.mp4" "$T/bars_$tube.png" "${COMMON[@]}" --frames "$FRAMES" \
    --camera-tube "$tube" --camera-spectral --camera-processing consumer 2>/dev/null
  items+=("$tube=$T/bars_$tube.png")
done
$PY python/vhslab/sheet.py docs/img/features_camera_materials.jpg "${items[@]}"
echo "wrote docs/img/features_camera_{lag,materials}.jpg"
