@echo off
rem 编译 Release（复用通用编译脚本，传入 Release 配置）
call "%~dp0build_debug.bat" Release
