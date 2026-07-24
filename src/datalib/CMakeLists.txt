set(DATALIB_SOURCES
    datamanager.cpp
    datacache.cpp
    httpadapter.cpp
)

set(DATALIB_HEADERS
    dynamicdata.h
    protocoladapter.h
    datamanager.h
    datacache.h
    httpadapter.h
)

add_library(cpss_data SHARED
    ${DATALIB_SOURCES}
    ${DATALIB_HEADERS}
)

set_target_properties(cpss_data PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
)

target_compile_definitions(cpss_data PRIVATE
    CPSS_DATA_EXPORT
)

target_include_directories(cpss_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/common
)

target_link_libraries(cpss_data PRIVATE
    cpss_common
    Qt5::Core
    Qt5::Network
)