# CONTRACT
"""The rest of the camera in the command line, its wizard and the window.

The command line takes the dynamic-beam-control Plumbicon, the colour layout, spectral taking, the
scan geometry and the tubes' temperature at switch-on. The window carries the layout, the
electronics, the target temperature, spectral taking and the geometry in its look, passes them to
the command line only when a tube is on and they differ from the engine's defaults, and keeps them
in look files.

A flag is checked against the `--set` of its config key: both must give the same picture, so the
flag reaches the right key with the right value, and a `--set` given with the flag must win.
"""
from __future__ import annotations

import shlex
import subprocess
from pathlib import Path

import pytest

from vhsgui import backend, model
from vhsgui.model import GuiError, Look, RunRequest

# Three frames are enough for every camera stage to read a picture, and keep each run short.
RUN_FRAMES = "3"
RUN_SIZE = "320x240"
# A clean deck, so the picture differs only by what the camera does.
RUN_PRESET = "pristine"
CLI_TIMEOUT_S = 300
WIZARD_TIMEOUT_S = 60

# The flags the window adds for the camera beyond the tube and the iris.
NEW_WINDOW_FLAGS = ("--camera-layout", "--camera-processing", "--camera-temperature", "--camera-spectral", "--camera-geometry")


# ---------------------------------------------------------------- command line


def run_cli(binary: Path, clip: Path, out: Path, *extra: str) -> subprocess.CompletedProcess[str]:
    cmd = [str(binary), "run", str(clip), str(out), "--size", RUN_SIZE, "--crf", "30", "--frames", RUN_FRAMES,
           "--audio", "none", "--preset", RUN_PRESET, *extra]
    return subprocess.run(cmd, capture_output=True, text=True, timeout=CLI_TIMEOUT_S)


def picture(binary: Path, clip: Path, ffmpeg: str, out: Path, *extra: str) -> bytes:
    """The decoded RGB frames of a run: the same arguments always give the same bytes."""
    r = run_cli(binary, clip, out, *extra)
    assert r.returncode == 0, r.stderr
    return subprocess.run([ffmpeg, "-v", "error", "-i", str(out), "-f", "rawvideo", "-pix_fmt", "rgb24", "-"],
                          capture_output=True, check=True, timeout=CLI_TIMEOUT_S).stdout


def test_the_dynamic_beam_control_plumbicon_is_a_tube(binary, clip, ffmpeg, tmp_path):
    flag = picture(binary, clip, ffmpeg, tmp_path / "flag.mp4", "--camera-tube", "plumbicon-dbc", "--iris-stops", "3")
    key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", "--camera-tube", "plumbicon", "--iris-stops", "3",
                  "--set", "camera.tube=plumbicon-dbc")
    plain = picture(binary, clip, ffmpeg, tmp_path / "plain.mp4", "--camera-tube", "plumbicon", "--iris-stops", "3")
    assert flag == key, "--camera-tube plumbicon-dbc is camera.tube=plumbicon-dbc"
    assert flag != plain, "the diode gun and its beam control change the picture"


def test_the_layout_flag_sets_the_layout_and_a_set_wins(binary, clip, ffmpeg, tmp_path):
    plain = picture(binary, clip, ffmpeg, tmp_path / "plain.mp4", "--camera-tube", "saticon")
    stripe = picture(binary, clip, ffmpeg, tmp_path / "stripe.mp4", "--camera-tube", "saticon", "--camera-layout", "stripe")
    key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", "--camera-tube", "saticon", "--set", "camera.layout=stripe")
    assert stripe == key, "--camera-layout stripe is camera.layout=stripe"
    assert stripe != plain, "one tube behind a stripe filter is not three tubes behind a prism"
    three = picture(binary, clip, ffmpeg, tmp_path / "three.mp4", "--camera-tube", "saticon", "--camera-layout", "three-tube")
    assert three == plain, "three-tube is the default layout"
    over = picture(binary, clip, ffmpeg, tmp_path / "over.mp4", "--camera-tube", "saticon", "--camera-layout", "stripe",
                   "--set", "camera.layout=three-tube")
    assert over == plain, "a --set of the same key wins over the flag"


