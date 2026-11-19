add_library(vhs90_tape_physics STATIC
    ../vhs90_tape_physics.cpp
)
target_include_directories(vhs90_tape_physics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(vhs90_tape_physics PUBLIC cxx_std_17)
target_link_libraries(vhs90_tape_physics PUBLIC vhs90_region_effects)
