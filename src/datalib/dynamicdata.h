#ifndef DYNAMICDATA_H
#define DYNAMICDATA_H

#include <QString>
#include <QList>
#include <QDateTime>

enum DataStatus {
    DataStatus_Normal,
    DataStatus_Expired,
    DataStatus_Invalid
};

enum ProtocolType {
    Protocol_Unknown,
    Protocol_DDS,
    Protocol_Redis,
    Protocol_UDP,
    Protocol_TCP,
    Protocol_WebSocket,
    Protocol_HTTP
};

struct ShipData {
    QString mmsi;
    QString name;
    double lon;
    double lat;
    double heading;
    double speed;
    bool isOwnShip;
    bool visible;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }
};

struct AisTarget {
    QString mmsi;
    QString name;
    double lon;
    double lat;
    double heading;
    double speed;
    int shipType;
    bool isDanger;
    bool visible;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }
};

struct WeaponData {
    QString id;
    double lon;
    double lat;
    double targetLon;
    double targetLat;
    QString type;
    bool active;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }
};

struct SensorData {
    QString id;
    double lon;
    double lat;
    double radius;
    double azimuth;
    double angle;
    QString type;
    bool active;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }
};

struct UserMarker {
    QString id;
    double lon;
    double lat;
    QString label;
    QString color;

    DataStatus dataStatus;
    qint64 validUntil;
    qint64 updateTime;
    ProtocolType sourceProtocol;

    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }
};

struct DynamicObjects {
    ShipData ownShip;
    QList<AisTarget> aisTargets;
    QList<WeaponData> weapons;
    QList<SensorData> sensors;
    QList<UserMarker> markers;
    qint64 timestamp;
};

#endif
