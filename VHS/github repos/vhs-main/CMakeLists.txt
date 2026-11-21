cmake_minimum_required(VERSION 3.20)
project(LofiFxVhs LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# MSVC: enable standards-conforming preprocessor + multi-process
# build, disable spurious "secure CRT" warnings on POSIX-style APIs
# (snprintf, fopen, sscanf etc) used in shared parser code.
# ============================================================
if(MSVC)
    add_compile_options(/W3 /MP /Zc:__cplusplus /utf-8 /EHsc)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_NONSTDC_NO_DEPRECATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# ============================================================
# Fetch OpenFX SDK headers
# ============================================================
include(FetchContent)
FetchContent_Declare(
    openfx
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openfx.git
    GIT_TAG        OFX_Release_1.5.1
)
FetchContent_GetProperties(openfx)
if(NOT openfx_POPULATED)
    FetchContent_Populate(openfx)
endif()

set(OFX_INCLUDE_DIR "${openfx_SOURCE_DIR}/include")

# ============================================================
# Platform detection
# ============================================================
if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 17)
endif()

# ============================================================
# Embed the shared VHS kernel source (VhsKernelCore.inc) into a
# C++ string-literal header so the Metal runtime compiler can
# consume the same text that the CUDA preprocessor will #include
# directly. Re-runs automatically whenever the .inc changes.
# ============================================================
set(VHS_KERNEL_CORE_INC "${CMAKE_CURRENT_SOURCE_DIR}/src/VhsKernelCore.inc")
set(VHS_KERNEL_CORE_TEXT_GEN
    "${CMAKE_CURRENT_BINARY_DIR}/generated/VhsKernelCoreText.h")

file(READ "${VHS_KERNEL_CORE_INC}" VHS_KERNEL_CORE_TEXT)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/VhsKernelCoreText.h.in"
    "${VHS_KERNEL_CORE_TEXT_GEN}"
    @ONLY
)
# Re-configure if the .inc content changes.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${VHS_KERNEL_CORE_INC}")

# ============================================================
# Sources
# ============================================================
set(PLUGIN_SOURCES
    src/LofiFxVhsPlugin.cpp
    src/CudaKernels.cpp
    src/PresetParser.cpp
    src/OsdRenderer.cpp
)

if(APPLE)
    list(APPEND PLUGIN_SOURCES src/MetalKernels.mm)
endif()

# ============================================================
# Build the .ofx shared library
# ============================================================
add_library(LofiFxVhs MODULE ${PLUGIN_SOURCES})

target_include_directories(LofiFxVhs PRIVATE
    ${OFX_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

set_target_properties(LofiFxVhs PROPERTIES
    PREFIX ""
    SUFFIX ".ofx"
    CXX_VISIBILITY_PRESET default
    C_VISIBILITY_PRESET   default
)

if(APPLE)
    target_link_libraries(LofiFxVhs
        "-framework Metal"
        "-framework Foundation"
    )
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
elseif(WIN32)
    # No extra libraries needed for the v1 stub CUDA path. When CUDA
    # support is added, link against CUDA::cudart_static here.
elseif(UNIX)
    # Same — Linux gets CUDA via FindCUDAToolkit when implemented.
endif()

# ============================================================
# Bundle layout — OFX spec subdirectory per platform
# ============================================================
set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/LofiFxVhs.ofx.bundle")

if(APPLE)
    set(BUNDLE_BIN_DIR "${BUNDLE_DIR}/Contents/MacOS")
elseif(WIN32)
    set(BUNDLE_BIN_DIR "${BUNDLE_DIR}/Contents/Win64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUNDLE_BIN_DIR "${BUNDLE_DIR}/Contents/Linux-x86-64")
else()
    set(BUNDLE_BIN_DIR "${BUNDLE_DIR}/Contents/Linux-x86")
endif()
set(BUNDLE_RES_DIR    "${BUNDLE_DIR}/Contents/Resources")
set(BUNDLE_PRESET_DIR "${BUNDLE_DIR}/Contents/Resources/Presets")

# ------------------------------------------------------------
# Post-build: stage the .ofx + Info.plist into the bundle
# ------------------------------------------------------------
add_custom_command(TARGET LofiFxVhs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_RES_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_PRESET_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:LofiFxVhs>"
        "${BUNDLE_BIN_DIR}/LofiFxVhs.ofx"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        "${BUNDLE_DIR}/Contents/Info.plist"
    COMMENT "Assembling LofiFxVhs.ofx.bundle"
)

# ------------------------------------------------------------
# Stage shared CST LUTs from the Film plugin's bundled set so
# both plugins offer the same Input/Output Transform dropdowns.
# Looks/ is VHS-specific.
# ------------------------------------------------------------
set(SHARED_CST_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/../spectra/LUTs/CST")
set(STAGED_LUTS_DIR "${CMAKE_BINARY_DIR}/staged_luts")

add_custom_command(TARGET LofiFxVhs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${STAGED_LUTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${STAGED_LUTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHARED_CST_DIR}"
        "${STAGED_LUTS_DIR}/CST"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/LUTs/Looks"
        "${STAGED_LUTS_DIR}/Looks"
    COMMENT "Staging CST + Looks for resources.pak"
)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(TARGET LofiFxVhs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${BUNDLE_RES_DIR}/resources.pak"
    COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/pack_luts.py"
        "${STAGED_LUTS_DIR}"
        "${BUNDLE_RES_DIR}/resources.pak"
    COMMENT "Packing LUTs into obfuscated resources.pak"
)

# Copy preset files into bundle
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/presets")
    add_custom_command(TARGET LofiFxVhs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${BUNDLE_PRESET_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/presets"
            "${BUNDLE_PRESET_DIR}"
        COMMENT "Syncing preset files into bundle (clean copy)"
    )
endif()

# ============================================================
# Install + install_local — per-platform OFX plugin directory.
#   macOS:   /Library/OFX/Plugins
#   Linux:   /usr/OFX/Plugins
#   Windows: C:\Program Files\Common Files\OFX\Plugins
# ============================================================
if(APPLE)
    set(OFX_PLUGIN_INSTALL_DIR "/Library/OFX/Plugins")
elseif(WIN32)
    if(DEFINED ENV{CommonProgramFiles})
        set(OFX_PLUGIN_INSTALL_DIR "$ENV{CommonProgramFiles}/OFX/Plugins")
    else()
        set(OFX_PLUGIN_INSTALL_DIR "C:/Program Files/Common Files/OFX/Plugins")
    endif()
else()
    set(OFX_PLUGIN_INSTALL_DIR "/usr/OFX/Plugins")
endif()

install(
    DIRECTORY "${BUNDLE_DIR}"
    DESTINATION "${OFX_PLUGIN_INSTALL_DIR}"
    USE_SOURCE_PERMISSIONS
)

add_custom_target(install_local
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${OFX_PLUGIN_INSTALL_DIR}/LofiFxVhs.ofx.bundle"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${BUNDLE_DIR}"
        "${OFX_PLUGIN_INSTALL_DIR}/LofiFxVhs.ofx.bundle"
    DEPENDS LofiFxVhs
    COMMENT "Installing to ${OFX_PLUGIN_INSTALL_DIR} (clean copy)"
)
