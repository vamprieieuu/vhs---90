# VHS6RIFT SDK Discovery — OSD Text Integration

Date: 2026-11-21
Repository branch: `work/osd-directwrite-integration-map`

## Confirmed SDK source

The user's Library archive `vhs.zip` contains:

`vhs/open source native plugin for ae/VHS6RIFT/extern/AfterEffectsSDK/`

The extracted SDK root is:

`VHS6RIFT/extern/AfterEffectsSDK`

Verified files include:

- `Headers/AE_Effect.h`
- `Headers/AE_EffectCB.h`
- `Headers/AE_EffectCBSuites.h`
- `Headers/AE_EffectUI.h`
- `Headers/AE_EffectVers.h`
- `Util/Param_Utils.h`
- `Util/AEFX_ArbParseHelper.h`
- `Util/AEGP_SuiteHandler.h`
- `Resources/PiPLtool.exe`
- SDK examples and guide PDFs.

This resolves the previously reported missing-SDK-header blocker. It does not by itself prove that the VHS-90 AEX compiles or loads.

## Verified arbitrary-data API

The supplied `Headers/AE_Effect.h` defines:

- `PF_Param_ARBITRARY_DATA`
- `PF_Cmd_ARBITRARY_CALLBACK`
- `PF_ArbParamsExtra`
- `PF_Arbitrary_NEW_FUNC`
- `PF_Arbitrary_DISPOSE_FUNC`
- `PF_Arbitrary_COPY_FUNC`
- `PF_Arbitrary_FLAT_SIZE_FUNC`
- `PF_Arbitrary_FLATTEN_FUNC`
- `PF_Arbitrary_UNFLATTEN_FUNC`
- `PF_Arbitrary_INTERP_FUNC`
- `PF_Arbitrary_COMPARE_FUNC`
- `PF_Arbitrary_PRINT_SIZE_FUNC`
- `PF_Arbitrary_PRINT_FUNC`
- `PF_Arbitrary_SCAN_FUNC`

The dispatch structure carries a 16-bit `A_short id`. The arbitrary parameter definition also carries an `A_short id`, so the OSD text parameter's arb id must remain within the signed 16-bit range and must be distinct from other arb parameter IDs.

The SDK's `Param_Utils.h` provides `PF_ADD_ARBITRARY2`, which sets the parameter type, UI flags, default handle, ID, and refcon. The SDK comments explicitly state that arbitrary data needs either `PF_PUI_TOPIC/PF_PUI_CONTROL` or `PF_PUI_NO_ECW`; a normal text-entry widget is not supplied automatically. A separate standard supervised control or a custom UI/editor interaction is required.

## VHS-90 integration state

- The existing CMake configuration already accepts `AE_SDK_ROOT` or `AE_SDK_PATH`, and discovers flat, `Examples/`, or nested SDK layouts.
- For this supplied archive, configure with:
  `-DAE_SDK_ROOT=<extracted VHS6RIFT>/extern/AfterEffectsSDK`
- `VHS90DirectWrite` is a Windows-only target and is already linked into the AE module.
- The DirectWrite adapter is **not yet called** from the OSD render path.
- Existing OSD bitmap rendering remains before Composite and subsequent signal processing; preserve this order.
- Transport, tape speed, timestamp format, independent OSD positions, and tape counter controls are already wired through Legacy and SmartFX.
- Free-text OSD parameter, arbitrary-data callbacks, text editor UI, DirectWrite render-path integration, and an actual Windows AEX build remain outstanding.

## Implementation constraints

1. Implement the arb payload with a bounded, versioned UTF-8 representation. Validate lengths and malformed UTF-8 on every load/unflatten path.
2. Implement all callbacks required by the SDK contract, including deterministic copy, flatten/unflatten, compare, and print/scan behavior. Do not treat an in-memory C++ object layout as a stable serialized format.
3. Keep the parameter's arb ID within 16-bit range and append the parameter at the end of the parameter list; do not renumber existing parameters or disk IDs.
4. Provide a real user-editing interaction. `PF_Param_ARBITRARY_DATA` alone is storage, not a text-entry UI.
5. Read the arb value in both Legacy and SmartFX paths and pass the same immutable UTF-8 text into the render settings.
6. Integrate DirectWrite for text labels while retaining bitmap-only indicators where appropriate. Avoid drawing the same text twice.
7. Keep DirectWrite/Windows headers out of `VHS90Core`.
8. Verify the Windows/MSVC build with this exact SDK root before claiming an AEX build. In-After-Effects Arabic shaping still requires the owner's manual host test.

## Validation performed

The archive was opened and the SDK file tree was inspected. The listed headers and utility files exist. The callback declarations and parameter UI notes were read directly from the supplied SDK headers.

No Windows/MSVC build or After Effects host test was performed in this environment.
