/**
 * @file redisadapter.h
 * @brief Redis 协议适配器（订阅 AFSIM 态势数据）
 * @details 继承 IProtocolAdapter，通过 Redis SDK 订阅频道 "target:data:channel"，
 *          接收 AFSIM sendCustomize_Info 发布的 HTProto::Customize_Info，
 *          映射为 CPSS PlatformData 后接入 DataManager 数据管线。
 *
 *          数据流：AFSIM --Redis PUBLISH--> CPSS(RedisAdapter) --> DataManager --> DataCache --> 海图
 *                                                              \--> 态势显示转发(待RTI DDS)
 *
 *          ⚠️ Redis SDK 只有 Release(/MD) 版。Debug(/MDd) 下若链接会 CRT 冲突崩溃，
 *             故仅当 CPSS_ENABLE_REDIS 定义（Release + SDK 就绪）时编入真实实现，
 *             否则为桩实现（start 返回 false，不影响工程编译/运行）。
 * @date 2026-11-22
 */

#ifndef REDISADAPTER_H
#define REDISADAPTER_H

#include "protocoladapter.h"
#include "dynamicdata.h"

#include <QString>
#include <memory>
#include <thread>
#include <atomic>

#ifdef CPSS_ENABLE_REDIS
// Redis SDK（仅 Release 启用，避免 Debug/Release CRT 混用）
#include "redisConnection.h"
struct redisContext;
#endif

class RedisAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit RedisAdapter(QObject *parent = nullptr);
    ~RedisAdapter() override;

    // ---- IProtocolAdapter 接口 ----
    ProtocolType protocolType() const override { return Protocol_Redis; }
    QString adapterName() const override { return QStringLiteral("Redis Adapter (AFSIM)"); }
    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    // ---- Redis 配置 ----
    void setHost(const QString &host) { m_host = host; }
    QString host() const { return m_host; }
    void setPort(int port) { m_port = port; }
    int port() const { return m_port; }
    void setChannel(const QString &channel) { m_channel = channel; }
    QString channel() const { return m_channel; }

signals:
    /** @brief 收到平台数据（工作线程发出，通过 QueuedConnection 切主线程） */
    void platformReceived(const PlatformData &platform);

private slots:
    /** @brief 主线程处理收到的平台数据 */
    void onPlatformReceived(const PlatformData &platform);

private:
#ifdef CPSS_ENABLE_REDIS
    /** @brief 订阅线程主循环：读 Redis → gzip解压 → protobuf解析 → emit */
    void subscriptionLoop();
#endif

private:
    AdapterStatus m_status = Stopped;
    QString m_lastError;
    QString m_host = QStringLiteral("127.0.0.1");
    int m_port = 6379;
    QString m_channel = QStringLiteral("target:data:channel");

#ifdef CPSS_ENABLE_REDIS
    std::shared_ptr<RedisConnection> m_connection;  // 持有连接对象
    redisContext *m_subContext = nullptr;            // 专用订阅连接
    std::thread m_subThread;
    std::atomic<bool> m_running{false};
#endif
};

#endif // REDISADAPTER_H
