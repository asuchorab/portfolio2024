cmake_minimum_required(VERSION 3.13)
project(luaext)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")

endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_NAME luaext_debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXECUTABLE_NAME luaext)
endif()

include_directories(src)

set(LUAEXT_SRC src/main.cpp src/Lexer.cpp src/Lexer.h src/Parser.cpp src/Parser.h)
add_executable(${EXECUTABLE_NAME} ${LUAEXT_SRC})