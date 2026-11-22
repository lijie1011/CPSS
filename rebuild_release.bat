@echo off
rem 安装 Release（复用通用安装脚本，传入 Release 配置）
call "%~dp0rebuild_debug.bat" Release
