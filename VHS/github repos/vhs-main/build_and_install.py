#!/usr/bin/env python3
"""Build LofiFxVhs OFX plugin and install to the system OFX plugins folder.

Cross-platform: macOS, Linux, and Windows. Install destination per OFX spec:
    macOS:   /Library/OFX/Plugins
    Linux:   /usr/OFX/Plugins
    Windows: %CommonProgramFiles%\\OFX\\Plugins
"""

import os
import shutil
import subprocess
import sys

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR   = os.path.join(SCRIPT_DIR, "build")
BUNDLE_NAME = "LofiFxVhs.ofx.bundle"
BUNDLE_SRC  = os.path.join(BUILD_DIR, BUNDLE_NAME)


def install_dir() -> str:
    if sys.platform == "darwin":
        return "/Library/OFX/Plugins"
    if sys.platform.startswith("win"):
        common = os.environ.get(
            "CommonProgramFiles",
            r"C:\Program Files\Common Files",
        )
        return os.path.join(common, "OFX", "Plugins")
    return "/usr/OFX/Plugins"


def run(cmd, **kwargs):
    print(">>>", " ".join(cmd))
    subprocess.check_call(cmd, **kwargs)


def adhoc_sign(bundle_path: str):
    """Ad-hoc sign the bundle on macOS (free, no Developer account required).

    This strips any quarantine issues that would prevent DaVinci Resolve from
    loading the plugin after the user downloads and copies the bundle manually.
    Silently skipped on non-macOS platforms.
    """
    if sys.platform != "darwin":
        return
    if not shutil.which("codesign"):
        print("codesign not found — skipping ad-hoc signing")
        return
    print("\n=== Ad-hoc signing bundle ===")
    run(["codesign", "--sign", "-", "--force", "--deep", bundle_path])


def strip_quarantine(bundle_path: str):
    """Remove the com.apple.quarantine extended attribute from the installed bundle.

    macOS sets this on files downloaded from the internet, which prevents
    DaVinci Resolve from loading the plugin. Silently skipped on non-macOS.
    """
    if sys.platform != "darwin":
        return
    if not shutil.which("xattr"):
        print("xattr not found — skipping quarantine removal")
        return
    print("\n=== Removing quarantine attribute ===")
    run(["sudo", "xattr", "-r", "-d", "com.apple.quarantine", bundle_path])


def copy_bundle(src: str, dst: str):
    """Copy the bundle to dst, escalating with sudo on POSIX if needed."""
    if os.path.exists(dst):
        try:
            shutil.rmtree(dst)
        except PermissionError:
            run(["sudo", "rm", "-rf", dst])
    try:
        shutil.copytree(src, dst)
    except PermissionError:
        if sys.platform.startswith("win"):
            print("Permission denied — re-run this script as Administrator.")
            raise
        run(["sudo", "cp", "-R", src, dst])


def main():
    os.makedirs(BUILD_DIR, exist_ok=True)

    print("\n=== Configuring ===")
    cfg_cmd = ["cmake", ".."]
    if sys.platform.startswith("win"):
        # Default to a 64-bit Visual Studio generator. Override by editing
        # this list (or pre-running `cmake -G "Ninja" ..` once) if needed.
        cfg_cmd += ["-A", "x64"]
    run(cfg_cmd, cwd=BUILD_DIR)

    print("\n=== Building ===")
    build_cmd = ["cmake", "--build", "."]
    if sys.platform.startswith("win"):
        build_cmd += ["--config", "Release"]
    run(build_cmd, cwd=BUILD_DIR)

    # Ad-hoc sign (macOS only)
    adhoc_sign(BUNDLE_SRC)

    dst = os.path.join(install_dir(), BUNDLE_NAME)
    print(f"\n=== Installing to {dst} ===")
    copy_bundle(BUNDLE_SRC, dst)

    # Strip quarantine so macOS doesn't block the plugin on other machines
    strip_quarantine(dst)

    print("\nDone. Restart DaVinci Resolve to load the updated plugin.")


if __name__ == "__main__":
    main()
