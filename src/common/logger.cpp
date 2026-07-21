#include "logger.h"
#include <QDateTime>
#include <cstdarg>
#include <cstdio>

QMutex Logger::s_mutex;
QFile Logger::s_file;
QTextStream Logger::s_stream;

void Logger::init(const QString &filename)
{
    s_file.setFileName(filename);
    bool opened = s_file.open(QIODevice::Append | QIODevice::Text);
    s_stream.setDevice(&s_file);
    
    if (!opened) {
        fprintf(stderr, "[LOGGER] Failed to open log file: %s\n", filename.toStdString().c_str());
        fprintf(stderr, "[LOGGER] Using stderr for logging\n");
    } else {
        fprintf(stderr, "[LOGGER] Log file: %s\n", filename.toStdString().c_str());
    }
}

void Logger::cleanup()
{
    s_file.close();
}

void Logger::debug(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
             << " [DEBUG] " << buffer << endl;
    s_stream.flush();
}

void Logger::info(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
             << " [INFO]  " << buffer << endl;
    s_stream.flush();
}

void Logger::warn(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
             << " [WARN]  " << buffer << endl;
    s_stream.flush();
}

void Logger::error(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
             << " [ERROR] " << buffer << endl;
    s_stream.flush();
}
