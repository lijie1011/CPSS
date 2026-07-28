/**
 * @file logger.cpp
 * @brief 日志系统实现
 * @details 提供线程安全的日志记录功能，支持 DEBUG、INFO、WARN、ERROR 四个级别，
 *          日志可输出到文件或标准错误流。
 * @date 2026-07-28
 */

#include "logger.h"
#include <QDateTime>
#include <cstdarg>
#include <cstdio>

// 静态成员变量初始化
QMutex Logger::s_mutex;
QFile Logger::s_file;
QTextStream Logger::s_stream;

/**
 * @brief 初始化日志系统
 * @param filename 日志文件路径
 */
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

/**
 * @brief 清理日志系统资源
 */
void Logger::cleanup()
{
    s_file.close();
}

/**
 * @brief 记录DEBUG级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Logger::debug(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (s_file.isOpen()) {
        s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                 << " [DEBUG] " << buffer << endl;
        s_stream.flush();
    } else {
        fprintf(stderr, "%s [DEBUG] %s\n", 
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(), 
                buffer);
    }
}

/**
 * @brief 记录INFO级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Logger::info(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (s_file.isOpen()) {
        s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                 << " [INFO]  " << buffer << endl;
        s_stream.flush();
    } else {
        fprintf(stderr, "%s [INFO]  %s\n", 
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(), 
                buffer);
    }
}

/**
 * @brief 记录WARN级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Logger::warn(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (s_file.isOpen()) {
        s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                 << " [WARN]  " << buffer << endl;
        s_stream.flush();
    } else {
        fprintf(stderr, "%s [WARN]  %s\n", 
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(), 
                buffer);
    }
}

/**
 * @brief 记录ERROR级别日志
 * @param format 格式化字符串
 * @param ... 可变参数
 */
void Logger::error(const char *format, ...)
{
    QMutexLocker locker(&s_mutex);
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if (s_file.isOpen()) {
        s_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
                 << " [ERROR] " << buffer << endl;
        s_stream.flush();
    } else {
        fprintf(stderr, "%s [ERROR] %s\n", 
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(), 
                buffer);
    }
}
