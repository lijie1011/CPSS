#include "webserver.h"
#include "chartbridge.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "common/logger.h"

WebServer::WebServer(quint16 port, QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer("CPSS WebServer",
              QWebSocketServer::NonSecureMode, this)),
      m_bridge(new ChartBridge(this))
{
    if (m_server->listen(QHostAddress::Any, port)) {
        Logger::info("WebServer listening on port %d", port);
        connect(m_server, &QWebSocketServer::newConnection,
                this, &WebServer::onNewConnection);
    } else {
        Logger::error("WebServer failed to listen on port %d", port);
    }
}

WebServer::~WebServer()
{
    m_server->close();
    qDeleteAll(m_clients);
}

void WebServer::setViewWidget(ViewWidget *view)
{
    m_bridge->setViewWidget(view);
}

void WebServer::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QWebSocket::textMessageReceived,
            this, &WebServer::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
            this, &WebServer::onSocketDisconnected);
    m_clients << socket;
    Logger::info("New client connected, total: %d", m_clients.size());
}

void WebServer::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
        Logger::info("Client disconnected, total: %d", m_clients.size());
    }
}

void WebServer::onTextMessageReceived(const QString &message)
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        Logger::error("Invalid JSON message: %s", error.errorString().toStdString().c_str());
        return;
    }

    QJsonObject req = doc.object();
    QString action = req.value("action").toString();
    int reqId = req.value("id").toInt();

    QJsonObject resp = m_bridge->handleRequest(action, req.value("data").toObject());

    QJsonObject reply;
    reply["id"] = reqId;
    reply["data"] = resp;
    reply["error"] = resp.contains("error") ? resp["error"] : QJsonValue();

    QString jsonStr = QString::fromUtf8(
        QJsonDocument(reply).toJson(QJsonDocument::Compact));
    socket->sendTextMessage(jsonStr);
}

void WebServer::broadcastDynamicUpdate(const QJsonObject &data)
{
    QJsonObject msg;
    msg["type"] = "dynamicUpdate";
    msg["data"] = data;

    QString jsonStr = QString::fromUtf8(
        QJsonDocument(msg).toJson(QJsonDocument::Compact));

    for (QWebSocket *client : m_clients) {
        client->sendTextMessage(jsonStr);
    }
}
