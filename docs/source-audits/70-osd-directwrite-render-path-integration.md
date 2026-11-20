# VHS-90 OSD DirectWrite Render-Path Integration

Date: 2026-11-21

## Implemented in this pass

- Added a read-only helper that validates and decodes the host-owned OSD arbitrary-data handle into an immutable UTF-8 string.
- Added the OSD text checkout to the SmartFX settings path.
- Added OSD text extraction from the Legacy render parameter array.
- Connected the UTF-8 string to the Windows DirectWrite renderer in `RenderWorld`.
- The DirectWrite overlay is drawn after the existing bitmap OSD and before the composite stage, preserving the existing OSD/composite order.
- Uses automatic reading direction, Segoe UI fallback, white text, and a bounded text surface.
- Fixed an existing compile-time error in `VHS90OsdEngine.cpp`: battery HUD positioning referenced undefined `mx` and `my`; it now uses an explicit three-cell margin.

## Still incomplete

- The arbitrary-data parameter is currently registered with `PF_PUI_NO_ECW_UI`. It persists text but does not provide a user-facing text editor.
- No Edit Text button, modal Unicode editor, or custom ECW text control has been implemented.
- The user cannot yet enter or change free text from the After Effects interface.
- No native Windows/MSVC AEX build was run in this pass.
- No After Effects host test was run; Arabic shaping, bidirectional mixed text, font fallback, save/reopen, and MFR behavior remain unverified in AE.

## Important implementation boundary

The renderer now consumes text in both Legacy and SmartFX paths, but the end-user editing surface is still absent. Do not describe VHS Tape free-text OSD as complete until an editor can safely update the arbitrary-data parameter through an AE-supported UI selector and the project has been built and tested against the supplied SDK.

## Commits

- `47a5f653019031c29c0eae0fc741b4c5de15fcb6` — fix battery OSD undefined margins
- `f48f399d7e9ca52b30e9aa28ff60e5e1c9f74903` — expose immutable OSD text handle reader
- `1ac6f14b350a02f681ccc5d61fd1575c909549ce` — implement safe OSD text reader
- `c4f3e1b1c1abf79bcac974eb17fac6b2e591b1d7` — connect Legacy and SmartFX render paths
