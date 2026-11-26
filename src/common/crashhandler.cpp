/**
 * @file crashhandler.cpp
 * @brief 崩溃转储实现（Windows MiniDumpWriteDump）
 * @date 2026-11-26
 */

#include "crashhandler.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <time.h>
#pragma comment(lib, "dbghelp.lib")

static wchar_t g_dumpDir[MAX_PATH] = {0};
static wchar_t g_reportPath[MAX_PATH] = {0};

static const wchar_t* exceptionCodeName(DWORD code)
{
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:         return L"ACCESS_VIOLATION";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return L"DATATYPE_MISALIGNMENT";
    case EXCEPTION_BREAKPOINT:               return L"BREAKPOINT";
    case EXCEPTION_SINGLE_STEP:              return L"SINGLE_STEP";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return L"ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return L"FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return L"FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:       return L"FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:    return L"FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:             return L"FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:          return L"FLT_STACK_CHECK";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return L"INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:             return L"INT_OVERFLOW";
    case EXCEPTION_INVALID_HANDLE:           return L"INVALID_HANDLE";
    case EXCEPTION_STACK_OVERFLOW:           return L"STACK_OVERFLOW";
    default:                                 return L"UNKNOWN";
    }
}

static void writeMiniDump(EXCEPTION_POINTERS *ep, const wchar_t *reason)
{
    wchar_t dumpPath[MAX_PATH];
    struct tm t;
    time_t now = time(nullptr);
    localtime_s(&t, &now);
    swprintf_s(dumpPath, MAX_PATH, L"%s\\cpss_%04d%02d%02d_%02d%02d%02d.dmp",
               g_dumpDir, t.tm_year+1900, t.tm_mon+1, t.tm_mday,
               t.tm_hour, t.tm_min, t.tm_sec);

    HANDLE hFile = CreateFileW(dumpPath, GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId          = GetCurrentThreadId();
        mdei.ExceptionPointers = ep;
        mdei.ClientPointers    = FALSE;

        // DataSegs + FullMemoryInfo：约 10~50MB，能看变量和调用栈
        MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(
            MiniDumpWithDataSegs | MiniDumpWithFullMemoryInfo |
            MiniDumpWithHandleData | MiniDumpWithThreadInfo);

        BOOL ok = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                                    hFile, dumpType, ep ? &mdei : nullptr,
                                    nullptr, nullptr);
        CloseHandle(hFile);

        // 文本报告
        FILE *rf = nullptr;
        _wfopen_s(&rf, g_reportPath, L"a");
        if (rf) {
            fwprintf(rf, L"\n=== CRASH %04d-%02d-%02d %02d:%02d:%02d ===\n",
                     t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
            fwprintf(rf, L"reason: %s\n", reason);
            if (ep && ep->ExceptionRecord) {
                DWORD code = ep->ExceptionRecord->ExceptionCode;
                fwprintf(rf, L"exception: 0x%08X (%s)\n", code, exceptionCodeName(code));
                fwprintf(rf, L"address: 0x%p\n", ep->ExceptionRecord->ExceptionAddress);
                if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2) {
                    fwprintf(rf, L"access: %s 0x%p\n",
                             ep->ExceptionRecord->ExceptionInformation[0] ? L"write" : L"read",
                             (void*)ep->ExceptionRecord->ExceptionInformation[1]);
                }
            }
            fwprintf(rf, L"dump: %s\n", dumpPath);
            fwprintf(rf, L"------------------------------\n");
            fclose(rf);
        }
        (void)ok;
    }
}

static LONG WINAPI sehHandler(EXCEPTION_POINTERS *ep)
{
    writeMiniDump(ep, L"SEH exception");
    // 让系统继续默认处理（生成 WerFault / 退出）
    return EXCEPTION_CONTINUE_SEARCH;
}

static void terminateHandler()
{
    writeMiniDump(nullptr, L"Unhandled C++ exception (std::terminate)");
    std::abort();
}

CPSS_CRASH_API void installCrashHandler()
{
    // 目录：exe 目录/dumps
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    wchar_t *slash = wcsrchr(exePath, L'\\');
    if (slash) *slash = L'\0';
    swprintf_s(g_dumpDir, MAX_PATH, L"%s\\dumps", exePath);
    CreateDirectoryW(g_dumpDir, nullptr);
    swprintf_s(g_reportPath, MAX_PATH, L"%s\\cpss_crash.log", exePath);

    SetUnhandledExceptionFilter(sehHandler);
    std::set_terminate(terminateHandler);
}

#else

void installCrashHandler() {}

#endif
