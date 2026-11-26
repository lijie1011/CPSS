cmake_minimum_required(VERSION 3.10)
project(hosierydataplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(DATALIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../datalib)
set(FASTDDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../datalib/comm/fastdds)
set(REDIS_INCLUDE_DIR ${THIRDPARTY_DIR}/redis_sdk/include)

set(SOURCES
    HosieryDataPlugin.cpp
    ReplayWidget.cpp
    recfile.cpp
    recorder.cpp
)

set(HEADERS
    HosieryDataPlugin.h
    ReplayWidget.h
    recfile.h
    recorder.h
)

add_library(hosierydataplugin SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_options(hosierydataplugin PRIVATE /utf-8)
endif()

target_include_directories(hosierydataplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
    ${DATALIB_INCLUDE_DIR}
    ${FASTDDS_INCLUDE_DIR}
    ${REDIS_INCLUDE_DIR}
)

target_link_libraries(hosierydataplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    cpss_plugin_framework
    cpss_data
    cpss_common
)

set_target_properties(hosierydataplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)
