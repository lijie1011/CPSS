/**
 * @file pgddsadapter.cpp
 * @brief PGDDS 协议适配器实现
 * @date 2026-11-24
 */

#include "pgddsadapter.h"
#include "pgddsmanager.h"
#include "ddsconverter.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>

PgDdsAdapter::PgDdsAdapter(QObject *parent)
    : IProtocolAdapter(parent)
{
    // DDS 内部线程 -> 主线程：用 QueuedConnection
    connect(this, &PgDdsAdapter::rawDataReceived,
            this, &PgDdsAdapter::onRawDataReceived, Qt::QueuedConnection);
}

PgDdsAdapter::~PgDdsAdapter()
{
    stop();
}

bool PgDdsAdapter::start()
{
    m_status = Starting;
    PgDdsManager *mgr = PgDdsManager::instance();

    // 初始化（绑定 IP）
    if (!mgr->initialize(m_domainId, m_bindAddress)) {
        m_lastError = QStringLiteral("PGDDS init failed, bind=%1").arg(m_bindAddress);
        m_status = Error;
        Logger::error("[PgDdsAdapter] %s", m_lastError.toUtf8().constData());
        return false;
    }

    // 发布 topic（本程序也发布数据）
    mgr->publishTopic(m_platformTopic, QStringLiteral("cpss_platform_pub"), m_reliable);
    mgr->publishTopic(m_eventTopic, QStringLiteral("cpss_event_pub"), m_reliable);

    // 订阅 topic：回调在 DDS 线程，转发为信号（QueuedConnection 切主线程）
    auto platformCb = [this](int domainId, const char* topic, const char* data, int len) {
        Q_UNUSED(topic);
        emit rawDataReceived(domainId, m_platformTopic, QByteArray(data, len));
    };
    auto eventCb = [this](int domainId, const char* topic, const char* data, int len) {
        Q_UNUSED(topic);
        emit rawDataReceived(domainId, m_eventTopic, QByteArray(data, len));
    };

    if (!mgr->subscribeTopic(m_platformTopic, QStringLiteral("cpss_platform_sub"), platformCb, m_reliable)) {
        m_lastError = QStringLiteral("subscribe platform topic failed: %1").arg(m_platformTopic);
        m_status = Error;
        return false;
    }
    if (!mgr->subscribeTopic(m_eventTopic, QStringLiteral("cpss_event_sub"), eventCb, m_reliable)) {
        m_lastError = QStringLiteral("subscribe event topic failed: %1").arg(m_eventTopic);
        m_status = Error;
        return false;
    }

    m_status = Running;
    Logger::info("[PgDdsAdapter] started domain=%d bind=%s platform=%s event=%s",
                 m_domainId, m_bindAddress.toUtf8().constData(),
                 m_platformTopic.toUtf8().constData(), m_eventTopic.toUtf8().constData());
    return true;
}

bool PgDdsAdapter::stop()
{
    if (m_status == Stopped) return true;
    PgDdsManager *mgr = PgDdsManager::instance();
    mgr->unsubscribeTopic(m_platformTopic, QStringLiteral("cpss_platform_sub"));
    mgr->unsubscribeTopic(m_eventTopic, QStringLiteral("cpss_event_sub"));
    mgr->unpublishTopic(m_platformTopic, QStringLiteral("cpss_platform_pub"));
    mgr->unpublishTopic(m_eventTopic, QStringLiteral("cpss_event_pub"));
    mgr->cleanup();
    m_status = Stopped;
    Logger::info("[PgDdsAdapter] stopped");
    return true;
}

bool PgDdsAdapter::publishPlatform(const PlatformData &platform)
{
    const QJsonObject json = DdsConverter::platformToJson(platform);
    const QByteArray payload = QJsonDocument(json).toJson(QJsonDocument::Compact);
    return PgDdsManager::instance()->sendData(m_platformTopic, payload.constData(),
                                              payload.size(), QStringLiteral("cpss_platform_pub"));
}

bool PgDdsAdapter::publishEvent(const SpecialEvent &event)
{
    const QJsonObject json = DdsConverter::eventToJson(event);
    const QByteArray payload = QJsonDocument(json).toJson(QJsonDocument::Compact);
    return PgDdsManager::instance()->sendData(m_eventTopic, payload.constData(),
                                              payload.size(), QStringLiteral("cpss_event_pub"));
}

void PgDdsAdapter::onRawDataReceived(int domainId, QString topic, QByteArray payload)
{
    Q_UNUSED(domainId);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        Logger::warn("[PgDdsAdapter] invalid JSON on topic %s", topic.toUtf8().constData());
        return;
    }
    handleJson(topic, doc.object());
}

void PgDdsAdapter::handleJson(const QString &topic, const QJsonObject &json)
{
    const QString type = json.value(QStringLiteral("type")).toString();
    Q_UNUSED(topic);

    if (type == QStringLiteral("platform")) {
        // 直接把 JSON 发给 DataManager 解析（原 jsonToPlatform 结果无消费者，属无效转换）
        emit dataReceived(json, Protocol_PgDds);
    } else if (type == QStringLiteral("event")) {
        SpecialEvent e = DdsConverter::jsonToEvent(json);
        emit eventReceived(e);
        emit dataReceived(json, Protocol_PgDds);
    } else if (type == QStringLiteral("batch")) {
        // batch 消息直接转发给 DataManager（它自己会拆 items）
        emit dataReceived(json, Protocol_PgDds);
    }
}
