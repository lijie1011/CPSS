/**
 * @file udpforwarder.cpp
 * @brief UDP 转发器实现（单播 + 组播）
 * @date 2026-11-24
 */

#include "udpforwarder.h"
#include "forwardconfig.h"
#include "logger.h"

#include <QNetworkInterface>

UdpForwarder::UdpForwarder()
{
}

UdpForwarder::~UdpForwarder()
{
    close();
}

bool UdpForwarder::open(const QJsonObject &cfg)
{
    const QString subMode = cfgStr(cfg, QStringLiteral("subMode"), QStringLiteral("Unicast"));
    m_multicast = (subMode.compare(QStringLiteral("Multicast"), Qt::CaseInsensitive) == 0);
    m_port = (quint16)cfgInt(cfg, QStringLiteral("port"), 12345);

    if (m_multicast) {
        const QString addr = cfgStr(cfg, QStringLiteral("multicastAddr"), QStringLiteral("239.255.0.1"));
        m_target.setAddress(addr);
        if (m_target.isNull() || !m_target.isMulticast()) {
            m_lastError = QStringLiteral("无效组播地址: %1").arg(addr);
            return false;
        }
        const int ttl = cfgInt(cfg, QStringLiteral("ttl"), 2);
        m_socket.setSocketOption(QUdpSocket::MulticastTtlOption, ttl);
        const bool loopback = cfgBool(cfg, QStringLiteral("loopback"), true);
        m_socket.setSocketOption(QUdpSocket::MulticastLoopbackOption, loopback ? 1 : 0);

        const QString ifName = cfgStr(cfg, QStringLiteral("localInterface"), QString());
        if (!ifName.isEmpty()) {
            QNetworkInterface iface = QNetworkInterface::interfaceFromName(ifName);
            if (iface.isValid()) {
                // Qt5.12 用 setMulticastInterface（Qt5.15+ 才有 MulticastInterfaceOption）
                m_socket.setMulticastInterface(iface);
            } else {
                m_lastError = QStringLiteral("无效网卡: %1").arg(ifName);
                return false;
            }
        }
    } else {
        const QString addr = cfgStr(cfg, QStringLiteral("targetIp"), QStringLiteral("127.0.0.1"));
        m_target.setAddress(addr);
        if (m_target.isNull()) {
            m_lastError = QStringLiteral("无效IP: %1").arg(addr);
            return false;
        }
    }

    m_ready = true;
    Logger::info("[UdpForwarder] ready mode=%s target=%s port=%d",
                 m_multicast ? "multicast" : "unicast",
                 m_target.toString().toUtf8().constData(), m_port);
    return true;
}

bool UdpForwarder::send(const QByteArray &payload)
{
    if (!m_ready) return false;
    if (payload.size() > MAX_DATAGRAM) {
        m_lastError = QStringLiteral("数据超过 %1 字节，已丢弃").arg(MAX_DATAGRAM);
        Logger::warn("[UdpForwarder] datagram too large: %d", payload.size());
        return false;
    }
    const qint64 sent = m_socket.writeDatagram(payload, m_target, m_port);
    if (sent < 0) {
        m_lastError = m_socket.errorString();
        return false;
    }
    return true;
}

void UdpForwarder::close()
{
    if (!m_ready) return;
    m_socket.close();
    m_ready = false;
}