def test_the_spectral_switch_sets_spectral_taking_and_a_set_wins(binary, clip, ffmpeg, tmp_path):
    plain = picture(binary, clip, ffmpeg, tmp_path / "plain.mp4", "--camera-tube", "plumbicon")
    flag = picture(binary, clip, ffmpeg, tmp_path / "flag.mp4", "--camera-tube", "plumbicon", "--camera-spectral")
    key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", "--camera-tube", "plumbicon", "--set", "camera.spectral=true")
    assert flag == key, "--camera-spectral is camera.spectral=true"
    assert flag != plain, "the prism and the targets' responses change the colours"
    over = picture(binary, clip, ffmpeg, tmp_path / "over.mp4", "--camera-tube", "plumbicon", "--camera-spectral",
                   "--set", "camera.spectral=false")
    assert over == plain, "a --set of the same key wins over the switch"


def test_the_geometry_switch_sets_the_scan_geometry_and_a_set_wins(binary, clip, ffmpeg, tmp_path):
    plain = picture(binary, clip, ffmpeg, tmp_path / "plain.mp4", "--camera-tube", "plumbicon")
    flag = picture(binary, clip, ffmpeg, tmp_path / "flag.mp4", "--camera-tube", "plumbicon", "--camera-geometry")
    key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", "--camera-tube", "plumbicon", "--set", "camera.geometry=true")
    assert flag == key, "--camera-geometry is camera.geometry=true"
    assert flag != plain, "shading, flare and registration change the picture"
    over = picture(binary, clip, ffmpeg, tmp_path / "over.mp4", "--camera-tube", "plumbicon", "--camera-geometry",
                   "--set", "camera.geometry=false")
    assert over == plain, "a --set of the same key wins over the switch"


def test_the_cold_start_flag_sets_the_switch_on_temperature_and_a_set_wins(binary, clip, ffmpeg, tmp_path):
    # below freezing, so the flag must take a negative number
    flag = picture(binary, clip, ffmpeg, tmp_path / "flag.mp4", "--camera-tube", "vidicon", "--camera-cold-start", "-10")
    key = picture(binary, clip, ffmpeg, tmp_path / "key.mp4", "--camera-tube", "vidicon", "--set", "camera.cold_start_c=-10.0")
    assert flag == key, "--camera-cold-start -10 is camera.cold_start_c=-10.0"
    # a value the engine cannot read reaches it only if the --set comes after the flag
    r = run_cli(binary, clip, tmp_path / "bad.mp4", "--camera-tube", "vidicon", "--camera-cold-start", "5",
                "--set", "camera.cold_start_c=bogus")
    assert r.returncode != 0
    assert "camera.cold_start_c" in r.stderr, r.stderr


@pytest.mark.parametrize(
    ("args", "message"),
    [
        (("--camera-layout", "prism"), "invalid value 'prism' for '--camera-layout"),
        (("--camera-cold-start", "warm"), "invalid value 'warm' for '--camera-cold-start"),
        (("--camera-spectral=yes",), "unexpected value 'yes' for '--camera-spectral'"),
        (("--camera-geometry=yes",), "unexpected value 'yes' for '--camera-geometry'"),
    ],
)
def test_a_bad_camera_flag_is_named_in_the_error(binary, clip, tmp_path, args, message):
    r = run_cli(binary, clip, tmp_path / "x.mp4", "--camera-tube", "plumbicon", *args)
    assert r.returncode != 0
    assert message in r.stderr, r.stderr


def test_a_bad_tube_lists_the_dynamic_beam_control_plumbicon(binary, clip, tmp_path):
    r = run_cli(binary, clip, tmp_path / "x.mp4", "--camera-tube", "plumbicon-dbx")
    assert r.returncode != 0
    assert "invalid value 'plumbicon-dbx' for '--camera-tube" in r.stderr, r.stderr
    assert "plumbicon-dbc" in r.stderr, "clap lists the possible values"


# ---------------------------------------------------------------- wizard


