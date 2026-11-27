#!/usr/bin/env python3
"""Convert the upstream NTSC-QT ringPattern.npy into a private C++ header."""
from __future__ import annotations
import argparse
import ast
import math
import struct
from pathlib import Path

def main() -> None:
    parser=argparse.ArgumentParser()
    parser.add_argument("--input",required=True)
    parser.add_argument("--output",required=True)
    args=parser.parse_args()
    raw=Path(args.input).read_bytes()
    if raw[:6]!=b"\x93NUMPY":
        raise SystemExit("ringPattern.npy has an invalid NumPy magic header")
    major,minor=raw[6],raw[7]
    if major==1:
        header_len=struct.unpack("<H",raw[8:10])[0];start=10
    elif major in (2,3):
        header_len=struct.unpack("<I",raw[8:12])[0];start=12
    else:
        raise SystemExit(f"unsupported NumPy format {major}.{minor}")
    header=ast.literal_eval(raw[start:start+header_len].decode("latin1").strip())
    if header.get("shape")!=(720,) or header.get("fortran_order") is not False:
        raise SystemExit(f"unexpected RingPattern shape/order: {header}")
    descr=header.get("descr")
    if descr not in ("<f4","=f4","|f4"):
        raise SystemExit(f"RingPattern must be float32, found {descr}")
    payload=raw[start+header_len:]
    if len(payload)!=720*4:
        raise SystemExit(f"RingPattern payload size mismatch: {len(payload)}")
    values=struct.unpack("<720f",payload)
    if not all(math.isfinite(v) for v in values):
        raise SystemExit("RingPattern contains non-finite values")
    lines=[]
    for i in range(0,720,6):
        chunk=[]
        for value in values[i:i+6]:
            token=f"{value:.9g}"
            if "." not in token and "e" not in token.lower():
                token += ".0"
            chunk.append(token+"f")
        lines.append("    "+", ".join(chunk)+",")
    out=Path(args.output)
    out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(
        "#pragma once\n#include <array>\nnamespace vhs90::detail {\n"
        "inline constexpr std::array<float,720> kNtscQtRingPattern{{\n"+
        "\n".join(lines)+"\n}};\n}\n",encoding="utf-8")
    print(f"Generated {out} from 720 upstream float32 samples")
if __name__=="__main__":
    main()
