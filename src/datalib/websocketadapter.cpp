#include "websocketadapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "logger.h"

WebSocketAdapter::WebSocketAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_socket(nullptr),
      m_status(Stopped)
{
}

WebSocketAdapter::~WebSocketAdapter()
{
    stop();
}

bool WebSocketAdapter::start()
{
    if (m_status == Running)
        return true;

    m_status = Starting;
    emit statusChanged(m_status);

    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_socket, &QWebSocket::connected, this, &WebSocketAdapter::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketAdapter::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketAdapter::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketAdapter::onBinaryMessageReceived);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &WebSocketAdapter::onError);

    if (!m_serverUrl.isEmpty()) {
        m_socket->open(QUrl(m_serverUrl));
        Logger::info("WebSocket Adapter connecting to %s", m_serverUrl.toStdString().c_str());
    }

    return true;
}

bool WebSocketAdapter::stop()
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
    Logger::info("WebSocket Adapter stopped");
    return true;
}

void WebSocketAdapter::setServerUrl(const QString &url)
{
    m_serverUrl = url;
}

void WebSocketAdapter::onConnected()
{
    m_status = Running;
    emit statusChanged(m_status);
    Logger::info("WebSocket Adapter connected");
}

void WebSocketAdapter::onDisconnected()
{
    m_status = Stopped;
    emit statusChanged(m_status);
    Logger::info("WebSocket Adapter disconnected");
}

void WebSocketAdapter::onTextMessageReceived(const QString &message)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        Logger::warn("WebSocket Adapter invalid JSON: %s", parseError.errorString().toStdString().c_str());
        return;
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

void WebSocketAdapter::onBinaryMessageReceived(const QByteArray &message)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        Logger::warn("WebSocket Adapter invalid binary JSON: %s", parseError.errorString().toStdString().c_str());
        return;
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

void WebSocketAdapter::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    m_lastError = m_socket->errorString();
    m_status = Error;
    emit statusChanged(m_status);
    emit errorOccurred(m_lastError);
    Logger::error("WebSocket Adapter error: %s", m_lastError.toStdString().c_str());
}