def wizard_command(binary: Path, answers: list[str]) -> list[str]:
    """The run command the wizard prints, as arguments, when its offer to run it is declined."""
    r = subprocess.run([str(binary), "wizard"], input="\n".join(answers) + "\n", capture_output=True, text=True,
                       timeout=WIZARD_TIMEOUT_S)
    assert r.returncode == 0, r.stdout + r.stderr
    lines = r.stdout.splitlines()
    i = next(n for n, line in enumerate(lines) if line.startswith("Generated command:"))
    return shlex.split(lines[i + 1])


def value_of(cmd: list[str], flag: str) -> str:
    assert flag in cmd, cmd
    return cmd[cmd.index(flag) + 1]


def test_the_wizard_offers_dynamic_beam_control_and_the_stripe_layout(binary, clip, tmp_path):
    out = tmp_path / "w.mp4"
    # input, output, NTSC, SP, pristine, tube 5 (Plumbicon+DBC), iris 3, layout 2 (stripe), no sound, 1 generation, do not run
    cmd = wizard_command(binary, [str(clip), str(out), "1", "1", "2", "5", "3", "2", "3", "1", "n"])
    assert value_of(cmd, "--camera-tube") == "plumbicon-dbc"
    assert float(value_of(cmd, "--iris-stops")) == 3.0
    assert value_of(cmd, "--camera-layout") == "stripe"
    assert value_of(cmd, "--audio") == "none", "the layout is one question, right after the iris"


def test_the_wizard_leaves_the_default_layout_off_the_command(binary, clip, tmp_path):
    out = tmp_path / "w.mp4"
    # tube 6 is the vidicon, behind the four Plumbicons; Enter takes the iris and the layout defaults
    cmd = wizard_command(binary, [str(clip), str(out), "1", "1", "2", "6", "", "", "3", "1", "n"])
    assert value_of(cmd, "--camera-tube") == "vidicon"
    assert "--camera-layout" not in cmd
    assert value_of(cmd, "--audio") == "none"


def test_the_wizard_asks_no_camera_question_without_a_tube(binary, clip, tmp_path):
    out = tmp_path / "w.mp4"
    cmd = wizard_command(binary, [str(clip), str(out), "1", "1", "2", "1", "3", "1", "n"])
    assert "--camera-tube" not in cmd and "--camera-layout" not in cmd
    assert value_of(cmd, "--audio") == "none", "the answer after the tube is the sound"


# ---------------------------------------------------------------- the window's look and command line


def _req(**look: object) -> RunRequest:
    return RunRequest(input="in.mp4", output="out.mp4", look=Look(**look))


def test_the_dynamic_beam_control_plumbicon_sits_with_the_other_plumbicons():
    assert model.CAMERA_TUBES[:5] == ("off", "plumbicon", "plumbicon-bias", "plumbicon-act", "plumbicon-dbc")
    assert model.CAMERA_TUBES[5:] == ("vidicon", "newvicon", "saticon", "chalnicon")
    args = backend.run_args("vhsphys", _req(camera_tube="plumbicon-dbc", iris_stops=3.0), None)
    assert value_of(args, "--camera-tube") == "plumbicon-dbc"
    assert backend.load_look(backend.save_look(Look(camera_tube="plumbicon-dbc"))).camera_tube == "plumbicon-dbc"


def test_the_camera_settings_start_at_the_engine_defaults():
    assert model.CAMERA_LAYOUTS == ("three-tube", "stripe")
    assert model.CAMERA_PROCESSING == ("studio", "consumer")
    assert (model.DEFAULT_CAMERA_LAYOUT, model.DEFAULT_CAMERA_PROCESSING) == ("three-tube", "studio")
    assert model.DEFAULT_CAMERA_TEMPERATURE_C == 25.0
    lo, hi = model.CAMERA_TEMPERATURE_RANGE
    assert lo < model.DEFAULT_CAMERA_TEMPERATURE_C < hi
    look = Look()
    assert (look.camera_layout, look.camera_processing, look.camera_temperature_c, look.camera_spectral, look.camera_geometry) == (
        "three-tube", "studio", 25.0, False, False)


def test_run_args_carry_no_camera_setting_without_a_tube():
    args = backend.run_args("vhsphys", _req(camera_layout="stripe", camera_processing="consumer", camera_temperature_c=40.0,
                                            camera_spectral=True, camera_geometry=True), None)
    for flag in NEW_WINDOW_FLAGS:
        assert flag not in args, flag


