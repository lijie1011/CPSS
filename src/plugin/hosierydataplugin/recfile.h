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
static const char REC_MAGIC[8]   = {'C','P','S','S','R','E','C','2'};
static const char IDX_MAGIC[8]   = {'C','P','S','S','I','D','X','2'};
static const int  REC_HEADER_SIZE = 128;
static const int  RECORD_HEADER_SIZE = 30;   // type(1)+source(1)+flags(2)+relTime(8)+payloadLen(4)+reserved(14)

/** @brief 记录类型 */
enum class RecType : quint8 {
    PlatformFull = 1,   ///< 完整平台状态
    PlatformDelta = 2,  ///< 差量（预留）
    Event = 3,          ///< 事件
    Remove = 4,         ///< 平台移除（payload = id 字符串）
    KeyFrame = 5        ///< 关键帧快照（payload = 所有平台列表 JSON）
};

/** @brief 单条记录 */
struct RecEntry {
    RecType type;
    ProtocolType source;
    qint64 relTime;       ///< 相对录制起点的毫秒
    QByteArray payload;   ///< 二进制内容
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
    qint64 m_dataOffset = 0;
    int m_cursor = 0;
};

// ===== 序列化工具（QDataStream 二进制）=====
QByteArray serializePlatform(const PlatformData &p);
bool deserializePlatform(const QByteArray &data, PlatformData &out);
QByteArray serializeEvent(const SpecialEvent &e);
bool deserializeEvent(const QByteArray &data, SpecialEvent &out);

#endif // RECFILE_H
