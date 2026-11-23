/**
 * @file commmanager.cpp
 * @brief 通信管理器实现
 * @date 2026-11-22
 */

#include "commmanager.h"
#include "datamanager.h"
#include "fastdds/ddsadapter.h"
#include "redis/redisadapter.h"
#include "rtidds/situationforwarder.h"
#ifdef CPSS_ENABLE_PGDDS
#include "pgdds/pgddsadapter.h"
#endif
// #include "http/httpadapter.h"   // 启用 HTTP 适配器时取消注释
#include "logger.h"
#include "config.h"

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

    // ---- Redis 适配器（订阅 AFSIM 态势数据）----
    Config &cfg = Config::instance();
    RedisAdapter* redis = new RedisAdapter(this);
    redis->setHost(cfg.getRedisHost());          // 默认 127.0.0.1
    redis->setPort(cfg.getRedisPort());          // 默认 6379
    redis->setChannel(QStringLiteral("target:data:channel"));
    dm->addAdapter(redis);
    m_adapters.append(redis);
    if (redis->start()) {
        commLog(QStringLiteral("Redis adapter started: %1:%2")
                .arg(redis->host()).arg(redis->port()));
    } else {
        commLog(QStringLiteral("Redis adapter start failed: ") + redis->lastError());
    }

    // ---- PGDDS 适配器（国产 DDS，绑定IP）----
#ifdef CPSS_ENABLE_PGDDS
    if (cfg.getPgDdsEnabled()) {
        PgDdsAdapter* pgdds = new PgDdsAdapter(this);
        pgdds->setDomainId(cfg.getPgDdsDomainId());
        pgdds->setBindAddress(cfg.getPgDdsBindAddress());
        pgdds->setPlatformTopic(cfg.getPgDdsPlatformTopic());
        pgdds->setEventTopic(cfg.getPgDdsEventTopic());
        dm->addAdapter(pgdds);
        m_adapters.append(pgdds);
        if (pgdds->start()) {
            commLog(QStringLiteral("PGDDS adapter started: bind=%1 domain=%2")
                    .arg(cfg.getPgDdsBindAddress()).arg(cfg.getPgDdsDomainId()));
        } else {
            commLog(QStringLiteral("PGDDS adapter start failed: ") + pgdds->lastError());
        }
    } else {
        commLog(QStringLiteral("PGDDS adapter disabled by config"));
    }
#endif

    // ---- 态势显示转发器（RTI DDS 发布端）----
    SituationForwarder::instance()->initialize();

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
    IProtocolAdapter* a = adapter(Protocol_FastDds);
    DdsAdapter* dds = qobject_cast<DdsAdapter*>(a);
    return dds ? dds->publishPlatform(platform) : false;
}
