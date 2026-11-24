/**
 * @file tcpforwarder.h
 * @brief TCP 转发器（4字节长度帧头 + 断线重连）
 * @date 2026-11-24
 */

#ifndef TCPFORWARDER_H
#define TCPFORWARDER_H

#include "idataforwarder.h"
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

class TcpForwarder : public QObject, public IDataForwarder
{
    Q_OBJECT

public:
    TcpForwarder();
    ~TcpForwarder() override;

    bool open(const QJsonObject &cfg) override;
    bool send(const QByteArray &payload) override;
    void close() override;
    QString lastError() const override { return m_lastError; }
    bool isReady() const override { return m_socket.state() == QTcpSocket::ConnectedState; }

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError err);
    void onReconnectTimer();

private:
    void connectToServer();

    QTcpSocket m_socket;
    QTimer m_reconnectTimer;
    QHostAddress m_target;
    quint16 m_port = 0;
    int m_reconnectMs = 3000;
    QString m_lastError;
    bool m_manualClosed = false;
};

#endif // TCPFORWARDER_H
