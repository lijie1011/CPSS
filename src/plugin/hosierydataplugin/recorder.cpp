/**
 * @file recorder.cpp
 * @brief 录制器与播放器实现
 * @date 2026-11-23
 */

#include "recorder.h"
#include "datacache.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonArray>

// ==================== RecordWriter ====================

RecordWriter::RecordWriter(QObject *parent)
    : QObject(parent)
{
    m_flushTimer.setInterval(100);   // 100ms 批量写入
    m_flushTimer.setSingleShot(false);
    connect(&m_flushTimer, &QTimer::timeout, this, &RecordWriter::onFlushTimer);
}

RecordWriter::~RecordWriter()
{
    stop();
}

bool RecordWriter::start(const QString &path, int sampleDiv, const QSet<int> &enabledSources,
                         int campFilter, const QString &typeFilter)
{
    stop();
    QList<int> sources;
    for (int s : enabledSources) sources.append(s);

    QJsonObject meta;
    meta["sampleDiv"] = sampleDiv;
    meta["campFilter"] = campFilter;
    meta["typeFilter"] = typeFilter;

    if (!m_writer.open(path, sources, meta)) {
        emit statusChanged(QStringLiteral("无法创建录制文件: ") + path);
        return false;
    }

    m_lastPath = path;
    m_sampleDiv = qMax(1, sampleDiv);
    m_enabledSources = enabledSources;
    m_campFilter = campFilter;
    m_typeFilter = typeFilter;
    m_startMs = QDateTime::currentMSecsSinceEpoch();
    m_recorded = 0;
    m_tick = 0;
    m_pending.clear();
    m_flushTimer.start();
    m_recording = true;

    // 连接 DataManager 信号
    DataManager *dm = DataManager::instance();
    connect(dm, &DataManager::platformUpdated, this, &RecordWriter::onPlatformUpdated, Qt::QueuedConnection);
    connect(dm, &DataManager::platformRemoved, this, &RecordWriter::onPlatformRemoved, Qt::QueuedConnection);
    connect(dm, &DataManager::eventAdded, this, &RecordWriter::onEventAdded, Qt::QueuedConnection);

    emit statusChanged(QStringLiteral("开始录制: ") + path);
    return true;
}

void RecordWriter::stop()
{
    if (!m_recording) return;
    m_recording = false;
    m_flushTimer.stop();
    onFlushTimer();   // 最后一批 flush

    // 断开信号
    DataManager *dm = DataManager::instance();
    disconnect(dm, &DataManager::platformUpdated, this, &RecordWriter::onPlatformUpdated);
    disconnect(dm, &DataManager::platformRemoved, this, &RecordWriter::onPlatformRemoved);
    disconnect(dm, &DataManager::eventAdded, this, &RecordWriter::onEventAdded);

    m_writer.close();
    emit statusChanged(QStringLiteral("停止录制，共 %1 条").arg(m_recorded));
}

qint64 RecordWriter::elapsedMs() const
{
    return m_recording ? (QDateTime::currentMSecsSinceEpoch() - m_startMs) : 0;
}

bool RecordWriter::shouldRecord(const PlatformData &platform) const
{
    // 过滤回放数据（Protocol_Unknown），避免循环录制
    if (!m_enabledSources.contains((int)platform.sourceProtocol)) return false;
    if (m_campFilter >= 0 && (int)platform.camp != m_campFilter) return false;
    if (!m_typeFilter.isEmpty() && platform.type != m_typeFilter) return false;
    return true;
}

void RecordWriter::onPlatformUpdated(const PlatformData &platform)
{
    if (!m_recording || !shouldRecord(platform)) return;
    if (++m_tick % m_sampleDiv != 0) return;   // 采样分频

    appendRecord(RecType::PlatformFull, platform.sourceProtocol, serializePlatform(platform));
}

void RecordWriter::onPlatformRemoved(const QString &id)
{
    if (!m_recording) return;
    appendRecord(RecType::Remove, Protocol_Unknown, id.toUtf8());
}

void RecordWriter::onEventAdded(const SpecialEvent &event)
{
    if (!m_recording) return;
    appendRecord(RecType::Event, Protocol_Unknown, serializeEvent(event));
}

