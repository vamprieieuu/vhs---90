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
    foreach(test_name IN ITEMS SignalKernels RegionEngine StaticEngine IIRFilter CompositeEngine SyncEngine TapeSurfaceEngine TapeTransportEngine BloomEngine TemporalEngine AnalogGeometryEngine AnalogNoiseEngine RfInstabilityEngine ImpulseNoiseEngine MultipathEchoEngine ReceiverInstabilityEngine ImageGradeEngine TapeDetailEngine ChromaShiftEngine WaveformEngine RingingEngine DropoutEngine SignalGhostEngine AnalogGlowEngine CrtEqualizerEngine TrackReadEngine HeadConditionEngine VcrSignalEngine PhysicalInterferenceEngine PyVhsGenerationEngine OsdEngine)
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

if(VHS90_BUILD_AE_PLUGIN)
    if(NOT WIN32 OR NOT MSVC)
        message(FATAL_ERROR "VHS90_BUILD_AE_PLUGIN requires Windows x64 and MSVC 2022.")
    endif()
    enable_language(C)
    enable_language(RC)
    if(NOT AE_SDK_ROOT OR NOT EXISTS "${AE_SDK_ROOT}/Headers/AE_Effect.h")
        message(FATAL_ERROR "Set AE_SDK_ROOT to the extracted Adobe After Effects SDK root (must contain Headers/AE_Effect.h).")
    endif()
    set(VHS90_PIPL_TOOL "${AE_SDK_ROOT}/Resources/PiPLtool.exe")
    if(NOT EXISTS "${VHS90_PIPL_TOOL}")
        set(VHS90_PIPL_TOOL "${AE_SDK_ROOT}/Examples/Resources/PiPLtool.exe")
    endif()
    if(NOT EXISTS "${VHS90_PIPL_TOOL}")
        file(GLOB_RECURSE VHS90_PIPL_CANDIDATES LIST_DIRECTORIES FALSE "${AE_SDK_ROOT}/*/PiPLtool.exe")
        if(VHS90_PIPL_CANDIDATES)
            list(GET VHS90_PIPL_CANDIDATES 0 VHS90_PIPL_TOOL)
        else()
            message(FATAL_ERROR "PiPLtool.exe was not found anywhere under AE_SDK_ROOT=${AE_SDK_ROOT}")
        endif()
    endif()

    set(VHS90_PIPL_DIR "${CMAKE_CURRENT_BINARY_DIR}/PiPL")
    set(VHS90_PIPL_RR "${VHS90_PIPL_DIR}/VHS90PiPL.rr")
    set(VHS90_PIPL_RRC "${VHS90_PIPL_DIR}/VHS90PiPL.rrc")
    set(VHS90_PIPL_RC "${VHS90_PIPL_DIR}/VHS90PiPL.rc")
    add_custom_command(
        OUTPUT "${VHS90_PIPL_RC}"
        COMMAND "${CMAKE_COMMAND}"
            "-DPIPL_TOOL=${VHS90_PIPL_TOOL}"
            "-DC_COMPILER=${CMAKE_C_COMPILER}"
            "-DAE_HEADERS=${AE_SDK_ROOT}/Headers"
            "-DPIPL_R=${CMAKE_CURRENT_SOURCE_DIR}/resources/VHS90PiPL.r"
            "-DPIPL_RR=${VHS90_PIPL_RR}"
            "-DPIPL_RRC=${VHS90_PIPL_RRC}"
            "-DPIPL_RC=${VHS90_PIPL_RC}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tools/compile_pipl.cmake"
        DEPENDS resources/VHS90PiPL.r tools/compile_pipl.cmake
        VERBATIM
    )

    add_library(VHS90 MODULE Source/VHS90Effect.cpp "${VHS90_PIPL_RC}")
    target_link_libraries(VHS90 PRIVATE VHS90Core)
    target_include_directories(VHS90 PRIVATE
        "${AE_SDK_ROOT}/Headers"
        "${AE_SDK_ROOT}/Util"
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
