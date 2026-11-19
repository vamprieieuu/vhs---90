cmake_minimum_required(VERSION 3.20)
project(VHS90Core LANGUAGES CXX)

option(VHS90_BUILD_TESTS "Build host-independent VHS-90 CPU smoke tests" ON)

add_library(VHS90Core STATIC
    Source/VHS90Core/VHS90SignalKernels.cpp
    Source/VHS90Core/VHS90RegionEngine.cpp
    Source/VHS90Core/VHS90StaticEngine.cpp
    Source/VHS90Core/VHS90IIRFilter.cpp
    Source/VHS90Core/VHS90CompositeEngine.cpp
)
target_include_directories(VHS90Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/VHS90Core
)
target_compile_features(VHS90Core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(VHS90Core PRIVATE /W4 /permissive-)
else()
    target_compile_options(VHS90Core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(VHS90_BUILD_TESTS)
    enable_testing()

    add_executable(VHS90SignalKernelsSmoke Tests/VHS90SignalKernelsSmoke.cpp)
    target_link_libraries(VHS90SignalKernelsSmoke PRIVATE VHS90Core)
    add_test(NAME VHS90SignalKernelsSmoke COMMAND VHS90SignalKernelsSmoke)

    add_executable(VHS90RegionEngineSmoke Tests/VHS90RegionEngineSmoke.cpp)
    target_link_libraries(VHS90RegionEngineSmoke PRIVATE VHS90Core)
    add_test(NAME VHS90RegionEngineSmoke COMMAND VHS90RegionEngineSmoke)

    add_executable(VHS90StaticEngineSmoke Tests/VHS90StaticEngineSmoke.cpp)
    target_link_libraries(VHS90StaticEngineSmoke PRIVATE VHS90Core)
    add_test(NAME VHS90StaticEngineSmoke COMMAND VHS90StaticEngineSmoke)

    add_executable(VHS90IIRFilterSmoke Tests/VHS90IIRFilterSmoke.cpp)
    target_link_libraries(VHS90IIRFilterSmoke PRIVATE VHS90Core)
    add_test(NAME VHS90IIRFilterSmoke COMMAND VHS90IIRFilterSmoke)

    add_executable(VHS90CompositeEngineSmoke Tests/VHS90CompositeEngineSmoke.cpp)
    target_link_libraries(VHS90CompositeEngineSmoke PRIVATE VHS90Core)
    add_test(NAME VHS90CompositeEngineSmoke COMMAND VHS90CompositeEngineSmoke)
endif()
