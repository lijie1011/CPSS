#include "datamanager.h"
#include <QDateTime>
#include <QMutexLocker>
#include <QJsonArray>
#include "logger.h"

DataManager* DataManager::s_instance = nullptr;
QMutex DataManager::s_mutex;

DataManager::DataManager(QObject *parent)
    : QObject(parent),
      m_defaultValidDuration(5000)
{
    m_priorityMap[Protocol_DDS] = 10;
    m_priorityMap[Protocol_Redis] = 9;
    m_priorityMap[Protocol_TCP] = 8;
    m_priorityMap[Protocol_UDP] = 7;
    m_priorityMap[Protocol_WebSocket] = 6;
    m_priorityMap[Protocol_HTTP] = 5;
    m_priorityMap[Protocol_Unknown] = 0;

    connect(&m_expireTimer, &QTimer::timeout, this, &DataManager::checkExpiredData);
    m_expireTimer.start(1000);
}

DataManager::~DataManager()
{
    stopAllAdapters();
}

DataManager* DataManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new DataManager();
    }
    return s_instance;
}

void DataManager::addAdapter(IProtocolAdapter *adapter)
{
    if (!adapter || m_adapters.contains(adapter))
        return;

    m_adapters.append(adapter);
    connect(adapter, &IProtocolAdapter::dataReceived,
            this, &DataManager::onDataReceived, Qt::QueuedConnection);
    connect(adapter, &IProtocolAdapter::statusChanged,
            this, [this, adapter](IProtocolAdapter::AdapterStatus status) {
        Logger::info("Adapter %s status changed to %d",
                     adapter->adapterName().toStdString().c_str(), status);
    });
    connect(adapter, &IProtocolAdapter::errorOccurred,
            this, [this, adapter](const QString &error) {
        Logger::error("Adapter %s error: %s",
                      adapter->adapterName().toStdString().c_str(),
                      error.toStdString().c_str());
    });
}

void DataManager::removeAdapter(IProtocolAdapter *adapter)
{
    int index = m_adapters.indexOf(adapter);
    if (index != -1) {
        adapter->disconnect(this);
        m_adapters.removeAt(index);
    }
}

void DataManager::startAllAdapters()
{
    for (auto adapter : m_adapters) {
        if (adapter->start()) {
            Logger::info("Adapter %s started", adapter->adapterName().toStdString().c_str());
        } else {
            Logger::error("Adapter %s failed to start: %s",
                          adapter->adapterName().toStdString().c_str(),
                          adapter->lastError().toStdString().c_str());
        }
    }
}

void DataManager::stopAllAdapters()
{
    for (auto adapter : m_adapters) {
        if (adapter->stop()) {
            Logger::info("Adapter %s stopped", adapter->adapterName().toStdString().c_str());
        }
    }
}

void DataManager::setDataSourcePriority(ProtocolType type, int priority)
{
    m_priorityMap[type] = priority;
}

int DataManager::getDataSourcePriority(ProtocolType type) const
{
    return m_priorityMap.value(type, 0);
}

void DataManager::setDefaultValidDuration(qint64 ms)
{
    m_defaultValidDuration = ms;
}

qint64 DataManager::defaultValidDuration() const
{
    return m_defaultValidDuration;
}

ShipData DataManager::getOwnShip()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.ownShip;
}

QList<AisTarget> DataManager::getAisTargets()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.aisTargets;
}

QList<AisTarget> DataManager::getValidAisTargets()
{
    QReadLocker locker(&m_dataLock);
    QList<AisTarget> result;
    for (const auto &target : m_dynamicData.aisTargets) {
        if (!target.isExpired()) {
            result.append(target);
        }
    }
    return result;
}

QList<WeaponData> DataManager::getWeapons()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.weapons;
}

QList<WeaponData> DataManager::getValidWeapons()
{
    QReadLocker locker(&m_dataLock);
    QList<WeaponData> result;
    for (const auto &weapon : m_dynamicData.weapons) {
        if (!weapon.isExpired()) {
            result.append(weapon);
        }
    }
    return result;
}

