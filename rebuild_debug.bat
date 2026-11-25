@echo off
chcp 65001 >nul
rem 安装脚本：清理旧产物 + configure + build + 部署随附 Qt 运行时 + install。
rem 若只需编译不安装，请用 build_debug.bat / build_release.bat
set CONFIG=Debug
if not "%1"=="" set CONFIG=%1

set LOG_FILE=%~dp0build_log.txt

if exist "%LOG_FILE%" del /Q "%LOG_FILE%" 2>nul

echo ========================================
echo  Install CPSS (%CONFIG%)  [full]
echo ========================================
echo.

cd /d %~dp0

echo [STEP 1] killing running cpss processes...
taskkill /F /IM cpss.exe >nul 2>&1
taskkill /F /IM cpssd.exe >nul 2>&1
timeout /t 2 /nobreak >nul

echo [STEP 2] removing stale plugin DLLs / logs...
if exist "src\build\bin\%CONFIG%\plugins" (
    del /Q src\build\bin\%CONFIG%\plugins\*.dll 2>nul
    del /Q src\build\bin\%CONFIG%\plugins\*.pdb 2>nul
)
del /Q src\build\bin\%CONFIG%\cpss.log 2>nul

echo [STEP 3] cmake configure (force bundled Qt)...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] configure (%CONFIG%) >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64 >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] cmake configure failed, see %LOG_FILE%
    pause
    exit /b 1
)

echo [STEP 4] building (%CONFIG%)...
echo [%DATE% %TIME%] build (%CONFIG%) >> "%LOG_FILE%"
cmake --build src\build --config %CONFIG% >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] build failed, see %LOG_FILE%
    pause
    exit /b 1
)

echo [STEP 5] deploying bundled Qt runtime...
set QT_BIN=%~dp0src\3dParty\qt\bin
set EXE_NAME=cpss.exe
if "%CONFIG%"=="Debug" (
    set DQ_FLAG=--debug
    set EXE_NAME=cpssd.exe
) else (
    set DQ_FLAG=--release
)
set EXE_PATH=src\build\bin\%CONFIG%\%EXE_NAME%
if exist "%QT_BIN%\windeployqt.exe" (
    if exist "%EXE_PATH%" (
        "%QT_BIN%\windeployqt.exe" %DQ_FLAG% --no-compiler-runtime "%EXE_PATH%" >> "%LOG_FILE%" 2>&1
        echo         windeployqt done.
    ) else (
        echo         [WARN] exe not found: %EXE_PATH%
    )
) else (
    echo         [WARN] windeployqt not found at %QT_BIN%
)

rem 兜底：只覆盖项目实际使用的 Qt 模块，保证编译/运行 Qt 环境一致
rem （windeployqt 可能部署了外部 Qt 的 DLL，混用会导致 0xC0000142 DllMain 失败）
rem 项目只用 Core/Gui/Widgets/Network/Xml 五个模块，其余 100+ DLL 全部不需要
set QT_MODULES=Core Gui Widgets Network Xml
if "%CONFIG%"=="Debug" (
    echo         overwriting required Qt5 debug DLLs from bundled Qt...
    for %%M in (%QT_MODULES%) do (
        if exist "%QT_BIN%\Qt5%%Md.dll" copy /Y "%QT_BIN%\Qt5%%Md.dll" "src\build\bin\%CONFIG%\" >> "%LOG_FILE%" 2>&1
    )
) else (
    echo         overwriting required Qt5 release DLLs from bundled Qt...
    for %%M in (%QT_MODULES%) do (
        if exist "%QT_BIN%\Qt5%%M.dll" copy /Y "%QT_BIN%\Qt5%%M.dll" "src\build\bin\%CONFIG%\" >> "%LOG_FILE%" 2>&1
    )
)

echo [STEP 6] installing to src\build\install\%CONFIG%...
echo [%DATE% %TIME%] install (%CONFIG%) >> "%LOG_FILE%"
cmake --install src\build --config %CONFIG% >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] install failed, see %LOG_FILE%
    pause
    exit /b 1
)

echo.
echo [DONE] Build + Install completed (%CONFIG%)
echo        Log: %LOG_FILE%
if "%CONFIG%"=="Debug" (
    echo        Run: src\build\bin\Debug\cpssd.exe
) else (
    echo        Run: src\build\bin\Release\cpss.exe
)
echo        Installed: src\build\install\%CONFIG%\bin\
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] done (%CONFIG%) >> "%LOG_FILE%"
pause
