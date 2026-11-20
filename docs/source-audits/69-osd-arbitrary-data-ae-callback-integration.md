# After Effects OSD arbitrary-data callback integration

Branch: `work/osd-directwrite-integration-map`

## Added

- `Source/VHS90Windows/VHS90OsdArbitraryData.h/.cpp`
- Added the Windows AE adapter to the `VHS90` module target.
- Added a hidden `PF_Param_ARBITRARY_DATA` parameter at the end of the parameter list, using arb/disk ID 9013. Existing parameter indices and disk IDs remain unchanged.
- Routed `PF_Cmd_ARBITRARY_CALLBACK` to the adapter.
- Implemented NEW, DISPOSE, COPY, FLAT_SIZE, FLATTEN, UNFLATTEN, INTERP, COMPARE, PRINT_SIZE, PRINT, and SCAN selectors.
- Data is stored in host-managed AE handles as the versioned, bounded UTF-8 blob implemented in `VHS90OsdTextData`.
- Handle reads validate the blob before it is copied, compared, printed, or flattened; unflatten and scan validate before creating host handles.

## Design notes

The arb ID is 9013 and fits the SDK's signed 16-bit arb ID field. The parameter is hidden from the Effect Controls Window for this storage-only milestone. It is not yet user-editable from the UI.

Text interpolation is discrete: the left keyframe value is retained for intermediate times. This avoids nonsensical character-by-character interpolation of strings.

## Remaining work

1. Add a supervised Edit Text control and a safe Windows text editor interaction.
2. Update the arb handle from that interaction without leaking or invalidating host-owned handles.
3. Read the decoded text in Legacy and SmartFX paths.
4. Call DirectWrite from the OSD rendering stage before Composite, while preserving existing bitmap indicators and avoiding duplicate labels.
5. Compile the AE module with MSVC against the supplied VHS6RIFT SDK and perform host testing.

No AEX build or After Effects host test is claimed for this change.
