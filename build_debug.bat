@echo off
chcp 65001 >nul
rem 通用构建脚本：默认 Debug，可通过命令行参数指定 Release，如 build_debug.bat Release
set CONFIG=Debug
if not "%1"=="" set CONFIG=%1

echo ========================================
echo  Build CPSS (%CONFIG%)
echo ========================================
echo.

cd /d %~dp0

echo [INFO] killing any running cpss.exe...
taskkill /F /IM cpss.exe >nul 2>&1

if not exist "src\build" (
    echo [INFO] build directory not found, running cmake configure...
    cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64
    if errorlevel 1 (
        echo [ERROR] cmake configure failed
        pause
        exit /b 1
    )
)

echo [INFO] building (%CONFIG%)...
cmake --build src\build --config %CONFIG%
if errorlevel 1 (
    echo [ERROR] build failed
    pause
    exit /b 1
)

echo.
echo [INFO] deploying Qt runtime (direct run)...
rem 使用工程内随附的本地 Qt（src/3dParty/qt），便于迁移到其他电脑
set QT_BIN=%~dp0src\3dParty\qt\bin
if "%CONFIG%"=="Debug" (
    set DQ_FLAG=--debug
) else (
    set DQ_FLAG=--release
)
if exist "%QT_BIN%\windeployqt.exe" (
    "%QT_BIN%\windeployqt.exe" %DQ_FLAG% --no-compiler-runtime src\build\bin\%CONFIG%\cpss.exe
    if errorlevel 1 (
        echo [WARN] windeployqt failed, plugins may not load
    ) else (
        echo [OK] Qt runtime deployed
    )
) else (
    echo [WARN] windeployqt not found, skipping Qt deployment
)

echo.
echo [INFO] installing to src\build\install\%CONFIG% ...
cmake --install src\build --config %CONFIG%
if errorlevel 1 (
    echo [ERROR] install failed
    pause
    exit /b 1
)

echo.
echo [OK] build completed
echo     Run:       src\build\bin\%CONFIG%\cpss.exe
echo     Installed: src\build\install\%CONFIG%\bin\cpss.exe
pause