def test_run_args_carry_a_camera_setting_only_when_it_differs_from_the_default():
    plain = backend.run_args("vhsphys", _req(camera_tube="newvicon"), None)
    for flag in NEW_WINDOW_FLAGS:
        assert flag not in plain, flag
    assert value_of(backend.run_args("vhsphys", _req(camera_tube="newvicon", camera_layout="stripe"), None), "--camera-layout") == "stripe"
    assert value_of(backend.run_args("vhsphys", _req(camera_tube="newvicon", camera_processing="consumer"), None), "--camera-processing") == "consumer"
    warm = backend.run_args("vhsphys", _req(camera_tube="newvicon", camera_temperature_c=37.5), None)
    assert float(value_of(warm, "--camera-temperature")) == 37.5
    for name, flag in (("camera_spectral", "--camera-spectral"), ("camera_geometry", "--camera-geometry")):
        args = backend.run_args("vhsphys", _req(camera_tube="newvicon", **{name: True}), None)
        assert args.count(flag) == 1, flag
        following = args[args.index(flag) + 1 :]
        assert not following or following[0].startswith("-"), f"{flag} is a switch and takes no value"


def test_the_window_s_camera_flags_are_ones_the_command_line_takes(binary, clip, tmp_path):
    look = Look(camera_tube="saticon", iris_stops=1.0, camera_layout="stripe", camera_processing="consumer",
                camera_temperature_c=35.0, camera_spectral=True, camera_geometry=True)
    req = RunRequest(str(clip), str(tmp_path / "out.mp4"), look, duration=0.2, size=(320, 240))
    args = backend.run_args(binary, req, None)
    for flag in NEW_WINDOW_FLAGS:
        assert flag in args, flag
    state = backend.Job(args).start().wait(timeout=CLI_TIMEOUT_S)
    assert state.returncode == 0, state.log


def test_look_files_keep_the_camera_settings():
    look = Look(camera_tube="chalnicon", iris_stops=1.5, camera_layout="stripe", camera_processing="consumer",
                camera_temperature_c=41.5, camera_spectral=True, camera_geometry=True)
    assert backend.load_look(backend.save_look(look)) == look
    assert backend.load_look(backend.save_look(Look())) == Look()


def test_an_older_look_file_loads_with_the_default_camera_settings():
    old = '[look]\npreset = "worn"\nseed = 1\ncamera_tube = "plumbicon"\niris_stops = 2.0\nadvanced = ""\n'
    look = backend.load_look(old)
    assert (look.camera_tube, look.iris_stops) == ("plumbicon", 2.0)
    assert (look.camera_layout, look.camera_processing, look.camera_temperature_c, look.camera_spectral, look.camera_geometry) == (
        "three-tube", "studio", 25.0, False, False)
    # a whole number written by hand is still a temperature
    assert backend.load_look("[look]\ncamera_temperature_c = 30\n").camera_temperature_c == 30.0


@pytest.mark.parametrize(
    "line",
    [
        'camera_layout = "prism"',
        'camera_processing = "broadcast"',
        "camera_temperature_c = 500.0",
        "camera_temperature_c = -40.0",
        'camera_temperature_c = "warm"',
        "camera_temperature_c = true",
        'camera_spectral = "yes"',
        "camera_geometry = 1",
    ],
)
def test_look_files_refuse_bad_camera_settings(line):
    with pytest.raises(GuiError):
        backend.load_look(f"[look]\n{line}\n")


@pytest.mark.parametrize(
    "settings",
    [{"camera_layout": "prism"}, {"camera_processing": "broadcast"}, {"camera_temperature_c": 500.0}],
)
def test_a_look_with_a_bad_camera_setting_is_not_saved(settings):
    with pytest.raises(GuiError):
        backend.save_look(Look(camera_tube="plumbicon", **settings))


# ---------------------------------------------------------------- the classic window


@pytest.fixture
def window(binary, ffmpeg, ffprobe, tmp_path):
    from PySide6.QtWidgets import QApplication

    from vhsgui.app import MainWindow

    app = QApplication.instance() or QApplication([])
    win = MainWindow(binary=binary, workdir=tmp_path / "work")
    win.show()
    yield win
    win.close()
    app.processEvents()


