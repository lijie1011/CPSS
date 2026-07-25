#include "datacache.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "logger.h"

DataCache* DataCache::s_instance = nullptr;
QMutex DataCache::s_mutex;

DataCache* DataCache::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_mutex);
        if (!s_instance) {
            s_instance = new DataCache();
        }
    }
    return s_instance;
}

DataCache::DataCache(QObject *parent)
    : QObject(parent)
{
    connect(&m_expireTimer, &QTimer::timeout, this, &DataCache::invalidateExpiredData);
    m_expireTimer.start(1000);
}

DataCache::~DataCache()
{
    stopDataPush();
    stopTestDataTimer();
}

bool DataCache::updatePlatform(const PlatformData &data)
{
    QWriteLocker locker(&m_dataLock);
    m_dynamicData.platforms[data.id] = data;
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    locker.unlock();
    emit platformUpdated(data);
    emit platformsUpdated(getAllPlatforms());
    emit dynamicDataChanged(getAllData());
    Logger::info("Platform cached: id=%s, lon=%f, lat=%f", 
                 data.id.toStdString().c_str(), data.lon, data.lat);
    return true;
}

bool DataCache::removePlatform(const QString &id)
{
    QWriteLocker locker(&m_dataLock);
    if (m_dynamicData.platforms.contains(id)) {
        m_dynamicData.platforms.remove(id);
        m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
        locker.unlock();
        emit platformsUpdated(getAllPlatforms());
        emit dynamicDataChanged(getAllData());
        Logger::info("Platform removed from cache: id=%s", id.toStdString().c_str());
        return true;
    }
    return false;
}

PlatformData DataCache::getPlatform(const QString &id) const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.platforms.value(id);
}

QList<PlatformData> DataCache::getAllPlatforms() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.platforms.values();
}

QList<PlatformData> DataCache::getValidPlatforms() const
{
    QReadLocker locker(&m_dataLock);
    QList<PlatformData> valid;
    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (!platform.isExpired()) {
            valid.append(platform);
        }
    }
    return valid;
}

bool DataCache::addEvent(const SpecialEvent &event)
{
    QWriteLocker locker(&m_dataLock);
    m_dynamicData.events.append(event);
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    locker.unlock();
    emit eventAdded(event);
    emit dynamicDataChanged(getAllData());
    Logger::info("Event cached: id=%s, type=%d", 
                 event.eventId.toStdString().c_str(), event.eventType);
    return true;
}

bool DataCache::removeEvent(const QString &eventId)
{
    QWriteLocker locker(&m_dataLock);
    for (auto it = m_dynamicData.events.begin(); it != m_dynamicData.events.end(); ++it) {
        if (it->eventId == eventId) {
            m_dynamicData.events.erase(it);
            m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
            locker.unlock();
            emit eventRemoved(eventId);
            emit dynamicDataChanged(getAllData());
            Logger::info("Event removed from cache: id=%s", eventId.toStdString().c_str());
            return true;
        }
    }
    return false;
}

void DataCache::clearEvents()
{
    QWriteLocker locker(&m_dataLock);
    m_dynamicData.events.clear();
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    emit dynamicDataChanged(getAllData());
    Logger::info("All events cleared from cache");
}

QList<SpecialEvent> DataCache::getAllEvents() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.events;
}

DynamicObjects DataCache::getAllData() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData;
}

void DataCache::invalidateExpiredData()
{
    QWriteLocker locker(&m_dataLock);
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QStringList expiredIds;
    for (auto it = m_dynamicData.platforms.begin(); it != m_dynamicData.platforms.end(); ++it) {
        if (it.value().dataStatus == DataStatus_Normal && 
            it.value().validUntil > 0 && 
            it.value().validUntil < now) {
            m_dynamicData.platforms[it.key()].dataStatus = DataStatus_Expired;
            expiredIds.append(it.key());
        }
    }
    if (!expiredIds.isEmpty()) {
        m_dynamicData.timestamp = now;
        locker.unlock();
        emit platformsUpdated(getAllPlatforms());
        emit dynamicDataChanged(getAllData());
        Logger::info("Expired platforms in cache: %d", expiredIds.size());
    }
}

qint64 DataCache::getTimestamp() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.timestamp;
}

void DataCache::registerDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.push_back(callback);
    Logger::info("Data push callback registered, total callbacks: %d", m_pushCallbacks.size());
}

void DataCache::unregisterDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.clear();
    Logger::info("Data push callback unregistered, total callbacks: %d", m_pushCallbacks.size());
}

void DataCache::startDataPush(int intervalMs)
{
    if (m_pushTimer.isActive()) {
        m_pushTimer.stop();
    }
    disconnect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    connect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    m_pushTimer.start(intervalMs);
    Logger::info("Data push started with interval: %d ms", intervalMs);
}

void DataCache::stopDataPush()
{
    m_pushTimer.stop();
    Logger::info("Data push stopped");
}

bool DataCache::isPushRunning() const
{
    return m_pushTimer.isActive();
}

void DataCache::startTestDataTimer(int intervalMs)
{
    initTestData();
    if (m_testDataTimer.isActive()) {
        m_testDataTimer.stop();
    }
    disconnect(&m_testDataTimer, &QTimer::timeout, this, &DataCache::updateTestData);
    connect(&m_testDataTimer, &QTimer::timeout, this, &DataCache::updateTestData);
    m_testDataTimer.start(intervalMs);
    Logger::info("Test data timer started with interval: %d ms", intervalMs);
}

void DataCache::stopTestDataTimer()
{
    m_testDataTimer.stop();
    Logger::info("Test data timer stopped");
}

