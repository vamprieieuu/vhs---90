#!/usr/bin/env python3
"""Extract representative source-reference frames for visual VHS analysis."""
from __future__ import annotations
from pathlib import Path
import math
import imageio.v2 as imageio
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
SOURCES = [
    ROOT / "VHS/Vhs cams/Fxc6/ScreenRecording_10-14-2026 18-22-43_1.mov",
    ROOT / "VHS/Vhs cams/Fxc6/ScreenRecording_10-14-2026 18-23-02_1.mov",
    ROOT / "VHS/Vhs cams/Gen 7/before vhs/IMG_3621.MP4",
    ROOT / "VHS/Vhs cams/Gen 7/After vhs_/ScreenRecording_10-14-2026 17-30-46_1.mov",
]
OUT = ROOT / "reference-frames"
OUT.mkdir(parents=True, exist_ok=True)
THUMB_W = 480

def extract(path: Path) -> None:
    if not path.exists():
        raise SystemExit(f"Missing source clip: {path}")
    reader = imageio.get_reader(str(path), format="FFMPEG")
    meta = reader.get_meta_data()
    fps = float(meta.get("fps") or 30.0)
    try:
        count = int(reader.count_frames())
    except Exception:
        duration = float(meta.get("duration") or 0.0)
        count = max(1, int(duration * fps))
    if count < 1:
        raise SystemExit(f"Could not determine frame count for {path}")
    samples = sorted(set(int(round(x)) for x in
                         [0, .05, .1, .2, .3, .4, .5, .6, .7, .8, .9, .95]
                         for x in [x * (count - 1)]))
    stem = path.stem.replace(" ", "_")
    tiles = []
    for n, idx in enumerate(samples):
        frame = reader.get_data(idx)
        image = Image.fromarray(frame).convert("RGB")
        ratio = THUMB_W / image.width
        image = image.resize((THUMB_W, max(1, int(image.height * ratio))))
        frame_path = OUT / f"{stem}_{idx:06d}.jpg"
        image.save(frame_path, quality=92)
        tile = Image.new("RGB", (THUMB_W, image.height + 34), "white")
        tile.paste(image, (0, 34))
        ImageDraw.Draw(tile).text((10, 10), f"{path.name} | frame {idx} | {idx/fps:.2f}s", fill="black")
        tiles.append(tile)
    reader.close()
    cols = 2
    rows = math.ceil(len(tiles) / cols)
    tile_h = max(t.height for t in tiles)
    sheet = Image.new("RGB", (cols * THUMB_W, rows * tile_h), "#222222")
    for i, tile in enumerate(tiles):
        sheet.paste(tile, ((i % cols) * THUMB_W, (i // cols) * tile_h))
    sheet.save(OUT / f"{stem}_contact.jpg", quality=92)
    print(f"{path.relative_to(ROOT)}: {count} frames at {fps:.3f} fps; sampled {len(samples)} frames")

for source in SOURCES:
    extract(source)
