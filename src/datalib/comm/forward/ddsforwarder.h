/**
 * @file ddsforwarder.h
 * @brief DDS 转发器（复用 PgDdsManager，仅发布）
 * @date 2026-11-24
 */

#ifndef DDSFORWARDER_H
#define DDSFORWARDER_H

#include "idataforwarder.h"
#include <QString>

class DdsForwarder : public IDataForwarder
{
public:
    DdsForwarder();
    ~DdsForwarder() override;

    bool open(const QJsonObject &cfg) override;
    bool send(const QByteArray &payload) override;
    void close() override;
    QString lastError() const override { return m_lastError; }
    bool isReady() const override { return m_ready; }

private:
    QString m_platformTopic;
    QString m_eventTopic;
    QString m_publisherName;
    QString m_lastError;
    bool m_ready = false;
    bool m_ownsDomain = false;   ///< 本转发器是否初始化了 domain（决定 close 是否 cleanup）
};

#endif // DDSFORWARDER_H
