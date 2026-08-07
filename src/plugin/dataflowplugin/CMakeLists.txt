cmake_minimum_required(VERSION 3.10)
project(dataflowplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(SOURCES
    DataFlowPlugin.cpp
    DataFlowWidget.cpp
    NodeFlowWidget.cpp
)

set(HEADERS
    DataFlowPlugin.h
    DataFlowWidget.h
    NodeFlowWidget.h
)

add_library(dataflowplugin SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_options(dataflowplugin PRIVATE /utf-8)
endif()

target_include_directories(dataflowplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
)

target_link_libraries(dataflowplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    cpss_plugin_framework
    cpss_data
    cpss_common
)

set_target_properties(dataflowplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)