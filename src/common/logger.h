#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>

#ifdef CPSS_COMMON_EXPORT
#   define CPSS_COMMON_API __declspec(dllexport)
#else
#   define CPSS_COMMON_API __declspec(dllimport)
#endif

class CPSS_COMMON_API Logger
{
public:
    static void init(const QString &filename);
    static void cleanup();

    static void debug(const char *format, ...);
    static void info(const char *format, ...);
    static void warn(const char *format, ...);
    static void error(const char *format, ...);

private:
    static QMutex s_mutex;
    static QFile s_file;
    static QTextStream s_stream;
};

#endif
