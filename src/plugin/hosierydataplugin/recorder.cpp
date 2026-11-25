/**
 * @file recorder.cpp
 * @brief 录制器与播放器实现
 * @date 2026-11-23
 */

#include "recorder.h"
#include "datacache.h"
#include "commmanager.h"
#include "ddsconverter.h"
#include "redis_fieldmap.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>

/**
 * @brief 把 CustomizeInfoC 按指定字节序序列化（第三方按 C struct 解析）
 * @details 与 C struct 内存布局字段顺序一致，总大小 265 字节；
 *          char 数组原样写出，整数按 bigEndian 指定的字节序写出。
 */
static QByteArray serializeCustomizeC(const CustomizeInfoC &cs, bool bigEndian)
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setByteOrder(bigEndian ? QDataStream::BigEndian : QDataStream::LittleEndian);
    ds << (qint32)cs.id              << (quint32)cs.entityId
       << (quint32)cs.entityCamp     << (quint32)cs.domain
       << (quint32)cs.category       << (qint32)cs.batchNumber
       << (qint32)cs.height          << (qint32)cs.longitude
       << (qint32)cs.latitude        << (qint32)cs.absoluteHeading
       << (qint32)cs.relativeHeading << (qint32)cs.threatLevel
       << (qint32)cs.distance        << (qint32)cs.azimuth
       << (qint32)cs.targetSpeed     << (qint32)cs.targetType1
       << (qint32)cs.enemyFriendship << (qint32)cs.civilianFriendship;
    ds.writeRawData(cs.name, sizeof(cs.name));          // 128 字节
    ds << (quint8)cs.isAlive;                           // 1 字节
    ds.writeRawData(cs.elevation, sizeof(cs.elevation)); // 64 字节
    return ba;
}

// ==================== RecordWriter ====================

RecordWriter::RecordWriter(QObject *parent)
    : QObject(parent)
{
    // 跨线程 QueuedConnection 传枚举需要注册元类型
    qRegisterMetaType<ProtocolType>("ProtocolType");
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

    // 连接各适配器的原始 dataReceived 信号（进 DataManager 融合之前的数据）
    for (IProtocolAdapter *adapter : CommManager::instance()->adapters()) {
        // 连接原始数据信号（解析前）：直接存外部软件的原始字节
        // Redis 的 rawDataReceived 在订阅线程 emit，用 QueuedConnection 切主线程
        connect(adapter, &IProtocolAdapter::rawDataReceived,
                this, &RecordWriter::onRawDataReceived, Qt::QueuedConnection);
    }
    // 平台移除仍用 DataManager（原始数据里没有独立的 remove 消息）
    connect(DataManager::instance(), &DataManager::platformRemoved,
            this, &RecordWriter::onPlatformRemoved, Qt::QueuedConnection);

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
    for (IProtocolAdapter *adapter : CommManager::instance()->adapters()) {
        disconnect(adapter, &IProtocolAdapter::rawDataReceived,
                   this, &RecordWriter::onRawDataReceived);
    }
    disconnect(DataManager::instance(), &DataManager::platformRemoved,
               this, &RecordWriter::onPlatformRemoved);

    m_writer.close();
    emit statusChanged(QStringLiteral("停止录制，共 %1 条").arg(m_recorded));
}

qint64 RecordWriter::elapsedMs() const
{
    return m_recording ? (QDateTime::currentMSecsSinceEpoch() - m_startMs) : 0;
}

void RecordWriter::onRawDataReceived(const QByteArray &rawPayload, ProtocolType source, int dataType)
{
    if (!m_recording) return;
    if (!m_enabledSources.contains((int)source)) return;
    // 原始数据直接存，不解析不融合；采样对原始数据不生效
    appendRecord((quint8)dataType, source, rawPayload);
}

void RecordWriter::onPlatformRemoved(const QString &id)
{
    if (!m_recording) return;
    appendRecord(RawType_Remove, Protocol_Unknown, id.toUtf8());
}

void RecordWriter::appendRecord(quint8 dataType, ProtocolType source, const QByteArray &payload)
{
    RecEntry entry;
    entry.dataType = dataType;
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
            // 未应用过滤时全部回放；应用过滤后必须勾选了对应源才回放
            if (!m_filterActive || m_enabledSources.contains((int)entry.source)) {
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
    ForwardPayload fp;
    fp.binary = entry.payload;   // 转发时始终发原始字节

    switch (entry.dataType) {
    case RawType_CustomizeInfo: {
        // AFSIM Customize_Info protobuf 原始字节 → PlatformData → 本机海图
        PlatformData p;
        if (RedisFieldMap::parseCustomizeInfo(entry.payload, p)) {
            const QJsonObject json = DdsConverter::platformToJson(p);
            DataManager::instance()->updatePlatformDirect(p, Protocol_Unknown);  // 强类型，跳过 JSON 解析
            fp.json = QJsonDocument(json).toJson(QJsonDocument::Compact);
        }
        // 【临时方案】转发用 C struct 字节，字节序由界面设置决定
        CustomizeInfoC cs;
        if (RedisFieldMap::parseCustomizeInfoC(entry.payload, cs)) {
            fp.binary = serializeCustomizeC(cs, m_bigEndian);
        }
        emit forwardData(fp);
        break;
    }
    case RawType_PlatformData: {
        PlatformData p;
        if (deserializePlatform(entry.payload, p)) {
            const QJsonObject json = DdsConverter::platformToJson(p);
            DataManager::instance()->updatePlatformDirect(p, Protocol_Unknown);  // 强类型
            fp.json = QJsonDocument(json).toJson(QJsonDocument::Compact);
            emit forwardData(fp);
        }
        break;
    }
    case RawType_SpecialEvent: {
        SpecialEvent ev;
        if (deserializeEvent(entry.payload, ev)) {
            const QJsonObject json = DdsConverter::eventToJson(ev);
            DataManager::instance()->onDataReceived(json, Protocol_Unknown);
            fp.json = QJsonDocument(json).toJson(QJsonDocument::Compact);
            emit forwardData(fp);
        }
        break;
    }
    case RawType_Remove: {
        const QString id = QString::fromUtf8(entry.payload);
        DataCache::instance()->removePlatform(id);
        QJsonObject obj;
        obj["type"] = QStringLiteral("remove");
        obj["id"] = id;
        fp.json = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        emit forwardData(fp);
        break;
    }
    case RawType_Json: {
        QJsonDocument doc = QJsonDocument::fromJson(entry.payload);
        if (doc.isObject()) {
            DataManager::instance()->onDataReceived(doc.object(), Protocol_Unknown);
        }
        fp.json = entry.payload;
        emit forwardData(fp);
        break;
    }
    default:
        // 未知类型：原样转发，不做本机显示
        emit forwardData(fp);
        break;
    }
}
