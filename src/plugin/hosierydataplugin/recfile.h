/**
 * @file recfile.h
 * @brief CRF2（CPSS Recording Format v2）二进制录制文件格式
 * @details 单文件三段式结构：
 *          [文件头 128B] [元数据 JSON] [数据区（变长记录流）] [索引区 + 尾部]
 *          支持多数据源、时间戳、关键帧、追加写入、按索引随机访问。
 * @date 2026-11-23
 */

#ifndef RECFILE_H
#define RECFILE_H

#include <QObject>
#include <QDataStream>
#include <QFile>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QVector>

#include "dynamicdata.h"
#include "ddsconverter.h"

// ===== CRF2 常量 =====
static const char REC_MAGIC[8]   = {'C','P','S','S','R','E','C','3'};
static const char IDX_MAGIC[8]   = {'C','P','S','S','I','D','X','3'};
static const int  REC_HEADER_SIZE = 128;
static const int  RECORD_HEADER_SIZE = 14;   // dataType(1)+source(1)+relTime(8)+payloadLen(4)
static const int  HEADER_ACTUAL_SOURCES_OFFSET = 56;  // 实际写入的数据源列表（每字节一个 ProtocolType，0 结束）
static const int  HEADER_MAX_SOURCES = 16;

/** @brief 单条记录（CRF3：1字节类型头 + 原始 payload） */
struct RecEntry {
    quint8 dataType;      ///< RawDataType：payload 的类型（1字节）
    ProtocolType source;  ///< 来源协议
    qint64 relTime;       ///< 相对录制起点的毫秒
    QByteArray payload;   ///< 原始数据（如 Customize_Info protobuf 字节）
};

/** @brief 索引项 */
struct RecIndexItem {
    qint64 relTime;
    qint64 fileOffset;
};

/** @brief 文件信息（列表展示/回放用） */
struct RecFileInfo {
    QString fileName;
    qint64 createTimeMs = 0;
    qint64 durationMs = 0;
    int recordCount = 0;
    qint64 fileSize = 0;
    QList<int> sources;      ///< 包含的数据源（ProtocolType 值）
};

/**
 * @class RecFileWriter
 * @brief CRF2 写入器（追加式：打开文件写头 → 写记录 → 关闭时写索引）
 */
class RecFileWriter
{
public:
    RecFileWriter();
    ~RecFileWriter();

    bool open(const QString &path, const QList<int> &sources, const QJsonObject &meta = QJsonObject());
    /** @brief 追加一条记录；返回索引项供调用方缓存（也可 close 时统一写） */
    bool append(const RecEntry &entry);
    /** @brief 关闭：写索引区并回填文件头 */
    void close();

    bool isOpen() const { return m_file.isOpen(); }
    qint64 recordCount() const { return m_index.size(); }

private:
    void writeHeader(const QJsonObject &meta, const QList<int> &sources);
    void writeIndex();

    QFile m_file;
    QDataStream m_stream;
    qint64 m_startMs = 0;
    QVector<RecIndexItem> m_index;
    QList<int> m_sources;
    QList<int> m_actualSources;   ///< 实际写入记录的 source（close 时写入头）
};

/**
 * @class RecFileReader
 * @brief CRF2 读取器（只读索引到内存，记录按需 seek 读取）
 */
class RecFileReader
{
public:
    RecFileReader();
    ~RecFileReader();

    bool open(const QString &path);
    void close();

    const QVector<RecIndexItem>& index() const { return m_index; }
    qint64 durationMs() const;
    qint64 createTimeMs() const { return m_createTimeMs; }   ///< 录制起始的绝对时间戳
    QList<int> sources() const { return m_sources; }
    QJsonObject meta() const { return m_meta; }

    /** @brief 读取指定索引位置的记录 */
    bool readAt(int index, RecEntry &out);
    /** @brief 顺序读取下一条（配合游标使用） */
    bool readNext(RecEntry &out);

    /** @brief 重置顺序读取游标到指定索引（对应时间轴跳转） */
    bool seekToIndex(int index);
    /** @brief 找 <= relTime 的最后一条索引位置（二分查找） */
    int findIndexByTime(qint64 relTime) const;

    /** @brief 静态：读取文件摘要信息（不加载索引） */
    static RecFileInfo fileInfo(const QString &path);

private:
    QFile m_file;
    QDataStream m_stream;
    QVector<RecIndexItem> m_index;
    QJsonObject m_meta;
    QList<int> m_sources;
    qint64 m_createTimeMs = 0;
    qint64 m_dataOffset = 0;
    int m_cursor = 0;
};

// ===== 序列化工具（QDataStream 二进制）=====
QByteArray serializePlatform(const PlatformData &p);
bool deserializePlatform(const QByteArray &data, PlatformData &out);
QByteArray serializeEvent(const SpecialEvent &e);
bool deserializeEvent(const QByteArray &data, SpecialEvent &out);

#endif // RECFILE_H
