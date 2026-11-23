/**
 * @file pgddsmanager.cpp
 * @brief PGDDS 全局管理器实现
 * @date 2026-11-24
 */

#include "pgddsmanager.h"
#include "logger.h"

#include "dds_config_builder.h"
#include <QMutexLocker>

PgDdsManager::PgDdsManager()
{
}

PgDdsManager::~PgDdsManager()
{
    cleanup();
}

PgDdsManager* PgDdsManager::instance()
{
    static PgDdsManager mgr;
    return &mgr;
}

bool PgDdsManager::initialize(int domainId, const QString &bindAddress)
{
    QMutexLocker locker(&m_mutex);
    if (m_initialized) return true;

    // 绑定 IP 初始化（参考 DdsManager.cpp）
    DDSConfig* config = dds_config_create_default(domainId);
    if (!config) {
        Logger::error("[PgDdsManager] dds_config_create_default failed");
        return false;
    }
    dds_config_disable_topic_name_validation(config, 1);   // 允许数字开头的 topic
    const QByteArray ipBytes = bindAddress.toUtf8();
    dds_config_set_bind_address(config, ipBytes.constData());

    int ret = nonfixed_init_with_config(config);
    dds_config_destroy(config);
    if (ret != 1) {
        Logger::error("[PgDdsManager] nonfixed_init_with_config failed, bind=%s",
                      bindAddress.toUtf8().constData());
        return false;
    }

    m_domainId = domainId;
    m_bindAddress = bindAddress;
    m_initialized = true;
    Logger::info("[PgDdsManager] initialized domain=%d bind=%s",
                 domainId, bindAddress.toUtf8().constData());
    return true;
}

void PgDdsManager::cleanup()
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return;

    m_callbacks.clear();
    nonfixed_cleanup(m_domainId);
    m_initialized = false;
    Logger::info("[PgDdsManager] cleaned up domain=%d", m_domainId);
}

bool PgDdsManager::isInitialized() const
{
    QMutexLocker locker(&m_mutex);
    return m_initialized;
}

bool PgDdsManager::publishTopic(const QString &topic, const QString &publisherName, bool reliable)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return false;

    int preset = reliable ? QOS_PRESET_RELIABLE : QOS_PRESET_BEST_EFFORT;
    DataWriterQos* qos = create_datawriter_qos_with_preset(preset);
    if (!qos) return false;
    const QByteArray topicBytes = topic.toUtf8();
    const QByteArray pubBytes = publisherName.toUtf8();
    int ret = nonfixed_pub_topic(m_domainId, topicBytes.constData(), qos, pubBytes.constData());
    free_datawriter_qos(qos);

    if (ret == 1) {
        Logger::info("[PgDdsManager] published topic=%s publisher=%s",
                     topicBytes.constData(), pubBytes.constData());
    }
    return ret == 1;
}

bool PgDdsManager::unpublishTopic(const QString &topic, const QString &publisherName)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return true;
    const QByteArray topicBytes = topic.toUtf8();
    const QByteArray pubBytes = publisherName.toUtf8();
    return nonfixed_unpub_topic(m_domainId, topicBytes.constData(), pubBytes.constData()) == 1;
}

bool PgDdsManager::sendData(const QString &topic, const char* data, int len, const QString &publisherName)
{
    // 不在锁内调用 SDK 发送，避免阻塞回调分发
    const QByteArray topicBytes = topic.toUtf8();
    const QByteArray pubBytes = publisherName.toUtf8();
    int domain = 0;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_initialized) return false;
        domain = m_domainId;
    }
    return nonfixed_send_data(domain, topicBytes.constData(),
                              const_cast<char*>(data), len, pubBytes.constData()) == 1;
}

bool PgDdsManager::subscribeTopic(const QString &topic, const QString &subscriberName,
                                  PgDdsDataCallback callback, bool reliable)
{
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return false;

    int preset = reliable ? QOS_PRESET_RELIABLE : QOS_PRESET_BEST_EFFORT;
    DataReaderQos* qos = create_datareader_qos_with_preset(preset);
    if (!qos) return false;
    const QByteArray topicBytes = topic.toUtf8();
    const QByteArray subBytes = subscriberName.toUtf8();
    int ret = nonfixed_sub_topic(m_domainId, topicBytes.constData(),
                                 &PgDdsManager::onDataReceived, qos, subBytes.constData());
    free_datareader_qos(qos);

    if (ret == 1) {
        m_callbacks[topic].append(callback);
        Logger::info("[PgDdsManager] subscribed topic=%s subscriber=%s",
                     topicBytes.constData(), subBytes.constData());
    }
    return ret == 1;
}

bool PgDdsManager::unsubscribeTopic(const QString &topic, const QString &subscriberName)
{
    Q_UNUSED(subscriberName);
    QMutexLocker locker(&m_mutex);
    if (!m_initialized) return true;
    m_callbacks.remove(topic);
    const QByteArray topicBytes = topic.toUtf8();
    const QByteArray subBytes = subscriberName.toUtf8();
    return nonfixed_unsub_topic(m_domainId, topicBytes.constData(), subBytes.constData()) == 1;
}

int PgDdsManager::onDataReceived(int domainId, const char* topicName, char* buff, int length)
{
    // SDK 内部线程调用：复制回调列表后在锁外执行，避免死锁
    QList<PgDdsDataCallback> callbacks;
    {
        QMutexLocker locker(&PgDdsManager::instance()->m_mutex);
        callbacks = PgDdsManager::instance()->m_callbacks.value(QString::fromUtf8(topicName));
    }
    for (const PgDdsDataCallback &cb : callbacks) {
        cb(domainId, topicName, buff, length);
    }
    return 1;
}
