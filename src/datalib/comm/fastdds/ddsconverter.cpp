/**
 * @file ddsconverter.cpp
 * @brief DDS 数据转换工具实现
 * @date 2026-11-22
 */

#include "ddsconverter.h"
#include <QJsonDocument>
#include <QJsonArray>

namespace DdsConverter {

QJsonObject platformToJson(const PlatformData &p)
{
    QJsonObject obj;
    obj["type"] = QStringLiteral("platform");
    obj["id"] = p.id;
    obj["name"] = p.name;
    obj["lon"] = p.lon;
    obj["lat"] = p.lat;
    obj["altitude"] = p.altitude;
    obj["speed"] = p.speed;
    obj["heading"] = p.heading;
    obj["pitch"] = p.pitch;
    obj["roll"] = p.roll;
    obj["platformType"] = p.type;
    obj["category"] = p.category;

    switch (p.camp) {
    case Camp_Mine:     obj["camp"] = QStringLiteral("mine"); break;
    case Camp_Friendly: obj["camp"] = QStringLiteral("friendly"); break;
    case Camp_Enemy:    obj["camp"] = QStringLiteral("enemy"); break;
    case Camp_Neutral:  obj["camp"] = QStringLiteral("neutral"); break;
    default:            obj["camp"] = QStringLiteral("unknown"); break;
    }
    if (!p.targetId.isEmpty()) obj["targetId"] = p.targetId;

    QJsonArray weapons;
    for (const WeaponInfo &w : p.weapons) {
        QJsonObject jo;
        jo["type"] = w.type;
        jo["count"] = w.count;
        jo["range"] = w.range;
        weapons.append(jo);
    }
    obj["weapons"] = weapons;

    QJsonArray sensors;
    for (const SensorInfo &s : p.sensors) {
        QJsonObject jo;
        jo["type"] = s.type;
        jo["count"] = s.count;
        jo["range"] = s.range;
        sensors.append(jo);
    }
    obj["sensors"] = sensors;

    QJsonArray track;
    for (const QPointF &pt : p.trackPoints) {
        QJsonArray xy;
        xy.append(pt.x());
        xy.append(pt.y());
        track.append(xy);
    }
    obj["trackPoints"] = track;

    obj["dataStatus"] = static_cast<int>(p.dataStatus);
    obj["validUntil"] = p.validUntil;
    obj["updateTime"] = p.updateTime;
    obj["sourceProtocol"] = static_cast<int>(p.sourceProtocol);
    return obj;
}

PlatformData jsonToPlatform(const QJsonObject &obj)
{
    PlatformData p;
    p.id = obj["id"].toString();
    p.name = obj["name"].toString();
    p.lon = obj["lon"].toDouble();
    p.lat = obj["lat"].toDouble();
    p.altitude = obj["altitude"].toDouble(0.0);
    p.speed = obj["speed"].toDouble(0.0);
    p.heading = obj["heading"].toDouble(0.0);
    p.pitch = obj["pitch"].toDouble(0.0);
    p.roll = obj["roll"].toDouble(0.0);
    p.type = obj["platformType"].toString();
    if (p.type.isEmpty()) p.type = obj["type"].toString();
    p.category = obj["category"].toString();

    const QString camp = obj["camp"].toString().toLower();
    if (camp == "mine" || camp == "self" || camp == "own")       p.camp = Camp_Mine;
    else if (camp == "friendly" || camp == "friend" || camp == "ally") p.camp = Camp_Friendly;
    else if (camp == "enemy" || camp == "hostile" || camp == "red")    p.camp = Camp_Enemy;
    else if (camp == "neutral")                                   p.camp = Camp_Neutral;
    else                                                          p.camp = Camp_Unknown;

    p.targetId = obj["targetId"].toString();

    for (const QJsonValue &v : obj["weapons"].toArray()) {
        QJsonObject jo = v.toObject();
        p.weapons.append(WeaponInfo{ jo["type"].toString(),
                                     jo["count"].toInt(),
                                     jo["range"].toDouble() });
    }
    for (const QJsonValue &v : obj["sensors"].toArray()) {
        QJsonObject jo = v.toObject();
        p.sensors.append(SensorInfo{ jo["type"].toString(),
                                     jo["count"].toInt(),
                                     jo["range"].toDouble() });
    }
    for (const QJsonValue &v : obj["trackPoints"].toArray()) {
        QJsonArray xy = v.toArray();
        if (xy.size() >= 2) p.trackPoints.append(QPointF(xy[0].toDouble(), xy[1].toDouble()));
    }

    p.dataStatus = static_cast<DataStatus>(obj["dataStatus"].toInt(DataStatus_Normal));
    p.validUntil = static_cast<qint64>(obj["validUntil"].toDouble());
    p.updateTime = static_cast<qint64>(obj["updateTime"].toDouble());
    // 从 JSON 还原原始来源（适配器的原始数据），而非硬编码 FastDds
    p.sourceProtocol = static_cast<ProtocolType>(obj["sourceProtocol"].toInt(Protocol_FastDds));
    return p;
}

QJsonObject eventToJson(const SpecialEvent &e)
{
    QJsonObject obj;
    obj["type"] = QStringLiteral("event");
    obj["eventId"] = e.eventId;
    obj["eventType"] = static_cast<int>(e.eventType);
    obj["eventName"] = e.eventName;
    obj["description"] = e.description;
    obj["timestamp"] = e.timestamp;
    obj["targetId"] = e.targetId;
    obj["sourceId"] = e.sourceId;
    obj["lon"] = e.lon;
    obj["lat"] = e.lat;
    obj["extraData"] = QString::fromUtf8(QJsonDocument(e.extraData).toJson(QJsonDocument::Compact));
    return obj;
}

SpecialEvent jsonToEvent(const QJsonObject &obj)
{
    SpecialEvent e;
    e.eventId = obj["eventId"].toString();
    e.eventType = static_cast<SpecialEventType>(obj["eventType"].toInt(Event_Unknown));
    e.eventName = obj["eventName"].toString();
    e.description = obj["description"].toString();
    e.timestamp = static_cast<qint64>(obj["timestamp"].toDouble());
    e.targetId = obj["targetId"].toString();
    e.sourceId = obj["sourceId"].toString();
    e.lon = obj["lon"].toDouble(0.0);
    e.lat = obj["lat"].toDouble(0.0);

    QString extra = obj["extraData"].toString();
    if (!extra.isEmpty()) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(extra.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) e.extraData = doc.object();
    }
    return e;
}

QJsonObject wrapPlatformMessage(const PlatformData &platform)
{
    return platformToJson(platform);
}

QJsonObject wrapEventMessage(const SpecialEvent &event)
{
    return eventToJson(event);
}

} // namespace DdsConverter
