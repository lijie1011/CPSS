#ifndef DYNAMICDATA_H
#define DYNAMICDATA_H

#ifdef CPSS_DATA_EXPORT
#   define CPSS_DATA_API __declspec(dllexport)
#else
#   define CPSS_DATA_API __declspec(dllimport)
#endif

#include <QString>
#include <QList>
#include <QDateTime>
#include <QMap>
#include <QJsonObject>
#include <QPointF>

enum DataStatus {
    DataStatus_Normal,
    DataStatus_Expired,
    DataStatus_Invalid
};

enum ProtocolType {
    Protocol_Unknown,
    Protocol_HTTP
};

enum CampType {
    Camp_Unknown,
    Camp_Friendly,
    Camp_Enemy,
    Camp_Neutral
};

enum SpecialEventType {
    Event_Unknown,
    Event_Attack,
    Event_Defense,
    Event_Alert,
    Event_MissionStart,
    Event_MissionEnd,
    Event_Contact,
    Event_Lost,
    Event_Damage,
    Event_Repair,
    Event_Custom
};

struct WeaponInfo {
    QString type;
    int count;
    double range;
};

struct SensorInfo {
    QString type;
    int count;
    double range;
};

struct PlatformData {
    QString id;
    QString name;
    double lon;
    double lat;
    double altitude;
    double speed;
    QString type;
    QString category;
    CampType camp;
    QList<WeaponInfo> weapons;
    QList<SensorInfo> sensors;

    QList<QPointF> trackPoints;
    static const int MAX_TRACK_POINTS = 1000;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }

    void addTrackPoint(double lon, double lat) {
        trackPoints.append(QPointF(lon, lat));
        if (trackPoints.size() > MAX_TRACK_POINTS) {
            trackPoints.removeFirst();
        }
    }
};

struct SpecialEvent {
    QString eventId;
    SpecialEventType eventType;
    QString eventName;
    QString description;
    qint64 timestamp;
    QString targetId;
    QString sourceId;
    QJsonObject extraData;
    double lon;
    double lat;
};

struct CPSS_DATA_API DynamicObjects {
    QMap<QString, PlatformData> platforms;
    QList<SpecialEvent> events;
    qint64 timestamp;
};

#endif