/**
 * @file ddsadapter.cpp
 * @brief DDS 协议适配器实现（Fast DDS）
 * @details CPSS_ENABLE_DDS 未定义时为空实现桩，保证无 Fast DDS 环境也能编译。
 * @date 2026-11-22
 */

#include "ddsadapter.h"
#include "ddsconverter.h"
#include "logger.h"

#include <QJsonDocument>
#include <QDateTime>
#include <QThread>
#include <QMutexLocker>

static inline void ddsLog(const QString &msg) {
    Logger::info("[DdsAdapter] %s", msg.toUtf8().constData());
}

DdsAdapter::DdsAdapter(QObject *parent)
    : IProtocolAdapter(parent)
{
}

DdsAdapter::~DdsAdapter()
{
    stop();
}

bool DdsAdapter::start()
{
    if (m_status == Running) return true;

#ifndef CPSS_ENABLE_DDS
    m_lastError = QStringLiteral("未启用 Fast DDS：请用 vcpkg 安装 fastdds 并在 CMake 中启用 CPSS_ENABLE_DDS");
    ddsLog(m_lastError);
    m_status = Error;
    emit errorOccurred(m_lastError);
    return false;
#else
    try {
        // ---- DomainParticipant ----
        DomainParticipantQos participantQos;
        participantQos.name("CPSS_DDS_Participant");
        m_participant = DomainParticipantFactory::get_instance()->create_participant(
            static_cast<uint32_t>(m_domainId), participantQos);
        if (!m_participant) throw std::runtime_error("create_participant failed");

        // ---- 注册类型 ----
        m_platformType.register_type(m_participant);
        m_eventType.register_type(m_participant);

        // ---- PlatformState topic / writer / reader ----
        m_platformStateTopic = m_participant->create_topic(
            m_platformTopic.toStdString(), m_platformType.get_type_name(), TOPIC_QOS_DEFAULT);
        m_publisher = m_participant->create_publisher(PUBLISHER_QOS_DEFAULT);
        m_platformWriter = m_publisher->create_datawriter(m_platformStateTopic, DATAWRITER_QOS_DEFAULT);
        m_subscriber = m_participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT);

        DataReaderQos readerQos;
        readerQos.durability().kind = VOLATILE_DURABILITY_QOS;
        readerQos.reliability().kind = BEST_EFFORT_RELIABILITY_QOS;
        m_platformReader = m_subscriber->create_datareader(m_platformStateTopic, readerQos);

        // ---- TacticalEvent topic / writer（事件只发布，接收用简化处理）----
        m_tacticalEventTopic = m_participant->create_topic(
            m_eventTopic.toStdString(), m_eventType.get_type_name(), TOPIC_QOS_DEFAULT);
        m_eventWriter = m_publisher->create_datawriter(m_tacticalEventTopic, DATAWRITER_QOS_DEFAULT);

        m_running = true;
        m_status = Running;
        emit statusChanged(m_status);
        ddsLog(QStringLiteral("started, domain=%1 platformTopic=%2").arg(m_domainId).arg(m_platformTopic));
        return true;
    } catch (const std::exception &e) {
        m_lastError = QString::fromUtf8(e.what());
        ddsLog(QStringLiteral("start error: ") + m_lastError);
        m_status = Error;
        emit errorOccurred(m_lastError);
        return false;
    }
#endif
}

bool DdsAdapter::stop()
{
    if (m_status == Stopped) return true;
    m_running = false;

#ifdef CPSS_ENABLE_DDS
    if (m_participant) {
        if (m_platformWriter) m_publisher->delete_datawriter(m_platformWriter);
        if (m_platformReader) m_subscriber->delete_datareader(m_platformReader);
        if (m_eventWriter)    m_publisher->delete_datawriter(m_eventWriter);
        if (m_platformStateTopic) m_participant->delete_topic(m_platformStateTopic);
        if (m_tacticalEventTopic) m_participant->delete_topic(m_tacticalEventTopic);
        if (m_publisher)   m_participant->delete_publisher(m_publisher);
        if (m_subscriber)  m_participant->delete_subscriber(m_subscriber);
        DomainParticipantFactory::get_instance()->delete_participant(m_participant);
        m_participant = nullptr;
        m_platformWriter = m_platformReader = nullptr;
        m_eventWriter = nullptr;
        m_publisher = m_subscriber = nullptr;
        m_platformStateTopic = m_tacticalEventTopic = nullptr;
    }
#endif
    m_status = Stopped;
    emit statusChanged(m_status);
    ddsLog(QStringLiteral("stopped"));
    return true;
}

bool DdsAdapter::publishPlatform(const PlatformData &platform)
{
#ifndef CPSS_ENABLE_DDS
    return false;
#else
    if (!m_platformWriter || !m_running) return false;
    PlatformState sample;
    // TODO: 由 fastddsgen 生成类型后，在此把 PlatformData 字段填入 sample
    Q_UNUSED(sample);
    Q_UNUSED(platform);
    return m_platformWriter->write(&sample) == RETCODE_OK;
#endif
}

bool DdsAdapter::publishEvent(const SpecialEvent &event)
{
#ifndef CPSS_ENABLE_DDS
    return false;
#else
    if (!m_eventWriter || !m_running) return false;
    TacticalEvent sample;
    Q_UNUSED(sample);
    Q_UNUSED(event);
    return m_eventWriter->write(&sample) == RETCODE_OK;
#endif
}

bool DdsAdapter::publishPlatforms(const QList<PlatformData> &platforms)
{
    bool ok = true;
    for (const PlatformData &p : platforms) ok &= publishPlatform(p);
    return ok;
}

void DdsAdapter::subscriptionLoop()
{
#ifndef CPSS_ENABLE_DDS
    return;
#else
    // 轮询读取 DDS 样本，转成 JSON 后通过 dataReceived 发给 DataManager
    while (m_running && m_platformReader) {
        PlatformState sample;
        SampleInfo info;
        if (m_platformReader->take_next_sample(&sample, &info) == RETCODE_OK && info.valid_data) {
            // TODO: sample -> PlatformData（用 DdsConverter 逻辑），此处先发平台 id 占位
            QJsonObject obj;
            obj["type"] = QStringLiteral("platform");
            obj["id"] = QString::fromStdString(sample.id());
            emit dataReceived(obj, Protocol_FastDds);
        }
        QThread::msleep(10);
    }
#endif
}
