# VHS-90 — VHS Tape OSD transport and layout controls

Date: 2026-11-21
Branch: work/osd-directwrite-integration-map

## Implemented in this checkpoint

- Appended a new AE parameter group, VHS Tape OSD Layout, after the existing parameter list. Existing parameter indices and disk IDs were not renumbered.
- Added deck transport selection: Play, Pause, Stop, Fast Forward, Rewind.
- Added tape-speed status rendering from the existing tape-speed control: SP, LP, EP.
- Added timestamp formats: ISO date + time, US date + time, European date + time, date only, time only.
- Added independent percentage anchors for REC, deck status, and timestamp (each X/Y).
- Extended the portable OSD model and bitmap renderer to draw the new deck states, speed labels, date formats, and positions.
- Routed the new controls through the shared SmartFX ReadSettings path and the LegacyRender parameter extraction path.
- Added smoke coverage for all five transport states, format variation, tape-speed variation, and independently moved status/timestamp anchors.

## Preserved behavior

- Existing parameter enum indices remain stable: new indices are appended after the prior final parameter.
- Existing disk IDs remain stable: new IDs start at 9001, separate from the historic sequential IDs.
- Existing OSD render position remains before Composite and downstream VHS signal processing.
- Existing REC, PLAY, timestamp, battery, opacity, scale, ghost and jitter controls remain supported.

## Still open

### Free text parameter and persistence

The SDK standard effect controls do not provide a simple plain-text parameter macro. A true editable, project-persistent free-text control needs a custom PF_Param_ARBITRARY_DATA value with a complete callback implementation (NEW, DISPOSE, COPY, FLAT_SIZE, FLATTEN, UNFLATTEN, INTERP, COMPARE, PRINT_SIZE, PRINT, SCAN) and a deliberate UI/editing strategy. PF_ArbParamsExtra::id is 16-bit, so the custom arb disk ID must fit that width. Do not register a high 32-bit-only ID and then dispatch against it.
The actual AE SDK headers are not in this repository; the custom ARB callbacks and custom text editor have not been added in this checkpoint.

### DirectWrite in-host integration

The DirectWrite adapter and its Windows smoke test are built as a Windows-only target and linked into the AE module, but the effect render path does not yet call the adapter. The old bitmap OSD remains the active renderer and fallback. The next safe step is to implement the ARB value/editor and feed its immutable UTF-8 value into DirectWrite in both LegacyRender and SmartFX, at the existing pre-Composite OSD stage.

### Verification

No Windows/MSVC compile, AE SDK compile, CTest run, AEX build, or in-host Arabic shaping test is claimed. GitHub Actions has not reported a run for this branch checkpoint.