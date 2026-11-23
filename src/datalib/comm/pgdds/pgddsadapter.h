/**
 * @file pgddsadapter.h
 * @brief PGDDS 协议适配器（国产 DDS，nonfixed 动态类型）
 * @details 继承 IProtocolAdapter，把 PGDDS 收发接入 DataManager 数据管线。
 *          订阅：DDS topic -> JSON -> dataReceived(QJsonObject, Protocol_PgDds)
 *          发布：PlatformData/Event -> JSON -> DDS topic
 *
 *          SDK 回调在 DDS 内部线程，用 QueuedConnection 切回主线程后再 emit。
 * @date 2026-11-24
 */

#ifndef PGDDSADAPTER_H
#define PGDDSADAPTER_H

#include "protocoladapter.h"
#include "dynamicdata.h"

#include <QString>
#include <QJsonObject>

class PgDdsAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit PgDdsAdapter(QObject *parent = nullptr);
    ~PgDdsAdapter() override;

    // ---- IProtocolAdapter ----
    ProtocolType protocolType() const override { return Protocol_PgDds; }
    QString adapterName() const override { return QStringLiteral("PGDDS Adapter"); }
    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    // ---- 配置 ----
    void setDomainId(int id) { m_domainId = id; }
    void setBindAddress(const QString &ip) { m_bindAddress = ip; }
    void setPlatformTopic(const QString &t) { m_platformTopic = t; }
    void setEventTopic(const QString &t) { m_eventTopic = t; }
    void setReliable(bool reliable) { m_reliable = reliable; }

    // ---- 发布 ----
    bool publishPlatform(const PlatformData &platform);
    bool publishEvent(const SpecialEvent &event);

signals:
    /** @brief 内部：DDS 线程收到原始数据，QueuedConnection 切到主线程 */
    void rawDataReceived(int domainId, QString topic, QByteArray payload);

    /** @brief 从 PGDDS 收到平台数据（主线程） */
    void platformReceived(const PlatformData &platform);
    /** @brief 从 PGDDS 收到事件数据（主线程） */
    void eventReceived(const SpecialEvent &event);

private slots:
    /** @brief DDS 内部线程收到数据后，切到主线程处理 */
    void onRawDataReceived(int domainId, QString topic, QByteArray payload);

private:
    void handleJson(const QString &topic, const QJsonObject &json);

    AdapterStatus m_status = Stopped;
    QString m_lastError;
    int m_domainId = 0;
    QString m_bindAddress = QStringLiteral("127.0.0.1");
    QString m_platformTopic = QStringLiteral("cpss/platform");
    QString m_eventTopic = QStringLiteral("cpss/event");
    bool m_reliable = false;
};

#endif // PGDDSADAPTER_H
