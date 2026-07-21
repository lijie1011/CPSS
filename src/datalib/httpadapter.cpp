#include "httpadapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include "logger.h"

HttpAdapter::HttpAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_manager(nullptr),
      m_status(Stopped),
      m_requestInterval(5000),
      m_requestType("GET")
{
}

HttpAdapter::~HttpAdapter()
{
    stop();
}

bool HttpAdapter::start()
{
    if (m_status == Running)
        return true;

    m_status = Starting;
    emit statusChanged(m_status);

    m_manager = new QNetworkAccessManager(this);

    connect(&m_requestTimer, &QTimer::timeout, this, &HttpAdapter::onRequestTimeout);

    if (!m_requestUrl.isEmpty()) {
        sendRequest();

        if (m_requestInterval > 0) {
            m_requestTimer.start(m_requestInterval);
        }
    }

    m_status = Running;
    emit statusChanged(m_status);
    Logger::info("HTTP Adapter started, url: %s, interval: %dms",
                 m_requestUrl.toStdString().c_str(), m_requestInterval);
    return true;
}

bool HttpAdapter::stop()
{
    if (m_status == Stopped)
        return true;

    m_requestTimer.stop();

    if (m_manager) {
        m_manager->disconnect(this);
        delete m_manager;
        m_manager = nullptr;
    }

    m_status = Stopped;
    emit statusChanged(m_status);
    Logger::info("HTTP Adapter stopped");
    return true;
}

void HttpAdapter::setRequestUrl(const QString &url)
{
    m_requestUrl = url;
}

void HttpAdapter::setRequestInterval(int ms)
{
    m_requestInterval = ms;
}

void HttpAdapter::setRequestType(const QString &type)
{
    m_requestType = type;
}

void HttpAdapter::setRequestData(const QByteArray &data)
{
    m_requestData = data;
}

void HttpAdapter::onRequestTimeout()
{
    sendRequest();
}

void HttpAdapter::sendRequest()
{
    if (!m_manager || m_requestUrl.isEmpty())
        return;

    QUrl url(m_requestUrl);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = nullptr;

    if (m_requestType.toUpper() == "POST") {
        reply = m_manager->post(req, m_requestData);
    } else if (m_requestType.toUpper() == "PUT") {
        reply = m_manager->put(req, m_requestData);
    } else {
        reply = m_manager->get(req);
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &HttpAdapter::onReplyError);
}

void HttpAdapter::onReplyFinished(QNetworkReply *reply)
{
    if (!reply)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            Logger::warn("HTTP Adapter invalid JSON: %s", parseError.errorString().toStdString().c_str());
        } else if (doc.isObject()) {
            emit dataReceived(doc.object(), protocolType());
        } else if (doc.isArray()) {
            QJsonObject batchObj;
            batchObj["type"] = "batch";
            batchObj["items"] = doc.array();
            emit dataReceived(batchObj, protocolType());
        }
    }

    reply->deleteLater();
}

void HttpAdapter::onReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        m_lastError = reply->errorString();
        Logger::error("HTTP Adapter error: %s", m_lastError.toStdString().c_str());
        reply->deleteLater();
    }
}
