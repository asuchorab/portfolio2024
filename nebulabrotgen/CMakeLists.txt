cmake_minimum_required(VERSION 3.13)
project(nebulabrotgen)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_NAME nebulabrotgen_debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXECUTABLE_NAME nebulabrotgen)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_executable(${EXECUTABLE_NAME} main.cpp libnebulabrotgen.cpp buddhabrotrenderer.hpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${EXECUTABLE_NAME} pthread stdc++fs)
endif()