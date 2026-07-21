#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include "dynamicdata.h"
#include "protocoladapter.h"

class DataManager : public QObject
{
    Q_OBJECT

public:
    static DataManager* instance();

    void addAdapter(IProtocolAdapter *adapter);
    void removeAdapter(IProtocolAdapter *adapter);
    void startAllAdapters();
    void stopAllAdapters();

    void setDataSourcePriority(ProtocolType type, int priority);
    int getDataSourcePriority(ProtocolType type) const;

    void setDefaultValidDuration(qint64 ms);
    qint64 defaultValidDuration() const;

    ShipData getOwnShip();
    QList<AisTarget> getAisTargets();
    QList<AisTarget> getValidAisTargets();
    QList<WeaponData> getWeapons();
    QList<WeaponData> getValidWeapons();
    QList<SensorData> getSensors();
    QList<SensorData> getValidSensors();
    QList<UserMarker> getMarkers();
    QList<UserMarker> getValidMarkers();
    DynamicObjects getAllData();

signals:
    void ownShipUpdated(const ShipData &data);
    void aisTargetsUpdated(const QList<AisTarget> &targets);
    void weaponsUpdated(const QList<WeaponData> &weapons);
    void sensorsUpdated(const QList<SensorData> &sensors);
    void markersUpdated(const QList<UserMarker> &markers);
    void dynamicDataChanged(const DynamicObjects &data);
    void dataExpired(const QString &id, ProtocolType source);

public slots:
    void onDataReceived(const QJsonObject &data, ProtocolType source);
    void checkExpiredData();

private:
    DataManager(QObject *parent = nullptr);
    ~DataManager();

    void parseAndUpdate(const QJsonObject &data, ProtocolType source);
    void updateOwnShip(const QJsonObject &obj, ProtocolType source);
    void updateAisTarget(const QJsonObject &obj, ProtocolType source);
    void updateWeapon(const QJsonObject &obj, ProtocolType source);
    void updateSensor(const QJsonObject &obj, ProtocolType source);
    void updateMarker(const QJsonObject &obj, ProtocolType source);

    qint64 calculateValidUntil(qint64 updateTime, const QJsonObject &obj);

    static DataManager *s_instance;
    static QMutex s_mutex;

    mutable QReadWriteLock m_dataLock;

    DynamicObjects m_dynamicData;

    QList<IProtocolAdapter*> m_adapters;
    QTimer m_expireTimer;

    QMap<ProtocolType, int> m_priorityMap;
    qint64 m_defaultValidDuration;
};

#endif
