add_library(vhs90_image_degradation STATIC
    ../vhs90_image_degradation.cpp
)
target_include_directories(vhs90_image_degradation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(vhs90_image_degradation PUBLIC cxx_std_17)
target_link_libraries(vhs90_image_degradation PUBLIC vhs90_region_effects)
