cmake_minimum_required(VERSION 3.20)
project(VHS90 LANGUAGES CXX)

option(VHS90_BUILD_TESTS "Build host-independent VHS-90 CPU smoke tests" ON)
option(VHS90_BUILD_AE_PLUGIN "Build the native Windows After Effects AEX target" OFF)
option(VHS90_ENABLE_SANITIZERS "Enable AddressSanitizer/UndefinedBehaviorSanitizer where supported" OFF)
set(AE_SDK_ROOT "$ENV{AE_SDK_ROOT}" CACHE PATH "Root directory of the Adobe After Effects SDK")

add_library(VHS90Core STATIC
    Source/VHS90Core/VHS90SignalKernels.cpp
    Source/VHS90Core/VHS90RegionEngine.cpp
    Source/VHS90Core/VHS90StaticEngine.cpp
    Source/VHS90Core/VHS90IIRFilter.cpp
    Source/VHS90Core/VHS90CompositeEngine.cpp
    Source/VHS90Core/VHS90SyncEngine.cpp
    Source/VHS90Core/VHS90TapeSurfaceEngine.cpp
    Source/VHS90Core/VHS90TapeTransportEngine.cpp
    Source/VHS90Core/VHS90BloomEngine.cpp
    Source/VHS90Core/VHS90TemporalEngine.cpp
    Source/VHS90Core/VHS90AnalogGeometryEngine.cpp
    Source/VHS90Core/VHS90AnalogNoiseEngine.cpp
    Source/VHS90Core/VHS90RfInstabilityEngine.cpp
    Source/VHS90Core/VHS90ImpulseNoiseEngine.cpp
    Source/VHS90Core/VHS90MultipathEchoEngine.cpp
    Source/VHS90Core/VHS90ReceiverInstabilityEngine.cpp
    Source/VHS90Core/VHS90ImageGradeEngine.cpp
    Source/VHS90Core/VHS90TapeDetailEngine.cpp
    Source/VHS90Core/VHS90ChromaShiftEngine.cpp
    Source/VHS90Core/VHS90WaveformEngine.cpp
    Source/VHS90Core/VHS90RingingEngine.cpp
    Source/VHS90Core/VHS90DropoutEngine.cpp
    Source/VHS90Core/VHS90SignalGhostEngine.cpp
    Source/VHS90Core/VHS90AnalogGlowEngine.cpp
    Source/VHS90Core/VHS90CrtEqualizerEngine.cpp
    Source/VHS90Core/VHS90TrackReadEngine.cpp
    Source/VHS90Core/VHS90HeadConditionEngine.cpp
    Source/VHS90Core/VHS90VcrSignalEngine.cpp
    Source/VHS90Core/VHS90PhysicalInterferenceEngine.cpp
    Source/VHS90Core/VHS90PyVhsGenerationEngine.cpp
    Source/VHS90Core/VHS90OsdEngine.cpp
    Source/VHS90Core/VHS90DeviceConnectionEngine.cpp
    Source/VHS90Core/VHS90ParticlePatternEngine.cpp
    Source/VHS90Core/VHS90TearChromaEngine.cpp
    ThirdParty/AnalogNTSC/src/ntsc_signal.cpp
    ThirdParty/AnalogNTSC/src/ntsc_params.cpp
    ThirdParty/AnalogNTSC/src/ntsc_effects.cpp
    ThirdParty/AnalogNTSC/src/ntsc_decoder.cpp
    ThirdParty/AnalogNTSC/src/ntsc_encoder.cpp
    ThirdParty/AnalogNTSC/src/ntsc_processor.cpp
)
target_include_directories(VHS90Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VHS90Core
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/AnalogNTSC/include
)
target_compile_features(VHS90Core PUBLIC cxx_std_17)
if(MSVC)
    target_compile_options(VHS90Core PRIVATE /W4 /permissive-)
