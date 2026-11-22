// htproto_export.h
#pragma once

#if defined(HTPROTO_BUILD_DLL)
#define HTPROTO_API __declspec(dllexport)
#else
#define HTPROTO_API __declspec(dllimport)
#endif
