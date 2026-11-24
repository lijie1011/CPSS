/**
 * @file udpforwarder.h
 * @brief UDP 转发器（单播 + 组播）
 * @date 2026-11-24
 */

#ifndef UDPFORWARDER_H
#define UDPFORWARDER_H

#include "idataforwarder.h"
#include <QUdpSocket>
#include <QHostAddress>

class UdpForwarder : public IDataForwarder
{
public:
    UdpForwarder();
    ~UdpForwarder() override;

    bool open(const QJsonObject &cfg) override;
    bool send(const QByteArray &payload) override;
    void close() override;
    QString lastError() const override { return m_lastError; }
    bool isReady() const override { return m_ready; }

private:
    QUdpSocket m_socket;
    QHostAddress m_target;
    quint16 m_port = 0;
    bool m_multicast = false;
    QString m_lastError;
    bool m_ready = false;
    static const int MAX_DATAGRAM = 1400;   ///< 超过则丢弃（避免 IP 分片）
};

#endif // UDPFORWARDER_H
