add_library(vhs90_analog_signal STATIC
    vhs90_signal_pipeline.cpp
)
target_include_directories(vhs90_analog_signal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(vhs90_analog_signal PUBLIC cxx_std_17)
target_link_libraries(vhs90_analog_signal PUBLIC vhs90_region_effects)
