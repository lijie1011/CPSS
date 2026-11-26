# unpack_resources.cmake
# cmake configure 时调用：把 src/3dParty 下所有 cpss_* 压缩包解压到 build/bin/Release/3dParty/
# 支持 .zip / .zip.001(分卷) / .7z
# stamp 判断：若 .unpacked 标记比所有压缩包都新则跳过（压缩包变化才重新解压）

if(NOT WIN32)
    return()
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3dParty")
set(DST_DIR "${CMAKE_BINARY_DIR}/bin/Release/3dParty")
set(STAMP "${DST_DIR}/.unpacked")

# 查找 7z
find_program(SEVENZIP NAMES 7z 7za
    PATHS "C:/Program Files/7-Zip" "C:/Program Files (x86)/7-Zip"
    NO_DEFAULT_PATH)
if(NOT SEVENZIP)
    message(WARNING "[unpack] 7z not found, skip unpacking resources")
    return()
endif()

# 收集所有 cpss_* 压缩包：分卷取 .001，单文件 zip/7z 直接取
file(GLOB ALL_CPSS_FILES "${SRC_DIR}/cpss_*")
set(ARCHIVES "")
set(NEWEST_ARCHIVE "")
foreach(f ${ALL_CPSS_FILES})
    get_filename_component(name "${f}" NAME)
    set(is_archive FALSE)
    if(name MATCHES "\\.zip\\.[0-9]+$")
        if(name MATCHES "\\.zip\\.001$")
            set(is_archive TRUE)
        endif()
    elseif(name MATCHES "\\.(zip|7z)$")
        set(is_archive TRUE)
    endif()
    if(is_archive)
        list(APPEND ARCHIVES "${f}")
        if(EXISTS "${STAMP}" AND "${f}" IS_NEWER_THAN "${STAMP}")
            set(NEWEST_ARCHIVE "${f}")
        endif()
    endif()
endforeach()

if(NOT ARCHIVES)
    message(STATUS "[unpack] no cpss_* archives found")
    return()
endif()

# stamp 判断：已解压且没有压缩包更新 → 跳过
if(EXISTS "${STAMP}" AND "${NEWEST_ARCHIVE}" STREQUAL "")
    message(STATUS "[unpack] all archives already unpacked (stamp up-to-date), skip")
    return()
endif()

list(LENGTH ARCHIVES N)
message(STATUS "[unpack] found ${N} cpss_* archives, unpacking to ${DST_DIR}...")

file(MAKE_DIRECTORY "${DST_DIR}")
foreach(arch ${ARCHIVES})
    get_filename_component(name "${arch}" NAME)
    message(STATUS "[unpack] extracting ${name} ...")
    execute_process(
        COMMAND "${SEVENZIP}" x "${arch}" "-o${DST_DIR}" -y
        WORKING_DIRECTORY "${SRC_DIR}"
        OUTPUT_QUIET
        ERROR_VARIABLE UNPACK_ERR
        RESULT_VARIABLE UNPACK_RES
    )
    if(UNPACK_RES EQUAL 0)
        message(STATUS "[unpack] ${name} done")
    else()
        message(WARNING "[unpack] ${name} failed (code=${UNPACK_RES}): ${UNPACK_ERR}")
    endif()
endforeach()

# 写 stamp
file(WRITE "${STAMP}" "unpacked\n")
message(STATUS "[unpack] all archives unpacked, stamp written")
