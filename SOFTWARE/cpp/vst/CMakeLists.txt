cmake_minimum_required(VERSION 3.22)
project(WBS_M405D_Extended_Range_Meter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(WBS_M405D_Extended_Range_Meter STATIC
    PluginProcessor.hpp
    PluginProcessor.cpp
    PluginEditor.hpp
    PluginEditor.cpp
)

target_link_libraries(WBS_M405D_Extended_Range_Meter PUBLIC wbs_core)
target_include_directories(WBS_M405D_Extended_Range_Meter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
