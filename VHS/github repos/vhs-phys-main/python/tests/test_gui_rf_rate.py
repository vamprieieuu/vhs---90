# CONTRACT
"""The RF sample rate in the GUI and on the command line: flags, look files, the control and its help."""
from __future__ import annotations

import subprocess

import pytest

from vhsgui import backend
from vhsgui.model import DEFAULT_RF_RATE, RF_RATES, Look, RunRequest


def _req(**look) -> RunRequest:
    return RunRequest(input="in.mp4", output="out.mp4", look=Look(**look))


def run_cli(binary, clip, out, *extra):
    cmd = [str(binary), "run", str(clip), str(out), "--size", "320x240", "--crf", "30", "-t", "1", "--audio", "none", *extra]
    return subprocess.run(cmd, capture_output=True, text=True, timeout=300)


def frame_count(path) -> int:
    out = subprocess.run(
        ["ffprobe", "-v", "error", "-select_streams", "v:0", "-count_frames", "-show_entries", "stream=nb_read_frames", "-of", "csv=p=0", str(path)],
        capture_output=True, text=True, check=True,
    ).stdout
    return int(out.strip())


def test_the_rates_and_the_default():
    assert RF_RATES == ("8x", "12x", "16x")
    assert DEFAULT_RF_RATE == "12x"
    assert Look().rf_rate == "12x"


def test_run_args_carry_the_rate_only_when_it_is_not_the_reference():
    assert "--rf-rate" not in backend.run_args("vhsphys", _req(), None)
    for rate in ("8x", "16x"):
        args = backend.run_args("vhsphys", _req(rf_rate=rate), None)
        assert args[args.index("--rf-rate") + 1] == rate


def test_look_files_keep_the_rate_and_refuse_an_unknown_one():
    assert backend.load_look(backend.save_look(Look(rf_rate="8x"))).rf_rate == "8x"
    assert backend.load_look(backend.save_look(Look())).rf_rate == "12x"
    with pytest.raises(Exception):
        backend.load_look('[look]\nrf_rate = "9x"\n')


@pytest.mark.parametrize("rate", ["8x", "16x"])
def test_the_cli_renders_at_every_rate(binary, clip, tmp_path, rate):
    r = run_cli(binary, clip, tmp_path / f"{rate}.mp4", "--rf-rate", rate)
    assert r.returncode == 0, r.stderr
    assert abs(frame_count(tmp_path / f"{rate}.mp4") - 30) <= 1


def test_the_cli_refuses_an_unknown_rate(binary, clip, tmp_path):
    r = run_cli(binary, clip, tmp_path / "x.mp4", "--rf-rate", "9x")
    assert r.returncode != 0
    assert "8x" in r.stderr and "16x" in r.stderr


def test_a_config_file_names_the_rate_and_the_flag_wins(binary, clip, tmp_path):
    cfg = tmp_path / "rate.toml"
    cfg.write_text('rf_rate = "16x"\n')
    r = run_cli(binary, clip, tmp_path / "c.mp4", "--config", str(cfg), "--frames", "3")
    assert r.returncode == 0, r.stderr
    r = run_cli(binary, clip, tmp_path / "f.mp4", "--config", str(cfg), "--rf-rate", "8x", "--frames", "3")
    assert r.returncode == 0, r.stderr
    assert "8x" in r.stderr


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


def test_the_window_has_the_rate_control_and_says_what_it_trades(window):
    from PySide6.QtWidgets import QComboBox

    combo = window.findChild(QComboBox, "field_rf_rate")
    assert combo is not None
    assert [combo.itemData(i) for i in range(combo.count())] == list(RF_RATES)
    assert combo.currentData() == DEFAULT_RF_RATE
    tip = combo.toolTip()
    for word in ("8x", "16x", "faster", "slower", "accura"):
        assert word in tip, f"the help does not mention {word!r}: {tip}"
    combo.setCurrentIndex(RF_RATES.index("8x"))
    assert window.look().rf_rate == "8x"
