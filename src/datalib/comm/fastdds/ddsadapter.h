/**
 * @file ddsadapter.h
 * @brief DDS 协议适配器（Fast DDS）
 * @details 继承 IProtocolAdapter，把 DDS 收发接入现有 DataManager 数据管线：
 *          订阅(接收)：DDS PlatformState/TacticalEvent -> QJsonObject -> dataReceived 信号
 *          发布(发送)：把本地 PlatformData/SpecialEvent 转成 DDS 数据发布出去
 *
 *          通过宏 CPSS_ENABLE_DDS 控制是否编译真实 Fast DDS 代码。
 *          未安装 Fast DDS 时该适配器为空实现桩，不影响工程编译。
 * @date 2026-11-22
 */

#ifndef DDSADAPTER_H
#define DDSADAPTER_H

#include "protocoladapter.h"
#include "dynamicdata.h"

#include <QString>
#include <QJsonObject>
#include <QObject>

// 由 CMake 在找到 Fast DDS 时定义
#ifdef CPSS_ENABLE_DDS
#include "CPSSDataCdr.h"        // fastddsgen 生成的类型
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>

using namespace eprosima::fastdds::dds;
using namespace cpss::dds;
#endif

/**
 * @class DdsAdapter
 * @brief DDS 协议适配器，支持双向收发
 */
class DdsAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit DdsAdapter(QObject *parent = nullptr);
    ~DdsAdapter() override;

    // ---- IProtocolAdapter 接口 ----
    ProtocolType protocolType() const override { return Protocol_FastDds; }
    QString adapterName() const override { return QStringLiteral("DDS Adapter (Fast DDS)"); }
    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    // ---- DDS 配置 ----
    void setDomainId(int domainId) { m_domainId = domainId; }
    int domainId() const { return m_domainId; }

    void setPlatformTopic(const QString &topic) { m_platformTopic = topic; }
    QString platformTopic() const { return m_platformTopic; }

    void setEventTopic(const QString &topic) { m_eventTopic = topic; }
    QString eventTopic() const { return m_eventTopic; }

    // ---- 发布接口（供外部调用，把本地数据发出去）----
    bool publishPlatform(const PlatformData &platform);
    bool publishEvent(const SpecialEvent &event);
    bool publishPlatforms(const QList<PlatformData> &platforms);

signals:
    /** @brief 从 DDS 收到平台数据（已转为现有 PlatformData） */
    void platformReceived(const PlatformData &platform);
    /** @brief 从 DDS 收到事件数据（已转为现有 SpecialEvent） */
    void eventReceived(const SpecialEvent &event);

private:
    /** @brief 订阅线程处理函数，循环 take 数据 */
    void subscriptionLoop();

    AdapterStatus m_status = Stopped;
    QString m_lastError;
    int m_domainId = 0;
    int m_platformDepth = 100;
    QString m_platformTopic = QStringLiteral("cpss/platform");
    QString m_eventTopic = QStringLiteral("cpss/event");
    bool m_running = false;

#ifdef CPSS_ENABLE_DDS
    // ---- Fast DDS 实体 ----
    DomainParticipant *m_participant = nullptr;
    Publisher *m_publisher = nullptr;
    Subscriber *m_subscriber = nullptr;

    Topic *m_platformStateTopic = nullptr;
    TypeSupport m_platformType;
    DataWriter *m_platformWriter = nullptr;
    DataReader *m_platformReader = nullptr;

    Topic *m_tacticalEventTopic = nullptr;
    TypeSupport m_eventType;
    DataWriter *m_eventWriter = nullptr;
    DataReader *m_eventReader = nullptr;
#endif
};

#endif // DDSADAPTER_H
