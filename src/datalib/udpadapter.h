#ifndef UDPADAPTER_H
#define UDPADAPTER_H

#include "protocoladapter.h"
#include <QUdpSocket>
#include <QHostAddress>

class UdpAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit UdpAdapter(QObject *parent = nullptr);
    ~UdpAdapter();

    ProtocolType protocolType() const override { return Protocol_UDP; }
    QString adapterName() const override { return "UDP Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setLocalPort(quint16 port);
    quint16 localPort() const { return m_localPort; }

    void setMulticastGroup(const QString &group);
    QString multicastGroup() const { return m_multicastGroup; }

private slots:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    QUdpSocket *m_socket;
    AdapterStatus m_status;
    QString m_lastError;
    quint16 m_localPort;
    QString m_multicastGroup;
};

#endif
