/**
 * @file recfile.cpp
 * @brief CRF2 二进制录制文件格式实现
 * @date 2026-11-23
 */

#include "recfile.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

// ==================== 序列化工具 ====================

static QByteArray jsonToBytes(const QJsonObject &obj)
{
    return QJsonDocument(obj).toBinaryData();
}

static QJsonObject bytesToJson(const QByteArray &data)
{
    return QJsonDocument::fromBinaryData(data).object();
}

QByteArray serializePlatform(const PlatformData &p)
{
    QByteArray ba;
    QDataStream s(&ba, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);
    s << p.id << p.name << p.lon << p.lat << p.altitude << p.speed << p.heading
      << p.type << p.category << (qint32)p.camp << p.targetId;
    // 传感器
    s << (quint32)p.sensors.size();
    for (const SensorInfo &w : p.sensors) s << w.type << w.count << w.range;
    // 武器
    s << (quint32)p.weapons.size();
    for (const WeaponInfo &w : p.weapons) s << w.type << w.count << w.range;
    // 航迹
    s << (quint32)p.trackPoints.size();
    for (const QPointF &pt : p.trackPoints) s << pt;
    // 状态
    s << (qint32)p.dataStatus << p.validUntil << p.updateTime << (qint32)p.sourceProtocol;
    return ba;
}

bool deserializePlatform(const QByteArray &data, PlatformData &out)
{
    QDataStream s(data);
    s.setVersion(QDataStream::Qt_5_12);
    s >> out.id >> out.name >> out.lon >> out.lat >> out.altitude >> out.speed >> out.heading
      >> out.type >> out.category >> (qint32&)out.camp >> out.targetId;
    quint32 n = 0;
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        SensorInfo w;
        s >> w.type >> w.count >> w.range;
        out.sensors.append(w);
    }
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        WeaponInfo w;
        s >> w.type >> w.count >> w.range;
        out.weapons.append(w);
    }
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        QPointF pt;
        s >> pt;
        out.trackPoints.append(pt);
    }
    qint32 v = 0;
    s >> v; out.dataStatus = (DataStatus)v;
    s >> out.validUntil >> out.updateTime;
    s >> v; out.sourceProtocol = (ProtocolType)v;
    return s.status() == QDataStream::Ok;
}

QByteArray serializeEvent(const SpecialEvent &e)
{
    QByteArray ba;
    QDataStream s(&ba, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);
    const QByteArray extra = jsonToBytes(e.extraData);
    s << e.eventId << (qint32)e.eventType << e.eventName << e.description
      << e.timestamp << e.targetId << e.sourceId << extra << e.lon << e.lat;
    return ba;
}

bool deserializeEvent(const QByteArray &data, SpecialEvent &out)
{
    QDataStream s(data);
    s.setVersion(QDataStream::Qt_5_12);
    qint32 t = 0;
    QByteArray extra;
    s >> out.eventId >> t >> out.eventName >> out.description
      >> out.timestamp >> out.targetId >> out.sourceId >> extra >> out.lon >> out.lat;
    out.eventType = (SpecialEventType)t;
    out.extraData = bytesToJson(extra);
    return s.status() == QDataStream::Ok;
}

// ==================== Writer ====================

RecFileWriter::RecFileWriter()
    : m_stream(&m_file)
{
}

RecFileWriter::~RecFileWriter()
{
    close();
}

bool RecFileWriter::open(const QString &path, const QList<int> &sources, const QJsonObject &meta)
{
    close();
    m_index.clear();
    m_sources = sources;
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::WriteOnly)) return false;
    m_stream.setVersion(QDataStream::Qt_5_12);
    m_startMs = QDateTime::currentMSecsSinceEpoch();
    writeHeader(meta, sources);
    return true;
}

void RecFileWriter::writeHeader(const QJsonObject &meta, const QList<int> &sources)
{
    // 头 128 字节：magic(8) version(2) headerSize(2) flags(4) createTime(8)
    //              dataOffset(8) indexOffset(8) recordCount(8) duration(8) reserved(...)
    QByteArray head(REC_HEADER_SIZE, '\0');
    QDataStream hs(&head, QIODevice::WriteOnly);
    hs.setVersion(QDataStream::Qt_5_12);
    for (int i = 0; i < 8; ++i) hs << (quint8)REC_MAGIC[i];
    hs << (quint16)2;                 // version
    hs << (quint16)REC_HEADER_SIZE;   // headerSize
    hs << (quint32)0;                 // flags
    hs << m_startMs;                  // createTime
    hs << (qint64)REC_HEADER_SIZE;    // dataOffset（元数据在头之后，先写占位）
    hs << (qint64)0;                  // indexOffset（close 时回填）
    hs << (qint64)0;                  // recordCount（close 时回填）
    hs << (qint64)0;                  // duration（close 时回填）
    m_file.write(head);

    // 元数据区：sources + 用户 meta，长度前缀
    QJsonObject m = meta;
    QJsonArray srcArr;
    for (int s : sources) srcArr.append(s);
    m.insert("sources", srcArr);
    const QByteArray metaBytes = jsonToBytes(m);
    m_stream << (quint32)metaBytes.size();
    m_file.write(metaBytes);
}

bool RecFileWriter::append(const RecEntry &entry)
{
    if (!m_file.isOpen()) return false;
    const qint64 offset = m_file.pos();
    // 记录头
    m_stream << (quint8)entry.type;
    m_stream << (quint8)entry.source;
    m_stream << (quint16)0;                       // flags（压缩预留）
    m_stream << entry.relTime;
    m_stream << (quint32)entry.payload.size();
    QByteArray reserved(RECORD_HEADER_SIZE - 16, '\0');
    m_file.write(reserved);
    m_file.write(entry.payload);

    m_index.append({entry.relTime, offset});
    return true;
}

