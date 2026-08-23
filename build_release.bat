@echo off
rem 一键构建 + 安装 Release（复用通用构建脚本，传入 Release 配置）
call "%~dp0build_debug.bat" Release
