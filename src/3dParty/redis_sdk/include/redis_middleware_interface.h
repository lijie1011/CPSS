#pragma once

#ifdef _WIN32
    #ifdef REDIS_MIDDLEWARE
    #define REDIS_MIDDLEWARE_EXPORT __declspec(dllexport)
    #else
    #define REDIS_MIDDLEWARE_EXPORT __declspec(dllimport)
    #endif
#else
    #define REDIS_MIDDLEWARE_EXPORT __attribute__((visibility("default")))
#endif
