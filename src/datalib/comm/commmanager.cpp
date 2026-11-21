/**
 * @file commmanager.cpp
 * @brief 通信管理器实现
 * @date 2026-11-22
 */

#include "commmanager.h"
#include "datamanager.h"
#include "dds/ddsadapter.h"
// #include "http/httpadapter.h"   // 启用 HTTP 适配器时取消注释
#include "logger.h"

static inline void commLog(const QString &msg) {
    Logger::info("[CommManager] %s", msg.toUtf8().constData());
}

CommManager* CommManager::s_instance = nullptr;

CommManager::CommManager(QObject *parent)
    : QObject(parent)
{
}

CommManager::~CommManager()
{
    stopAll();
}

CommManager* CommManager::instance()
{
    if (!s_instance) s_instance = new CommManager();
    return s_instance;
}

void CommManager::startAll()
{
    if (m_started) return;
    m_started = true;

    DataManager* dm = DataManager::instance();

    // ---- DDS 适配器（订阅+发布）----
    DdsAdapter* dds = new DdsAdapter(this);
    dds->setDomainId(0);
    dds->setPlatformTopic(QStringLiteral("cpss/platform"));
    dds->setEventTopic(QStringLiteral("cpss/event"));
    dm->addAdapter(dds);
    m_adapters.append(dds);
    if (dds->start()) {
        commLog(QStringLiteral("DDS adapter started"));
    } else {
        commLog(QStringLiteral("DDS adapter start failed (stub or no Fast DDS): ") + dds->lastError());
    }

    // ---- HTTP 适配器（预留：需要配置 URL 时才启用）----
    // HttpAdapter* http = new HttpAdapter(this);
    // http->setRequestUrl(...);
    // dm->addAdapter(http);
    // m_adapters.append(http);
    // http->start();

    dm->startAllAdapters();
    commLog(QStringLiteral("comm adapters initialized, count=%1").arg(m_adapters.size()));
}

void CommManager::stopAll()
{
    if (!m_started) return;
    for (IProtocolAdapter* a : m_adapters) {
        a->stop();
        DataManager::instance()->removeAdapter(a);
    }
    m_adapters.clear();
    m_started = false;
    commLog(QStringLiteral("all adapters stopped"));
}

IProtocolAdapter* CommManager::adapter(ProtocolType type) const
{
    for (IProtocolAdapter* a : m_adapters) {
        if (a->protocolType() == type) return a;
    }
    return nullptr;
}

bool CommManager::publishPlatformViaDds(const PlatformData &platform)
{
    IProtocolAdapter* a = adapter(Protocol_DDS);
    DdsAdapter* dds = qobject_cast<DdsAdapter*>(a);
    return dds ? dds->publishPlatform(platform) : false;
}