QList<SensorData> DataManager::getSensors()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.sensors;
}

QList<SensorData> DataManager::getValidSensors()
{
    QReadLocker locker(&m_dataLock);
    QList<SensorData> result;
    for (const auto &sensor : m_dynamicData.sensors) {
        if (!sensor.isExpired()) {
            result.append(sensor);
        }
    }
    return result;
}

QList<UserMarker> DataManager::getMarkers()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.markers;
}

QList<UserMarker> DataManager::getValidMarkers()
{
    QReadLocker locker(&m_dataLock);
    QList<UserMarker> result;
    for (const auto &marker : m_dynamicData.markers) {
        if (!marker.isExpired()) {
            result.append(marker);
        }
    }
    return result;
}

DynamicObjects DataManager::getAllData()
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData;
}

void DataManager::onDataReceived(const QJsonObject &data, ProtocolType source)
{
    parseAndUpdate(data, source);
}

void DataManager::checkExpiredData()
{
    QWriteLocker locker(&m_dataLock);
    bool changed = false;

    if (m_dynamicData.ownShip.isExpired() && 
        m_dynamicData.ownShip.dataStatus == DataStatus_Normal) {
        m_dynamicData.ownShip.dataStatus = DataStatus_Expired;
        changed = true;
        emit dataExpired(m_dynamicData.ownShip.mmsi, m_dynamicData.ownShip.sourceProtocol);
    }

    for (auto &target : m_dynamicData.aisTargets) {
        if (target.isExpired() && target.dataStatus == DataStatus_Normal) {
            target.dataStatus = DataStatus_Expired;
            changed = true;
            emit dataExpired(target.mmsi, target.sourceProtocol);
        }
    }

    for (auto &weapon : m_dynamicData.weapons) {
        if (weapon.isExpired() && weapon.dataStatus == DataStatus_Normal) {
            weapon.dataStatus = DataStatus_Expired;
            changed = true;
            emit dataExpired(weapon.id, weapon.sourceProtocol);
        }
    }

    for (auto &sensor : m_dynamicData.sensors) {
        if (sensor.isExpired() && sensor.dataStatus == DataStatus_Normal) {
            sensor.dataStatus = DataStatus_Expired;
            changed = true;
            emit dataExpired(sensor.id, sensor.sourceProtocol);
        }
    }

    for (auto &marker : m_dynamicData.markers) {
        if (marker.isExpired() && marker.dataStatus == DataStatus_Normal) {
            marker.dataStatus = DataStatus_Expired;
            changed = true;
            emit dataExpired(marker.id, marker.sourceProtocol);
        }
    }

    if (changed) {
        m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
        emit dynamicDataChanged(m_dynamicData);
    }
}

void DataManager::parseAndUpdate(const QJsonObject &data, ProtocolType source)
{
    QString type = data["type"].toString();

    if (type == "ownShip") {
        updateOwnShip(data, source);
    } else if (type == "aisTarget") {
        updateAisTarget(data, source);
    } else if (type == "weapon") {
        updateWeapon(data, source);
    } else if (type == "sensor") {
        updateSensor(data, source);
    } else if (type == "marker") {
        updateMarker(data, source);
    } else if (type == "batch") {
        QJsonArray items = data["items"].toArray();
        for (const auto &item : items) {
            parseAndUpdate(item.toObject(), source);
        }
    }
}

