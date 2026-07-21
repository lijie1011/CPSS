#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QJsonObject>

class ViewWidget;
class ChartBridge;

class WebServer : public QObject
{
    Q_OBJECT

public:
    explicit WebServer(quint16 port, QObject *parent = nullptr);
    ~WebServer();

    void setViewWidget(ViewWidget *view);

private slots:
    void onNewConnection();
    void onSocketDisconnected();
    void onTextMessageReceived(const QString &message);

private:
    void broadcastDynamicUpdate(const QJsonObject &data);

    QWebSocketServer *m_server;
    QList<QWebSocket*> m_clients;
    ChartBridge *m_bridge;
};

#endif
