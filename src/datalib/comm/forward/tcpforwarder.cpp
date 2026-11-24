/**
 * @file tcpforwarder.cpp
 * @brief TCP 转发器实现
 * @date 2026-11-24
 */

#include "tcpforwarder.h"
#include "forwardconfig.h"
#include "logger.h"

#include <QDataStream>

TcpForwarder::TcpForwarder()
{
    QObject::connect(&m_socket, &QTcpSocket::connected, this, &TcpForwarder::onConnected);
    QObject::connect(&m_socket, &QTcpSocket::disconnected, this, &TcpForwarder::onDisconnected);
    QObject::connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                     this, &TcpForwarder::onError);
    QObject::connect(&m_reconnectTimer, &QTimer::timeout, this, &TcpForwarder::onReconnectTimer);
}

TcpForwarder::~TcpForwarder()
{
    close();
}

bool TcpForwarder::open(const QJsonObject &cfg)
{
    const QString ip = cfgStr(cfg, QStringLiteral("targetIp"), QStringLiteral("127.0.0.1"));
    m_port = (quint16)cfgInt(cfg, QStringLiteral("port"), 12346);
    m_reconnectMs = cfgInt(cfg, QStringLiteral("reconnectMs"), 3000);
    m_target.setAddress(ip);
    if (m_target.isNull()) {
        m_lastError = QStringLiteral("无效IP: %1").arg(ip);
        return false;
    }
    m_manualClosed = false;
    connectToServer();
    Logger::info("[TcpForwarder] connecting %s:%d", ip.toUtf8().constData(), m_port);
    return true;   // 异步连接
}

void TcpForwarder::connectToServer()
{
    if (m_manualClosed) return;
    m_socket.abort();
    m_socket.connectToHost(m_target, m_port);
}

bool TcpForwarder::send(const QByteArray &payload)
{
    if (m_socket.state() != QTcpSocket::ConnectedState) return false;

    // 4 字节长度前缀（网络字节序）+ payload
    QByteArray frame;
    QDataStream ds(&frame, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << (quint32)payload.size();
    frame.append(payload);

    return m_socket.write(frame) == frame.size();
}

void TcpForwarder::close()
{
    m_manualClosed = true;
    m_reconnectTimer.stop();
    m_socket.abort();
}

void TcpForwarder::onConnected()
{
    m_reconnectTimer.stop();
    m_lastError.clear();
    Logger::info("[TcpForwarder] connected to %s:%d",
                 m_target.toString().toUtf8().constData(), m_port);
}

void TcpForwarder::onDisconnected()
{
    if (m_manualClosed) return;
    Logger::warn("[TcpForwarder] disconnected, will retry in %d ms", m_reconnectMs);
    m_reconnectTimer.start(m_reconnectMs);
}

void TcpForwarder::onError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err);
    m_lastError = m_socket.errorString();
    if (!m_manualClosed && !m_reconnectTimer.isActive()) {
        m_reconnectTimer.start(m_reconnectMs);
    }
}

void TcpForwarder::onReconnectTimer()
{
    if (m_manualClosed) { m_reconnectTimer.stop(); return; }
    connectToServer();
}