void DataManager::updateOwnShip(const QJsonObject &obj, ProtocolType source)
{
    QWriteLocker locker(&m_dataLock);

    int currentPriority = m_priorityMap.value(m_dynamicData.ownShip.sourceProtocol, 0);
    int newPriority = m_priorityMap.value(source, 0);

    if (newPriority < currentPriority && !m_dynamicData.ownShip.mmsi.isEmpty()) {
        return;
    }

    qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

    m_dynamicData.ownShip.mmsi = obj["mmsi"].toString();
    m_dynamicData.ownShip.name = obj["name"].toString();
    m_dynamicData.ownShip.lon = obj["lon"].toDouble();
    m_dynamicData.ownShip.lat = obj["lat"].toDouble();
    m_dynamicData.ownShip.heading = obj["heading"].toDouble();
    m_dynamicData.ownShip.speed = obj["speed"].toDouble();
    m_dynamicData.ownShip.isOwnShip = true;
    m_dynamicData.ownShip.visible = true;

    m_dynamicData.ownShip.dataStatus = DataStatus_Normal;
    m_dynamicData.ownShip.updateTime = updateTime;
    m_dynamicData.ownShip.sourceProtocol = source;
    m_dynamicData.ownShip.validUntil = calculateValidUntil(updateTime, obj);

    m_dynamicData.timestamp = updateTime;
    emit ownShipUpdated(m_dynamicData.ownShip);
    emit dynamicDataChanged(m_dynamicData);
}

void DataManager::updateAisTarget(const QJsonObject &obj, ProtocolType source)
{
    QWriteLocker locker(&m_dataLock);

    QString mmsi = obj["mmsi"].toString();
    if (mmsi.isEmpty())
        return;

    int currentPriority = 0;
    int newPriority = m_priorityMap.value(source, 0);
    int targetIndex = -1;

    for (int i = 0; i < m_dynamicData.aisTargets.size(); ++i) {
        if (m_dynamicData.aisTargets[i].mmsi == mmsi) {
            targetIndex = i;
            currentPriority = m_priorityMap.value(m_dynamicData.aisTargets[i].sourceProtocol, 0);
            break;
        }
    }

    if (targetIndex != -1 && newPriority < currentPriority) {
        return;
    }

    qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

    AisTarget target;
    target.mmsi = mmsi;
    target.name = obj["name"].toString();
    target.lon = obj["lon"].toDouble();
    target.lat = obj["lat"].toDouble();
    target.heading = obj["heading"].toDouble();
    target.speed = obj["speed"].toDouble();
    target.shipType = obj["shipType"].toInt();
    target.isDanger = obj["isDanger"].toBool();
    target.visible = true;

    target.dataStatus = DataStatus_Normal;
    target.updateTime = updateTime;
    target.sourceProtocol = source;
    target.validUntil = calculateValidUntil(updateTime, obj);

    if (targetIndex != -1) {
        m_dynamicData.aisTargets[targetIndex] = target;
    } else {
        m_dynamicData.aisTargets.append(target);
    }

    m_dynamicData.timestamp = updateTime;
    emit aisTargetsUpdated(m_dynamicData.aisTargets);
    emit dynamicDataChanged(m_dynamicData);
}

void DataManager::updateWeapon(const QJsonObject &obj, ProtocolType source)
{
    QWriteLocker locker(&m_dataLock);

    QString id = obj["id"].toString();
    if (id.isEmpty())
        return;

    int currentPriority = 0;
    int newPriority = m_priorityMap.value(source, 0);
    int weaponIndex = -1;

    for (int i = 0; i < m_dynamicData.weapons.size(); ++i) {
        if (m_dynamicData.weapons[i].id == id) {
            weaponIndex = i;
            currentPriority = m_priorityMap.value(m_dynamicData.weapons[i].sourceProtocol, 0);
            break;
        }
    }

    if (weaponIndex != -1 && newPriority < currentPriority) {
        return;
    }

    qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

    WeaponData weapon;
    weapon.id = id;
    weapon.lon = obj["lon"].toDouble();
    weapon.lat = obj["lat"].toDouble();
    weapon.targetLon = obj["targetLon"].toDouble();
    weapon.targetLat = obj["targetLat"].toDouble();
    weapon.type = obj["weaponType"].toString();
    weapon.active = obj["active"].toBool();

    weapon.dataStatus = DataStatus_Normal;
    weapon.updateTime = updateTime;
    weapon.sourceProtocol = source;
    weapon.validUntil = calculateValidUntil(updateTime, obj);

    if (weaponIndex != -1) {
        m_dynamicData.weapons[weaponIndex] = weapon;
    } else {
        m_dynamicData.weapons.append(weapon);
    }

    m_dynamicData.timestamp = updateTime;
    emit weaponsUpdated(m_dynamicData.weapons);
    emit dynamicDataChanged(m_dynamicData);
}

