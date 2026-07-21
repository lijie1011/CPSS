#include "udpadapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "logger.h"

UdpAdapter::UdpAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_socket(nullptr),
      m_status(Stopped),
      m_localPort(12345)
{
}

UdpAdapter::~UdpAdapter()
{
    stop();
}

bool UdpAdapter::start()
{
    if (m_status == Running)
        return true;

    m_status = Starting;
    emit statusChanged(m_status);

    m_socket = new QUdpSocket(this);

    if (!m_socket->bind(QHostAddress::AnyIPv4, m_localPort, QUdpSocket::ShareAddress)) {
        m_lastError = m_socket->errorString();
        m_status = Error;
        emit statusChanged(m_status);
        emit errorOccurred(m_lastError);
        Logger::error("UDP Adapter bind failed: %s", m_lastError.toStdString().c_str());
        return false;
    }

    if (!m_multicastGroup.isEmpty()) {
        if (!m_socket->joinMulticastGroup(QHostAddress(m_multicastGroup))) {
            m_lastError = m_socket->errorString();
            Logger::warn("UDP Adapter failed to join multicast group: %s", m_lastError.toStdString().c_str());
        }
    }

    connect(m_socket, &QUdpSocket::readyRead, this, &UdpAdapter::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::error),
            this, &UdpAdapter::onSocketError);

    m_status = Running;
    emit statusChanged(m_status);
    Logger::info("UDP Adapter started on port %d", m_localPort);
    return true;
}

bool UdpAdapter::stop()
{
    if (m_status == Stopped)
        return true;

    if (m_socket) {
        m_socket->close();
        m_socket->disconnect(this);
        delete m_socket;
        m_socket = nullptr;
    }

    m_status = Stopped;
    emit statusChanged(m_status);
    Logger::info("UDP Adapter stopped");
    return true;
}

void UdpAdapter::setLocalPort(quint16 port)
{
    if (m_status == Stopped) {
        m_localPort = port;
    }
}

void UdpAdapter::setMulticastGroup(const QString &group)
{
    m_multicastGroup = group;
}

void UdpAdapter::onReadyRead()
{
    while (m_socket && m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            Logger::warn("UDP Adapter invalid JSON: %s", parseError.errorString().toStdString().c_str());
            continue;
        }

        if (doc.isObject()) {
            emit dataReceived(doc.object(), protocolType());
        } else if (doc.isArray()) {
            QJsonObject batchObj;
            batchObj["type"] = "batch";
            batchObj["items"] = doc.array();
            emit dataReceived(batchObj, protocolType());
        }
    }
}

void UdpAdapter::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_lastError = m_socket->errorString();
    m_status = Error;
    emit statusChanged(m_status);
    emit errorOccurred(m_lastError);
    Logger::error("UDP Adapter error: %s", m_lastError.toStdString().c_str());
}
