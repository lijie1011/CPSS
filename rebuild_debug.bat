@echo off
chcp 65001 >nul
rem 通用全量重建脚本：默认 Debug，可通过命令行参数指定 Release，如 rebuild_debug.bat Release
set CONFIG=Debug
if not "%1"=="" set CONFIG=%1

set LOG_FILE=%~dp0build_log.txt

if exist "%LOG_FILE%" del /Q "%LOG_FILE%" 2>nul

echo ========================================
echo  Full Clean Rebuild CPSS (%CONFIG%)
echo ========================================
echo.

cd /d %~dp0

echo [STEP 1] killing all cpss.exe...
taskkill /F /IM cpss.exe >nul 2>&1
timeout /t 2 /nobreak >nul
echo         done.

echo.
echo [STEP 2] removing stale plugin DLLs...
if exist "src\build\bin\%CONFIG%\plugins" (
    del /Q src\build\bin\%CONFIG%\plugins\*.dll 2>nul
    del /Q src\build\bin\%CONFIG%\plugins\*.pdb 2>nul
)
del /Q src\build\bin\%CONFIG%\cpss.log 2>nul
del /Q src\build\bin\%CONFIG%\plugin_debug.txt 2>nul
echo         done.

echo.
echo [STEP 3] re-running cmake configure...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] cmake configure (%CONFIG%) >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64 >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] cmake configure failed
    echo [ERROR] cmake configure failed >> "%LOG_FILE%"
    pause
    exit /b 1
)
echo         done.

echo.
echo [STEP 4] building (%CONFIG%)...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] cmake build (%CONFIG%) >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
cmake --build src\build --config %CONFIG% >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] build failed
    echo [ERROR] build failed >> "%LOG_FILE%"
    pause
    exit /b 1
)
echo         done.

echo.
echo [STEP 5] deploying Qt runtime (direct run)...
rem 使用工程内随附的本地 Qt（src/3dParty/qt），便于迁移到其他电脑
set QT_BIN=%~dp0src\3dParty\qt\bin
if "%CONFIG%"=="Debug" (
    set DQ_FLAG=--debug
) else (
    set DQ_FLAG=--release
)
if exist "%QT_BIN%\windeployqt.exe" (
    "%QT_BIN%\windeployqt.exe" %DQ_FLAG% --no-compiler-runtime src\build\bin\%CONFIG%\cpss.exe >> "%LOG_FILE%" 2>&1
    echo         done.
) else (
    echo         [WARN] windeployqt not found, skipping.
)

echo.
echo [STEP 6] installing to src\build\install\%CONFIG% ...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] cmake install (%CONFIG%) >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
cmake --install src\build --config %CONFIG% >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] install failed
    echo [ERROR] install failed >> "%LOG_FILE%"
    pause
    exit /b 1
)
echo         done.

echo.
echo [DONE] Build complete!
echo        Log: %LOG_FILE%
echo        Run:       src\build\bin\%CONFIG%\cpss.exe
echo        Installed: src\build\install\%CONFIG%\bin\cpss.exe
echo        Check cpss.log for plugin load result.
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] build complete (%CONFIG%) >> "%LOG_FILE%"
pause
