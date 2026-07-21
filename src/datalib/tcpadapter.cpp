#include "tcpadapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "logger.h"

TcpAdapter::TcpAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_mode(ServerMode),
      m_server(nullptr),
      m_clientSocket(nullptr),
      m_status(Stopped),
      m_serverPort(12346),
      m_clientPort(12346)
{
}

TcpAdapter::~TcpAdapter()
{
    stop();
}

bool TcpAdapter::start()
{
    if (m_status == Running)
        return true;

    m_status = Starting;
    emit statusChanged(m_status);

    if (m_mode == ServerMode) {
        m_server = new QTcpServer(this);

        if (!m_server->listen(QHostAddress(m_serverAddress), m_serverPort)) {
            m_lastError = m_server->errorString();
            m_status = Error;
            emit statusChanged(m_status);
            emit errorOccurred(m_lastError);
            Logger::error("TCP Adapter server listen failed: %s", m_lastError.toStdString().c_str());
            return false;
        }

        connect(m_server, &QTcpServer::newConnection, this, &TcpAdapter::onServerNewConnection);
        m_status = Running;
        emit statusChanged(m_status);
        Logger::info("TCP Adapter server started on port %d", m_serverPort);
        return true;
    } else {
        m_clientSocket = new QTcpSocket(this);

        connect(m_clientSocket, &QTcpSocket::connected, this, &TcpAdapter::onClientConnected);
        connect(m_clientSocket, &QTcpSocket::disconnected, this, &TcpAdapter::onClientDisconnected);
        connect(m_clientSocket, &QTcpSocket::readyRead, this, &TcpAdapter::onReadyRead);
        connect(m_clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                this, &TcpAdapter::onSocketError);

        m_clientSocket->connectToHost(m_clientAddress, m_clientPort);
        Logger::info("TCP Adapter client connecting to %s:%d", m_clientAddress.toStdString().c_str(), m_clientPort);
        return true;
    }
}

bool TcpAdapter::stop()
{
    if (m_status == Stopped)
        return true;

    if (m_server) {
        m_server->close();
        m_server->disconnect(this);
        delete m_server;
        m_server = nullptr;
    }

    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->disconnect(this);
        delete m_clientSocket;
        m_clientSocket = nullptr;
    }

    for (auto socket : m_clientSockets) {
        socket->close();
        socket->disconnect(this);
        delete socket;
    }
    m_clientSockets.clear();

    m_status = Stopped;
    emit statusChanged(m_status);
    Logger::info("TCP Adapter stopped");
    return true;
}

void TcpAdapter::setMode(Mode mode)
{
    if (m_status == Stopped) {
        m_mode = mode;
    }
}

void TcpAdapter::setServerPort(quint16 port)
{
    if (m_status == Stopped) {
        m_serverPort = port;
    }
}

void TcpAdapter::setServerAddress(const QString &address)
{
    m_serverAddress = address;
}

void TcpAdapter::setClientAddress(const QString &address)
{
    m_clientAddress = address;
}

void TcpAdapter::setClientPort(quint16 port)
{
    if (m_status == Stopped) {
        m_clientPort = port;
    }
}

void TcpAdapter::onServerNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, &QTcpSocket::disconnected, this, &TcpAdapter::onClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpAdapter::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &TcpAdapter::onSocketError);

    m_clientSockets.append(socket);
    Logger::info("TCP Adapter new client connected: %s",
                 socket->peerAddress().toString().toStdString().c_str());
}

void TcpAdapter::onClientConnected()
{
    m_status = Running;
    emit statusChanged(m_status);
    Logger::info("TCP Adapter client connected to %s:%d",
                 m_clientAddress.toStdString().c_str(), m_clientPort);
}

void TcpAdapter::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->disconnect(this);
        m_clientSockets.removeOne(socket);
        delete socket;

        if (m_clientSockets.isEmpty() && m_mode == ServerMode) {
            Logger::info("TCP Adapter all clients disconnected");
        } else if (m_mode == ClientMode) {
            m_status = Stopped;
            emit statusChanged(m_status);
            Logger::info("TCP Adapter client disconnected");
        }
    }
}

void TcpAdapter::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            Logger::warn("TCP Adapter invalid JSON: %s", parseError.errorString().toStdString().c_str());
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

void TcpAdapter::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_lastError = socket->errorString();
        Logger::error("TCP Adapter socket error: %s", m_lastError.toStdString().c_str());

        if (m_mode == ClientMode) {
            m_status = Error;
            emit statusChanged(m_status);
            emit errorOccurred(m_lastError);
        }
    }
}
