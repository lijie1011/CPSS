#ifndef HTTPADAPTER_H
#define HTTPADAPTER_H

#include "protocoladapter.h"
#include "dynamicdata.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

class HttpAdapter : public IProtocolAdapter
{
    Q_OBJECT

public:
    explicit HttpAdapter(QObject *parent = nullptr);
    ~HttpAdapter();

    ProtocolType protocolType() const override { return Protocol_HTTP; }
    QString adapterName() const override { return "HTTP Adapter"; }

    bool start() override;
    bool stop() override;
    AdapterStatus status() const override { return m_status; }
    QString lastError() const override { return m_lastError; }

    void setRequestUrl(const QString &url);
    QString requestUrl() const { return m_requestUrl; }

    void setRequestInterval(int ms);
    int requestInterval() const { return m_requestInterval; }

    void setRequestType(const QString &type);
    QString requestType() const { return m_requestType; }

    void setRequestData(const QByteArray &data);
    QByteArray requestData() const { return m_requestData; }

private slots:
    void onRequestTimeout();
    void onReplyFinished(QNetworkReply *reply);
    void onReplyError(QNetworkReply::NetworkError error);

private:
    void sendRequest();
    void parseAndUpdate(const QJsonObject &data, ProtocolType source);
    PlatformData parsePlatform(const QJsonObject &obj, ProtocolType source);
    SpecialEvent parseEvent(const QJsonObject &obj);

    QNetworkAccessManager *m_manager;
    AdapterStatus m_status;
    QString m_lastError;
    QString m_requestUrl;
    int m_requestInterval;
    QString m_requestType;
    QByteArray m_requestData;
    QTimer m_requestTimer;
};

#endif