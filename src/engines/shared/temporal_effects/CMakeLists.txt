add_library(vhs90_temporal_effects STATIC
    ../vhs90_temporal_effects.cpp
)
target_include_directories(vhs90_temporal_effects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(vhs90_temporal_effects PUBLIC cxx_std_17)
target_link_libraries(vhs90_temporal_effects PUBLIC vhs90_region_effects)
