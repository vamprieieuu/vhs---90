add_library(vhs90_region_effects STATIC
    vhs90_region_effects.cpp
)
target_include_directories(vhs90_region_effects PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(vhs90_region_effects PUBLIC cxx_std_17)
