# 需要 3.13+ 以便 install() 的 DESTINATION 支持 $<CONFIG> 生成器表达式，
# 从而将 Debug / Release 安装到各自的子目录。
cmake_minimum_required(VERSION 3.13)
project(CPSS VERSION 1.0 LANGUAGES CXX)

# Debug 版本所有可执行文件/库加 d 后缀（与 Qt 命名风格一致）：
#   cpss.exe -> cpssd.exe, cpss_data.dll -> cpss_datad.dll, chartplugin.dll -> chartplugind.dll
# 设置在 project() 之后、add_subdirectory() 之前，自动作用于本工程所有 target。
set(CMAKE_DEBUG_POSTFIX "d")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    set(PLATFORM_NAME "windows")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_NAME "linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

message(STATUS "Building CPSS for platform: ${PLATFORM_NAME}")

if(WIN32)
    # 使用工程内随附的本地 Qt 环境（src/3dParty/qt），路径相对源码树，
    # 迁移到其他电脑无需单独安装 Qt 即可配置/编译。
    # 必须用 CACHE FORCE 覆盖：否则 find_package 首次找到的外部 Qt 路径会缓存在
    # CMakeCache.txt 中，后续 configure 时 set() 无法覆盖，导致"编译用外部 Qt、
    # windeployqt 用随附 Qt"的不一致，Release 运行报缺 DLL / 初始化失败。
    # 必须用 CACHE ... FORCE 覆盖：find_package 首次找到的 Qt 路径会缓存在
    # CMakeCache.txt 中，普通 set() 无法覆盖，导致编译/部署 Qt 环境不一致。
    set(Qt5_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3dParty/qt/lib/cmake/Qt5" CACHE PATH "Qt5 dir" FORCE)
    message(STATUS "Using bundled MSVC Qt5: ${Qt5_DIR}")
endif()

find_package(Qt5 5.12 REQUIRED COMPONENTS
    Widgets
    Gui
    Core
    Network
    Xml
)

set(ENCLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3dParty/Enclib)
set(ENCLIB_INCLUDE_DIR ${ENCLIB_DIR}/include)
set(ENCLIB_BIN_DIR ${ENCLIB_DIR}/bin)

if(NOT EXISTS "${ENCLIB_INCLUDE_DIR}/encl.h")
    message(FATAL_ERROR "Enclib headers not found at ${ENCLIB_INCLUDE_DIR}\n"
                        "Please copy Enclib SDK from ChartDemo to src/3dParty/Enclib")
endif()

add_subdirectory(common)
add_subdirectory(datalib)
add_subdirectory(plugin)

set(SOURCES
    core/main.cpp
    core/mainwindow.cpp
    core/attitudesingleview.cpp
    core/attitudeviewer.cpp
    core/eventhistorydialog.cpp
    core/platformcontrolpanel.cpp
    core/resource.qrc
    core/mainwindow.ui
    settings/displaycategory.cpp
    settings/displaycategory.ui
    settings/viewinggroupdialog.cpp
    settings/viewinggroupdialog.ui
    settings/waterdepthsetting.cpp
    settings/waterdepthsetting.ui
)

set(HEADERS
    core/mainwindow.h
    core/attitudesingleview.h
    core/attitudeviewer.h
    core/eventhistorydialog.h
    core/platformcontrolpanel.h
    core/displaystate.h
    settings/displaycategory.h
    settings/viewinggroupdialog.h
    settings/waterdepthsetting.h
)

add_executable(cpss WIN32 ${SOURCES} ${HEADERS})
# 显式设置主程序 Debug 后缀：CMake 生成 vcxproj 时会把 exe 的 TargetName 写死，
# 全局 CMAKE_DEBUG_POSTFIX 对 WIN32 可执行程序不生效，需单独指定。
set_target_properties(cpss PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(cpss PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/datalib
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/settings
    ${ENCLIB_INCLUDE_DIR}
)

target_link_libraries(cpss PRIVATE
    cpss_common
    cpss_data
    cpss_plugin_framework
    Qt5::Widgets
    Qt5::Gui
    Qt5::Core
    Qt5::Network
    Qt5::Xml
)

if(WIN32)
    target_compile_options(cpss PRIVATE /utf-8)
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

    add_custom_command(TARGET cpss POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/bin
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/s57data
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_BIN_DIR}/enccore.dll $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_BIN_DIR}/enccore.lib $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_BIN_DIR}/msn.exe $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_BIN_DIR}/s52symbol.exe $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/bin
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_DIR}/config.xml $<TARGET_FILE_DIR:cpss>/3dParty/Enclib
        COMMAND ${CMAKE_COMMAND} -E copy ${ENCLIB_DIR}/lic.dat $<TARGET_FILE_DIR:cpss>/3dParty/Enclib
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENCLIB_DIR}/mapdata $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/mapdata
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENCLIB_DIR}/fonts $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/fonts
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ENCLIB_DIR}/s57data $<TARGET_FILE_DIR:cpss>/3dParty/Enclib/s57data
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/settings/chartinfodisplay.xml $<TARGET_FILE_DIR:cpss>
        # 拷贝资源目录（JB / JB_Souce 平台图标等）到运行期资源路径：
        # IconManager::resourcePath() = <exe目录>/../resource，即 build/bin/resource
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resource $<TARGET_FILE_DIR:cpss>/../resource
        # 拷贝 Redis SDK 运行期 DLL（redis_middleware / redis_protocol / protobuf / hiredis / zlib）
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/3dParty/redis_sdk/bin $<TARGET_FILE_DIR:cpss>
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

