cmake_minimum_required(VERSION 3.29)
cmake_policy(SET CMP0076 NEW)


project(iPlug2)

set(IPLUG2_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(iPlug2.cmake)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(WDL)
add_subdirectory(Dependencies)

add_subdirectory(IPlug)
add_subdirectory(IGraphics)

option(IPLUG2_BUILD_EXAMPLES "Build examples" ON)

if (IPLUG2_BUILD_EXAMPLES)
    add_subdirectory(Examples/IPlugControls)
    add_subdirectory(Examples/IPlugEffect)
endif()
