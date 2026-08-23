@echo off
rem 一键全量重建 + 安装 Release（复用通用重建脚本，传入 Release 配置）
call "%~dp0rebuild_debug.bat" Release
