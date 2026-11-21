#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 || ! -f "$1" ]]; then
    echo "usage: scripts/smoke_bundle.sh ARCHIVE" >&2
    exit 2
fi
archive="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
temp="$(mktemp -d)"
cleanup() { rm -rf "$temp"; }
trap cleanup EXIT
case "$archive" in
    *.tar.gz) tar -xzf "$archive" -C "$temp" ;;
    *.zip)
        if [[ "$(uname -s)" == Darwin* ]]; then
            ditto -x -k "$archive" "$temp"
        elif command -v unzip >/dev/null 2>&1; then
            unzip -q "$archive" -d "$temp"
        else
            "${PYTHON:-python}" -m zipfile -e "$archive" "$temp"
        fi
        ;;
    *) echo "unsupported archive: $archive" >&2; exit 1 ;;
esac
package="$(find "$temp" -mindepth 1 -maxdepth 1 -type d -print -quit)"
[[ -n "$package" ]] || { echo "unpack step failed: package folder missing" >&2; exit 1; }
case "$(uname -s)" in
    Darwin*) gui="$package/VHS-Phys.app/Contents/MacOS/vhs-phys" ;;
    MINGW*|MSYS*|CYGWIN*) gui="$package/vhs-phys.exe" ;;
    *) gui="$package/vhs-phys" ;;
esac
cli="$package/vhsphys"
[[ -f "$cli" || -f "$cli.exe" ]] || { echo "binary discovery step failed: bundled vhsphys missing" >&2; exit 1; }
if [[ -f "$cli.exe" ]]; then cli="$cli.exe"; fi
version="$($cli --version)" || { echo "version step failed" >&2; exit 1; }
[[ "$version" == *vhsphys* ]] || { echo "version step failed: unexpected output: $version" >&2; exit 1; }
command -v ffmpeg >/dev/null 2>&1 || { echo "ffmpeg step failed: ffmpeg not found" >&2; exit 1; }
(cd "$temp" && ffmpeg -v error -f lavfi -i testsrc=size=320x240:rate=30 -frames:v 3 -y in.mp4) || { echo "input clip step failed" >&2; exit 1; }
(cd "$temp" && "$cli" run in.mp4 out.mp4 --preset good --frames 3) || { echo "render step failed" >&2; exit 1; }
[[ -f "$temp/out.mp4" ]] || { echo "render step failed: output missing" >&2; exit 1; }
[[ -f "$gui" ]] || { echo "window step failed: executable missing: $gui" >&2; exit 1; }
(cd "$package" && env -u VHSPHYS_BIN PATH="$PATH" QT_QPA_PLATFORM=offscreen "$gui" --screenshot "$temp/shot.png") || { echo "window screenshot step failed" >&2; exit 1; }
(cd "$package" && env -u VHSPHYS_BIN PATH="$PATH" QT_QPA_PLATFORM=offscreen "$gui" --page camera --screenshot "$temp/camera.png") || { echo "camera screenshot step failed" >&2; exit 1; }
for png in "$temp/shot.png" "$temp/camera.png"; do
    [[ -f "$png" && $(wc -c < "$png") -gt 10000 ]] || { echo "screenshot step failed: $png missing or too small" >&2; exit 1; }
done
echo "bundle smoke test passed: $archive"
