# CONTRACT
"""The tubes' beam current, beam-bending blooming and cold start in the command line and the window.

The engine has long had `camera.beam_ratio` (beam current as a multiple of what peak white needs, 2.0 by
the data sheet), `camera.blooming` (beam-bending strength, 0 = off) and `camera.cold_start_c` (faceplate
temperature at switch-on). The command line gains `--camera-beam-ratio N` and `--camera-blooming N`
beside the existing `--camera-cold-start DEG_C`. The window carries all three in its look, in the camera
group and on the schematic camera page's PICKUP TUBES block, under the same rules as the other camera
settings: on the command line only with a tube and only when they differ from the engine's defaults,
kept in look files. Cold start is a switch with its own temperature: the temperature line works only
while the switch is on, and the flag goes out only then.

    model.DEFAULT_CAMERA_BEAM_RATIO, model.CAMERA_BEAM_RATIO_RANGE
    model.DEFAULT_CAMERA_BLOOMING, model.CAMERA_BLOOMING_RANGE
    model.DEFAULT_CAMERA_COLD_START_C, model.CAMERA_COLD_START_RANGE
    Look.camera_beam_ratio: float, Look.camera_blooming: float,
    Look.camera_cold_start: bool, Look.camera_cold_start_c: float
    widgets field_camera_beam_ratio, field_camera_blooming, field_camera_cold_start_c (QDoubleSpinBox),
            field_camera_cold_start (QCheckBox)
    camera page lines of the same ids, in the "tubes" block
"""
from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from vhsgui import backend, model
from vhsgui.model import GuiError, Look, RunRequest

RUN_FRAMES = "3"
RUN_SIZE = "320x240"
RUN_PRESET = "pristine"
CLI_TIMEOUT_S = 300

CAMERA_DIR = Path(__file__).resolve().parents[1] / "vhsgui" / "schematic" / "camera"
NEW_FLAGS = ("--camera-beam-ratio", "--camera-blooming", "--camera-cold-start")
# The new lines that need only a tube, and the one that also needs the cold-start switch.
NEEDS_A_TUBE = ("camera_beam_ratio", "camera_blooming", "camera_cold_start")
NEEDS_COLD_START = "camera_cold_start_c"
NEW_IDS = (*NEEDS_A_TUBE, NEEDS_COLD_START)


# ---------------------------------------------------------------- command line


def run_cli(binary: Path, clip: Path, out: Path, *extra: str) -> subprocess.CompletedProcess[str]:
    cmd = [str(binary), "run", str(clip), str(out), "--size", RUN_SIZE, "--crf", "30", "--frames", RUN_FRAMES,
           "--audio", "none", "--preset", RUN_PRESET, *extra]
    return subprocess.run(cmd, capture_output=True, text=True, timeout=CLI_TIMEOUT_S)


