#ifndef TCPADAPTER_H
#define TCPADAPTER_H

#include "protocoladapter.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QList>

class TcpAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    enum Mode {
        ClientMode,
        ServerMode
    };

    explicit TcpAdapter(QObject *parent = nullptr);
    ~TcpAdapter();

    ProtocolType protocolType() const override { return Protocol_TCP; }
    QString adapterName() const override { return "TCP Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setMode(Mode mode);
    Mode mode() const { return m_mode; }

    void setServerPort(quint16 port);
    quint16 serverPort() const { return m_serverPort; }

    void setServerAddress(const QString &address);
    QString serverAddress() const { return m_serverAddress; }

    void setClientAddress(const QString &address);
    QString clientAddress() const { return m_clientAddress; }

    void setClientPort(quint16 port);
    quint16 clientPort() const { return m_clientPort; }

private slots:
    void onServerNewConnection();
    void onClientConnected();
    void onClientDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    Mode m_mode;
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;
    QList<QTcpSocket*> m_clientSockets;
    AdapterStatus m_status;
    QString m_lastError;
    quint16 m_serverPort;
    QString m_serverAddress;
    QString m_clientAddress;
    quint16 m_clientPort;
};

#endif
