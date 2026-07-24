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
            parseAndUpdate(doc.object(), protocolType());
        } else if (doc.isArray()) {
            QJsonObject batchObj;
            batchObj["type"] = "batch";
            batchObj["items"] = doc.array();
            parseAndUpdate(batchObj, protocolType());
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

void HttpAdapter::parseAndUpdate(const QJsonObject &data, ProtocolType source)
{
    QString type = data["type"].toString();

    if (type == "platform") {
        PlatformData platform = parsePlatform(data, source);
        if (!platform.id.isEmpty()) {
            emit dataReceived(data, source);
        }
    } else if (type == "event") {
        SpecialEvent event = parseEvent(data);
        if (!event.eventId.isEmpty()) {
            emit dataReceived(data, source);
        }
    } else if (type == "batch") {
        QJsonArray items = data["items"].toArray();
        for (const auto &item : items) {
            if (item.isObject()) {
                parseAndUpdate(item.toObject(), source);
            }
        }
    }
}

PlatformData HttpAdapter::parsePlatform(const QJsonObject &obj, ProtocolType source)
{
    PlatformData platform;
    platform.id = obj["id"].toString();
    platform.name = obj["name"].toString();
    platform.lon = obj["lon"].toDouble();
    platform.lat = obj["lat"].toDouble();
    platform.altitude = obj["altitude"].toDouble();
    platform.speed = obj["speed"].toDouble();
    platform.type = obj["platformType"].toString();
    if (platform.type.isEmpty()) {
        platform.type = obj["type"].toString();
    }
    platform.category = obj["category"].toString();
    
    QString campStr = obj["camp"].toString().toLower();
    if (campStr == "friendly") {
        platform.camp = Camp_Friendly;
    } else if (campStr == "enemy") {
        platform.camp = Camp_Enemy;
    } else if (campStr == "neutral") {
        platform.camp = Camp_Neutral;
    } else {
        platform.camp = Camp_Unknown;
    }

    QJsonArray weaponsArray = obj["weapons"].toArray();
    for (const auto &weapon : weaponsArray) {
        QJsonObject weaponObj = weapon.toObject();
        WeaponInfo wi;
        wi.type = weaponObj["type"].toString();
        wi.count = weaponObj["count"].toInt();
        platform.weapons.append(wi);
    }

    QJsonArray sensorsArray = obj["sensors"].toArray();
    for (const auto &sensor : sensorsArray) {
        QJsonObject sensorObj = sensor.toObject();
        SensorInfo si;
        si.type = sensorObj["type"].toString();
        si.count = sensorObj["count"].toInt();
        platform.sensors.append(si);
    }

    platform.dataStatus = DataStatus_Normal;
    platform.updateTime = QDateTime::currentMSecsSinceEpoch();
    platform.validUntil = platform.updateTime + (obj.contains("validDuration") ? obj["validDuration"].toInt() : m_validDuration);
    platform.sourceProtocol = source;

    return platform;
}

SpecialEvent HttpAdapter::parseEvent(const QJsonObject &obj)
{
    SpecialEvent event;
    event.eventId = obj["eventId"].toString();
    
    QString eventTypeStr = obj["eventType"].toString().toLower();
    if (eventTypeStr == "attack") {
        event.eventType = Event_Attack;
    } else if (eventTypeStr == "defense") {
        event.eventType = Event_Defense;
    } else if (eventTypeStr == "alert") {
        event.eventType = Event_Alert;
    } else if (eventTypeStr == "missionstart") {
        event.eventType = Event_MissionStart;
    } else if (eventTypeStr == "missionend") {
        event.eventType = Event_MissionEnd;
    } else if (eventTypeStr == "contact") {
        event.eventType = Event_Contact;
    } else if (eventTypeStr == "lost") {
        event.eventType = Event_Lost;
    } else if (eventTypeStr == "damage") {
        event.eventType = Event_Damage;
    } else if (eventTypeStr == "repair") {
        event.eventType = Event_Repair;
    } else if (eventTypeStr == "custom") {
        event.eventType = Event_Custom;
    } else {
        event.eventType = Event_Unknown;
    }
    
    event.eventName = obj["eventName"].toString();
    event.description = obj["description"].toString();
    event.timestamp = static_cast<qint64>(obj["timestamp"].toDouble());
    if (event.timestamp == 0) {
        event.timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    event.targetId = obj["targetId"].toString();
    event.sourceId = obj["sourceId"].toString();
    
    if (obj.contains("extraData") && obj["extraData"].isObject()) {
        event.extraData = obj["extraData"].toObject();
    }

    return event;
}