/**
 * @file outputsink.cpp
 * @brief 全局输出汇聚器实现
 * @date 2026-11-22
 */

#include "outputsink.h"
#include "logger.h"
#include <QMutexLocker>
#include <QDebug>
#include <utility>

OutputSink* OutputSink::s_instance = nullptr;

OutputSink::OutputSink(QObject *parent)
    : QObject(parent),
      m_maxEntries(1000)
{
}

OutputSink::~OutputSink()
{
}

OutputSink* OutputSink::instance()
{
    if (!s_instance) {
        s_instance = new OutputSink();
    }
    return s_instance;
}

void OutputSink::enqueue(LogLevel level, const QString &text)
{
    if (text.isEmpty()) return;

    // 1) 线程安全地写入历史（在锁内完成，锁外再发信号）
    {
        QMutexLocker locker(&m_mutex);
        m_history.enqueue(qMakePair(level, text));
        while (m_history.size() > m_maxEntries) {
            m_history.dequeue();
        }
    }

    // 2) 双写到 cpss.log（Logger 内部自带锁）
    const QByteArray utf8 = text.toUtf8();
    switch (level) {
    case LogLevel::Debug: Logger::debug("%s", utf8.constData()); break;
    case LogLevel::Info:  Logger::info("%s",  utf8.constData()); break;
    case LogLevel::Warn:  Logger::warn("%s",  utf8.constData()); break;
    case LogLevel::Error: Logger::error("%s", utf8.constData()); break;
    }

    // 3) 发信号给输出插件（AutoConnection：跨线程自动 QueuedConnection）
    //    保护措施：若输出插件未加载（无任何接收者），降级输出到 qDebug()，
    //    保证开发者在调试器/控制台仍能看到消息；消息已写入 cpss.log，不会丢失。
    if (receivers(SIGNAL(messageArrived(int,QString))) == 0) {
        qDebug().noquote() << "[ut::log]" << text;
    }
    emit messageArrived((int)level, text);
}

QList<QPair<LogLevel, QString>> OutputSink::history() const
{
    QMutexLocker locker(&m_mutex);
    QList<QPair<LogLevel, QString>> result;
    for (const auto &item : m_history) {
        result.append(item);
    }
    return result;
}

void OutputSink::clearHistory()
{
    QMutexLocker locker(&m_mutex);
    m_history.clear();
}

// ============================================================
//  ut::log::Stream 实现
// ============================================================

namespace ut {
namespace log {

Stream::Stream(LogLevel level)
    : m_level(level), m_moved(false)
{
}

Stream::~Stream()
{
    // 关键：语句结束时一次性提交（若已被移动则跳过）
    if (!m_moved && !m_buffer.isEmpty()) {
        OutputSink::instance()->enqueue(m_level, m_buffer);
    }
}

Stream::Stream(Stream &&other) noexcept
    : m_level(other.m_level), m_buffer(std::move(other.m_buffer)), m_moved(false)
{
    other.m_moved = true;
}

Stream& Stream::operator=(Stream &&other) noexcept
{
    if (this != &other) {
        // 赋值前先提交自己已有内容
        if (!m_moved && !m_buffer.isEmpty()) {
            OutputSink::instance()->enqueue(m_level, m_buffer);
        }
        m_level = other.m_level;
        m_buffer = std::move(other.m_buffer);
        m_moved = false;
        other.m_moved = true;
    }
    return *this;
}

Stream& Stream::operator<<(const QString &v)      { m_buffer.append(v); return *this; }
Stream& Stream::operator<<(const char *v)         { m_buffer.append(QString::fromUtf8(v ? v : "")); return *this; }
Stream& Stream::operator<<(int v)                 { m_buffer.append(QString::number(v)); return *this; }
Stream& Stream::operator<<(unsigned int v)        { m_buffer.append(QString::number(v)); return *this; }
Stream& Stream::operator<<(long long v)           { m_buffer.append(QString::number(v)); return *this; }
Stream& Stream::operator<<(unsigned long long v)  { m_buffer.append(QString::number(v)); return *this; }
Stream& Stream::operator<<(double v)              { m_buffer.append(QString::number(v)); return *this; }
Stream& Stream::operator<<(bool v)                { m_buffer.append(v ? QStringLiteral("true") : QStringLiteral("false")); return *this; }

Stream debug() { return Stream(LogLevel::Debug); }
Stream info()  { return Stream(LogLevel::Info); }
Stream warn()  { return Stream(LogLevel::Warn); }
Stream error() { return Stream(LogLevel::Error); }

} // namespace log
} // namespace ut