# ============================================================================
#  安装规则（install）
#  目标：cmake --install src\build --config Debug|Release 后，得到一份可直接运行
#        的完整程序目录，安装根位于 src/build 下并区分 Debug / Release。
#
#  安装布局（$<CONFIG> 为 Debug 或 Release）：
#    build/install/<Config>/
#      ├── bin/
#      │     ├── cpss.exe                     主程序
#      │     ├── cpss_common.dll              公共库
#      │     ├── cpss_data.dll                数据库
#      │     ├── chartinfodisplay.xml         海图信息配置（运行期同级读取）
#      │     ├── Qt5*.dll / platforms/ ...    Qt 运行时（由 windeployqt 部署）
#      │     ├── plugins/                     外部插件 DLL（运行期扫描此目录加载）
#      │     │     ├── sampleplugin.dll
#      │     │     ├── helloplugin.dll
#      │     │     └── dataflowplugin.dll
#      │     └── 3dParty/Enclib/              海图库运行期依赖
#      │           ├── enccore.dll / msn.exe / s52symbol.exe
#      │           ├── config.xml / lic.dat
#      │           └── mapdata/ fonts/ s57data/
#      └── resource/                          平台图标资源（运行期按 exe/../resource 读取）
#
#  说明：cpss.log、cpss.xml、cpss.ini 为运行期生成文件，无需安装。
# ============================================================================

# 安装根固定在 src/build/install，避免依赖外部传入的 CMAKE_INSTALL_PREFIX
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "CPSS 安装根目录" FORCE)

# 各配置各自的安装子目录（bin 与 resource 使用生成器表达式区分 Debug/Release）
set(CPSS_INSTALL_BIN     "$<CONFIG>/bin")                 # 可执行文件与运行期 DLL 目录
set(CPSS_INSTALL_PLUGINS "$<CONFIG>/bin/plugins")         # 外部插件目录
set(CPSS_INSTALL_ENCLIB  "$<CONFIG>/bin/3dParty/Enclib")  # 海图库目录
set(CPSS_INSTALL_RES     "$<CONFIG>/resource")            # 资源目录（与 bin 同级）

# ---- 主程序与业务 DLL（cpss_common / cpss_data 为 SHARED，随 RUNTIME 一并安装）----
install(TARGETS cpss cpss_common cpss_data
    RUNTIME DESTINATION ${CPSS_INSTALL_BIN}
    LIBRARY DESTINATION ${CPSS_INSTALL_BIN}
)

# ---- 外部插件 DLL：安装到 bin/plugins，供运行期 PluginManager 扫描加载 ----
install(TARGETS sampleplugin helloplugin dataflowplugin chartplugin outputplugin hosierydataplugin htmapplugin
    RUNTIME DESTINATION ${CPSS_INSTALL_PLUGINS}
    LIBRARY DESTINATION ${CPSS_INSTALL_PLUGINS}
)

