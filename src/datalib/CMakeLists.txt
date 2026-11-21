set(DATALIB_SOURCES
    datamanager.cpp
    datacache.cpp
    comm/commmanager.cpp
    comm/http/httpadapter.cpp
    comm/fastdds/ddsadapter.cpp
    comm/fastdds/ddsconverter.cpp
)

set(DATALIB_HEADERS
    dynamicdata.h
    protocoladapter.h
    datamanager.h
    datacache.h
    comm/commmanager.h
    comm/http/httpadapter.h
    comm/fastdds/ddsadapter.h
    comm/fastdds/ddsconverter.h
)

add_library(cpss_data SHARED
    ${DATALIB_SOURCES}
    ${DATALIB_HEADERS}
)

if(WIN32)
    target_compile_options(cpss_data PRIVATE /utf-8)
endif()

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
    ${CMAKE_CURRENT_SOURCE_DIR}/comm
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/http
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/dds
    ${PROJECT_SOURCE_DIR}/common
)

target_link_libraries(cpss_data PRIVATE
    cpss_common
    Qt5::Core
    Qt5::Gui
    Qt5::Network
)

# ============================================================
#  Fast DDS 可选依赖
#  未安装时自动降级为桩实现（CPSS_ENABLE_DDS 不定义），工程照常编译。
#  安装方式：vcpkg install fastdds:x64-windows
#  启用：cmake -S src -B src/build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
# ============================================================
find_package(fastdds CONFIG QUIET)
if(fastdds_FOUND)
    message(STATUS "Fast DDS found: ${fastdds_DIR} -> enabling CPSS_ENABLE_DDS")
    target_compile_definitions(cpss_data PRIVATE CPSS_ENABLE_DDS)
    target_link_libraries(cpss_data PRIVATE fastdds fastcdr)

    # fastddsgen 生成的代码目录（先手工生成后放入 fastdds/generated）
    set(DDS_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/comm/fastdds/generated)
    if(EXISTS "${DDS_GENERATED_DIR}/CPSSDataCdr.h")
        target_include_directories(cpss_data PRIVATE ${DDS_GENERATED_DIR})
        target_sources(cpss_data PRIVATE
            ${DDS_GENERATED_DIR}/CPSSDataCdr.cxx
            ${DDS_GENERATED_DIR}/CPSSDataPubSubTypes.cxx
        )
    else()
        message(WARNING "Fast DDS 已找到，但未发现生成的 IDL 代码 ${DDS_GENERATED_DIR}，DDS 收发无法工作")
    endif()
else()
    message(STATUS "Fast DDS not found -> DDS adapter builds as stub (no real communication)")
endif()
