add_library(vhs90_tape_surface STATIC
    ../vhs90_tape_surface.cpp
)
target_include_directories(vhs90_tape_surface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(vhs90_tape_surface PUBLIC cxx_std_17)
target_link_libraries(vhs90_tape_surface PUBLIC vhs90_region_effects)
