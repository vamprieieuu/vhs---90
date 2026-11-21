# CONTRACT
"""The camera page of the schematic: a third drawing beside the circuit and the outside world.

It holds every setting of the camera (tube, iris, layout, electronics, tube temperature, spectral colour,
scan geometry, beam current, blooming, cold start, lens cap, beam off), drawn as the light travels: lens and iris, colour split, pickup tubes, head amplifier,
processing amplifier, encoder, TV. The layout switch changes the drawing itself (a prism and three tubes,
or a stripe filter and one tube). Without a tube the camera's other lines are inert, as the window's own
camera controls are disabled. The surface fixed here, beside the one in test_schematic_view.py:

    SchematicView.visible_glyphs() -> list[str]   the glyph names of the instances drawn now, in layout order
    SchematicView.set_enabled(id, on) / is_enabled(id)
                                                  a disabled line is drawn dim and ignores the mouse, the wheel
                                                  and the keys (no valueChanged, no resetRequested); hovering it
                                                  still shows its help; set_value still works
    an instance in layout.json may carry `when: {choice control: [values]}`, as a label may
    MainWindow.camera                             the camera page's SchematicView, object name "cameraView"
    "cameraButton"                                the page button, beside "circuitButton" and "outsideButton"
    python -m vhsgui --page camera                starts on the camera page
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
import time
from pathlib import Path

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

pytest.importorskip("PySide6")
from PySide6.QtCore import QEvent, QPoint, QPointF, Qt  # noqa: E402
from PySide6.QtGui import QImage, QKeyEvent, QMouseEvent, QWheelEvent  # noqa: E402
from PySide6.QtWidgets import QAbstractButton, QApplication, QCheckBox, QComboBox, QDoubleSpinBox, QWidget  # noqa: E402

from vhsgui import model  # noqa: E402
from vhsgui.schematic.view import SchematicView  # noqa: E402

PYTHON_DIR = Path(__file__).resolve().parents[1]
DATA = PYTHON_DIR / "vhsgui" / "schematic"
CAMERA_DIR = DATA / "camera"
CW, CH = 1180, 620
LEFT, NOBTN, NOMOD = Qt.MouseButton.LeftButton, Qt.MouseButton.NoButton, Qt.KeyboardModifier.NoModifier
CAMERA_IDS = {"camera_tube", "iris_stops", "camera_layout", "camera_processing", "camera_temperature_c", "camera_spectral", "camera_geometry",
              "camera_beam_ratio", "camera_blooming", "camera_cold_start", "camera_cold_start_c", "camera_idle_s", "camera_beam_off_s"}
# The camera's lines that have nothing to act on without a tube (the tube itself always works).
NEEDS_A_TUBE = ("iris_stops", "camera_layout", "camera_processing", "camera_temperature_c", "camera_spectral", "camera_geometry",
                "camera_beam_ratio", "camera_blooming", "camera_cold_start", "camera_idle_s", "camera_beam_off_s")
# The switch-on temperature works only while the cold-start switch is on as well.
NEEDS_COLD_START = "camera_cold_start_c"
THREE_TUBE_GLYPHS = {"prism", "tubes_three"}
STRIPE_GLYPHS = {"stripe_filter", "tube_one"}


@pytest.fixture(scope="session")
def app():
    return QApplication.instance() or QApplication([])


@pytest.fixture
def page(app):
    v = SchematicView(data_dir=CAMERA_DIR)
    v.resize(CW, CH)
    yield v
    v.hide()
    v.deleteLater()


def mouse(view, kind, pos, button=LEFT, buttons=None):
    if buttons is None:
        buttons = NOBTN if kind == QEvent.Type.MouseButtonRelease else button
    p = QPointF(pos)
    QApplication.sendEvent(view, QMouseEvent(kind, p, QPointF(view.mapToGlobal(p.toPoint())), button, buttons, NOMOD))


def click(view, pos):
    mouse(view, QEvent.Type.MouseButtonPress, pos)
    mouse(view, QEvent.Type.MouseButtonRelease, pos)


def double_click(view, pos):
    click(view, pos)
    mouse(view, QEvent.Type.MouseButtonDblClick, pos)
    mouse(view, QEvent.Type.MouseButtonRelease, pos)


def hover(view, pos):
    mouse(view, QEvent.Type.MouseMove, pos, NOBTN, NOBTN)


def wheel(view, pos, up=True):
    p = QPointF(pos)
    QApplication.sendEvent(view, QWheelEvent(p, QPointF(view.mapToGlobal(p.toPoint())), QPoint(0, 0), QPoint(0, 120 if up else -120), NOBTN, NOMOD, Qt.ScrollPhase.NoScrollPhase, False))


def key(view, k):
    QApplication.sendEvent(view, QKeyEvent(QEvent.Type.KeyPress, k, NOMOD))
    QApplication.sendEvent(view, QKeyEvent(QEvent.Type.KeyRelease, k, NOMOD))


def at(view, cid, frac=0.5):
    r = view.control_rect(cid)
    return QPointF(r.left() + frac * r.width(), r.top() + 0.4 * r.height())


def rgb(image: QImage) -> np.ndarray:
    img = image.convertToFormat(QImage.Format.Format_RGB32)
    w, h = img.width(), img.height()
    raw = np.frombuffer(img.constBits(), np.uint8).reshape(h, img.bytesPerLine() // 4, 4)[:, :w]
    return raw[:, :, [2, 1, 0]].astype(np.float64)


def row_light(view, cid) -> float:
    """How much light the row of `cid` gives, rendered at canvas scale without effects."""
    a = rgb(view.render_to_image(CW))
    r = view.control_rect(cid)
    return float(a[int(r.top()) : int(r.bottom()) + 1, int(r.left()) : int(r.right()) + 1].sum())


# ---------------------------------------------------------------- the page on its own


def test_the_camera_page_holds_every_camera_setting_and_nothing_else(page):
    assert set(page.control_ids()) == CAMERA_IDS
    assert page.choices("camera_tube") == list(model.CAMERA_TUBES)
    assert page.choices("camera_layout") == list(model.CAMERA_LAYOUTS)
    assert page.choices("camera_processing") == list(model.CAMERA_PROCESSING)
    circuit, outside = SchematicView(), SchematicView(data_dir=DATA / "outside")
    assert not set(page.control_ids()) & (set(circuit.control_ids()) | set(outside.control_ids())), "one line per setting"
    assert page.screen_rect() is not None, "the page has its own TV"
    circuit.deleteLater()
    outside.deleteLater()


def test_the_circuit_keeps_its_camera_drawing_and_points_to_the_page(app):
    circuit = SchematicView()
    assert not CAMERA_IDS & set(circuit.control_ids())
    assert "pickup_tube" in circuit.visible_glyphs(), "the camera still feeds the recorder on the circuit"
    assert any("CAMERA PAGE" in text for text in circuit.visible_labels())
    circuit.deleteLater()


def test_the_layout_switch_redraws_the_colour_split_and_the_tubes(page):
    page.set_value("camera_layout", "three-tube")
    shown = set(page.visible_glyphs())
    assert THREE_TUBE_GLYPHS <= shown and not STRIPE_GLYPHS & shown
    three = rgb(page.render_to_image(CW))
    assert any("PRISM" in t for t in page.visible_labels())
    page.set_value("camera_layout", "stripe")
    shown = set(page.visible_glyphs())
    assert STRIPE_GLYPHS <= shown and not THREE_TUBE_GLYPHS & shown
    assert any("STRIPE" in t for t in page.visible_labels()) and not any("PRISM" in t for t in page.visible_labels())
    stripe = rgb(page.render_to_image(CW))
    layout = json.loads((CAMERA_DIR / "layout.json").read_text())
    blocks = {b["id"]: b for b in layout["blocks"]}
    for name in ("split", "tubes"):
        b = blocks[name]
        region = (slice(b["y"] + 20, b["y"] + 130), slice(b["x"], b["x"] + b["w"]))
        assert np.abs(three[region] - stripe[region]).sum() > 0, f"the {name} block is drawn differently"
    assert page.visible_glyphs().count("lens_iris") == 1, "drawings without a condition stay"


@pytest.mark.parametrize("when", [{"no_such_line": ["x"]}, {"camera_layout": ["prism"]}, {"camera_spectral": ["true"]}])
def test_a_drawing_s_condition_must_name_a_choice_of_its_page(app, tmp_path, when):
    layout = json.loads((CAMERA_DIR / "layout.json").read_text())
    layout["instances"][0]["when"] = when
    (tmp_path / "layout.json").write_text(json.dumps(layout))
    (tmp_path / "glyphs.json").write_text((CAMERA_DIR / "glyphs.json").read_text())
    with pytest.raises(ValueError, match="when"):
        SchematicView(data_dir=tmp_path)


def test_every_line_starts_enabled_and_unknown_lines_are_errors(page):
    assert all(page.is_enabled(cid) for cid in page.control_ids())
    with pytest.raises(KeyError):
        page.set_enabled("no_such_line", False)
    with pytest.raises(KeyError):
        page.is_enabled("no_such_line")


def test_a_disabled_line_is_inert_but_explains_itself(page):
    page.show()
    page.set_help({"iris_stops": "The iris. Needs a tube."})
    page.set_value("iris_stops", 1.0)
    lit = row_light(page, "iris_stops")
    seen, resets = [], []
    page.valueChanged.connect(lambda cid, v: seen.append(cid))
    page.resetRequested.connect(resets.append)
    page.set_enabled("iris_stops", False)
    assert not page.is_enabled("iris_stops")
    assert row_light(page, "iris_stops") < lit, "a disabled line is drawn dimmer"
    click(page, at(page, "iris_stops", 0.95))
    wheel(page, at(page, "iris_stops"))
    page.set_current_control("iris_stops")
    for k in (Qt.Key.Key_Right, Qt.Key.Key_End, Qt.Key.Key_Home):
        key(page, k)
    double_click(page, at(page, "iris_stops", 0.2))
    assert seen == [] and resets == [] and page.value("iris_stops") == 1.0
    hover(page, at(page, "iris_stops"))
    assert page.hint_text() == "The iris. Needs a tube.", "the help says why nothing happens"
    page.set_value("iris_stops", 3.0)
    assert page.value("iris_stops") == 3.0, "the window still mirrors its value"
    page.set_enabled("iris_stops", True)
    click(page, at(page, "iris_stops", 0.95))
    assert seen == ["iris_stops"] and page.value("iris_stops") > 3.0
    assert row_light(page, "iris_stops") > 0


def test_a_disabled_toggle_and_choice_do_not_flip(page):
    page.show()
    for cid in ("camera_spectral", "camera_layout"):
        page.set_enabled(cid, False)
        before = page.value(cid)
        click(page, at(page, cid))
        page.set_current_control(cid)
        key(page, Qt.Key.Key_Space)
        key(page, Qt.Key.Key_Right)
        assert page.value(cid) == before, cid


# ---------------------------------------------------------------- the page in the window


@pytest.fixture
def window(app, binary, ffmpeg, ffprobe, tmp_path):
    from vhsgui.app import MainWindow

    win = MainWindow(binary=binary, workdir=tmp_path / "work")
    win.resize(1280, 900)
    win.show()
    yield win
    win.close()
    app.processEvents()


def child(win, cls, name):
    found = win.findChild(cls, name)
    assert found is not None, f"no {cls.__name__} named {name}"
    return found


def camera_look(window) -> dict:
    look = window.look()
    return {cid: getattr(look, cid) for cid in CAMERA_IDS}


def assert_camera_mirrors(window):
    want = camera_look(window)
    for cid, value in want.items():
        got = window.camera.value(cid)
        if isinstance(value, float):
            assert got == pytest.approx(value, abs=1e-6), cid
        else:
            assert got == value, cid


def to_camera_page(app, window) -> SchematicView:
    window.set_schematic_mode(True)
    window.set_schematic_page("camera")
    window.resize(1280, 900)
    app.processEvents()
    return window.camera


def test_the_camera_page_is_a_page_of_the_schematic(app, window):
    view = child(window, SchematicView, "cameraView")
    assert window.camera is view and set(view.control_ids()) == CAMERA_IDS
    button = child(window, QAbstractButton, "cameraButton")
    assert button.isCheckable() and not button.isVisibleTo(window) and not view.isVisibleTo(window)
    window.set_schematic_mode(True)
    app.processEvents()
    assert button.isVisibleTo(window) and window.schematic_page == "circuit" and not view.isVisibleTo(window)
    button.click()
    app.processEvents()
    assert window.schematic_page == "camera" and button.isChecked()
    assert view.isVisibleTo(window) and not window.schematic.isVisibleTo(window) and not window.outside.isVisibleTo(window)
    assert view.is_ticking() and view.boot_progress() < 1.0, "the page plays its own start-up the first time"
    view.skip_boot()
    child(window, QAbstractButton, "circuitButton").click()
    assert not button.isChecked() and window.schematic_page == "circuit" and not view.is_ticking()
    button.click()
    app.processEvents()
    assert view.boot_progress() == 1.0, "and only the first time"
    with pytest.raises(ValueError):
        window.set_schematic_page("nowhere")
    window.set_schematic_mode(False)
    assert not button.isVisibleTo(window) and not view.isVisibleTo(window)


def test_the_camera_page_starts_as_the_window_does_and_follows_its_editors(app, window):
    assert_camera_mirrors(window)
    assert not any(window.camera.is_edited(cid) for cid in CAMERA_IDS), "the window's defaults are not edits"
    tube = child(window, QComboBox, "field_camera_tube")
    tube.setCurrentIndex(tube.findData("saticon"))
    layout = child(window, QComboBox, "field_camera_layout")
    layout.setCurrentIndex(layout.findData("stripe"))
    processing = child(window, QComboBox, "field_camera_processing")
    processing.setCurrentIndex(processing.findData("consumer"))
    child(window, QDoubleSpinBox, "field_camera_temperature_c").setValue(40.0)
    child(window, QCheckBox, "field_camera_spectral").setChecked(True)
    child(window, QCheckBox, "field_camera_geometry").setChecked(True)
    child(window, QDoubleSpinBox, "field_iris_stops").setValue(2.5)
    child(window, QDoubleSpinBox, "field_camera_beam_ratio").setValue(3.0)
    child(window, QDoubleSpinBox, "field_camera_blooming").setValue(1.0)
    child(window, QCheckBox, "field_camera_cold_start").setChecked(True)
    child(window, QDoubleSpinBox, "field_camera_cold_start_c").setValue(-5.0)
    child(window, QDoubleSpinBox, "field_camera_idle_s").setValue(60.0)
    child(window, QDoubleSpinBox, "field_camera_beam_off_s").setValue(1.5)
    assert_camera_mirrors(window)
    assert STRIPE_GLYPHS <= set(window.camera.visible_glyphs())
    assert all(window.camera.is_edited(cid) for cid in CAMERA_IDS), "every camera line now differs from its default"


def test_without_a_tube_the_camera_lines_are_inert(app, window):
    view = to_camera_page(app, window)
    assert window.look().camera_tube == "off"
    for cid in (*NEEDS_A_TUBE, NEEDS_COLD_START):
        assert not view.is_enabled(cid), f"{cid} has nothing to act on without a tube"
    assert view.is_enabled("camera_tube")
    before = window.look()
    for cid in NEEDS_A_TUBE:
        click(view, at(view, cid, 0.95))
    assert window.look() == before, "nothing changed"
    click(view, at(view, "camera_tube"))
    assert window.look().camera_tube == "plumbicon"
    for cid in NEEDS_A_TUBE:
        assert view.is_enabled(cid), cid
    tube = child(window, QComboBox, "field_camera_tube")
    tube.setCurrentIndex(tube.findData("off"))
    assert not view.is_enabled("iris_stops"), "the editors switch the lines too"


def test_the_camera_page_drives_the_editors(app, window):
    view = to_camera_page(app, window)
    tube = child(window, QComboBox, "field_camera_tube")
    tube.setCurrentIndex(tube.findData("newvicon"))
    click(view, at(view, "camera_layout"))
    assert child(window, QComboBox, "field_camera_layout").currentData() == "stripe"
    click(view, at(view, "camera_processing"))
    assert child(window, QComboBox, "field_camera_processing").currentData() == "consumer"
    click(view, at(view, "camera_spectral"))
    assert child(window, QCheckBox, "field_camera_spectral").isChecked()
    click(view, at(view, "camera_geometry"))
    assert child(window, QCheckBox, "field_camera_geometry").isChecked()
    r = view.control_rect("camera_temperature_c")
    mouse(view, QEvent.Type.MouseButtonPress, QPointF(r.right() - 1, r.top() + 6))
    mouse(view, QEvent.Type.MouseButtonRelease, QPointF(r.right() - 1, r.top() + 6))
    assert child(window, QDoubleSpinBox, "field_camera_temperature_c").value() >= model.CAMERA_TEMPERATURE_RANGE[1] - 1.0
    r = view.control_rect("iris_stops")
    mouse(view, QEvent.Type.MouseButtonPress, QPointF(r.right() - 1, r.top() + 6))
    mouse(view, QEvent.Type.MouseButtonRelease, QPointF(r.right() - 1, r.top() + 6))
    assert child(window, QDoubleSpinBox, "field_iris_stops").value() >= model.IRIS_STOPS_RANGE[1] - 0.5
    look = window.look()
    assert (look.camera_layout, look.camera_processing, look.camera_spectral, look.camera_geometry) == ("stripe", "consumer", True, True)
    assert window.preview_stale is True
    assert_camera_mirrors(window)


def test_a_double_click_puts_a_camera_line_back_to_its_default(app, window):
    view = to_camera_page(app, window)
    tube = child(window, QComboBox, "field_camera_tube")
    tube.setCurrentIndex(tube.findData("vidicon"))
    temp = child(window, QDoubleSpinBox, "field_camera_temperature_c")
    temp.setValue(45.0)
    layout = child(window, QComboBox, "field_camera_layout")
    layout.setCurrentIndex(layout.findData("stripe"))
    double_click(view, at(view, "camera_temperature_c", 0.5))
    assert temp.value() == pytest.approx(model.DEFAULT_CAMERA_TEMPERATURE_C)
    assert layout.currentData() == "stripe", "only the line that was clicked"
    double_click(view, at(view, "camera_layout"))
    assert layout.currentData() == model.DEFAULT_CAMERA_LAYOUT
    assert_camera_mirrors(window)


def test_a_loaded_look_shows_on_the_camera_page(app, window, tmp_path):
    to_camera_page(app, window)
    tube = child(window, QComboBox, "field_camera_tube")
    tube.setCurrentIndex(tube.findData("chalnicon"))
    child(window, QCheckBox, "field_camera_geometry").setChecked(True)
    path = tmp_path / "camera.toml"
    window.save_look(str(path))
    tube.setCurrentIndex(tube.findData("off"))
    child(window, QCheckBox, "field_camera_geometry").setChecked(False)
    window.load_look(str(path))
    assert window.camera.value("camera_tube") == "chalnicon" and window.camera.value("camera_geometry") is True
    assert window.camera.is_enabled("camera_geometry")
    assert_camera_mirrors(window)


def test_every_camera_line_has_a_help_text_from_the_window(app, window):
    view = to_camera_page(app, window)
    default = view.hint_text()
    for cid in sorted(CAMERA_IDS):
        hover(view, view.control_rect(cid).center())
        assert view.hint_text() not in ("", default), cid


def test_the_camera_page_shows_the_picture_on_its_tv(app, window, clip):
    window.open_input(str(clip))
    preview = child(window, QWidget, "previewView")
    deadline = time.time() + 90
    while preview.image() is None and time.time() < deadline:
        app.processEvents()
        time.sleep(0.02)
    assert preview.image() is not None, "the first frame never came"
    assert window.camera.screen_image() == preview.image()


# ---------------------------------------------------------------- the module entry point


def _run_module(*args: str, timeout: float = 120):
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", PYTHONPATH=str(PYTHON_DIR))
    return subprocess.run([sys.executable, "-m", "vhsgui", *args], env=env, capture_output=True, text=True, timeout=timeout)


def test_the_module_can_start_on_the_camera_page(binary, tmp_path):
    png = tmp_path / "camera.png"
    result = _run_module("--schematic", "--page", "camera", "--screenshot", str(png))
    assert result.returncode == 0, result.stderr
    image = QImage(str(png)).convertToFormat(QImage.Format.Format_RGB32)
    assert image.width() >= 1000 and image.height() >= 700
    raw = np.frombuffer(image.constBits(), np.uint8).reshape(image.height(), image.bytesPerLine() // 4, 4)[:, : image.width()].astype(float)
    r, g, b = raw[..., 2], raw[..., 1], raw[..., 0]
    assert ((g > 120) & (g > 1.6 * r) & (g > 1.3 * b)).mean() > 0.01, "the camera page is on the picture and fully drawn"
    assert _run_module("--page", "nowhere", "--screenshot", str(tmp_path / "x.png")).returncode != 0
