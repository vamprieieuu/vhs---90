# CONTRACT
"""Switching the camera on, in the command line and the window: the lens cap before the shot and the beam
held off at its start.

The engine's `camera.idle_s` is lens-capped time with the beam on before the first frame (a cold camera
warms up under it) and `camera.beam_off_s` holds the beam off for the first seconds of the shot with the
lens open (the targets store the scene unread; the picture comes up when the beam does). The command
line takes them as `--camera-idle SECONDS` and `--camera-beam-off SECONDS`. The window carries both in its
look and its camera group, and the camera page draws them as LENS CAP (lens block) and BEAM OFF (tubes
block); they reach the command line only with a tube and only when above 0, and are kept in look files.

    model.DEFAULT_CAMERA_IDLE_S, model.CAMERA_IDLE_RANGE
    model.DEFAULT_CAMERA_BEAM_OFF_S, model.CAMERA_BEAM_OFF_RANGE
    Look.camera_idle_s: float, Look.camera_beam_off_s: float
    widgets field_camera_idle_s, field_camera_beam_off_s (QDoubleSpinBox)
"""
from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from vhsgui import backend, model
from vhsgui.model import GuiError, Look, RunRequest

RUN_FRAMES = 6
RUN_W, RUN_H = 320, 240
RUN_PRESET = "pristine"
CLI_TIMEOUT_S = 600
# 0.1 s is three frames at 29.97 Hz: the first three frames are read with the beam off.
BEAM_OFF_S = "0.1"
BEAM_OFF_FRAMES = 3

CAMERA_DIR = Path(__file__).resolve().parents[1] / "vhsgui" / "schematic" / "camera"
NEW_FLAGS = ("--camera-idle", "--camera-beam-off")
NEW_IDS = ("camera_idle_s", "camera_beam_off_s")


# ---------------------------------------------------------------- command line


def run_cli(binary: Path, clip: Path, out: Path, *extra: str) -> subprocess.CompletedProcess[str]:
    cmd = [str(binary), "run", str(clip), str(out), "--size", f"{RUN_W}x{RUN_H}", "--crf", "30", "--frames",
           str(RUN_FRAMES), "--audio", "none", "--preset", RUN_PRESET, *extra]
    return subprocess.run(cmd, capture_output=True, text=True, timeout=CLI_TIMEOUT_S)