void RecordWriter::appendRecord(RecType type, ProtocolType source, const QByteArray &payload)
{
    RecEntry entry;
    entry.type = type;
    entry.source = source;
    entry.relTime = QDateTime::currentMSecsSinceEpoch() - m_startMs;
    entry.payload = payload;
    m_pending.append(entry);
    ++m_recorded;
}

void RecordWriter::onFlushTimer()
{
    if (m_pending.isEmpty()) return;
    for (const RecEntry &e : m_pending) m_writer.append(e);
    m_pending.clear();
}

// ==================== ReplayPlayer ====================

ReplayPlayer::ReplayPlayer(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(50);   // 20Hz 驱动
    m_timer.setSingleShot(false);
    connect(&m_timer, &QTimer::timeout, this, &ReplayPlayer::onTick);
}

ReplayPlayer::~ReplayPlayer()
{
    stop();
    unload();
}

bool ReplayPlayer::load(const QString &path)
{
    unload();
    if (!m_reader.open(path)) {
        emit statusChanged(QStringLiteral("无法打开录制文件: ") + path);
        return false;
    }
    m_cursor = 0;
    m_positionMs = 0;
    m_injected = 0;
    emit statusChanged(QStringLiteral("已加载: %1 (%2 条, %3 ms)")
                       .arg(path).arg(recordCount()).arg(durationMs()));
    return true;
}

void ReplayPlayer::unload()
{
    stop();
    m_reader.close();
}

void ReplayPlayer::play()
{
    if (m_reader.index().isEmpty()) return;
    if (m_cursor >= m_reader.index().size()) {   // 播放完从头
        m_cursor = 0;
        m_positionMs = 0;
        m_injected = 0;
    }
    m_playing = true;
    m_timer.start();
    emit statusChanged(QStringLiteral("回放中"));
}

void ReplayPlayer::pause()
{
    m_playing = false;
    m_timer.stop();
    emit statusChanged(QStringLiteral("已暂停"));
}

void ReplayPlayer::stop()
{
    m_playing = false;
    m_timer.stop();
}

void ReplayPlayer::seekTo(qint64 ms)
{
    const int idx = m_reader.findIndexByTime(ms);
    m_cursor = (idx < 0) ? 0 : idx;
    m_positionMs = ms;
    emit positionChanged(m_positionMs, (int)m_injected);
}

void ReplayPlayer::onTick()
{
    if (!m_playing) return;
    const qint64 step = (qint64)(m_timer.interval() * m_speed);
    m_positionMs += step;

    // 注入 [上次位置, 当前位置] 之间所有记录
    const QVector<RecIndexItem> &idx = m_reader.index();
    while (m_cursor < idx.size() && idx[m_cursor].relTime <= m_positionMs) {
        RecEntry entry;
        if (m_reader.readAt(m_cursor, entry)) {
            if (m_enabledSources.isEmpty() || m_enabledSources.contains((int)entry.source)) {
                inject(entry);
                ++m_injected;
            }
        }
        ++m_cursor;
    }

    emit positionChanged(m_positionMs, (int)m_injected);

    if (m_cursor >= idx.size()) {
        m_playing = false;
        m_timer.stop();
        emit playbackFinished();
        emit statusChanged(QStringLiteral("回放结束"));
    }
}

void ReplayPlayer::inject(const RecEntry &entry)
{
    switch (entry.type) {
    case RecType::PlatformFull:
    case RecType::PlatformDelta: {
        PlatformData p;
        if (deserializePlatform(entry.payload, p)) {
            // 回放数据标记为 Protocol_Unknown，录制时会过滤掉，避免循环
            const QJsonObject json = DdsConverter::platformToJson(p);
            DataManager::instance()->onDataReceived(json, Protocol_Unknown);
        }
        break;
    }
    case RecType::Event: {
        SpecialEvent ev;
        if (deserializeEvent(entry.payload, ev)) {
            const QJsonObject json = DdsConverter::eventToJson(ev);
            DataManager::instance()->onDataReceived(json, Protocol_Unknown);
        }
        break;
    }
    case RecType::Remove: {
        const QString id = QString::fromUtf8(entry.payload);
        DataCache::instance()->removePlatform(id);
        break;
    }
    case RecType::KeyFrame:
        // 预留：第一版未生成关键帧
        break;
    }
}