void RecFileWriter::close()
{
    if (!m_file.isOpen()) return;
    // 写索引区
    const qint64 indexOffset = m_file.pos();
    m_stream << (quint64)m_index.size();
    for (const RecIndexItem &it : m_index) {
        m_stream << it.relTime << it.fileOffset;
    }
    for (int i = 0; i < 8; ++i) m_stream << (quint8)IDX_MAGIC[i];

    // 回填文件头：indexOffset / recordCount / duration
    const qint64 duration = m_index.isEmpty() ? 0 : m_index.last().relTime;
    QByteArray patch(24, '\0');
    QDataStream ps(&patch, QIODevice::WriteOnly);
    ps.setVersion(QDataStream::Qt_5_12);
    ps << indexOffset << (qint64)m_index.size() << duration;
    // 头中 indexOffset 位置 = magic8 + version2 + headerSize2 + flags4 + createTime8 + dataOffset8 = 32
    m_file.seek(32);
    m_file.write(patch);   // indexOffset(8) + recordCount(8) + duration(8)
    m_file.close();
}

// ==================== Reader ====================

RecFileReader::RecFileReader()
    : m_stream(&m_file)
{
}

RecFileReader::~RecFileReader()
{
    close();
}

bool RecFileReader::open(const QString &path)
{
    close();
    m_file.setFileName(path);
    if (!m_file.open(QIODevice::ReadOnly)) return false;
    m_stream.setVersion(QDataStream::Qt_5_12);

    // 文件头
    QByteArray head = m_file.read(REC_HEADER_SIZE);
    if (head.size() < REC_HEADER_SIZE) { m_file.close(); return false; }
    if (head.left(8) != QByteArray(REC_MAGIC, 8)) { m_file.close(); return false; }

    QDataStream hs(head);
    hs.setVersion(QDataStream::Qt_5_12);
    hs.skipRawData(8 + 2 + 2 + 4);
    qint64 createTime = 0, dataOffset = 0, indexOffset = 0, recordCount = 0, duration = 0;
    hs >> createTime >> dataOffset >> indexOffset >> recordCount >> duration;

    // 元数据
    m_file.seek(REC_HEADER_SIZE);
    quint32 metaLen = 0;
    m_stream >> metaLen;
    QByteArray metaBytes = m_file.read(metaLen);
    QJsonObject meta = bytesToJson(metaBytes);
    QJsonArray srcArr = meta.take("sources").toArray();
    for (const QJsonValue &v : srcArr) m_sources.append(v.toInt());
    m_meta = meta;
    m_dataOffset = m_file.pos();

    // 索引区（尾部）
    if (indexOffset > 0 && recordCount > 0) {
        m_file.seek(indexOffset);
        quint64 n = 0;
        m_stream >> n;
        m_index.clear();
        m_index.reserve(n);
        for (quint64 i = 0; i < n; ++i) {
            qint64 t = 0, off = 0;
            m_stream >> t >> off;
            m_index.append({t, off});
        }
    }
    m_cursor = 0;
    return true;
}

void RecFileReader::close()
{
    if (m_file.isOpen()) m_file.close();
    m_index.clear();
    m_sources.clear();
    m_cursor = 0;
}

qint64 RecFileReader::durationMs() const
{
    return m_index.isEmpty() ? 0 : m_index.last().relTime;
}

bool RecFileReader::readAt(int index, RecEntry &out)
{
    if (!m_file.isOpen() || index < 0 || index >= m_index.size()) return false;
    m_file.seek(m_index[index].fileOffset);
    quint8 t = 0, src = 0;
    quint16 flags = 0;
    qint64 relTime = 0;
    quint32 payloadLen = 0;
    m_stream >> t >> src >> flags >> relTime >> payloadLen;
    m_file.seek(m_file.pos() + (RECORD_HEADER_SIZE - 16));
    out.type = (RecType)t;
    out.source = (ProtocolType)src;
    out.relTime = relTime;
    out.payload = m_file.read(payloadLen);
    return (quint32)out.payload.size() == payloadLen;
}

bool RecFileReader::readNext(RecEntry &out)
{
    if (m_cursor >= m_index.size()) return false;
    if (!readAt(m_cursor, out)) return false;
    ++m_cursor;
    return true;
}

bool RecFileReader::seekToIndex(int index)
{
    if (index < 0 || index > m_index.size()) return false;
    m_cursor = index;
    return true;
}

int RecFileReader::findIndexByTime(qint64 relTime) const
{
    int lo = 0, hi = m_index.size() - 1, result = -1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (m_index[mid].relTime <= relTime) { result = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    return result;
}

RecFileInfo RecFileReader::fileInfo(const QString &path)
{
    RecFileInfo info;
    QFileInfo fi(path);
    info.fileName = fi.fileName();
    info.fileSize = fi.size();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly) || f.size() < REC_HEADER_SIZE) return info;
    QDataStream hs(&f);
    hs.setVersion(QDataStream::Qt_5_12);
    hs.skipRawData(8 + 2 + 2 + 4 + 8 + 8);
    qint64 indexOffset = 0, recordCount = 0, duration = 0;
    hs >> indexOffset >> recordCount >> duration;
    info.recordCount = (int)recordCount;
    info.durationMs = duration;

    // 元数据中的 sources
    f.seek(REC_HEADER_SIZE);
    QDataStream ds(&f);
    quint32 metaLen = 0;
    ds >> metaLen;
    QJsonObject meta = bytesToJson(f.read(metaLen));
    QJsonArray arr = meta.value("sources").toArray();
    for (const QJsonValue &v : arr) info.sources.append(v.toInt());
    f.close();
    return info;
}
