#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

binary=""
if [[ "${1:-}" == "--binary" ]]; then
    if [[ $# -ne 2 ]]; then
        echo "usage: scripts/package.sh [--binary PATH]" >&2
        exit 2
    fi
    binary="$2"
elif [[ $# -ne 0 ]]; then
    echo "usage: scripts/package.sh [--binary PATH]" >&2
    exit 2
fi

if [[ -z "$binary" ]]; then
    binary="${VHSPHYS_BIN:-}"
fi
if [[ -z "$binary" ]]; then
    cargo build --release -p vhsphys-cli
    binary="$ROOT/target/release/vhsphys"
    [[ "$(uname -s)" == MINGW* || "$(uname -s)" == MSYS* || "$(uname -s)" == CYGWIN* ]] && binary+=".exe"
fi
if [[ ! -f "$binary" ]]; then
    echo "vhsphys binary not found: $binary" >&2
    exit 1
fi

PYTHON="${PYTHON:-}"
if [[ -z "$PYTHON" ]]; then
    if [[ -x .venv/bin/python ]]; then
        PYTHON="$ROOT/.venv/bin/python"
    else
        PYTHON=python3
    fi
fi
version="$("$PYTHON" -c 'import pathlib,tomllib; d=tomllib.loads(pathlib.Path("Cargo.toml").read_text()); print(d["workspace"]["package"]["version"])')"
system="$(uname -s)"
machine="$(uname -m)"
case "$system" in
    Linux*) platform=linux-x86_64; archive_ext=tar.gz ;;
    MINGW*|MSYS*|CYGWIN*) platform=windows-x86_64; archive_ext=zip ;;
    Darwin*)
        case "$machine" in
            arm64|aarch64) platform=macos-arm64 ;;
            x86_64) platform=macos-x86_64 ;;
            *) echo "unsupported macOS architecture: $machine" >&2; exit 1 ;;
        esac
        archive_ext=zip
        ;;
    *) echo "unsupported platform: $system" >&2; exit 1 ;;
esac

rm -rf build/pyinstaller dist
"$PYTHON" -m PyInstaller --noconfirm --distpath build/pyinstaller/dist --workpath build/pyinstaller/work packaging/vhs-phys.spec
package="dist/vhs-phys-$version-$platform"
mkdir -p "$package"
if [[ "$system" == Darwin* ]]; then
    app="build/pyinstaller/dist/VHS-Phys.app"
    [[ -d "$app" ]] || { echo "PyInstaller app not found: $app" >&2; exit 1; }
    cp -R "$app" "$package/VHS-Phys.app"
    mkdir -p "$package/VHS-Phys.app/Contents/MacOS"
    cp "$binary" "$package/VHS-Phys.app/Contents/MacOS/vhsphys"
    cp "$binary" "$package/vhsphys"
    # Adding a file changes the bundle, so seal it again (ad hoc): an Apple-silicon Mac refuses a broken seal.
    codesign --force --deep --sign - "$package/VHS-Phys.app"
else
    app_dir="build/pyinstaller/dist/vhs-phys"
    [[ -d "$app_dir" ]] || { echo "PyInstaller app folder not found: $app_dir" >&2; exit 1; }
    cp -R "$app_dir/." "$package/"
    cli_name=vhsphys
    [[ "$system" == MINGW* || "$system" == MSYS* || "$system" == CYGWIN* ]] && cli_name+=".exe"
    cp "$binary" "$package/$cli_name"
fi
cp LICENSE "$package/LICENSE"
cat > "$package/README.txt" <<'EOF'
vhs-phys desktop window: vhs-phys (VHS-Phys.app on macOS).
Command line tool: vhsphys.
ffmpeg and ffprobe must be on PATH or placed beside the app.
Project: https://github.com/allthemalware/vhs-phys
EOF

archive="dist/vhs-phys-$version-$platform.$archive_ext"
if [[ "$system" == Linux* ]]; then
    tar -czf "$archive" -C dist "vhs-phys-$version-$platform"
elif [[ "$system" == Darwin* ]]; then
    ditto -c -k --keepParent "$package" "$archive"
else
    (cd dist && "$PYTHON" -m zipfile -c "vhs-phys-$version-$platform.zip" "vhs-phys-$version-$platform")
fi
echo "$archive"
