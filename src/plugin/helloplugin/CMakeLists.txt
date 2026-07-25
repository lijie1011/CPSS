cmake_minimum_required(VERSION 3.10)
project(helloplugin VERSION 1.0.0 LANGUAGES CXX)

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

set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(SOURCES
    helloplugin.cpp
    HelloWidget.cpp
)

set(HEADERS
    helloplugin.h
    HelloWidget.h
)

add_library(helloplugin SHARED ${SOURCES} ${HEADERS})

target_include_directories(helloplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
)

target_link_libraries(helloplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    cpss_plugin_framework
)

set_target_properties(helloplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)