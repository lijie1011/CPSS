set(COMMON_SOURCES
    logger.cpp
    config.cpp
    xmlconfig.cpp
    IconManager.cpp
)

set(COMMON_HEADERS
    logger.h
    config.h
    xmlconfig.h
    IconManager.h
)

add_library(cpss_common SHARED
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)

set_target_properties(cpss_common PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

target_compile_definitions(cpss_common PRIVATE
    CPSS_COMMON_EXPORT
)

target_include_directories(cpss_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cpss_common PRIVATE
    Qt5::Core
    Qt5::Xml
    Qt5::Gui
)

if(WIN32)
    target_compile_options(cpss_common PRIVATE /utf-8)
endif()
