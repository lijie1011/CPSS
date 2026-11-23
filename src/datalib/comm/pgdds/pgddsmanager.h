/**
 * @file pgddsmanager.h
 * @brief PGDDS 全局管理器（singleton）
 * @details 管理 DDS domain 生命周期（绑定IP初始化/清理）、topic 订阅/发布、
 *          以及 C 回调到多个订阅者的分发。
 *          参考 DdsManager.cpp 的初始化方式：
 *            dds_config_create_default -> dds_config_set_bind_address ->
 *            nonfixed_init_with_config
 * @date 2026-11-24
 */

#ifndef PGDDSMANAGER_H
#define PGDDSMANAGER_H

#include "dynamicdata.h"
#include "dds_nonfixed.h"

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QString>
#include <functional>

/**
 * @brief 数据接收回调（运行在 DDS 内部线程，接收方负责切线程）
 */
typedef std::function<void(int domainId, const char* topic, const char* data, int len)> PgDdsDataCallback;

/**
 * @class PgDdsManager
 * @brief PGDDS 全局单例：一次 init（一个 domain），多个 topic 订阅者共享
 */
class PgDdsManager
{
public:
    static PgDdsManager* instance();

    /** @brief 初始化 DDS（绑定指定IP）；返回 true 成功 */
    bool initialize(int domainId, const QString &bindAddress);
    /** @brief 清理 DDS */
    void cleanup();
    bool isInitialized() const;

    /** @brief 发布 topic */
    bool publishTopic(const QString &topic, const QString &publisherName, bool reliable = false);
    bool unpublishTopic(const QString &topic, const QString &publisherName);
    /** @brief 发送数据 */
    bool sendData(const QString &topic, const char* data, int len, const QString &publisherName);

    /** @brief 订阅 topic（callback 在 DDS 内部线程调用） */
    bool subscribeTopic(const QString &topic, const QString &subscriberName,
                        PgDdsDataCallback callback, bool reliable = false);
    bool unsubscribeTopic(const QString &topic, const QString &subscriberName);

    int domainId() const { return m_domainId; }

private:
    PgDdsManager();
    ~PgDdsManager();

    /** @brief SDK 静态回调（C 函数指针） */
    static int onDataReceived(int domainId, const char* topicName, char* buff, int length);

    mutable QMutex m_mutex;
    bool m_initialized = false;
    int m_domainId = 0;
    QString m_bindAddress;

    // topic -> 订阅者列表（一个 topic 可能有多个订阅者）
    QMap<QString, QList<PgDdsDataCallback>> m_callbacks;
};

#endif // PGDDSMANAGER_H
