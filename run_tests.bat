@echo off
chcp 65001 >nul
rem 运行测试程序：路径基于本脚本所在目录（工程根），不再写死旧路径
rem 注意：当前工程尚未提交测试可执行文件（tst_*.exe），若不存在会给出提示并跳过
setlocal
cd /d "%~dp0"

set TEST_DIR=src\build\bin\Release
set PATH=%CD%\src\3dParty\qt\bin;%PATH%

echo ========================================
echo  CPSS Tests
echo ========================================

set FOUND=0
for %%T in (tst_datamanager.exe tst_dynamicdata.exe tst_protocoladapter.exe) do (
    if exist "%TEST_DIR%\%%T" (
        set FOUND=1
        echo.
        echo Running %%T...
        "%TEST_DIR%\%%T"
        echo Exit code: %errorlevel%
    ) else (
        echo [SKIP] %%T not found in %TEST_DIR%
    )
)

if "%FOUND%"=="0" (
    echo.
    echo [WARN] 未发现任何测试可执行文件。
    echo 当前工程尚未提交测试程序（tst_*.exe），请先构建测试目标后再运行。
)

echo.
echo All tests completed.
endlocal
