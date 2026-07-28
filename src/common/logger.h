/**
 * @file logger.h
 * @brief 日志记录器类定义
 * @details 提供线程安全的日志记录功能，支持DEBUG、INFO、WARN、ERROR四个日志级别。
 * @date 2026-07-28
 */

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

/**
 * @class Logger
 * @brief 日志记录器类
 * @details 提供静态方法进行日志记录，支持多线程环境下的线程安全操作
 */
class CPSS_COMMON_API Logger
{
public:
    /**
     * @brief 初始化日志系统
     * @param filename 日志文件路径
     */
    static void init(const QString &filename);
    
    /**
     * @brief 清理日志系统
     * @details 关闭日志文件
     */
    static void cleanup();

    /**
     * @brief 记录DEBUG级别日志
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    static void debug(const char *format, ...);
    
    /**
     * @brief 记录INFO级别日志
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    static void info(const char *format, ...);
    
    /**
     * @brief 记录WARN级别日志
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    static void warn(const char *format, ...);
    
    /**
     * @brief 记录ERROR级别日志
     * @param format 格式化字符串
     * @param ... 可变参数
     */
    static void error(const char *format, ...);

private:
    static QMutex s_mutex;       ///< 互斥锁，保证线程安全
    static QFile s_file;         ///< 日志文件
    static QTextStream s_stream; ///< 文本流
};

#endif
