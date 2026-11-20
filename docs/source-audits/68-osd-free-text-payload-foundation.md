# OSD free-text payload foundation

Branch: `work/osd-directwrite-integration-map`

## Implemented

Added `VHS90OsdTextData` as a platform-independent data layer:

- UTF-8 payload limit: 4096 bytes.
- Strict UTF-8 validation rejects malformed continuation bytes, overlong encodings, UTF-16 surrogate code points, truncated sequences, and values above U+10FFFF.
- Versioned serialization format: 4-byte `V90T` magic, 16-bit little-endian version, 16-bit reserved field, 32-bit little-endian UTF-8 byte count, then the UTF-8 payload.
- Unflatten validates magic, version, reserved bytes, exact total length, size cap, and UTF-8 before accepting data.
- Added smoke coverage for Arabic, Arabic-Indic digits, mixed Latin/Arabic text, round-trip serialization, malformed UTF-8, oversized input, and corrupt headers.
- Registered source and test in CMake.

## Why this is separate

The payload/serialization code is deliberately independent of the AE SDK and Windows APIs. It can be unit-tested on any supported build host and can later back the AE `PF_Param_ARBITRARY_DATA` callbacks without serializing compiler-specific object layouts.

## Still outstanding

This commit does **not** yet expose an editable text field in Effect Controls, implement the AE arbitrary-data callback dispatcher, or call DirectWrite from the render path. The SDK documents arbitrary-data storage callbacks but does not provide a normal text-entry widget for them. The next integration step must pair the persistent payload with a real edit interaction (custom UI/editor) and then pass the decoded UTF-8 text into both Legacy and SmartFX rendering.

No AEX build or After Effects host test is claimed by this milestone.