else()
    target_compile_options(VHS90Core PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(VHS90_ENABLE_SANITIZERS)
    if(MSVC)
        target_compile_options(VHS90Core PRIVATE /fsanitize=address)
    else()
        target_compile_options(VHS90Core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()
endif()

if(VHS90_BUILD_TESTS)
    enable_testing()
    foreach(test_name IN ITEMS SignalKernels RegionEngine StaticEngine IIRFilter CompositeEngine SyncEngine TapeSurfaceEngine TapeTransportEngine BloomEngine TemporalEngine AnalogGeometryEngine AnalogNoiseEngine RfInstabilityEngine ImpulseNoiseEngine MultipathEchoEngine ReceiverInstabilityEngine ImageGradeEngine TapeDetailEngine ChromaShiftEngine WaveformEngine RingingEngine DropoutEngine SignalGhostEngine AnalogGlowEngine CrtEqualizerEngine TrackReadEngine HeadConditionEngine VcrSignalEngine PhysicalInterferenceEngine PyVhsGenerationEngine OsdEngine DeviceConnectionEngine ParticlePatternEngine TearChromaEngine)
        add_executable(VHS90${test_name}Smoke Tests/VHS90${test_name}Smoke.cpp)
        target_link_libraries(VHS90${test_name}Smoke PRIVATE VHS90Core)
        # The smoke tests use assert(); keep those checks active in Release too.
        if(MSVC)
            target_compile_options(VHS90${test_name}Smoke PRIVATE /UNDEBUG)
        else()
            target_compile_options(VHS90${test_name}Smoke PRIVATE -UNDEBUG)
        endif()
        if(VHS90_ENABLE_SANITIZERS)
            if(MSVC)
                target_compile_options(VHS90${test_name}Smoke PRIVATE /fsanitize=address)
                target_link_options(VHS90${test_name}Smoke PRIVATE /fsanitize=address)
            else()
                target_compile_options(VHS90${test_name}Smoke PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
                target_link_options(VHS90${test_name}Smoke PRIVATE -fsanitize=address,undefined)
            endif()
        endif()
        add_test(NAME VHS90${test_name}Smoke COMMAND VHS90${test_name}Smoke)
    endforeach()
endif()

if(WIN32)
    # Windows-only text backend; never leaks platform dependencies into VHS90Core.
    add_library(VHS90DirectWrite STATIC Source/VHS90Windows/VHS90DirectWriteText.cpp)
    target_include_directories(VHS90DirectWrite PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/VHS90Windows
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/VHS90Core
    )
    target_compile_features(VHS90DirectWrite PUBLIC cxx_std_17)
    target_compile_definitions(VHS90DirectWrite PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    if(MSVC)
        target_compile_options(VHS90DirectWrite PRIVATE /W4 /permissive- /EHsc)
    else()
        target_compile_options(VHS90DirectWrite PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    target_link_libraries(VHS90DirectWrite PUBLIC VHS90Core PRIVATE d2d1 dwrite windowscodecs ole32)

    if(VHS90_BUILD_TESTS)
        add_executable(VHS90DirectWriteSmoke Tests/VHS90DirectWriteSmoke.cpp)
        target_link_libraries(VHS90DirectWriteSmoke PRIVATE VHS90DirectWrite)
        if(MSVC)
            target_compile_options(VHS90DirectWriteSmoke PRIVATE /UNDEBUG)
        else()
            target_compile_options(VHS90DirectWriteSmoke PRIVATE -UNDEBUG)
        endif()
        add_test(NAME VHS90DirectWriteSmoke COMMAND VHS90DirectWriteSmoke)
    endif()
endif()

if(VHS90_BUILD_AE_PLUGIN)
    if(NOT WIN32 OR NOT MSVC)
        message(FATAL_ERROR "VHS90_BUILD_AE_PLUGIN requires Windows x64 and MSVC 2022.")
    endif()
    enable_language(C)
    enable_language(RC)
    # VHS6RIFT build-foundation port: discover flat, Examples/, or nested SDK layouts.
    if(NOT AE_SDK_ROOT OR AE_SDK_ROOT STREQUAL "")
        if(DEFINED ENV{AE_SDK_PATH} AND NOT "$ENV{AE_SDK_PATH}" STREQUAL "")
            set(AE_SDK_ROOT "$ENV{AE_SDK_PATH}" CACHE PATH "Root directory of the Adobe After Effects SDK" FORCE)
        endif()
    endif()
    if(NOT AE_SDK_ROOT OR NOT EXISTS "${AE_SDK_ROOT}")
        message(FATAL_ERROR "Set AE_SDK_ROOT (or AE_SDK_PATH) to the extracted Adobe After Effects SDK root.")
    endif()
    if(EXISTS "${AE_SDK_ROOT}/Headers/AE_Effect.h")
        set(VHS90_AE_INCLUDE_ROOT "${AE_SDK_ROOT}")
        set(VHS90_AE_HEADERS "${AE_SDK_ROOT}/Headers")
    elseif(EXISTS "${AE_SDK_ROOT}/Examples/Headers/AE_Effect.h")
        set(VHS90_AE_INCLUDE_ROOT "${AE_SDK_ROOT}/Examples")
        set(VHS90_AE_HEADERS "${VHS90_AE_INCLUDE_ROOT}/Headers")
    else()
        file(GLOB_RECURSE VHS90_AE_HEADER_CANDIDATES LIST_DIRECTORIES FALSE "${AE_SDK_ROOT}/*/AE_Effect.h")
        if(VHS90_AE_HEADER_CANDIDATES)
            list(GET VHS90_AE_HEADER_CANDIDATES 0 VHS90_AE_EFFECT_H)
            get_filename_component(VHS90_AE_HEADERS "${VHS90_AE_EFFECT_H}" DIRECTORY)
            get_filename_component(VHS90_AE_INCLUDE_ROOT "${VHS90_AE_HEADERS}" DIRECTORY)
        else()
            message(FATAL_ERROR "AE_Effect.h was not found under AE_SDK_ROOT=${AE_SDK_ROOT}")
        endif()
    endif()
    set(VHS90_AE_UTIL "${VHS90_AE_INCLUDE_ROOT}/Util")
    if(NOT EXISTS "${VHS90_AE_UTIL}/Param_Utils.h")
        set(VHS90_AE_UTIL "${AE_SDK_ROOT}/Util")
    endif()
    set(VHS90_AE_REQUIRED_HEADERS AE_Effect.h AE_EffectCB.h AE_EffectCBSuites.h AE_EffectUI.h AE_Macros.h)
    foreach(VHS90_HEADER IN LISTS VHS90_AE_REQUIRED_HEADERS)
        if(NOT EXISTS "${VHS90_AE_HEADERS}/${VHS90_HEADER}")
            message(FATAL_ERROR "Incomplete AE SDK: missing ${VHS90_AE_HEADERS}/${VHS90_HEADER}")
        endif()
    endforeach()
    if(NOT EXISTS "${VHS90_AE_UTIL}/Param_Utils.h")
        message(FATAL_ERROR "Incomplete AE SDK: missing Util/Param_Utils.h (checked ${VHS90_AE_INCLUDE_ROOT}/Util and ${AE_SDK_ROOT}/Util)")
    endif()
    set(VHS90_PIPL_TOOL "")
    foreach(VHS90_PIPL_CANDIDATE
        "${AE_SDK_ROOT}/Resources/PiPLtool.exe"
        "${VHS90_AE_INCLUDE_ROOT}/Resources/PiPLtool.exe"
        "${AE_SDK_ROOT}/Examples/Resources/PiPLtool.exe")
        if(EXISTS "${VHS90_PIPL_CANDIDATE}")
            set(VHS90_PIPL_TOOL "${VHS90_PIPL_CANDIDATE}")
            break()
        endif()
    endforeach()
    if(NOT VHS90_PIPL_TOOL)
        file(GLOB_RECURSE VHS90_PIPL_CANDIDATES LIST_DIRECTORIES FALSE "${AE_SDK_ROOT}/*/PiPLtool.exe")
        if(VHS90_PIPL_CANDIDATES)
            list(GET VHS90_PIPL_CANDIDATES 0 VHS90_PIPL_TOOL)
        else()
            message(FATAL_ERROR "PiPLtool.exe was not found anywhere under AE_SDK_ROOT=${AE_SDK_ROOT}")
        endif()
    endif()
    message(STATUS "Using AE headers: ${VHS90_AE_HEADERS}")
    message(STATUS "Using AE Param_Utils: ${VHS90_AE_UTIL}/Param_Utils.h")
    message(STATUS "Using PiPLtool: ${VHS90_PIPL_TOOL}")

    set(VHS90_PIPL_DIR "${CMAKE_CURRENT_BINARY_DIR}/PiPL")
    set(VHS90_PIPL_RR "${VHS90_PIPL_DIR}/VHS90PiPL.rr")
    set(VHS90_PIPL_RRC "${VHS90_PIPL_DIR}/VHS90PiPL.rrc")
    set(VHS90_PIPL_RC "${VHS90_PIPL_DIR}/VHS90PiPL.rc")
    add_custom_command(
        OUTPUT "${VHS90_PIPL_RC}"
        COMMAND "${CMAKE_COMMAND}"
            "-DPIPL_TOOL=${VHS90_PIPL_TOOL}"
            "-DC_COMPILER=${CMAKE_C_COMPILER}"
            "-DAE_HEADERS=${VHS90_AE_HEADERS}"
            "-DPIPL_R=${CMAKE_CURRENT_SOURCE_DIR}/resources/VHS90PiPL.r"
            "-DPIPL_RR=${VHS90_PIPL_RR}"
            "-DPIPL_RRC=${VHS90_PIPL_RRC}"
            "-DPIPL_RC=${VHS90_PIPL_RC}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tools/compile_pipl.cmake"
        DEPENDS resources/VHS90PiPL.r tools/compile_pipl.cmake
        VERBATIM
    )

    add_library(VHS90 MODULE Source/VHS90Effect.cpp "${VHS90_PIPL_RC}")
    target_link_libraries(VHS90 PRIVATE VHS90Core VHS90DirectWrite)
    target_include_directories(VHS90 PRIVATE
        "${VHS90_AE_HEADERS}"
        "${VHS90_AE_UTIL}"
        "${VHS90_AE_INCLUDE_ROOT}/Examples/Headers"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/VHS90Core"
    )
    target_compile_features(VHS90 PRIVATE cxx_std_17)
    target_compile_definitions(VHS90 PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(VHS90 PRIVATE /W4 /permissive- /EHsc)
    set_target_properties(VHS90 PROPERTIES
        PREFIX ""
        OUTPUT_NAME "VHS-90"
        SUFFIX ".aex"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/AEX"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/AEX"
    )
    add_custom_command(TARGET VHS90 POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/dist"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:VHS90>" "${CMAKE_CURRENT_BINARY_DIR}/dist/VHS-90.aex"
        VERBATIM
    )
    add_custom_target(VHS90_AEX DEPENDS VHS90)
endif()
