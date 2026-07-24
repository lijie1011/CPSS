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
};

struct SensorInfo {
    QString type;
    int count;
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

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
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
};

struct CPSS_DATA_API DynamicObjects {
    QMap<QString, PlatformData> platforms;
    QList<SpecialEvent> events;
    qint64 timestamp;
};

#endif