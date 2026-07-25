cmake_minimum_required(VERSION 3.10)
project(cpss_plugin_framework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    set(Qt5_DIR "E:/Qt5.12.11/5.12.11/msvc2017_64/lib/cmake/Qt5")
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS
    Widgets
    Gui
    Core
)

set(FRAMEWORK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    PluginManager.cpp
)

set(HEADERS
    IPlugin.h
    IPluginHost.h
    PluginManager.h
)

add_library(cpss_plugin_framework STATIC ${SOURCES} ${HEADERS})

target_include_directories(cpss_plugin_framework PUBLIC
    ${FRAMEWORK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../datalib
)

target_link_libraries(cpss_plugin_framework PUBLIC
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    cpss_common
)

set_target_properties(cpss_plugin_framework PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

add_subdirectory(sampleplugin)
add_subdirectory(helloplugin)