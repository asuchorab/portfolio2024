cmake_minimum_required(VERSION 3.9)
project(minesweeper)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_NAME minesweeper_debug)
    if (WIN32)
        set(LIBS sfml-graphics-d sfml-window-d sfml-system-d)
    else ()
        set(LIBS sfml-graphics sfml-window sfml-system)
    endif (WIN32)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXECUTABLE_NAME minesweeper)
    set(LIBS sfml-graphics sfml-window sfml-system)
endif ()

include_directories(include)
link_directories(lib)

add_executable(${EXECUTABLE_NAME} src/main.cpp src/GameInstance.cpp src/GameInstance.h src/MinesweeperCell.h src/MinesweeperConfig.h src/WindowManager.cpp src/WindowManager.h src/TimeUtil.h src/Core.cpp src/Core.h src/menu/MenuButton.cpp src/menu/MenuButton.h src/menu/MenuItem.cpp src/menu/MenuItem.h src/menu/MenuIntInput.cpp src/menu/MenuIntInput.h src/menu/MenuList.cpp src/menu/MenuList.h src/solvers/Solver.h src/solvers/ObviousMovesSolver.cpp src/solvers/ObviousMovesSolver.h src/solvers/Solver.cpp src/menu/MenuEnumChoice.cpp src/menu/MenuEnumChoice.h src/solvers/DumbCellGroupsSolver.cpp src/solvers/DumbCellGroupsSolver.h src/solvers/CellGroupsSolver.cpp src/solvers/CellGroupsSolver.h src/solvers/CellGroups.h)
target_link_libraries(${EXECUTABLE_NAME} "${LIBS}")
