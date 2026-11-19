# VHS-90 — VHS6RIFT ABI / Build-Harness Verification Addendum

Date: 2026-11-19
Input: supplied VHS6RIFT source subtree and bundled Adobe After Effects SDK headers.

## Confirmed native ABI mismatches

### EffectMain

The public declaration in include/VHS6RIFT.h has six arguments, including the SDK's final void* extra parameter. The definition in src/VHS6RIFT.cpp has only five arguments. This is a conflicting C-linkage function declaration and prevents a normal C++ compile.

### PluginDataEntryFunction2

The public declaration in include/VHS6RIFT.h declares four generic void* arguments. The actual SDK's AE_PluginData.h defines PluginDataEntryFunction2Ptr with five typed arguments: PF_PluginDataPtr, PF_PluginDataCB2, SPBasicSuite*, host name and host version. The implementation in src/VHS6RIFT.cpp uses the five-argument typed form, which conflicts with the header declaration. The header must include the correct SDK declaration and use the exact ABI signature.

### PF_InData time representation

The actual bundled SDK's PF_InData has integer current_time, time_step and total_time fields, plus a separate time_scale. The implementation reads current_time.value, current_time.scale and time_step.scale. Those members do not exist in the actual SDK structure. Use current_time / time_scale for seconds, with proper null/zero guards and time-step semantics.

## PiPL / GlobalSetup flag comparison

Using the bundled SDK enum values, GlobalSetup's flags are:
- WIDE_TIME_INPUT = 1 << 1
- DEEP_COLOR_AWARE = 1 << 25
- AUDIO_FLOAT_ONLY = 1 << 27
- AUDIO_EFFECT_TOO = 1 << 30

Their combined value is 0x4A000002. The PiPL currently declares 0x48000046. The PiPL outflags2 value is 0x04000000, while SUPPORTS_THREADED_RENDERING is 1 << 27 = 0x08000000. The existing Python validator does not actually compare the parsed C++ flag expression with the PiPL values; it merely checks for expected strings. Recompute PiPL flags from the SDK constants and add a numeric comparison test.

## Fallback SDK layer

VHS6RIFT.h contains locally invented PF_* fallback types and flag constants. These do not match the bundled SDK ABI: PF_InData time fields, PF_SoundWorld layout, PF_OutFlag bit positions and PF_OutFlag2 threaded-rendering bit positions differ. They must not be used to compile a production AEX. Remove the fake fallback from the production build path; compile only against the exact AE SDK. If a standalone algorithm test is needed, isolate host-independent kernels from PF_* types.

## Native render test harness is not a native build test

tools/test_native_render.cpp includes the plugin implementation directly and constructs the fallback PF_* structures. It expects PF_InData.current_time.value/current_time.scale and PF_SoundWorld.data[2]/rate/num_channels, which do not match the actual SDK. It is not invoked by build.ps1 and is not wired into CMake/CTest.

A local g++ attempt to compile tools/test_native_render.cpp against the fallback declarations failed. The errors include missing parameter utility macros, missing audio structures/macros, PF_InData/PF_SoundWorld layout mismatches, the EffectMain conflicting declaration, missing SDK suite types and the PluginDataEntryFunction2 export type. This is a failed compile attempt, not a successful runtime test.

## Static test suite findings

The following scripts do pass when run from the extracted project directory:
- validate_project.py: PASS (83 parameter enum slots; 9 topic groups).
- audit_parameter_wiring.py: PASS (64 active control names occur at least twice).
- test_vhs6rift.py: PASS (14/14 model-level gates).

However, test_vhs6rift.py reimplements small equations in Python rather than invoking the C++ render functions. Gate 2 tests a trivial arithmetic identity blend; Gate 6 tests a separately coded YIQ matrix; Gate 14 is hard-coded True. audit_parameter_wiring.py counts token occurrences and does not prove that the value changes output. validate_project.py checks string/enum patterns and did not catch the ABI or PiPL numeric mismatch.

## Corrected integration decision

VHS6RIFT remains a valuable reference for signal math and project architecture, but the present source cannot be treated as a proven buildable AE base. Before VHS-90 uses it as a host base, fix both exported function declarations, the real SDK time representation, PiPL flag parity, and the fake fallback layer. Then build against the bundled SDK with MSVC 2022, inspect the generated AEX/PiPL metadata, and test load/apply/render in AE 23.2.1. Only then can the host base be accepted.

## Verification boundary

The exact SDK headers and relevant plugin declarations were inspected. The three Python scripts were executed and passed. The standalone C++ harness compile was attempted and failed. No MSVC Windows build or After Effects runtime test was performed.