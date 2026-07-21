set(DATALIB_SOURCES
    datamanager.cpp
    udpadapter.cpp
    tcpadapter.cpp
    httpadapter.cpp
    websocketadapter.cpp
    ddsadapter.cpp
    redisadapter.cpp
)

set(DATALIB_HEADERS
    dynamicdata.h
    protocoladapter.h
    datamanager.h
    udpadapter.h
    tcpadapter.h
    httpadapter.h
    websocketadapter.h
    ddsadapter.h
    redisadapter.h
)

add_library(cpss_data STATIC
    ${DATALIB_SOURCES}
    ${DATALIB_HEADERS}
)

target_include_directories(cpss_data PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

target_link_libraries(cpss_data PRIVATE
    Qt5::Core
    Qt5::Network
    Qt5::WebSockets
)



add_subdirectory(tests)
