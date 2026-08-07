@echo off
chcp 65001 >nul
set LOG_FILE=%~dp0build_log.txt

if exist "%LOG_FILE%" del /Q "%LOG_FILE%" 2>nul

echo ========================================
echo  Full Clean Rebuild CPSS (Debug)
echo ========================================
echo.

cd /d %~dp0

echo [STEP 1] killing all cpss.exe...
taskkill /F /IM cpss.exe >nul 2>&1
timeout /t 2 /nobreak >nul
echo         done.

echo.
echo [STEP 2] removing stale plugin DLLs...
if exist "src\build\bin\Debug\plugins" (
    del /Q src\build\bin\Debug\plugins\*.dll 2>nul
    del /Q src\build\bin\Debug\plugins\*.pdb 2>nul
)
del /Q src\build\bin\Debug\cpss.log 2>nul
del /Q src\build\bin\Debug\plugin_debug.txt 2>nul
echo         done.

echo.
echo [STEP 3] re-running cmake configure...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] cmake configure >> "%LOG_FILE%"
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
echo [STEP 4] building...
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] cmake build >> "%LOG_FILE%"
echo ======================================== >> "%LOG_FILE%"
cmake --build src\build --config Debug >> "%LOG_FILE%" 2>&1
if errorlevel 1 (
    echo [ERROR] build failed
    echo [ERROR] build failed >> "%LOG_FILE%"
    pause
    exit /b 1
)
echo         done.

echo.
echo [STEP 5] deploying Qt runtime...
set QT_BIN=E:\Qt5.12.11\5.12.11\msvc2017_64\bin
if exist "%QT_BIN%\windeployqt.exe" (
    "%QT_BIN%\windeployqt.exe" --debug --no-compiler-runtime src\build\bin\Debug\cpss.exe 2>nul
    echo         done.
) else (
    echo         [WARN] windeployqt not found, skipping.
)

echo.
echo [DONE] Build complete!
echo        Log: %LOG_FILE%
echo        Run: src\build\bin\Debug\cpss.exe
echo        Check cpss.log for plugin load result.
echo ======================================== >> "%LOG_FILE%"
echo [%DATE% %TIME%] build complete >> "%LOG_FILE%"
pause
