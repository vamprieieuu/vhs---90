add_library(vhs90_region_effects STATIC
    vhs90_region_effects.cpp
)
target_include_directories(vhs90_region_effects PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(vhs90_region_effects PUBLIC cxx_std_17)
add_subdirectory(analog_signal)
add_subdirectory(tape_surface)
add_subdirectory(image_degradation)
add_subdirectory(bloom)
add_subdirectory(../analog_signal/ntsc_crt ${CMAKE_CURRENT_BINARY_DIR}/ntsc_crt)
add_subdirectory(tape_physics)
