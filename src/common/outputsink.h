/**
 * @file outputsink.h
 * @brief 全局输出汇聚器（ut::log 流式输出）
 * @details 设计要点：
 *          1. 单例、线程安全：任意线程任意模块调用 ut::log::xxx() 都安全；
 *          2. 流式调用：ut::log::debug() << "平台" << id，整条语句结束后一次性提交，
 *             避免多线程输出内容交错；
 *          3. 信号投递：通过 Qt 信号（QueuedConnection）把消息送到 GUI 线程的输出插件，
 *             本类本身绝不触碰任何 QWidget；
 *          4. 双写：同时写入 Logger（cpss.log 文件）。
 *
 *          输出插件（outputplugin）未加载时，消息仍写入 cpss.log，不会丢失。
 * @date 2026-11-22
 */

#ifndef OUTPUTSINK_H
#define OUTPUTSINK_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QQueue>

// DLL 导出宏（与 logger.h 一致）
#ifdef CPSS_COMMON_EXPORT
#   define OUTPUT_API __declspec(dllexport)
#else
#   define OUTPUT_API __declspec(dllimport)
#endif

/** @brief 输出级别 */
enum class LogLevel {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3
};

/**
 * @class OutputSink
 * @brief 全局输出汇聚单例（线程安全，不依赖 GUI）
 */
class OUTPUT_API OutputSink : public QObject
{
    Q_OBJECT

public:
    static OutputSink* instance();

    /** @brief 提交一条消息（线程安全，任意线程可调用） */
    void enqueue(LogLevel level, const QString &text);

    /** @brief 获取历史消息（供插件界面初始化时加载） */
    QList<QPair<LogLevel, QString>> history() const;

    /** @brief 清空历史 */
    void clearHistory();

    /** @brief 历史保留上限（默认 1000） */
    void setMaxEntries(int max) { m_maxEntries = max; }
    int maxEntries() const { return m_maxEntries; }

signals:
    /** @brief 新消息到达（跨线程自动 QueuedConnection 投递到 GUI 线程） */
    void messageArrived(int level, const QString &text);

private:
    explicit OutputSink(QObject *parent = nullptr);
    ~OutputSink();

    static OutputSink *s_instance;
    mutable QMutex m_mutex;                              // 保护历史队列
    QQueue<QPair<LogLevel, QString>> m_history;          // 历史消息（含级别）
    int m_maxEntries;
};

// ============================================================
//  ut::log 流式接口
//  用法：ut::log::debug() << "平台" << id << " 已更新";
// ============================================================
namespace ut {
namespace log {

/**
 * @class Stream
 * @brief 临时流对象：累积 << 的内容，析构时一次性提交给 OutputSink
 * @details 不允许拷贝（避免重复提交）；移动构造用于从 debug() 等函数返回。
 */
class OUTPUT_API Stream
{
public:
    explicit Stream(LogLevel level);
    ~Stream();

    Stream(Stream &&other) noexcept;
    Stream& operator=(Stream &&other) noexcept;
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    Stream& operator<<(const QString &v);
    Stream& operator<<(const char *v);
    Stream& operator<<(int v);
    Stream& operator<<(unsigned int v);
    Stream& operator<<(long long v);
    Stream& operator<<(unsigned long long v);
    Stream& operator<<(double v);
    Stream& operator<<(bool v);

private:
    LogLevel m_level;
    QString m_buffer;
    bool m_moved = false;
};

OUTPUT_API Stream debug();
OUTPUT_API Stream info();
OUTPUT_API Stream warn();
OUTPUT_API Stream error();

} // namespace log
} // namespace ut

#endif // OUTPUTSINK_H
