"""Pictures of the desktop front end for the README: the settings window and the schematic pages.

usage: scripts/screenshots.sh        (needs the built binary, ffmpeg and the python venv: see README)

The window has a camera tube on, so the camera controls are live: the tube, the iris, the colour
pickup, the electronics, the tube temperature, spectral colour and scan geometry. The preview
shows one frame of the source taken through them, and the TV of each page shows that frame.
"""
from __future__ import annotations

import argparse
import os
import shutil
import sys
import time
from collections.abc import Callable
from pathlib import Path

from PySide6.QtCore import Qt
from PySide6.QtGui import QImage
from PySide6.QtWidgets import QApplication, QScrollArea

from vhsgui import backend
from vhsgui.app import MainWindow

WINDOW_SIZE = (1180, 1020)  # the settings window: wide enough for the preview, tall enough for the Look group
SCHEMATIC_SIZE = (1180, 620)  # size of the schematic drawings in docs/img
WAIT_S = 600.0  # longest wait for the source frame or the preview render
POLL_S = 0.05
JPEG_QUALITY = 88  # as scripts/sheet.py
PRESET = "worn"
TUBE = "plumbicon"
IRIS_STOPS = 0.5
SEED = 5


def wait_for(app: QApplication, ready: Callable[[], bool], what: str) -> None:
    """Run the event loop until `ready()` is true."""
    deadline = time.monotonic() + WAIT_S
    while not ready():
        if time.monotonic() > deadline:
            sys.exit(f"timed out waiting for {what}")
        app.processEvents()
        time.sleep(POLL_S)


def set_camera(window: MainWindow) -> None:
    """A studio camera with its optics on, so that every camera control is live."""
    window.preset_combo.setCurrentText(PRESET)
    window.seed_spin.setValue(SEED)
    window.camera_tube_combo.setCurrentIndex(window.camera_tube_combo.findData(TUBE))
    window.iris_spin.setValue(IRIS_STOPS)
    window.camera_spectral_check.setChecked(True)
    window.camera_geometry_check.setChecked(True)


def render_preview(app: QApplication, window: MainWindow) -> None:
    """One preview frame, rendered by the command line tool as the window does it."""
    window.length_spin.setValue(0.0)
    window.preview_button.click()
    wait_for(app, lambda: window.busy, "the preview to start")
    wait_for(app, lambda: not window.busy, "the preview render")
    if window.processed_frames is None:
        sys.exit("the preview produced no frame")


def save_jpeg(image: QImage, path: Path, size: tuple[int, int] | None = None) -> None:
    if size is not None:
        image = image.scaled(*size, Qt.AspectRatioMode.IgnoreAspectRatio, Qt.TransformationMode.SmoothTransformation)
    if not image.save(str(path), "JPG", JPEG_QUALITY):
        sys.exit(f"cannot save {path}")
    print(f"wrote {path}")


def settings_window(app: QApplication, window: MainWindow, out: Path) -> None:
    window.set_schematic_mode(False)
    window.resize(*WINDOW_SIZE)
    app.processEvents()
    scroll = window.findChild(QScrollArea)
    if scroll is not None:
        scroll.verticalScrollBar().setValue(0)
    app.processEvents()
    save_jpeg(window.grab().toImage(), out)


def schematic_page(app: QApplication, window: MainWindow, page: str, out: Path) -> None:
    window.set_schematic_page(page)
    window.set_schematic_mode(True)
    app.processEvents()
    # The view takes what the window leaves it, so the window is grown or shrunk until the view is
    # the size of the other drawings.
    view = window.camera if page == "camera" else window.schematic
    window.resize(
        window.width() + SCHEMATIC_SIZE[0] - view.width(),
        window.height() + SCHEMATIC_SIZE[1] - view.height(),
    )
    app.processEvents()
    if (view.width(), view.height()) != SCHEMATIC_SIZE:
        sys.exit(f"the {page} page is {view.width()}x{view.height()} and must be {SCHEMATIC_SIZE[0]}x{SCHEMATIC_SIZE[1]}")
    view.skip_boot()
    app.processEvents()
    save_jpeg(view.grab().toImage(), out)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("out_dir", type=Path)
    parser.add_argument("video", type=Path)
    parser.add_argument("--binary", help="the vhsphys executable (default: as python -m vhsgui)")
    args = parser.parse_args()
    out_dir, video = args.out_dir.resolve(), args.video.resolve()  # the script changes directory below

    binary = backend.find_binary(str(Path(args.binary).resolve()) if args.binary else None)
    ffmpeg, ffprobe = shutil.which("ffmpeg"), shutil.which("ffprobe")
    if binary is None or ffmpeg is None or ffprobe is None:
        sys.exit("needs the vhsphys binary, ffmpeg and ffprobe")

    app = QApplication(sys.argv[:1])
    app.setStyle("Fusion")
    window = MainWindow(binary, ffmpeg=ffmpeg, ffprobe=ffprobe)
    window.show()
    # The path shows in the input and output boxes, so the window is opened on the bare file name.
    os.chdir(video.parent)
    window.open_input(video.name)
    window.set_output(f"{video.stem}_vhs.mp4")
    wait_for(app, lambda: window.media is not None and not window.source_pending, "the source frame")
    set_camera(window)
    render_preview(app, window)
    settings_window(app, window, out_dir / "gui.jpg")
    schematic_page(app, window, "camera", out_dir / "schematic_camera.jpg")
    schematic_page(app, window, "circuit", out_dir / "schematic_circuit.jpg")
    window.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
