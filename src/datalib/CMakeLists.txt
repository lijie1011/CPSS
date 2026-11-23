set(DATALIB_SOURCES
    datamanager.cpp
    datacache.cpp
    comm/commmanager.cpp
    comm/http/httpadapter.cpp
    comm/fastdds/ddsadapter.cpp
    comm/fastdds/ddsconverter.cpp
    comm/redis/redisadapter.cpp
    comm/redis/redis_fieldmap.cpp
    comm/rtidds/situationforwarder.cpp
    comm/pgdds/pgddsmanager.cpp
    comm/pgdds/pgddsadapter.cpp
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
    comm/redis/redisadapter.h
    comm/redis/redis_fieldmap.h
    comm/rtidds/situationforwarder.h
    comm/pgdds/pgddsmanager.h
    comm/pgdds/pgddsadapter.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/fastdds
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/redis
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/rtidds
    ${CMAKE_CURRENT_SOURCE_DIR}/comm/pgdds
    ${PROJECT_SOURCE_DIR}/common
)

target_link_libraries(cpss_data PRIVATE
    cpss_common
    Qt5::Core
    Qt5::Gui
    Qt5::Network
)

# ============================================================
#  Fast DDS 可选依赖（未安装时降级为桩实现）
#  安装：vcpkg install fastdds:x64-windows
# ============================================================
find_package(fastdds CONFIG QUIET)
if(fastdds_FOUND)
    message(STATUS "Fast DDS found: ${fastdds_DIR} -> enabling CPSS_ENABLE_DDS")
    target_compile_definitions(cpss_data PRIVATE CPSS_ENABLE_DDS)
    target_link_libraries(cpss_data PRIVATE fastdds fastcdr)

    set(DDS_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/comm/fastdds/generated)
    if(EXISTS "${DDS_GENERATED_DIR}/CPSSDataCdr.h")
        target_include_directories(cpss_data PRIVATE ${DDS_GENERATED_DIR})
        target_sources(cpss_data PRIVATE
            ${DDS_GENERATED_DIR}/CPSSDataCdr.cxx
            ${DDS_GENERATED_DIR}/CPSSDataPubSubTypes.cxx
        )
    else()
        message(WARNING "Fast DDS 已找到，但未发现生成的 IDL 代码 ${DDS_GENERATED_DIR}")
    endif()
else()
    message(STATUS "Fast DDS not found -> DDS adapter builds as stub")
endif()

# ============================================================
#  Redis SDK（AFSIM 态势数据接入）
#  位置：src/3dParty/redis_sdk
#  依赖顺序：必须先找 protobuf / hiredis / ZLIB，再找 RedisSDK
# ============================================================
set(REDIS_SDK_ROOT "${PROJECT_SOURCE_DIR}/3dParty/redis_sdk")
if(EXISTS "${REDIS_SDK_ROOT}/lib/cmake/RedisSDK/RedisSDKConfig.cmake")
    find_package(protobuf CONFIG REQUIRED PATHS "${REDIS_SDK_ROOT}/cmake" NO_DEFAULT_PATH)
    find_package(hiredis CONFIG REQUIRED PATHS "${REDIS_SDK_ROOT}/share/hiredis" NO_DEFAULT_PATH)
    find_package(ZLIB CONFIG REQUIRED PATHS "${REDIS_SDK_ROOT}/lib/cmake/zlib" NO_DEFAULT_PATH)
    find_package(RedisSDK 1.0 CONFIG REQUIRED PATHS "${REDIS_SDK_ROOT}/lib/cmake/RedisSDK" NO_DEFAULT_PATH)

    message(STATUS "Redis SDK found -> enabling Redis adapter (Release only)")
    target_include_directories(cpss_data PRIVATE ${REDIS_SDK_ROOT}/include)
    # Redis SDK 只有 Release(/MD) 版，Debug(/MDd) 链接会 CRT 冲突崩溃，
    # 故仅在 Release 及非多配置生成器下链接并定义 CPSS_ENABLE_REDIS
    target_link_libraries(cpss_data PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:Redis::redis_middleware>
        $<$<NOT:$<CONFIG:Debug>>:hiredis::hiredis>
        $<$<NOT:$<CONFIG:Debug>>:ZLIB::ZLIB>
    )
    target_compile_definitions(cpss_data PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:CPSS_ENABLE_REDIS>
    )
else()
    message(WARNING "Redis SDK not found at ${REDIS_SDK_ROOT} -> Redis adapter may fail to compile")
endif()

# ============================================================
#  RTI Connext DDS（态势显示转发，可选）
#  商业库，需设置 NDDSHOME 并安装授权；未安装时降级为桩实现
# ============================================================
if(DEFINED ENV{NDDSHOME} AND EXISTS "$ENV{NDDSHOME}")
    message(STATUS "NDDSHOME found: $ENV{NDDSHOME} -> TODO: integrate RTI Connext DDS")
    # TODO: 安装 RTI Connext 后：find_package(RTIConnextDDS) + target_compile_definitions(CPSS_ENABLE_RTI_DDS)
else()
    message(STATUS "RTI Connext DDS not found -> situation forwarder builds as stub")
endif()

# ============================================================
#  PGDDS SDK（国产 DDS，nonfixed 动态类型 C API）
#  位置：src/3dParty/pgdds（从 DDS-SDK C v1.0.7 win64 复制）
#  未找到时适配器仍编译（头文件在工程内），但链接会失败，故用条件排除
# ============================================================
set(PGDD_SDK_ROOT "${PROJECT_SOURCE_DIR}/3dParty/pgdds")
if(EXISTS "${PGDD_SDK_ROOT}/lib/dds_nonfixed.lib")
    message(STATUS "PGDDS SDK found -> enabling PGDDS adapter")
    target_compile_definitions(cpss_data PRIVATE CPSS_ENABLE_PGDDS)
    target_include_directories(cpss_data PRIVATE ${PGDD_SDK_ROOT}/include)
    target_link_libraries(cpss_data PRIVATE ${PGDD_SDK_ROOT}/lib/dds_nonfixed.lib)

    # 运行期部署 DLL 到生成目录（Debug/Release 均需）
    add_custom_command(TARGET cpss_data POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PGDD_SDK_ROOT}/bin/dds_nonfixed.dll"
                $<TARGET_FILE_DIR:cpss_data>
        COMMENT "Copying dds_nonfixed.dll to output dir"
    )
else()
    message(WARNING "PGDDS SDK not found at ${PGDD_SDK_ROOT}/lib/dds_nonfixed.lib -> PGDDS disabled")
endif()