void DataCache::pushData()
{
    static int pushCount = 0;
    pushCount++;
    DynamicObjects data = getAllData();
    Logger::info("pushData: count=%d, platforms=%d", pushCount, data.platforms.size());
    for (const auto &callback : m_pushCallbacks) {
        callback(data);
    }
    emit dataPushed(data);
}

void DataCache::initTestData()
{
    PlatformData ownShip;
    ownShip.id = "SHIP_001";
    ownShip.name = "Own Ship";
    ownShip.lon = 121.5;
    ownShip.lat = 31.2;
    ownShip.altitude = 0.0;
    ownShip.speed = 12.5;
    ownShip.type = "warship";
    ownShip.category = "destroyer";
    ownShip.camp = Camp_Friendly;
    ownShip.dataStatus = DataStatus_Normal;
    ownShip.updateTime = QDateTime::currentMSecsSinceEpoch();
    ownShip.validUntil = ownShip.updateTime + 5000;
    ownShip.sourceProtocol = Protocol_Unknown;
    WeaponInfo w1;
    w1.type = "missile";
    w1.count = 8;
    ownShip.weapons.append(w1);
    WeaponInfo w2;
    w2.type = "gun";
    w2.count = 2;
    ownShip.weapons.append(w2);
    SensorInfo s1;
    s1.type = "radar";
    s1.count = 1;
    ownShip.sensors.append(s1);
    SensorInfo s2;
    s2.type = "sonar";
    s2.count = 1;
    ownShip.sensors.append(s2);
    updatePlatform(ownShip);

    PlatformData target1;
    target1.id = "SHIP_002";
    target1.name = "Merchant A";
    target1.lon = 121.51;
    target1.lat = 31.22;
    target1.altitude = 0.0;
    target1.speed = 8.0;
    target1.type = "merchant";
    target1.category = "cargo";
    target1.camp = Camp_Neutral;
    target1.dataStatus = DataStatus_Normal;
    target1.updateTime = QDateTime::currentMSecsSinceEpoch();
    target1.validUntil = target1.updateTime + 5000;
    target1.sourceProtocol = Protocol_Unknown;
    updatePlatform(target1);

    PlatformData target2;
    target2.id = "SHIP_003";
    target2.name = "Fishing B";
    target2.lon = 121.48;
    target2.lat = 31.18;
    target2.altitude = 0.0;
    target2.speed = 5.0;
    target2.type = "fishing";
    target2.category = "fishing";
    target2.camp = Camp_Neutral;
    target2.dataStatus = DataStatus_Normal;
    target2.updateTime = QDateTime::currentMSecsSinceEpoch();
    target2.validUntil = target2.updateTime + 5000;
    target2.sourceProtocol = Protocol_Unknown;
    updatePlatform(target2);

    PlatformData target3;
    target3.id = "SHIP_004";
    target3.name = "Enemy Ship";
    target3.lon = 121.55;
    target3.lat = 31.25;
    target3.altitude = 0.0;
    target3.speed = 15.0;
    target3.type = "warship";
    target3.category = "cruiser";
    target3.camp = Camp_Enemy;
    target3.dataStatus = DataStatus_Normal;
    target3.updateTime = QDateTime::currentMSecsSinceEpoch();
    target3.validUntil = target3.updateTime + 5000;
    target3.sourceProtocol = Protocol_Unknown;
    WeaponInfo ew1;
    ew1.type = "missile";
    ew1.count = 16;
    target3.weapons.append(ew1);
    SensorInfo es1;
    es1.type = "radar";
    es1.count = 2;
    target3.sensors.append(es1);
    updatePlatform(target3);

    SpecialEvent alertEvent;
    alertEvent.eventId = "EVENT_001";
    alertEvent.eventType = Event_Alert;
    alertEvent.eventName = "Enemy Detected";
    alertEvent.description = "Enemy ship SHIP_004 detected in the area";
    alertEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
    alertEvent.targetId = "SHIP_004";
    alertEvent.sourceId = "SHIP_001";
    addEvent(alertEvent);

    SpecialEvent missionEvent;
    missionEvent.eventId = "EVENT_002";
    missionEvent.eventType = Event_MissionStart;
    missionEvent.eventName = "Patrol Mission Started";
    missionEvent.description = "Own ship SHIP_001 started patrol mission";
    missionEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
    missionEvent.targetId = "SHIP_001";
    QJsonObject extraData;
    extraData["missionName"] = "Area Patrol Alpha";
    extraData["missionDuration"] = 3600;
    missionEvent.extraData = extraData;
    addEvent(missionEvent);

    SpecialEvent standaloneEvent;
    standaloneEvent.eventId = "EVENT_003";
    standaloneEvent.eventType = Event_Custom;
    standaloneEvent.eventName = "Special Event";
    standaloneEvent.description = "A special event occurred at this location";
    standaloneEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
    standaloneEvent.targetId = "";
    standaloneEvent.sourceId = "SYSTEM";
    standaloneEvent.lon = 121.52;
    standaloneEvent.lat = 31.21;
    addEvent(standaloneEvent);

    Logger::info("Test data initialized, total platforms: %d, total events: %d", 
                 getAllPlatforms().size(), getAllEvents().size());
}

void DataCache::updateTestData()
{
    Logger::info("updateTestData called, platforms count: %d", getAllPlatforms().size());
    QList<PlatformData> platforms = getAllPlatforms();
    for (PlatformData &platform : platforms) {
        platform.lon += 0.00005;
        platform.lat += 0.00003;
        platform.updateTime = QDateTime::currentMSecsSinceEpoch();
        platform.validUntil = platform.updateTime + 5000;
        updatePlatform(platform);
    }
}
