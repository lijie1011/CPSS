cmake_minimum_required(VERSION 3.10)
project(chartplugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 插件框架头文件目录（plugin/），提供 IPlugin.h / IPluginHost.h / PluginManager.h
set(PLUGIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
# 项目根目录（src/）与 core 目录（提供 displaystate.h 等共享头文件）
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
# 数据层目录（datalib/），提供 dynamicdata.h / datamanager.h 等
set(DATALIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../datalib)

set(SOURCES
    ChartPlugin.cpp
    graphicsviewwidget.cpp
    platformitem.cpp
    propertyboxitem.cpp
)

set(HEADERS
    ChartPlugin.h
    IChartPlugin.h
    graphicsviewwidget.h
    platformitem.h
    propertyboxitem.h
)

add_library(chartplugin SHARED ${SOURCES} ${HEADERS})

if(WIN32)
    target_compile_options(chartplugin PRIVATE /utf-8)
endif()

target_include_directories(chartplugin PRIVATE
    ${PLUGIN_INCLUDE_DIR}
    ${SRC_ROOT_DIR}
    ${CORE_DIR}
    ${SRC_ROOT_DIR}/common
    ${DATALIB_DIR}
    ${ENCLIB_INCLUDE_DIR}
)

target_link_libraries(chartplugin PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    cpss_plugin_framework
    cpss_data
)

if(WIN32)
    # Enclib 静态库：海图渲染与初始化都在本 DLL 内使用
    target_link_libraries(chartplugin PRIVATE
        ${ENCLIB_BIN_DIR}/enccore.lib
    )
endif()

set_target_properties(chartplugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/plugins
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/plugins
)