void DataManager::updateSensor(const QJsonObject &obj, ProtocolType source)
{
    QWriteLocker locker(&m_dataLock);

    QString id = obj["id"].toString();
    if (id.isEmpty())
        return;

    int currentPriority = 0;
    int newPriority = m_priorityMap.value(source, 0);
    int sensorIndex = -1;

    for (int i = 0; i < m_dynamicData.sensors.size(); ++i) {
        if (m_dynamicData.sensors[i].id == id) {
            sensorIndex = i;
            currentPriority = m_priorityMap.value(m_dynamicData.sensors[i].sourceProtocol, 0);
            break;
        }
    }

    if (sensorIndex != -1 && newPriority < currentPriority) {
        return;
    }

    qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

    SensorData sensor;
    sensor.id = id;
    sensor.lon = obj["lon"].toDouble();
    sensor.lat = obj["lat"].toDouble();
    sensor.radius = obj["radius"].toDouble();
    sensor.azimuth = obj["azimuth"].toDouble();
    sensor.angle = obj["angle"].toDouble();
    sensor.type = obj["sensorType"].toString();
    sensor.active = obj["active"].toBool();

    sensor.dataStatus = DataStatus_Normal;
    sensor.updateTime = updateTime;
    sensor.sourceProtocol = source;
    sensor.validUntil = calculateValidUntil(updateTime, obj);

    if (sensorIndex != -1) {
        m_dynamicData.sensors[sensorIndex] = sensor;
    } else {
        m_dynamicData.sensors.append(sensor);
    }

    m_dynamicData.timestamp = updateTime;
    emit sensorsUpdated(m_dynamicData.sensors);
    emit dynamicDataChanged(m_dynamicData);
}

void DataManager::updateMarker(const QJsonObject &obj, ProtocolType source)
{
    QWriteLocker locker(&m_dataLock);

    QString id = obj["id"].toString();
    if (id.isEmpty())
        return;

    int currentPriority = 0;
    int newPriority = m_priorityMap.value(source, 0);
    int markerIndex = -1;

    for (int i = 0; i < m_dynamicData.markers.size(); ++i) {
        if (m_dynamicData.markers[i].id == id) {
            markerIndex = i;
            currentPriority = m_priorityMap.value(m_dynamicData.markers[i].sourceProtocol, 0);
            break;
        }
    }

    if (markerIndex != -1 && newPriority < currentPriority) {
        return;
    }

    qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

    UserMarker marker;
    marker.id = id;
    marker.lon = obj["lon"].toDouble();
    marker.lat = obj["lat"].toDouble();
    marker.label = obj["label"].toString();
    marker.color = obj["color"].toString("red");

    marker.dataStatus = DataStatus_Normal;
    marker.updateTime = updateTime;
    marker.sourceProtocol = source;
    marker.validUntil = calculateValidUntil(updateTime, obj);

    if (markerIndex != -1) {
        m_dynamicData.markers[markerIndex] = marker;
    } else {
        m_dynamicData.markers.append(marker);
    }

    m_dynamicData.timestamp = updateTime;
    emit markersUpdated(m_dynamicData.markers);
    emit dynamicDataChanged(m_dynamicData);
}

qint64 DataManager::calculateValidUntil(qint64 updateTime, const QJsonObject &obj)
{
    if (obj.contains("validUntil") && obj["validUntil"].isDouble()) {
        return obj["validUntil"].toDouble();
    }

    if (obj.contains("validDuration") && obj["validDuration"].isDouble()) {
        return updateTime + obj["validDuration"].toDouble();
    }

    return updateTime + m_defaultValidDuration;
}
