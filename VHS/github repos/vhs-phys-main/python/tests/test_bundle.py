# CONTRACT
"""Acceptance tests for the packaged desktop app (a frozen PyInstaller build).

In a packaged app the window finds the `vhsphys` binary that ships with it, beside its own
executable or in the bundle's internal folder, after an explicit `--binary` and `$VHSPHYS_BIN`
and before a source checkout's `target/` and the PATH. It also puts its own folders at the front
of the PATH, so an ffmpeg placed beside the app is found by the window and by `vhsphys`.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

import pytest

from vhsgui import __main__ as entry
from vhsgui import backend

EXE_SUFFIX = ".exe" if os.name == "nt" else ""


def _fake_exe(path: Path) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("#!/bin/sh\nexit 0\n")
    path.chmod(0o755)
    return path


@pytest.fixture
def frozen_app(tmp_path, monkeypatch):
    """A packaged app in `tmp_path/app` with an internal folder, run without $VHSPHYS_BIN."""
    app = tmp_path / "app"
    internal = app / "_internal"
    internal.mkdir(parents=True)
    monkeypatch.setattr(sys, "frozen", True, raising=False)
    monkeypatch.setattr(sys, "executable", str(_fake_exe(app / ("vhs-phys" + EXE_SUFFIX))))
    monkeypatch.setattr(sys, "_MEIPASS", str(internal), raising=False)
    monkeypatch.delenv("VHSPHYS_BIN", raising=False)
    monkeypatch.setenv("PATH", str(tmp_path / "empty"))
    return app


def test_a_source_checkout_has_no_bundle_folders(monkeypatch):
    monkeypatch.delattr(sys, "frozen", raising=False)
    monkeypatch.delattr(sys, "_MEIPASS", raising=False)
    assert backend.bundle_dirs() == []


def test_the_bundle_folders_are_the_app_folder_then_the_internal_folder(frozen_app):
    assert backend.bundle_dirs() == [frozen_app.resolve(), (frozen_app / "_internal").resolve()]


def test_a_packaged_app_finds_the_binary_beside_itself(frozen_app):
    exe = _fake_exe(frozen_app / ("vhsphys" + EXE_SUFFIX))
    assert backend.find_binary() == exe.resolve()


def test_a_packaged_app_finds_the_binary_in_its_internal_folder(frozen_app):
    exe = _fake_exe(frozen_app / "_internal" / ("vhsphys" + EXE_SUFFIX))
    assert backend.find_binary() == exe.resolve()


def test_the_binary_beside_the_app_comes_before_the_internal_one(frozen_app):
    beside = _fake_exe(frozen_app / ("vhsphys" + EXE_SUFFIX))
    _fake_exe(frozen_app / "_internal" / ("vhsphys" + EXE_SUFFIX))
    assert backend.find_binary() == beside.resolve()


def test_the_argument_and_the_environment_still_come_first(frozen_app, tmp_path, monkeypatch):
    _fake_exe(frozen_app / ("vhsphys" + EXE_SUFFIX))
    a, b = _fake_exe(tmp_path / "a"), _fake_exe(tmp_path / "b")
    assert backend.find_binary(str(a)) == a
    monkeypatch.setenv("VHSPHYS_BIN", str(b))
    assert backend.find_binary() == b


def test_a_packaged_app_puts_its_folders_first_on_the_path(frozen_app, monkeypatch):
    monkeypatch.setenv("PATH", os.pathsep.join(["/usr/local/bin", "/usr/bin"]))
    entry.add_bundle_to_path()
    parts = os.environ["PATH"].split(os.pathsep)
    assert parts[:2] == [str(frozen_app.resolve()), str((frozen_app / "_internal").resolve())]
    assert parts[2:] == ["/usr/local/bin", "/usr/bin"]
    # a second call does not add them again
    entry.add_bundle_to_path()
    assert os.environ["PATH"].split(os.pathsep).count(str(frozen_app.resolve())) == 1


def test_an_ffmpeg_beside_the_app_is_found(frozen_app):
    ffmpeg = _fake_exe(frozen_app / ("ffmpeg" + EXE_SUFFIX))
    entry.add_bundle_to_path()
    import shutil

    assert Path(shutil.which("ffmpeg")).resolve() == ffmpeg.resolve()


def test_the_path_is_left_alone_outside_a_packaged_app(monkeypatch):
    monkeypatch.delattr(sys, "frozen", raising=False)
    monkeypatch.delattr(sys, "_MEIPASS", raising=False)
    monkeypatch.setenv("PATH", "/usr/bin")
    entry.add_bundle_to_path()
    assert os.environ["PATH"] == "/usr/bin"
