@echo off
chcp 65001 >nul
echo ========================================
echo  Build CPSS (Debug)
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

echo [INFO] building...
cmake --build src\build --config Debug
if errorlevel 1 (
    echo [ERROR] build failed
    pause
    exit /b 1
)

echo.
echo [INFO] deploying Qt runtime...
set QT_BIN=E:\Qt5.12.11\5.12.11\msvc2017_64\bin
if exist "%QT_BIN%\windeployqt.exe" (
    "%QT_BIN%\windeployqt.exe" --debug --no-compiler-runtime src\build\bin\Debug\cpss.exe
    if errorlevel 1 (
        echo [WARN] windeployqt failed, plugins may not load
    ) else (
        echo [OK] Qt runtime deployed
    )
) else (
    echo [WARN] windeployqt not found, skipping Qt deployment
)

echo.
echo [OK] build completed
echo.
echo Run: src\build\bin\Debug\cpss.exe
pause
