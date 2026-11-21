/**
 * @file httpadapter.cpp
 * @brief HTTP协议适配器实现
 * @details 负责通过HTTP协议从服务器获取数据，解析JSON格式的平台和事件数据，
 *          并通过信号将解析结果发送给数据管理器。支持GET、POST、PUT等请求方法。
 * @date 2026-07-28
 */

#include "httpadapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include "logger.h"

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
HttpAdapter::HttpAdapter(QObject *parent)
    : IProtocolAdapter(parent),
      m_manager(nullptr),
      m_status(Stopped),
      m_requestInterval(5000),
      m_requestType("GET")
{
}

/**
 * @brief 析构函数
 */
HttpAdapter::~HttpAdapter()
{
    stop();
}

/**
 * @brief 启动HTTP适配器
 * @return true 表示启动成功，false 表示失败
 */
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
    return true;
}

/**
 * @brief 停止HTTP适配器
 * @return true 表示停止成功，false 表示失败
 */
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
    return true;
}

/**
 * @brief 设置请求URL
 * @param url HTTP请求的目标URL
 */
void HttpAdapter::setRequestUrl(const QString &url)
{
    m_requestUrl = url;
}

/**
 * @brief 设置请求间隔时间
 * @param ms 间隔时间（毫秒）
 */
void HttpAdapter::setRequestInterval(int ms)
{
    m_requestInterval = ms;
}

/**
 * @brief 设置请求类型
 * @param type 请求类型（GET/POST/PUT）
 */
void HttpAdapter::setRequestType(const QString &type)
{
    m_requestType = type;
}

/**
 * @brief 设置请求数据
 * @param data POST/PUT请求时发送的数据
 */
void HttpAdapter::setRequestData(const QByteArray &data)
{
    m_requestData = data;
}

/**
 * @brief 请求超时处理函数
 */
void HttpAdapter::onRequestTimeout()
{
    sendRequest();
}

/**
 * @brief 发送HTTP请求
 */
void HttpAdapter::sendRequest()
{
    if (!m_manager || m_requestUrl.isEmpty())
        return;

    QUrl url(m_requestUrl);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = nullptr;

    // 根据请求类型选择对应的HTTP方法
    if (m_requestType.toUpper() == "POST") {
        reply = m_manager->post(req, m_requestData);
    } else if (m_requestType.toUpper() == "PUT") {
        reply = m_manager->put(req, m_requestData);
    } else {
        reply = m_manager->get(req);
    }

    // 连接完成和错误信号
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &HttpAdapter::onReplyError);
}

/**
 * @brief 处理HTTP响应完成
 * @param reply 网络响应对象
 */
void HttpAdapter::onReplyFinished(QNetworkReply *reply)
{
    if (!reply)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
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

/**
 * @brief 处理HTTP响应错误
 * @param error 网络错误类型
 */
void HttpAdapter::onReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        m_lastError = reply->errorString();
        reply->deleteLater();
    }
}

/**
 * @brief 解析并更新数据
 * @param data JSON格式数据对象
 * @param source 数据来源协议类型
 */
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

/**
 * @brief 解析平台数据
 * @param obj JSON格式平台对象
 * @param source 数据来源协议类型
 * @return 解析后的PlatformData对象
 */
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
    
    // 阵营字符串转枚举
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

    // 解析武器列表
    QJsonArray weaponsArray = obj["weapons"].toArray();
    for (const auto &weapon : weaponsArray) {
        QJsonObject weaponObj = weapon.toObject();
        WeaponInfo wi;
        wi.type = weaponObj["type"].toString();
        wi.count = weaponObj["count"].toInt();
        platform.weapons.append(wi);
    }

    // 解析传感器列表
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

/**
 * @brief 解析事件数据
 * @param obj JSON格式事件对象
 * @return 解析后的SpecialEvent对象
 */
SpecialEvent HttpAdapter::parseEvent(const QJsonObject &obj)
{
    SpecialEvent event;
    event.eventId = obj["eventId"].toString();
    
    // 事件类型字符串转枚举
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
    event.lon = obj["lon"].toDouble();
    event.lat = obj["lat"].toDouble();
    
    if (obj.contains("extraData") && obj["extraData"].isObject()) {
        event.extraData = obj["extraData"].toObject();
    }

    return event;
}