def frames(binary: Path, clip: Path, ffmpeg: str, out: Path, *extra: str) -> np.ndarray:
    """The decoded RGB frames of a run, shape (n, h, w, 3)."""
    r = run_cli(binary, clip, out, *extra)
    assert r.returncode == 0, r.stderr
    raw = subprocess.run([ffmpeg, "-v", "error", "-i", str(out), "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                         capture_output=True, check=True, timeout=CLI_TIMEOUT_S).stdout
    return np.frombuffer(raw, np.uint8).reshape(-1, RUN_H, RUN_W, 3)


def test_the_idle_flag_sets_its_config_key_and_a_set_wins(binary, clip, ffmpeg, tmp_path):
    cold = ("--camera-tube", "vidicon", "--camera-cold-start", "-10")
    with_flag = frames(binary, clip, ffmpeg, tmp_path / "flag.mp4", *cold, "--camera-idle", "120")
    with_key = frames(binary, clip, ffmpeg, tmp_path / "key.mp4", *cold, "--set", "camera.idle_s=120.0")
    assert np.array_equal(with_flag, with_key), "--camera-idle 120 is camera.idle_s=120.0"
    r = run_cli(binary, clip, tmp_path / "bad.mp4", *cold, "--camera-idle", "5", "--set", "camera.idle_s=bogus")
    assert r.returncode != 0
    assert "camera.idle_s" in r.stderr, r.stderr


def test_the_beam_off_flag_sets_its_config_key_and_the_picture_comes_up_with_the_beam(binary, clip, ffmpeg, tmp_path):
    tube = ("--camera-tube", "plumbicon")
    with_flag = frames(binary, clip, ffmpeg, tmp_path / "flag.mp4", *tube, "--camera-beam-off", BEAM_OFF_S)
    with_key = frames(binary, clip, ffmpeg, tmp_path / "key.mp4", *tube, "--set", f"camera.beam_off_s={BEAM_OFF_S}")
    plain = frames(binary, clip, ffmpeg, tmp_path / "plain.mp4", *tube)
    assert np.array_equal(with_flag, with_key), f"--camera-beam-off {BEAM_OFF_S} is camera.beam_off_s={BEAM_OFF_S}"
    for n in range(BEAM_OFF_FRAMES - 1):
        assert with_flag[n].mean() < 0.5 * plain[n].mean(), f"frame {n}: the beam is off, so nothing is read"
    last = RUN_FRAMES - 1
    assert with_flag[last].mean() > 0.5 * plain[last].mean(), "the picture comes up once the beam is on"
    r = run_cli(binary, clip, tmp_path / "bad.mp4", *tube, "--camera-beam-off", "1", "--set", "camera.beam_off_s=bogus")
    assert r.returncode != 0
    assert "camera.beam_off_s" in r.stderr, r.stderr


@pytest.mark.parametrize(
    ("args", "message"),
    [
        (("--camera-idle", "-1"), "--camera-idle"),
        (("--camera-idle", "long"), "--camera-idle"),
        (("--camera-beam-off", "-0.5"), "--camera-beam-off"),
        (("--camera-beam-off", "soon"), "--camera-beam-off"),
    ],
)
def test_a_bad_switch_on_time_is_named_in_the_error(binary, clip, tmp_path, args, message):
    r = run_cli(binary, clip, tmp_path / "x.mp4", "--camera-tube", "plumbicon", *args)
    assert r.returncode != 0
    assert "invalid value" in r.stderr and message in r.stderr, r.stderr


# ---------------------------------------------------------------- the look and its command line


def _req(**look: object) -> RunRequest:
    return RunRequest(input="in.mp4", output="out.mp4", look=Look(**look))


def value_of(cmd: list[str], flag: str) -> str:
    assert flag in cmd, cmd
    return cmd[cmd.index(flag) + 1]


def test_the_settings_start_at_none():
    assert model.DEFAULT_CAMERA_IDLE_S == 0.0 == model.CAMERA_IDLE_RANGE[0]
    assert model.CAMERA_IDLE_RANGE[1] >= 600.0, "long enough for a cold camera to warm up"
    assert model.DEFAULT_CAMERA_BEAM_OFF_S == 0.0 == model.CAMERA_BEAM_OFF_RANGE[0] < model.CAMERA_BEAM_OFF_RANGE[1]
    look = Look()
    assert (look.camera_idle_s, look.camera_beam_off_s) == (0.0, 0.0)


def test_run_args_carry_them_only_with_a_tube_and_above_zero():
    args = backend.run_args("vhsphys", _req(camera_idle_s=60.0, camera_beam_off_s=2.0), None)
    for flag in NEW_FLAGS:
        assert flag not in args, flag
    plain = backend.run_args("vhsphys", _req(camera_tube="saticon"), None)
    for flag in NEW_FLAGS:
        assert flag not in plain, flag
    both = backend.run_args("vhsphys", _req(camera_tube="saticon", camera_idle_s=60.0, camera_beam_off_s=2.5), None)
    assert float(value_of(both, "--camera-idle")) == 60.0
    assert float(value_of(both, "--camera-beam-off")) == 2.5


def test_the_window_s_flags_are_ones_the_command_line_takes(binary, clip, tmp_path):
    look = Look(camera_tube="newvicon", camera_idle_s=10.0, camera_beam_off_s=0.1)
    req = RunRequest(str(clip), str(tmp_path / "out.mp4"), look, duration=0.2, size=(320, 240))
    args = backend.run_args(binary, req, None)
    for flag in NEW_FLAGS:
        assert flag in args, flag
    state = backend.Job(args).start().wait(timeout=CLI_TIMEOUT_S)
    assert state.returncode == 0, state.log


def test_look_files_keep_them_and_older_files_load():
    look = Look(camera_tube="saticon", camera_idle_s=300.0, camera_beam_off_s=4.5)
    assert backend.load_look(backend.save_look(look)) == look
    old = backend.load_look('[look]\ncamera_tube = "plumbicon"\n')
    assert (old.camera_idle_s, old.camera_beam_off_s) == (0.0, 0.0)
    whole = backend.load_look("[look]\ncamera_idle_s = 60\ncamera_beam_off_s = 2\n")
    assert (whole.camera_idle_s, whole.camera_beam_off_s) == (60.0, 2.0)


@pytest.mark.parametrize(
    "line",
    [
        "camera_idle_s = -1.0",
        "camera_idle_s = 1e9",
        'camera_idle_s = "long"',
        "camera_idle_s = true",
        "camera_beam_off_s = -0.5",
        "camera_beam_off_s = 1e9",
        'camera_beam_off_s = "soon"',
        "camera_beam_off_s = false",
    ],
)
def test_look_files_refuse_bad_times(line):
    with pytest.raises(GuiError):
        backend.load_look(f"[look]\n{line}\n")


@pytest.mark.parametrize("settings", [{"camera_idle_s": -1.0}, {"camera_beam_off_s": 1e9}])
def test_a_look_with_a_bad_time_is_not_saved(settings):
    with pytest.raises(GuiError):
        backend.save_look(Look(camera_tube="plumbicon", **settings))


# ---------------------------------------------------------------- the window and the camera page


@pytest.fixture
def window(binary, ffmpeg, ffprobe, tmp_path):
    from PySide6.QtWidgets import QApplication

    from vhsgui.app import MainWindow

    app = QApplication.instance() or QApplication([])
    win = MainWindow(binary=binary, workdir=tmp_path / "work")
    win.resize(1280, 900)
    win.show()
    yield win
    win.close()
    app.processEvents()


def _controls(window):
    from PySide6.QtWidgets import QComboBox, QDoubleSpinBox

    found = {
        "tube": window.findChild(QComboBox, "field_camera_tube"),
        "camera_idle_s": window.findChild(QDoubleSpinBox, "field_camera_idle_s"),
        "camera_beam_off_s": window.findChild(QDoubleSpinBox, "field_camera_beam_off_s"),
    }
    for name, widget in found.items():
        assert widget is not None, f"no control for {name}"
    return found


def test_the_window_has_the_controls_and_they_need_a_tube(window, tmp_path):
    c = _controls(window)
    assert (c["camera_idle_s"].minimum(), c["camera_idle_s"].maximum()) == model.CAMERA_IDLE_RANGE
    assert (c["camera_beam_off_s"].minimum(), c["camera_beam_off_s"].maximum()) == model.CAMERA_BEAM_OFF_RANGE
    for name in NEW_IDS:
        assert c[name].value() == 0.0 and c[name].toolTip(), name
        assert not c[name].isEnabled(), f"{name} needs a tube"
    c["tube"].setCurrentIndex(c["tube"].findData("vidicon"))
    for name in NEW_IDS:
        assert c[name].isEnabled(), name
    c["camera_idle_s"].setValue(120.0)
    c["camera_beam_off_s"].setValue(1.5)
    look = window.look()
    assert (look.camera_idle_s, look.camera_beam_off_s) == (120.0, 1.5)
    assert window.preview_stale is True
    path = tmp_path / "switch.look.toml"
    window.save_look(str(path))
    c["camera_idle_s"].setValue(0.0)
    c["camera_beam_off_s"].setValue(0.0)
    window.load_look(str(path))
    assert window.look() == look


def test_the_page_draws_the_cap_on_the_lens_and_the_beam_on_the_tubes(window):
    from PySide6.QtWidgets import QApplication

    layout = json.loads((CAMERA_DIR / "layout.json").read_text())
    controls = {c["id"]: c for c in layout["controls"]}
    assert (controls["camera_idle_s"]["block"], controls["camera_idle_s"]["kind"]) == ("lens", "slider")
    assert (controls["camera_idle_s"]["min"], controls["camera_idle_s"]["max"]) == model.CAMERA_IDLE_RANGE
    assert (controls["camera_beam_off_s"]["block"], controls["camera_beam_off_s"]["kind"]) == ("tubes", "slider")
    assert (controls["camera_beam_off_s"]["min"], controls["camera_beam_off_s"]["max"]) == model.CAMERA_BEAM_OFF_RANGE
    c = _controls(window)
    window.set_schematic_mode(True)
    window.set_schematic_page("camera")
    QApplication.processEvents()
    view = window.camera
    for cid in NEW_IDS:
        assert not view.is_enabled(cid), cid
    c["tube"].setCurrentIndex(c["tube"].findData("saticon"))
    for cid in NEW_IDS:
        assert view.is_enabled(cid), cid
    view.set_value("camera_beam_off_s", 2.0, emit=True)
    QApplication.processEvents()
    assert c["camera_beam_off_s"].value() == pytest.approx(2.0), "the page drives the window"
    c["camera_idle_s"].setValue(300.0)
    assert view.value("camera_idle_s") == pytest.approx(300.0), "and the window the page"