# ---- 海图信息配置：运行期从 exe 同级目录读取 ----
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/settings/chartinfodisplay.xml
    DESTINATION ${CPSS_INSTALL_BIN}
)

# ---- 资源目录（JB / JB_Souce 平台图标等）：运行期按 exe/../resource 读取 ----
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resource/
    DESTINATION ${CPSS_INSTALL_RES}
)

if(WIN32)
    # ---- Enclib 海图库运行期文件（Windows）----
    # 关键：enccore.dll 是 cpss.exe 的隐式链接依赖（通过 enccore.lib 链接），
    #       Windows 启动时会从 exe 同级目录加载它，故必须额外装到 bin/ 根下；
    #       否则运行会提示“缺少 enccore.dll”。3dParty/Enclib/bin 下再放一份，
    #       与构建输出布局保持一致，供 Enclib 内部按需查找。
    install(FILES ${ENCLIB_BIN_DIR}/enccore.dll
        DESTINATION ${CPSS_INSTALL_BIN}
    )
    install(FILES
        ${ENCLIB_BIN_DIR}/enccore.dll
        ${ENCLIB_BIN_DIR}/msn.exe
        ${ENCLIB_BIN_DIR}/s52symbol.exe
        DESTINATION ${CPSS_INSTALL_ENCLIB}/bin
    )
    install(FILES ${ENCLIB_DIR}/config.xml ${ENCLIB_DIR}/lic.dat
        DESTINATION ${CPSS_INSTALL_ENCLIB}
    )
    install(DIRECTORY
        ${ENCLIB_DIR}/mapdata
        ${ENCLIB_DIR}/fonts
        ${ENCLIB_DIR}/s57data
        DESTINATION ${CPSS_INSTALL_ENCLIB}
    )

    # ---- Qt 运行时：调用 windeployqt 部署到安装后的 bin 目录 ----
    # 通过 Qt5::qmake 导入目标定位 Qt 的 bin 目录，进而找到 windeployqt.exe
    set(_qt_bin_dir "")
    if(TARGET Qt5::qmake)
        get_target_property(_qmake_loc Qt5::qmake IMPORTED_LOCATION)
        if(_qmake_loc)
            get_filename_component(_qt_bin_dir "${_qmake_loc}" DIRECTORY)
        endif()
    endif()
    find_program(WINDEPLOYQT_EXE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXE)
        # Debug 配置用 --debug，其它配置用 --release，部署到已安装的 cpss.exe 旁
        install(CODE "
            set(_cfg \"\${CMAKE_INSTALL_CONFIG_NAME}\")
            if(_cfg STREQUAL \"Debug\")
                set(_dq_flag \"--debug\")
            else()
                set(_dq_flag \"--release\")
            endif()
            message(STATUS \"[windeployqt] \${_dq_flag} 部署 Qt 运行时到 \${CMAKE_INSTALL_PREFIX}/\${_cfg}/bin\")
            execute_process(COMMAND \"${WINDEPLOYQT_EXE}\" \${_dq_flag} --no-compiler-runtime
                \"\${CMAKE_INSTALL_PREFIX}/\${_cfg}/bin/cpss.exe\")
        ")
    else()
        message(WARNING "未找到 windeployqt，安装后需手动部署 Qt 运行时 DLL")
    endif()

elseif(UNIX)
    # ---- Enclib 海图库运行期文件（Linux）----
    install(FILES ${ENCLIB_BIN_DIR}/libenccore.so
        ${ENCLIB_BIN_DIR}/msn
        ${ENCLIB_BIN_DIR}/s52symbol
        DESTINATION ${CPSS_INSTALL_ENCLIB}/bin
    )
    install(FILES ${ENCLIB_DIR}/config.xml ${ENCLIB_DIR}/lic.dat
        DESTINATION ${CPSS_INSTALL_ENCLIB}
    )
    install(DIRECTORY
        ${ENCLIB_DIR}/mapdata
        ${ENCLIB_DIR}/fonts
        ${ENCLIB_DIR}/s57data
        DESTINATION ${CPSS_INSTALL_ENCLIB}
    )
endif()
