@echo off
set PATH=E:\Qt5.12.11\5.12.11\msvc2017_64\bin;%PATH%
cd d:\ZCHH\CPSS\build\datalib\tests\Release
echo Running tst_datamanager...
tst_datamanager.exe
echo Exit code: %errorlevel%
echo.
echo Running tst_dynamicdata...
tst_dynamicdata.exe
echo Exit code: %errorlevel%
echo.
echo Running tst_protocoladapter...
tst_protocoladapter.exe
echo Exit code: %errorlevel%
echo.
echo All tests completed.