/**
 * @file ddsforwarder.cpp
 * @brief DDS 转发器实现（复用 PgDdsManager）
 * @date 2026-11-24
 */

#include "ddsforwarder.h"
#include "forwardconfig.h"
#include "pgddsmanager.h"
#include "logger.h"

#include <QJsonDocument>

DdsForwarder::DdsForwarder()
    : m_publisherName(QStringLiteral("cpss_replay_pub"))
{
}

DdsForwarder::~DdsForwarder()
{
    close();
}

bool DdsForwarder::open(const QJsonObject &cfg)
{
    const int domain = cfgInt(cfg, QStringLiteral("domain"), 0);
    const QString bind = cfgStr(cfg, QStringLiteral("bindAddress"), QStringLiteral("127.0.0.1"));
    m_platformTopic = cfgStr(cfg, QStringLiteral("platformTopic"), QStringLiteral("cpss/platform"));
    m_eventTopic = cfgStr(cfg, QStringLiteral("eventTopic"), QStringLiteral("cpss/event"));
    const bool reliable = cfgBool(cfg, QStringLiteral("reliable"), false);

    PgDdsManager *mgr = PgDdsManager::instance();
    if (!mgr->isInitialized()) {
        if (!mgr->initialize(domain, bind)) {
            m_lastError = QStringLiteral("PGDDS init failed: %1").arg(bind);
            return false;
        }
        m_ownsDomain = true;
    }

    mgr->publishTopic(m_platformTopic, m_publisherName, reliable);
    mgr->publishTopic(m_eventTopic, m_publisherName, reliable);
    m_ready = true;
    Logger::info("[DdsForwarder] ready domain=%d bind=%s", domain, bind.toUtf8().constData());
    return true;
}

bool DdsForwarder::send(const QByteArray &payload)
{
    if (!m_ready) return false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;
    const QString type = doc.object().value(QStringLiteral("type")).toString();
    const QString topic = (type == QStringLiteral("event")) ? m_eventTopic : m_platformTopic;
    return PgDdsManager::instance()->sendData(topic, payload.constData(), payload.size(), m_publisherName);
}

void DdsForwarder::close()
{
    if (!m_ready) return;
    PgDdsManager *mgr = PgDdsManager::instance();
    mgr->unpublishTopic(m_platformTopic, m_publisherName);
    mgr->unpublishTopic(m_eventTopic, m_publisherName);
    if (m_ownsDomain) mgr->cleanup();
    m_ready = false;
    m_ownsDomain = false;
}
