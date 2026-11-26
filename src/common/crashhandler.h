/**
 * @file crashhandler.h
 * @brief 崩溃转储（dump）处理器
 * @details Windows 下用 MiniDumpWriteDump 生成 .dmp 文件 + 文本崩溃报告。
 *          捕获 SEH 异常（访问违规/栈溢出等）和未处理的 C++ 异常。
 * @date 2026-11-26
 */

#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#ifdef CPSS_COMMON_EXPORT
#   define CPSS_CRASH_API __declspec(dllexport)
#else
#   define CPSS_CRASH_API __declspec(dllimport)
#endif

/** @brief 安装崩溃处理器（在 main() 最开始调用，越早越好） */
CPSS_CRASH_API void installCrashHandler();

#endif // CRASHHANDLER_H
