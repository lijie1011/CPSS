/**
 * @file recorder.h
 * @brief 录制器与播放器
 * @details RecordWriter 连接 DataManager 信号录制到 CRF2 文件；
 *          ReplayPlayer 从 CRF2 文件按时间驱动回放。
 * @date 2026-11-23
 */

#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QSet>
#include <QByteArray>

/**
 * @brief 回放转发数据（同时提供 JSON 与原始二进制两种形式）
 */
struct ForwardPayload {
    QByteArray json;      ///< Compact JSON（DDS/JSON模式用）
    QByteArray binary;    ///< 原始 QDataStream 二进制（录制时 serializeXxx 的输出）
};
Q_DECLARE_METATYPE(ForwardPayload)
#include "recfile.h"
#include "datamanager.h"

/**
 * @class RecordWriter
 * @brief 录制器：订阅 DataManager 信号，按采样/过滤写入 CRF2 文件
 */
class RecordWriter : public QObject
{
    Q_OBJECT

public:
    explicit RecordWriter(QObject *parent = nullptr);
    ~RecordWriter();

    /** @brief 开始录制；sampleDiv=采样分频(1=全量,2=1/2,5=1/5)；enabledSources=启用的源(ProtocolType) */
    bool start(const QString &path, int sampleDiv, const QSet<int> &enabledSources,
               int campFilter = -1, const QString &typeFilter = QString());
    void stop();
    bool isRecording() const { return m_recording; }
    qint64 recordedCount() const { return m_recorded; }
    qint64 elapsedMs() const;
    QString lastFilePath() const { return m_lastPath; }

signals:
    void statusChanged(const QString &msg);

private slots:
    void onRawDataReceived(const QByteArray &rawPayload, ProtocolType source, int dataType);
    void onPlatformRemoved(const QString &id);
    void onFlushTimer();

private:
    void appendRecord(quint8 dataType, ProtocolType source, const QByteArray &payload);

    RecFileWriter m_writer;
    QTimer m_flushTimer;
    bool m_recording = false;
    qint64 m_startMs = 0;
    qint64 m_recorded = 0;
    int m_tick = 0;
    int m_sampleDiv = 1;
    QSet<int> m_enabledSources;
    int m_campFilter = -1;
    QString m_typeFilter;
    QString m_lastPath;          ///< 最近一次录制的文件路径
    QList<RecEntry> m_pending;   ///< 批量写入缓冲
};

/**
 * @class ReplayPlayer
 * @brief 播放器：按时间戳把 CRF2 记录重新注入 DataManager
 */
class ReplayPlayer : public QObject
{
    Q_OBJECT

public:
    explicit ReplayPlayer(QObject *parent = nullptr);
    ~ReplayPlayer();

    bool load(const QString &path);
    void unload();

    void play();
    void pause();
    void stop();
    /** @brief 跳转到指定时间（毫秒），seek 后从该位置继续播放 */
    void seekTo(qint64 ms);
    void setSpeed(double speed) { m_speed = speed; }

    bool isPlaying() const { return m_playing; }
    qint64 positionMs() const { return m_positionMs; }
    qint64 createTimeMs() const { return m_reader.createTimeMs(); }
    int injectedCount() const { return (int)m_injected; }
    qint64 durationMs() const { return m_reader.durationMs(); }
    int recordCount() const { return m_reader.index().size(); }
    /** @brief 设置回放源过滤；空集合 = 不回放任何源（未勾选任何回放源） */
    void setEnabledSources(const QSet<int> &sources) { m_enabledSources = sources; m_filterActive = true; }
    /** @brief 转发二进制数据的字节序（true=大端, false=小端） */
    void setBigEndian(bool big) { m_bigEndian = big; }

signals:
    void positionChanged(qint64 ms, int injectedCount);
    void statusChanged(const QString &msg);
    void playbackFinished();
    /** @brief 回放数据需要转发（由界面按选择的通信方式/格式处理） */
    void forwardData(const ForwardPayload &payload);

private slots:
    void onTick();

private:
    void inject(const RecEntry &entry);

    RecFileReader m_reader;
    QTimer m_timer;
    bool m_playing = false;
    qint64 m_positionMs = 0;
    qint64 m_injected = 0;
    double m_speed = 1.0;
    int m_cursor = 0;
    QSet<int> m_enabledSources;   ///< 启用的来源
    bool m_filterActive = false;  ///< 是否已应用过滤（false=不过滤全部回放）
    bool m_bigEndian = true;      ///< 转发二进制字节序（默认大端）
};

#endif // RECORDER_H
