@echo off
chcp 65001 >nul
rem 编译脚本：只做 CMake configure + build，不部署 Qt、不安装。
rem 需要完整安装（部署 Qt 运行时 + install）请用 rebuild_debug.bat / rebuild_release.bat
set CONFIG=Debug
if not "%1"=="" set CONFIG=%1

echo ========================================
echo  Build CPSS (%CONFIG%)  [compile only]
echo ========================================
echo.

cd /d %~dp0

echo [STEP 1] killing running cpss processes...
taskkill /F /IM cpss.exe >nul 2>&1
taskkill /F /IM cpssd.exe >nul 2>&1

echo [STEP 2] cmake configure...
cmake -S src -B src\build -G "Visual Studio 16 2019" -A x64
if errorlevel 1 (
    echo [ERROR] cmake configure failed
    pause
    exit /b 1
)

echo [STEP 3] building (%CONFIG%)...
cmake --build src\build --config %CONFIG%
if errorlevel 1 (
    echo [ERROR] build failed
    pause
    exit /b 1
)

echo.
echo [OK] build completed (%CONFIG%)
if "%CONFIG%"=="Debug" (
    echo     Exe: src\build\bin\Debug\cpssd.exe
) else (
    echo     Exe: src\build\bin\Release\cpss.exe
)
echo     To deploy + install, run: rebuild_%CONFIG%.bat
pause
