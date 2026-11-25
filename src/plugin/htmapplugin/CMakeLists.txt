cmake_minimum_required(VERSION 3.10)
project(htmap VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(DATALIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../datalib)

set(SOURCES
    HtMapPlugin.cpp
    MapWidget.cpp
    TileProvider.cpp
    WmsProvider.cpp
)

set(HEADERS
    HtMapPlugin.h
    MapWidget.h
    TileProvider.h
    WmsProvider.h
)

add_library(htmapplugin SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_options(htmapplugin PRIVATE /utf-8)
endif()

target_include_directories(htmapplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
    ${DATALIB_INCLUDE_DIR}
)

target_link_libraries(htmapplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    cpss_plugin_framework
    cpss_data
    cpss_common
)

set_target_properties(htmapplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)
