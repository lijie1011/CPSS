cmake_minimum_required(VERSION 3.10)
project(CPSS VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    set(PLATFORM_NAME "windows")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

message(STATUS "Building CPSS for platform: ${PLATFORM_NAME}")

find_package(Qt5 5.12 REQUIRED COMPONENTS
    Widgets
    Gui
    Core
    Network
    WebSockets
)

set(ENCLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3dParty/Enclib)
set(ENCLIB_INCLUDE_DIR ${ENCLIB_DIR}/include)
set(ENCLIB_BIN_DIR ${ENCLIB_DIR}/bin)

if(NOT EXISTS "${ENCLIB_INCLUDE_DIR}/encl.h")
    message(FATAL_ERROR "Enclib headers not found at ${ENCLIB_INCLUDE_DIR}\n"
                        "Please copy Enclib SDK from ChartDemo to src/3dParty/Enclib")
endif()

add_subdirectory(datalib)

set(SOURCES
    main.cpp
    mainwindow.cpp
    viewwidget.cpp
    webserver.cpp
    chartbridge.cpp
    common/config.cpp
    common/logger.cpp
)

set(HEADERS
    mainwindow.h
    viewwidget.h
    webserver.h
    chartbridge.h
    common/config.h
    common/logger.h
)

add_executable(cpss WIN32 ${SOURCES} ${HEADERS})

target_include_directories(cpss PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/datalib
    ${ENCLIB_INCLUDE_DIR}
)

target_link_libraries(cpss PRIVATE
    cpss_data
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
)

if(WIN32)
    target_link_libraries(cpss PRIVATE
        ${ENCLIB_BIN_DIR}/enccore.lib
    )
    target_compile_definitions(cpss PRIVATE
        OS_WINDOWS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _UNICODE
        UNICODE
    )
    

elseif(UNIX)
    target_link_libraries(cpss PRIVATE
        ${ENCLIB_BIN_DIR}/libenccore.so
        pthread
        dl
    )
    target_compile_definitions(cpss PRIVATE
        OS_LINUX
    )
    set_target_properties(cpss PROPERTIES
        BUILD_RPATH "$ORIGIN/../lib"
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

include(GNUInstallDirs)

install(TARGETS cpss
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${ENCLIB_BIN_DIR}/enccore.dll
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${ENCLIB_BIN_DIR}/libenccore.so
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${ENCLIB_DIR}/mapdata
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${ENCLIB_DIR}/fonts
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${ENCLIB_DIR}/config.xml ${ENCLIB_DIR}/lic.dat
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(WIN32)
    file(WRITE ${CMAKE_BINARY_DIR}/deploy.bat
"@echo off
windeployqt --release --compiler-runtime bin\\\\cpss.exe
copy ..\\\\3dParty\\\\Enclib\\\\bin\\\\enccore.dll bin\\\\
copy ..\\\\3dParty\\\\Enclib\\\\bin\\\\msn.exe bin\\\\
copy ..\\\\3dParty\\\\Enclib\\\\bin\\\\s52symbol.exe bin\\\\
xcopy /E /I ..\\\\3dParty\\\\Enclib\\\\mapdata bin\\\\mapdata
xcopy /E /I ..\\\\3dParty\\\\Enclib\\\\fonts bin\\\\fonts
copy ..\\\\3dParty\\\\Enclib\\\\config.xml bin\\\\
copy ..\\\\3dParty\\\\Enclib\\\\lic.dat bin\\\\
echo Deploy complete!
"
    )
elseif(UNIX)
    file(WRITE ${CMAKE_BINARY_DIR}/deploy.sh
"#!/bin/bash
set -e
linuxdeployqt bin/cpss -verbose=2
cp ../3dParty/Enclib/bin/libenccore.so bin/
cp ../3dParty/Enclib/bin/msn bin/
cp ../3dParty/Enclib/bin/s52symbol bin/
cp -r ../3dParty/Enclib/mapdata bin/
cp -r ../3dParty/Enclib/fonts bin/
cp ../3dParty/Enclib/config.xml bin/
cp ../3dParty/Enclib/lic.dat bin/
echo 'Deploy complete!'
"
    )
    execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/deploy.sh)
endif()
