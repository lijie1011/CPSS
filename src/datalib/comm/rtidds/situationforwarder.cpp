/**
 * @file situationforwarder.cpp
 * @brief 态势数据转发器实现（RTI DDS 发布端）
 * @details CPSS_ENABLE_RTI_DDS 未定义时为空实现桩。
 * @date 2026-11-22
 */

#include "situationforwarder.h"
#include "logger.h"

#include <QJsonDocument>
#include <QDateTime>

static inline void rtiLog(const QString &msg) {
    Logger::info("[SituationForwarder] %s", msg.toUtf8().constData());
}

SituationForwarder* SituationForwarder::s_instance = nullptr;

SituationForwarder::SituationForwarder(QObject *parent)
    : QObject(parent)
{
}

SituationForwarder::~SituationForwarder()
{
    shutdown();
}

SituationForwarder* SituationForwarder::instance()
{
    if (!s_instance) s_instance = new SituationForwarder();
    return s_instance;
}

bool SituationForwarder::initialize()
{
#ifndef CPSS_ENABLE_RTI_DDS
    rtiLog(QStringLiteral("RTI Connext DDS 未启用：安装 RTI Connext 并在 CMake 定义 CPSS_ENABLE_RTI_DDS 后可转发到态势显示"));
    return false;
#else
    // TODO: 安装 RTI Connext DDS 后在此创建 DomainParticipant / Publisher / DataWriter
    // 发布 Topic 名称需与态势显示系统约定（如 "CPSS::PlatformState"）
    m_initialized = true;
    rtiLog(QStringLiteral("RTI DDS publisher initialized"));
    return true;
#endif
}

void SituationForwarder::shutdown()
{
#ifndef CPSS_ENABLE_RTI_DDS
    m_initialized = false;
#else
    // TODO: delete DataWriter / Publisher / Participant
    m_initialized = false;
    rtiLog(QStringLiteral("RTI DDS publisher shutdown"));
#endif
}

bool SituationForwarder::isEnabled() const
{
#ifdef CPSS_ENABLE_RTI_DDS
    return m_initialized;
#else
    return false;
#endif
}

void SituationForwarder::forwardPlatform(const PlatformData &platform)
{
#ifndef CPSS_ENABLE_RTI_DDS
    Q_UNUSED(platform);
    return;
#else
    if (!m_initialized) return;
    // TODO: PlatformData -> RTI DDS PlatformState 类型，write()
    // 由 fastdds/idl/CPSSData.idl 可转换为 RTI 的 .idl（字段定义通用）
    rtiLog(QStringLiteral("forward platform: ") + platform.id);
#endif
}

void SituationForwarder::forwardPlatforms(const QList<PlatformData> &platforms)
{
    for (const PlatformData &p : platforms) forwardPlatform(p);
}
