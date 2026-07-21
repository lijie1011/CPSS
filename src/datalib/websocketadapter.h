#ifndef WEBSOCKETADAPTER_H
#define WEBSOCKETADAPTER_H

#include "protocoladapter.h"
#include <QWebSocket>
#include <QUrl>

class WebSocketAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit WebSocketAdapter(QObject *parent = nullptr);
    ~WebSocketAdapter();

    ProtocolType protocolType() const override { return Protocol_WebSocket; }
    QString adapterName() const override { return "WebSocket Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setServerUrl(const QString &url);
    QString serverUrl() const { return m_serverUrl; }

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket *m_socket;
    AdapterStatus m_status;
    QString m_lastError;
    QString m_serverUrl;
};

#endif
