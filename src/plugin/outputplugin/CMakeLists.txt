cmake_minimum_required(VERSION 3.10)
project(outputplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(COMMON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

set(SOURCES
    OutputPlugin.cpp
    OutputWidget.cpp
)

set(HEADERS
    OutputPlugin.h
    OutputWidget.h
)

add_library(outputplugin SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_options(outputplugin PRIVATE /utf-8)
endif()

target_include_directories(outputplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
    ${COMMON_INCLUDE_DIR}
)

target_link_libraries(outputplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    cpss_plugin_framework
    cpss_common
)

set_target_properties(outputplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)