def picture(binary: Path, clip: Path, ffmpeg: str, out: Path, *extra: str) -> bytes:
    r = run_cli(binary, clip, out, *extra)
    assert r.returncode == 0, r.stderr
    return subprocess.run([ffmpeg, "-v", "error", "-i", str(out), "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                          capture_output=True, check=True, timeout=CLI_TIMEOUT_S).stdout


@pytest.mark.parametrize(
    ("flag", "value", "key"),
    [("--camera-beam-ratio", "3.5", "camera.beam_ratio=3.5"), ("--camera-blooming", "1.5", "camera.blooming=1.5")],
)
def test_the_flag_sets_its_config_key_and_a_set_wins(binary, clip, ffmpeg, tmp_path, flag, value, key):
    tube = ("--camera-tube", "plumbicon", "--iris-stops", "3")
    with_flag = picture(binary, clip, ffmpeg, tmp_path / "flag.mp4", *tube, flag, value)
    with_key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", *tube, "--set", key)
    plain = picture(binary, clip, ffmpeg, tmp_path / "plain.mp4", *tube)
    assert with_flag == with_key, f"{flag} {value} is {key}"
    assert with_flag != plain, f"{flag} {value} changes an overexposed picture"
    name = key.split("=")[0]
    r = run_cli(binary, clip, tmp_path / "bad.mp4", *tube, flag, value, "--set", f"{name}=bogus")
    assert r.returncode != 0
    assert name in r.stderr, r.stderr


@pytest.mark.parametrize(
    ("args", "message"),
    [
        (("--camera-beam-ratio", "0"), "--camera-beam-ratio"),
        (("--camera-beam-ratio", "-1"), "--camera-beam-ratio"),
        (("--camera-beam-ratio", "lots"), "--camera-beam-ratio"),
        (("--camera-blooming", "-0.5"), "--camera-blooming"),
        (("--camera-blooming", "much"), "--camera-blooming"),
    ],
)
def test_a_bad_beam_or_blooming_is_named_in_the_error(binary, clip, tmp_path, args, message):
    r = run_cli(binary, clip, tmp_path / "x.mp4", "--camera-tube", "plumbicon", *args)
    assert r.returncode != 0
    assert "invalid value" in r.stderr and message in r.stderr, r.stderr


# ---------------------------------------------------------------- the look and its command line


def _req(**look: object) -> RunRequest:
    return RunRequest(input="in.mp4", output="out.mp4", look=Look(**look))


def value_of(cmd: list[str], flag: str) -> str:
    assert flag in cmd, cmd
    return cmd[cmd.index(flag) + 1]


def test_the_settings_start_at_the_engine_defaults():
    assert model.DEFAULT_CAMERA_BEAM_RATIO == 2.0, "the data sheet's beam: twice what peak white needs"
    lo, hi = model.CAMERA_BEAM_RATIO_RANGE
    assert 0.0 < lo < 1.0 < model.DEFAULT_CAMERA_BEAM_RATIO < hi, "a starved beam is offered as well as a strong one"
    assert model.DEFAULT_CAMERA_BLOOMING == 0.0
    assert model.CAMERA_BLOOMING_RANGE[0] == 0.0 < model.CAMERA_BLOOMING_RANGE[1]
    lo, hi = model.CAMERA_COLD_START_RANGE
    assert lo < 0.0 < model.DEFAULT_CAMERA_COLD_START_C < model.DEFAULT_CAMERA_TEMPERATURE_C <= hi, \
        "a camera can be switched on below freezing"
    look = Look()
    assert (look.camera_beam_ratio, look.camera_blooming, look.camera_cold_start, look.camera_cold_start_c) == (
        2.0, 0.0, False, model.DEFAULT_CAMERA_COLD_START_C)


def test_run_args_carry_none_of_them_without_a_tube():
    args = backend.run_args("vhsphys", _req(camera_beam_ratio=3.0, camera_blooming=1.0, camera_cold_start=True,
                                            camera_cold_start_c=-5.0), None)
    for flag in NEW_FLAGS:
        assert flag not in args, flag


def test_run_args_carry_a_setting_only_when_it_differs_from_the_default():
    plain = backend.run_args("vhsphys", _req(camera_tube="saticon"), None)
    for flag in NEW_FLAGS:
        assert flag not in plain, flag
    beam = backend.run_args("vhsphys", _req(camera_tube="saticon", camera_beam_ratio=3.5), None)
    assert float(value_of(beam, "--camera-beam-ratio")) == 3.5
    bloom = backend.run_args("vhsphys", _req(camera_tube="saticon", camera_blooming=0.75), None)
    assert float(value_of(bloom, "--camera-blooming")) == 0.75


def test_cold_start_goes_out_only_while_its_switch_is_on():
    off = backend.run_args("vhsphys", _req(camera_tube="vidicon", camera_cold_start_c=-5.0), None)
    assert "--camera-cold-start" not in off, "a temperature with the switch off is only remembered"
    on = backend.run_args("vhsphys", _req(camera_tube="vidicon", camera_cold_start=True, camera_cold_start_c=-5.0), None)
    assert float(value_of(on, "--camera-cold-start")) == -5.0
    # switched on at the default temperature is still a cold start
    at_default = backend.run_args("vhsphys", _req(camera_tube="vidicon", camera_cold_start=True), None)
    assert float(value_of(at_default, "--camera-cold-start")) == model.DEFAULT_CAMERA_COLD_START_C


def test_the_window_s_flags_are_ones_the_command_line_takes(binary, clip, tmp_path):
    look = Look(camera_tube="saticon", iris_stops=1.0, camera_beam_ratio=3.0, camera_blooming=1.0,
                camera_cold_start=True, camera_cold_start_c=-10.0)
    req = RunRequest(str(clip), str(tmp_path / "out.mp4"), look, duration=0.2, size=(320, 240))
    args = backend.run_args(binary, req, None)
    for flag in NEW_FLAGS:
        assert flag in args, flag
    state = backend.Job(args).start().wait(timeout=CLI_TIMEOUT_S)
    assert state.returncode == 0, state.log


def test_look_files_keep_the_settings():
    look = Look(camera_tube="saticon", camera_beam_ratio=3.5, camera_blooming=1.25, camera_cold_start=True,
                camera_cold_start_c=-7.5)
    assert backend.load_look(backend.save_look(look)) == look
    # the temperature is kept with the switch off too
    look = Look(camera_tube="vidicon", camera_cold_start=False, camera_cold_start_c=3.0)
    assert backend.load_look(backend.save_look(look)) == look


def test_an_older_look_file_loads_with_the_default_settings():
    look = backend.load_look('[look]\ncamera_tube = "plumbicon"\niris_stops = 2.0\n')
    assert (look.camera_beam_ratio, look.camera_blooming, look.camera_cold_start, look.camera_cold_start_c) == (
        2.0, 0.0, False, model.DEFAULT_CAMERA_COLD_START_C)
    # whole numbers written by hand are still numbers
    look = backend.load_look("[look]\ncamera_beam_ratio = 3\ncamera_blooming = 1\ncamera_cold_start_c = -5\n")
    assert (look.camera_beam_ratio, look.camera_blooming, look.camera_cold_start_c) == (3.0, 1.0, -5.0)


@pytest.mark.parametrize(
    "line",
    [
        "camera_beam_ratio = 0.0",
        "camera_beam_ratio = 99.0",
        'camera_beam_ratio = "high"',
        "camera_beam_ratio = true",
        "camera_blooming = -1.0",
        "camera_blooming = 99.0",
        'camera_blooming = "on"',
        'camera_cold_start = "yes"',
        "camera_cold_start = 1",
        "camera_cold_start_c = -100.0",
        "camera_cold_start_c = 500.0",
        'camera_cold_start_c = "cold"',
        "camera_cold_start_c = false",
    ],
)
def test_look_files_refuse_bad_settings(line):
    with pytest.raises(GuiError):
        backend.load_look(f"[look]\n{line}\n")


@pytest.mark.parametrize(
    "settings",
    [{"camera_beam_ratio": 0.0}, {"camera_blooming": -1.0}, {"camera_cold_start": True, "camera_cold_start_c": 500.0}],
)
def test_a_look_with_a_bad_setting_is_not_saved(settings):
    with pytest.raises(GuiError):
        backend.save_look(Look(camera_tube="plumbicon", **settings))


# ---------------------------------------------------------------- the window


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
    from PySide6.QtWidgets import QCheckBox, QComboBox, QDoubleSpinBox

    found = {
        "tube": window.findChild(QComboBox, "field_camera_tube"),
        "camera_beam_ratio": window.findChild(QDoubleSpinBox, "field_camera_beam_ratio"),
        "camera_blooming": window.findChild(QDoubleSpinBox, "field_camera_blooming"),
        "camera_cold_start": window.findChild(QCheckBox, "field_camera_cold_start"),
        "camera_cold_start_c": window.findChild(QDoubleSpinBox, "field_camera_cold_start_c"),
    }
    for name, widget in found.items():
        assert widget is not None, f"no control for {name}"
    return found


def test_the_window_has_the_controls(window):
    c = _controls(window)
    assert (c["camera_beam_ratio"].minimum(), c["camera_beam_ratio"].maximum()) == model.CAMERA_BEAM_RATIO_RANGE
    assert (c["camera_blooming"].minimum(), c["camera_blooming"].maximum()) == model.CAMERA_BLOOMING_RANGE
    assert (c["camera_cold_start_c"].minimum(), c["camera_cold_start_c"].maximum()) == model.CAMERA_COLD_START_RANGE
    assert c["camera_beam_ratio"].value() == model.DEFAULT_CAMERA_BEAM_RATIO
    assert c["camera_blooming"].value() == model.DEFAULT_CAMERA_BLOOMING
    assert c["camera_cold_start_c"].value() == model.DEFAULT_CAMERA_COLD_START_C
    assert not c["camera_cold_start"].isChecked()
    for name in NEW_IDS:
        assert c[name].toolTip(), f"{name} explains itself"


def test_the_controls_need_a_tube_and_the_temperature_needs_the_switch(window):
    c = _controls(window)
    assert c["tube"].currentData() == "off"
    for name in NEW_IDS:
        assert not c[name].isEnabled(), name
    c["tube"].setCurrentIndex(c["tube"].findData("saticon"))
    for name in NEEDS_A_TUBE:
        assert c[name].isEnabled(), name
    assert not c[NEEDS_COLD_START].isEnabled(), "the switch-on temperature waits for the cold-start switch"
    c["camera_cold_start"].setChecked(True)
    assert c[NEEDS_COLD_START].isEnabled()
    c["tube"].setCurrentIndex(c["tube"].findData("off"))
    assert not c[NEEDS_COLD_START].isEnabled() and not c["camera_cold_start"].isEnabled()


def test_the_controls_drive_the_look_and_a_look_file(window, tmp_path):
    c = _controls(window)
    c["tube"].setCurrentIndex(c["tube"].findData("vidicon"))
    c["camera_beam_ratio"].setValue(3.5)
    c["camera_blooming"].setValue(1.5)
    c["camera_cold_start"].setChecked(True)
    c["camera_cold_start_c"].setValue(-10.0)
    look = window.look()
    assert (look.camera_beam_ratio, look.camera_blooming, look.camera_cold_start, look.camera_cold_start_c) == (
        3.5, 1.5, True, -10.0)
    assert window.preview_stale is True
    path = tmp_path / "tube.look.toml"
    window.save_look(str(path))
    c["camera_beam_ratio"].setValue(model.DEFAULT_CAMERA_BEAM_RATIO)
    c["camera_blooming"].setValue(model.DEFAULT_CAMERA_BLOOMING)
    c["camera_cold_start"].setChecked(False)
    c["tube"].setCurrentIndex(c["tube"].findData("off"))
    window.load_look(str(path))
    assert window.look() == look
    assert c[NEEDS_COLD_START].isEnabled(), "a loaded cold start enables its temperature"


# ---------------------------------------------------------------- the camera page


def test_the_page_draws_the_lines_in_the_tubes_block():
    layout = json.loads((CAMERA_DIR / "layout.json").read_text())
    controls = {c["id"]: c for c in layout["controls"]}
    blocks = {b["id"]: b for b in layout["blocks"]}
    want = {
        "camera_beam_ratio": ("slider", model.CAMERA_BEAM_RATIO_RANGE),
        "camera_blooming": ("slider", model.CAMERA_BLOOMING_RANGE),
        "camera_cold_start": ("toggle", None),
        "camera_cold_start_c": ("slider", model.CAMERA_COLD_START_RANGE),
    }
    for cid, (kind, span) in want.items():
        c = controls[cid]
        assert (c["block"], c["kind"]) == ("tubes", kind), cid
        if span is not None:
            assert (c["min"], c["max"]) == span, cid
    # every line of the page sits inside its block and on a row of its own
    rows = []
    for c in layout["controls"]:
        b = blocks[c["block"]]
        assert b["x"] <= c["x"] and c["x"] + c["w"] <= b["x"] + b["w"], c["id"]
        assert b["y"] < c["y"] < b["y"] + b["h"] - 16, f"{c['id']} sits above its block's caption"
        rows.append((c["block"], c["y"]))
    assert len(rows) == len(set(rows)), "two lines share a row"
    # and no block overlaps another
    boxes = list(blocks.values())
    for i, a in enumerate(boxes):
        for b in boxes[i + 1 :]:
            apart = (a["x"] + a["w"] <= b["x"] or b["x"] + b["w"] <= a["x"]
                     or a["y"] + a["h"] <= b["y"] or b["y"] + b["h"] <= a["y"])
            assert apart, f"{a['id']} overlaps {b['id']}"


def test_the_page_follows_the_switches_and_drives_the_editors(window):
    from PySide6.QtWidgets import QApplication

    c = _controls(window)
    window.set_schematic_mode(True)
    window.set_schematic_page("camera")
    QApplication.processEvents()
    view = window.camera
    for cid in NEW_IDS:
        assert not view.is_enabled(cid), cid
    c["tube"].setCurrentIndex(c["tube"].findData("plumbicon"))
    for cid in NEEDS_A_TUBE:
        assert view.is_enabled(cid), cid
    assert not view.is_enabled(NEEDS_COLD_START)
    view.set_value("camera_cold_start", True, emit=True)
    QApplication.processEvents()
    assert c["camera_cold_start"].isChecked(), "the page's switch drives the window's"
    assert view.is_enabled(NEEDS_COLD_START)
    c["camera_beam_ratio"].setValue(3.0)
    c["camera_blooming"].setValue(2.0)
    c["camera_cold_start_c"].setValue(-5.0)
    for cid, want in (("camera_beam_ratio", 3.0), ("camera_blooming", 2.0), ("camera_cold_start_c", -5.0)):
        assert view.value(cid) == pytest.approx(want), cid
    assert view.value("camera_cold_start") is True
