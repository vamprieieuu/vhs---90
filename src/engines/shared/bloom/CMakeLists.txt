add_library(vhs90_bloom STATIC
    ../vhs90_bloom.cpp
)
target_include_directories(vhs90_bloom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_features(vhs90_bloom PUBLIC cxx_std_17)
target_link_libraries(vhs90_bloom PUBLIC vhs90_region_effects)
