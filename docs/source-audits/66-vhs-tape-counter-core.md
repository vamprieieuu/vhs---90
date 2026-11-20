# VHS Tape Counter — portable core milestone

## Implemented
- Added a separate optional tape-counter overlay to `VhsOsdParams`.
- Added three formats: six-digit frame counter, elapsed `HH:MM:SS`, and `HH:MM:SS:FF` timecode.
- Timecode is derived deterministically from the current render frame and frame rate, not wall-clock time.
- Added independent normalized X/Y placement and amount.
- Kept the feature disabled by default, preserving existing project appearance and old parameter indices/disk IDs.
- Added smoke coverage for all formats and repositioning.

## Not yet connected
This is a portable engine capability only. No new After Effects parameter controls have been registered for the counter in this commit, and neither Legacy nor SmartFX currently exposes the new fields. The counter is not yet a user-adjustable VHS Tape UI feature.

## DirectWrite integration status
The current DirectWrite adapter creates a WIC bitmap at the full destination dimensions per draw call. Calling it per OSD label/frame would be expensive, and the existing bitmap labels would duplicate DirectWrite labels unless the renderer is refactored to separate icon drawing from text drawing. This milestone deliberately does not insert that expensive or visually duplicating call into the render path.

## AE free-text input blocker
After Effects arbitrary data supports persistent custom data, but does not provide a native text-entry field. A usable editor needs a custom control or a deliberate button/dialog strategy plus complete arbitrary-data callbacks. The exact SDK headers and UI/AEGP suites are not present in this repository checkout, so no speculative parameter or callback code was added.

## Validation
The CMake smoke test target includes this test. CI execution and an actual After Effects render test must be confirmed separately; this document does not claim either has passed.


## DirectWrite bounded-surface improvement
The Windows DirectWrite adapter was changed to allocate its WIC render target to the requested text bounds / remaining visible region rather than the full destination frame. Compositing now touches only that rectangle. A Windows smoke assertion checks that a bounded label leaves all pixels outside its requested rectangle unchanged. This removes the full-frame WIC allocation cost for small labels, but the adapter is still not invoked by the effect's OSD render path.