def _camera_controls(window):
    from PySide6.QtWidgets import QCheckBox, QComboBox, QDoubleSpinBox

    found = {
        "tube": window.findChild(QComboBox, "field_camera_tube"),
        "layout": window.findChild(QComboBox, "field_camera_layout"),
        "processing": window.findChild(QComboBox, "field_camera_processing"),
        "temperature": window.findChild(QDoubleSpinBox, "field_camera_temperature_c"),
        "spectral": window.findChild(QCheckBox, "field_camera_spectral"),
        "geometry": window.findChild(QCheckBox, "field_camera_geometry"),
    }
    for name, widget in found.items():
        assert widget is not None, f"no control for the camera {name}"
    return found


def test_the_window_has_the_camera_controls(window):
    c = _camera_controls(window)
    # the item data are the values the command line takes; the texts are for people
    assert [c["layout"].itemData(i) for i in range(c["layout"].count())] == list(model.CAMERA_LAYOUTS)
    assert [c["processing"].itemData(i) for i in range(c["processing"].count())] == list(model.CAMERA_PROCESSING)
    assert (c["temperature"].minimum(), c["temperature"].maximum()) == model.CAMERA_TEMPERATURE_RANGE
    assert c["temperature"].value() == model.DEFAULT_CAMERA_TEMPERATURE_C
    assert not c["spectral"].isChecked() and not c["geometry"].isChecked()
    look = window.look()
    assert (look.camera_layout, look.camera_processing, look.camera_temperature_c, look.camera_spectral, look.camera_geometry) == (
        "three-tube", "studio", 25.0, False, False)


def test_the_camera_controls_need_a_tube(window):
    c = _camera_controls(window)
    settings = ("layout", "processing", "temperature", "spectral", "geometry")
    assert c["tube"].currentData() == "off"
    for name in settings:
        assert not c[name].isEnabled(), f"the camera {name} has nothing to act on without a tube"
    c["tube"].setCurrentIndex(c["tube"].findData("plumbicon-dbc"))
    for name in settings:
        assert c[name].isEnabled(), name
    c["tube"].setCurrentIndex(c["tube"].findData("off"))
    assert not c["layout"].isEnabled()


def test_the_camera_controls_drive_the_look(window):
    c = _camera_controls(window)
    c["tube"].setCurrentIndex(c["tube"].findData("saticon"))
    c["layout"].setCurrentIndex(c["layout"].findData("stripe"))
    c["processing"].setCurrentIndex(c["processing"].findData("consumer"))
    c["temperature"].setValue(40.0)
    c["spectral"].setChecked(True)
    c["geometry"].setChecked(True)
    look = window.look()
    assert (look.camera_tube, look.camera_layout, look.camera_processing, look.camera_temperature_c, look.camera_spectral,
            look.camera_geometry) == ("saticon", "stripe", "consumer", 40.0, True, True)
    assert window.preview_stale is True


def test_the_window_keeps_the_camera_settings_in_a_look_file(window, tmp_path):
    c = _camera_controls(window)
    c["tube"].setCurrentIndex(c["tube"].findData("newvicon"))
    c["layout"].setCurrentIndex(c["layout"].findData("stripe"))
    c["processing"].setCurrentIndex(c["processing"].findData("consumer"))
    c["temperature"].setValue(33.0)
    c["spectral"].setChecked(True)
    c["geometry"].setChecked(True)
    saved = window.look()
    path = tmp_path / "camera.look.toml"
    window.save_look(str(path))
    assert path.is_file()
    c["layout"].setCurrentIndex(c["layout"].findData("three-tube"))
    c["processing"].setCurrentIndex(c["processing"].findData("studio"))
    c["temperature"].setValue(25.0)
    c["spectral"].setChecked(False)
    c["geometry"].setChecked(False)
    c["tube"].setCurrentIndex(c["tube"].findData("off"))
    window.load_look(str(path))
    assert window.look() == saved
    assert c["layout"].isEnabled(), "a loaded tube enables its settings"
