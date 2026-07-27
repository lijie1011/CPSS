#include "datacache.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "logger.h"
#include "datamanager.h"

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
    : QObject(parent),
      m_maxHistorySize(100),
      m_testStartTime(0),
      m_eventSecondPhase(false),
      m_eventCounter(0)
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
    
    PlatformData newData = data;
    if (m_dynamicData.platforms.contains(data.id)) {
        PlatformData existing = m_dynamicData.platforms[data.id];
        newData.trackPoints = existing.trackPoints;
        if (existing.lon != data.lon || existing.lat != data.lat) {
            newData.addTrackPoint(existing.lon, existing.lat);
        }
    }
    
    m_dynamicData.platforms[data.id] = newData;
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    locker.unlock();
    emit platformUpdated(newData);
    emit platformsUpdated(getAllPlatforms());
    emit dynamicDataChanged(getAllData());
    // Logger::info("Platform cached: id=%s, lon=%f, lat=%f, track points=%d", 
                 // newData.id.toStdString().c_str(), newData.lon, newData.lat, 
                 // newData.trackPoints.size());
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
        // Logger::info("Platform removed from cache: id=%s", id.toStdString().c_str());
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
    m_eventHistory.prepend(event);
    while (m_eventHistory.size() > m_maxHistorySize) {
        m_eventHistory.removeLast();
    }
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    locker.unlock();
    emit eventAdded(event);
    emit dynamicDataChanged(getAllData());
    // Logger::info("Event cached: id=%s, type=%d, history size=%d", 
                 // event.eventId.toStdString().c_str(), event.eventType, m_eventHistory.size());
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
            // Logger::info("Event removed from cache: id=%s", eventId.toStdString().c_str());
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
    // Logger::info("All events cleared from cache");
}

QList<SpecialEvent> DataCache::getAllEvents() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.events;
}

QList<SpecialEvent> DataCache::getEventHistory() const
{
    QReadLocker locker(&m_dataLock);
    return m_eventHistory;
}

void DataCache::clearEventHistory()
{
    QWriteLocker locker(&m_dataLock);
    m_eventHistory.clear();
    // Logger::info("Event history cleared");
}

int DataCache::getMaxHistorySize() const
{
    return m_maxHistorySize;
}

void DataCache::setMaxHistorySize(int size)
{
    QWriteLocker locker(&m_dataLock);
    m_maxHistorySize = size;
    while (m_eventHistory.size() > m_maxHistorySize) {
        m_eventHistory.removeLast();
    }
    // Logger::info("Max history size set to: %d", m_maxHistorySize);
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
        
        for (const QString& id : expiredIds) {
            DataManager::instance()->onPlatformExpired(id);
        }
        
        emit platformsUpdated(getAllPlatforms());
        emit dynamicDataChanged(getAllData());
        // Logger::info("Expired platforms in cache: %d", expiredIds.size());
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
    // Logger::info("Data push callback registered, total callbacks: %d", m_pushCallbacks.size());
}

void DataCache::unregisterDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.clear();
    // Logger::info("Data push callback unregistered, total callbacks: %d", m_pushCallbacks.size());
}

void DataCache::startDataPush(int intervalMs)
{
    if (m_pushTimer.isActive()) {
        m_pushTimer.stop();
    }
    disconnect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    connect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    m_pushTimer.start(intervalMs);
    // Logger::info("Data push started with interval: %d ms", intervalMs);
}

void DataCache::stopDataPush()
{
    m_pushTimer.stop();
    // Logger::info("Data push stopped");
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
    // Logger::info("Test data timer started with interval: %d ms", intervalMs);
}

void DataCache::stopTestDataTimer()
{
    m_testDataTimer.stop();
    // Logger::info("Test data timer stopped");
}

void DataCache::pushData()
{
    static int pushCount = 0;
    pushCount++;
    DynamicObjects data = getAllData();
    // Logger::info("pushData: count=%d, platforms=%d", pushCount, data.platforms.size());
    for (const auto &callback : m_pushCallbacks) {
        callback(data);
    }
    emit dataPushed(data);
}

void DataCache::initTestData()
{
    double redBaseLon = 121.5;
    double redBaseLat = 31.2;
    double purpleBaseLon = redBaseLon + 1.05;
    double purpleBaseLat = redBaseLat;

    // Red Team - 5 ships
    PlatformData redShip1;
    redShip1.id = "1";
    redShip1.name = "Red Ship 1";
    redShip1.lon = redBaseLon;
    redShip1.lat = redBaseLat;
    redShip1.altitude = 0.0;
    redShip1.speed = 12.0;
    redShip1.heading = 90.0;
    redShip1.type = "warship";
    redShip1.category = "destroyer";
    redShip1.camp = Camp_Red;
    redShip1.dataStatus = DataStatus_Normal;
    redShip1.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip1.validUntil = redShip1.updateTime + 5000;
    redShip1.sourceProtocol = Protocol_Unknown;
    WeaponInfo r1w1 = {"missile", 8, 80.0};
    WeaponInfo r1w2 = {"naval_gun", 2, 20.0};
    SensorInfo r1s1 = {"radar", 1, 50.0};
    SensorInfo r1s2 = {"sonar", 1, 20.0};
    redShip1.weapons.append(r1w1);
    redShip1.weapons.append(r1w2);
    redShip1.sensors.append(r1s1);
    redShip1.sensors.append(r1s2);
    updatePlatform(redShip1);

    PlatformData redShip2;
    redShip2.id = "2";
    redShip2.name = "Red Ship 2";
    redShip2.lon = redBaseLon - 0.05;
    redShip2.lat = redBaseLat + 0.03;
    redShip2.altitude = 0.0;
    redShip2.speed = 10.0;
    redShip2.heading = 70.0;
    redShip2.type = "warship";
    redShip2.category = "frigate";
    redShip2.camp = Camp_Red;
    redShip2.dataStatus = DataStatus_Normal;
    redShip2.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip2.validUntil = redShip2.updateTime + 5000;
    redShip2.sourceProtocol = Protocol_Unknown;
    WeaponInfo r2w1 = {"missile", 6, 60.0};
    SensorInfo r2s1 = {"radar", 1, 40.0};
    redShip2.weapons.append(r2w1);
    redShip2.sensors.append(r2s1);
    updatePlatform(redShip2);

    PlatformData redShip3;
    redShip3.id = "3";
    redShip3.name = "Red Ship 3";
    redShip3.lon = redBaseLon + 0.05;
    redShip3.lat = redBaseLat + 0.03;
    redShip3.altitude = 0.0;
    redShip3.speed = 11.0;
    redShip3.heading = 110.0;
    redShip3.type = "warship";
    redShip3.category = "cruiser";
    redShip3.camp = Camp_Red;
    redShip3.dataStatus = DataStatus_Normal;
    redShip3.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip3.validUntil = redShip3.updateTime + 5000;
    redShip3.sourceProtocol = Protocol_Unknown;
    WeaponInfo r3w1 = {"missile", 12, 100.0};
    SensorInfo r3s1 = {"radar", 2, 60.0};
    redShip3.weapons.append(r3w1);
    redShip3.sensors.append(r3s1);
    updatePlatform(redShip3);

    PlatformData redShip4;
    redShip4.id = "4";
    redShip4.name = "Red Ship 4";
    redShip4.lon = redBaseLon - 0.05;
    redShip4.lat = redBaseLat - 0.03;
    redShip4.altitude = 0.0;
    redShip4.speed = 9.0;
    redShip4.heading = 80.0;
    redShip4.type = "warship";
    redShip4.category = "destroyer";
    redShip4.camp = Camp_Red;
    redShip4.dataStatus = DataStatus_Normal;
    redShip4.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip4.validUntil = redShip4.updateTime + 5000;
    redShip4.sourceProtocol = Protocol_Unknown;
    WeaponInfo r4w1 = {"missile", 8, 70.0};
    WeaponInfo r4w2 = {"naval_gun", 1, 15.0};
    SensorInfo r4s1 = {"radar", 1, 45.0};
    SensorInfo r4s2 = {"sonar", 1, 18.0};
    redShip4.weapons.append(r4w1);
    redShip4.weapons.append(r4w2);
    redShip4.sensors.append(r4s1);
    redShip4.sensors.append(r4s2);
    updatePlatform(redShip4);

    PlatformData redShip5;
    redShip5.id = "5";
    redShip5.name = "Red Ship 5";
    redShip5.lon = redBaseLon + 0.05;
    redShip5.lat = redBaseLat - 0.03;
    redShip5.altitude = 0.0;
    redShip5.speed = 13.0;
    redShip5.heading = 100.0;
    redShip5.type = "warship";
    redShip5.category = "frigate";
    redShip5.camp = Camp_Red;
    redShip5.dataStatus = DataStatus_Normal;
    redShip5.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip5.validUntil = redShip5.updateTime + 5000;
    redShip5.sourceProtocol = Protocol_Unknown;
    WeaponInfo r5w1 = {"missile", 6, 55.0};
    SensorInfo r5s1 = {"radar", 1, 35.0};
    redShip5.weapons.append(r5w1);
    redShip5.sensors.append(r5s1);
    updatePlatform(redShip5);

    // Red Team - 3 planes
    PlatformData redPlane1;
    redPlane1.id = "6";
    redPlane1.name = "Red Plane 1";
    redPlane1.lon = redBaseLon;
    redPlane1.lat = redBaseLat + 0.08;
    redPlane1.altitude = 5000.0;
    redPlane1.speed = 800.0;
    redPlane1.heading = 90.0;
    redPlane1.type = "aircraft";
    redPlane1.category = "fighter";
    redPlane1.camp = Camp_Red;
    redPlane1.dataStatus = DataStatus_Normal;
    redPlane1.updateTime = QDateTime::currentMSecsSinceEpoch();
    redPlane1.validUntil = redPlane1.updateTime + 5000;
    redPlane1.sourceProtocol = Protocol_Unknown;
    WeaponInfo rp1w1 = {"missile", 4, 150.0};
    SensorInfo rp1s1 = {"radar", 1, 200.0};
    redPlane1.weapons.append(rp1w1);
    redPlane1.sensors.append(rp1s1);
    updatePlatform(redPlane1);

    PlatformData redPlane2;
    redPlane2.id = "7";
    redPlane2.name = "Red Plane 2";
    redPlane2.lon = redBaseLon - 0.06;
    redPlane2.lat = redBaseLat + 0.06;
    redPlane2.altitude = 6000.0;
    redPlane2.speed = 750.0;
    redPlane2.heading = 60.0;
    redPlane2.type = "aircraft";
    redPlane2.category = "recon";
    redPlane2.camp = Camp_Red;
    redPlane2.dataStatus = DataStatus_Normal;
    redPlane2.updateTime = QDateTime::currentMSecsSinceEpoch();
    redPlane2.validUntil = redPlane2.updateTime + 5000;
    redPlane2.sourceProtocol = Protocol_Unknown;
    SensorInfo rp2s1 = {"radar", 1, 250.0};
    SensorInfo rp2s2 = {"sonar", 1, 100.0};
    redPlane2.sensors.append(rp2s1);
    redPlane2.sensors.append(rp2s2);
    updatePlatform(redPlane2);

    PlatformData redPlane3;
    redPlane3.id = "8";
    redPlane3.name = "Red Plane 3";
    redPlane3.lon = redBaseLon + 0.06;
    redPlane3.lat = redBaseLat + 0.06;
    redPlane3.altitude = 5500.0;
    redPlane3.speed = 780.0;
    redPlane3.heading = 120.0;
    redPlane3.type = "aircraft";
    redPlane3.category = "fighter";
    redPlane3.camp = Camp_Red;
    redPlane3.dataStatus = DataStatus_Normal;
    redPlane3.updateTime = QDateTime::currentMSecsSinceEpoch();
    redPlane3.validUntil = redPlane3.updateTime + 5000;
    redPlane3.sourceProtocol = Protocol_Unknown;
    WeaponInfo rp3w1 = {"missile", 6, 120.0};
    SensorInfo rp3s1 = {"radar", 1, 180.0};
    redPlane3.weapons.append(rp3w1);
    redPlane3.sensors.append(rp3s1);
    updatePlatform(redPlane3);

    // Purple Team - 4 ships
    PlatformData purpleShip1;
    purpleShip1.id = "9";
    purpleShip1.name = "Purple Ship 1";
    purpleShip1.lon = purpleBaseLon;
    purpleShip1.lat = purpleBaseLat;
    purpleShip1.altitude = 0.0;
    purpleShip1.speed = 14.0;
    purpleShip1.heading = 270.0;
    purpleShip1.type = "warship";
    purpleShip1.category = "cruiser";
    purpleShip1.camp = Camp_Purple;
    purpleShip1.dataStatus = DataStatus_Normal;
    purpleShip1.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip1.validUntil = purpleShip1.updateTime + 5000;
    purpleShip1.sourceProtocol = Protocol_Unknown;
    WeaponInfo p1w1 = {"missile", 16, 120.0};
    SensorInfo p1s1 = {"radar", 2, 120.0};
    purpleShip1.weapons.append(p1w1);
    purpleShip1.sensors.append(p1s1);
    updatePlatform(purpleShip1);

    PlatformData purpleShip2;
    purpleShip2.id = "10";
    purpleShip2.name = "Purple Ship 2";
    purpleShip2.lon = purpleBaseLon - 0.05;
    purpleShip2.lat = purpleBaseLat + 0.03;
    purpleShip2.altitude = 0.0;
    purpleShip2.speed = 12.0;
    purpleShip2.heading = 250.0;
    purpleShip2.type = "warship";
    purpleShip2.category = "destroyer";
    purpleShip2.camp = Camp_Purple;
    purpleShip2.dataStatus = DataStatus_Normal;
    purpleShip2.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip2.validUntil = purpleShip2.updateTime + 5000;
    purpleShip2.sourceProtocol = Protocol_Unknown;
    WeaponInfo p2w1 = {"missile", 8, 80.0};
    WeaponInfo p2w2 = {"naval_gun", 2, 25.0};
    SensorInfo p2s1 = {"radar", 1, 50.0};
    SensorInfo p2s2 = {"sonar", 1, 25.0};
    purpleShip2.weapons.append(p2w1);
    purpleShip2.weapons.append(p2w2);
    purpleShip2.sensors.append(p2s1);
    purpleShip2.sensors.append(p2s2);
    updatePlatform(purpleShip2);

    PlatformData purpleShip3;
    purpleShip3.id = "11";
    purpleShip3.name = "Purple Ship 3";
    purpleShip3.lon = purpleBaseLon + 0.05;
    purpleShip3.lat = purpleBaseLat + 0.03;
    purpleShip3.altitude = 0.0;
    purpleShip3.speed = 13.0;
    purpleShip3.heading = 290.0;
    purpleShip3.type = "warship";
    purpleShip3.category = "frigate";
    purpleShip3.camp = Camp_Purple;
    purpleShip3.dataStatus = DataStatus_Normal;
    purpleShip3.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip3.validUntil = purpleShip3.updateTime + 5000;
    purpleShip3.sourceProtocol = Protocol_Unknown;
    WeaponInfo p3w1 = {"missile", 6, 70.0};
    SensorInfo p3s1 = {"radar", 1, 45.0};
    purpleShip3.weapons.append(p3w1);
    purpleShip3.sensors.append(p3s1);
    updatePlatform(purpleShip3);

    PlatformData purpleShip4;
    purpleShip4.id = "12";
    purpleShip4.name = "Purple Ship 4";
    purpleShip4.lon = purpleBaseLon;
    purpleShip4.lat = purpleBaseLat - 0.03;
    purpleShip4.altitude = 0.0;
    purpleShip4.speed = 11.0;
    purpleShip4.heading = 260.0;
    purpleShip4.type = "warship";
    purpleShip4.category = "destroyer";
    purpleShip4.camp = Camp_Purple;
    purpleShip4.dataStatus = DataStatus_Normal;
    purpleShip4.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip4.validUntil = purpleShip4.updateTime + 5000;
    purpleShip4.sourceProtocol = Protocol_Unknown;
    WeaponInfo p4w1 = {"missile", 10, 90.0};
    SensorInfo p4s1 = {"radar", 1, 55.0};
    SensorInfo p4s2 = {"sonar", 1, 20.0};
    purpleShip4.weapons.append(p4w1);
    purpleShip4.sensors.append(p4s1);
    purpleShip4.sensors.append(p4s2);
    updatePlatform(purpleShip4);

    // Purple Team - 2 planes
    PlatformData purplePlane1;
    purplePlane1.id = "13";
    purplePlane1.name = "Purple Plane 1";
    purplePlane1.lon = purpleBaseLon;
    purplePlane1.lat = purpleBaseLat + 0.08;
    purplePlane1.altitude = 5500.0;
    purplePlane1.speed = 850.0;
    purplePlane1.heading = 270.0;
    purplePlane1.type = "aircraft";
    purplePlane1.category = "fighter";
    purplePlane1.camp = Camp_Purple;
    purplePlane1.dataStatus = DataStatus_Normal;
    purplePlane1.updateTime = QDateTime::currentMSecsSinceEpoch();
    purplePlane1.validUntil = purplePlane1.updateTime + 5000;
    purplePlane1.sourceProtocol = Protocol_Unknown;
    WeaponInfo pp1w1 = {"missile", 6, 120.0};
    SensorInfo pp1s1 = {"radar", 1, 120.0};
    purplePlane1.weapons.append(pp1w1);
    purplePlane1.sensors.append(pp1s1);
    updatePlatform(purplePlane1);

    PlatformData purplePlane2;
    purplePlane2.id = "14";
    purplePlane2.name = "Purple Plane 2";
    purplePlane2.lon = purpleBaseLon + 0.05;
    purplePlane2.lat = purpleBaseLat + 0.06;
    purplePlane2.altitude = 6000.0;
    purplePlane2.speed = 800.0;
    purplePlane2.heading = 250.0;
    purplePlane2.type = "aircraft";
    purplePlane2.category = "recon";
    purplePlane2.camp = Camp_Purple;
    purplePlane2.dataStatus = DataStatus_Normal;
    purplePlane2.updateTime = QDateTime::currentMSecsSinceEpoch();
    purplePlane2.validUntil = purplePlane2.updateTime + 5000;
    purplePlane2.sourceProtocol = Protocol_Unknown;
    SensorInfo pp2s1 = {"radar", 1, 200.0};
    purplePlane2.sensors.append(pp2s1);
    updatePlatform(purplePlane2);

    SpecialEvent contactEvent;
    contactEvent.eventId = "EVENT_001";
    contactEvent.eventType = Event_Contact;
    contactEvent.eventName = "Purple Detected";
    contactEvent.description = "Red forces detected purple forces at 100km distance";
    contactEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
    contactEvent.targetId = "9";
    contactEvent.sourceId = "1";
    contactEvent.lon = purpleBaseLon;
    contactEvent.lat = purpleBaseLat;
    addEvent(contactEvent);

    m_testStartTime = QDateTime::currentMSecsSinceEpoch();
    m_eventSecondPhase = false;
    m_eventCounter = 0;

    // Logger::info("Test data initialized (Red-Purple scenario), total platforms: %d, total events: %d", 
                 // getAllPlatforms().size(), getAllEvents().size());
}

void DataCache::updateTestData()
{
    QList<PlatformData> platforms = getAllPlatforms();
    
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_testStartTime;
    
    for (PlatformData &platform : platforms) {
        if (platform.type == "missile" && !platform.targetId.isEmpty()) {
            PlatformData target = getPlatform(platform.targetId);
            if (!target.id.isEmpty()) {
                double dx = target.lon - platform.lon;
                double dy = target.lat - platform.lat;
                double distance = sqrt(dx * dx + dy * dy);
                
                if (distance < 0.0005) {
                    removePlatform(platform.id);
                    continue;
                }
                
                double headingRad = atan2(dx, dy);
                double headingDeg = headingRad * 180.0 / 3.14159265358979323846;
                if (headingDeg < 0) headingDeg += 360.0;
                platform.heading = headingDeg;
                
                double moveDistance = (platform.speed / 111000.0) * 0.1;
                platform.lon += (dx / distance) * moveDistance;
                platform.lat += (dy / distance) * moveDistance;
            }
        } else {
            int idNum = platform.id.toInt();
            
            if (idNum >= 1 && idNum <= 8) {
                platform.lon += 0.0001;
                platform.lat += 0.00005;
            } else if (idNum >= 9 && idNum <= 14) {
                platform.lon -= 0.0001;
                platform.lat -= 0.00005;
            }
        }
        
        platform.updateTime = QDateTime::currentMSecsSinceEpoch();
        platform.validUntil = platform.updateTime + 5000;
        updatePlatform(platform);
    }

    if (elapsed >= 5000 && !m_eventSecondPhase) {
        m_eventSecondPhase = true;
        
        PlatformData shipSource = getPlatform("9");
        PlatformData planeSource = getPlatform("13");
        PlatformData target = getPlatform("1");
        
        double dx1 = target.lon - shipSource.lon;
        double dy1 = target.lat - shipSource.lat;
        double heading1 = atan2(dx1, dy1) * 180.0 / 3.14159265358979323846;
        if (heading1 < 0) heading1 += 360.0;
        
        double dx2 = target.lon - planeSource.lon;
        double dy2 = target.lat - planeSource.lat;
        double heading2 = atan2(dx2, dy2) * 180.0 / 3.14159265358979323846;
        if (heading2 < 0) heading2 += 360.0;
        
        PlatformData missile1;
        missile1.id = "M1";
        missile1.name = "Ship Missile";
        missile1.type = "missile";
        missile1.category = "anti-ship";
        missile1.camp = Camp_Purple;
        missile1.lon = shipSource.lon;
        missile1.lat = shipSource.lat;
        missile1.altitude = 0.0;
        missile1.speed = 100.0;
        missile1.heading = heading1;
        missile1.targetId = "1";
        missile1.dataStatus = DataStatus_Normal;
        missile1.updateTime = QDateTime::currentMSecsSinceEpoch();
        missile1.validUntil = missile1.updateTime + 30000;
        updatePlatform(missile1);
        // Logger::info("Missile M1 launched from Purple Ship 1 (id=9)");

        PlatformData missile2;
        missile2.id = "M2";
        missile2.name = "Air Missile";
        missile2.type = "missile";
        missile2.category = "air-to-surface";
        missile2.camp = Camp_Purple;
        missile2.lon = planeSource.lon;
        missile2.lat = planeSource.lat;
        missile2.altitude = 0.0;
        missile2.speed = 100.0;
        missile2.heading = heading2;
        missile2.targetId = "1";
        missile2.dataStatus = DataStatus_Normal;
        missile2.updateTime = QDateTime::currentMSecsSinceEpoch();
        missile2.validUntil = missile2.updateTime + 30000;
        updatePlatform(missile2);
        // Logger::info("Missile M2 launched from Purple Plane 1 (id=13)");

        SpecialEvent shipAttackEvent;
        shipAttackEvent.eventId = QString("EVENT_%1").arg(100 + ++m_eventCounter, 3, 10, QChar('0'));
        shipAttackEvent.eventType = Event_Attack;
        shipAttackEvent.eventName = "Missile Attack";
        shipAttackEvent.description = "Purple Ship 1 (id=9) launched missile at Red Ship 1 (id=1)";
        shipAttackEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
        shipAttackEvent.targetId = "1";
        shipAttackEvent.sourceId = "9";
        shipAttackEvent.lon = target.lon;
        shipAttackEvent.lat = target.lat;
        addEvent(shipAttackEvent);
        // Logger::info("Attack event added: Purple Ship 1 (id=9) attacked Red Ship 1 (id=1)");

        SpecialEvent planeAttackEvent;
        planeAttackEvent.eventId = QString("EVENT_%1").arg(100 + ++m_eventCounter, 3, 10, QChar('0'));
        planeAttackEvent.eventType = Event_Attack;
        planeAttackEvent.eventName = "Air-to-Surface Missile";
        planeAttackEvent.description = "Purple Plane 1 (id=13) launched missile at Red Ship 1 (id=1)";
        planeAttackEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
        planeAttackEvent.targetId = "1";
        planeAttackEvent.sourceId = "13";
        planeAttackEvent.lon = target.lon;
        planeAttackEvent.lat = target.lat;
        addEvent(planeAttackEvent);
        // Logger::info("Attack event added: Purple Plane 1 (id=13) attacked Red Ship 1 (id=1)");
    } else if (elapsed >= 15000 && m_eventSecondPhase) {
        m_eventSecondPhase = false;
        m_testStartTime = QDateTime::currentMSecsSinceEpoch();
        
        PlatformData target = getPlatform("1");
        
        SpecialEvent alertEvent;
        alertEvent.eventId = QString("EVENT_%1").arg(200 + ++m_eventCounter, 3, 10, QChar('0'));
        alertEvent.eventType = Event_Alert;
        alertEvent.eventName = "Incoming Missile Alert";
        alertEvent.description = "Red Ship 1 (id=1) detected incoming missiles from Purple forces";
        alertEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
        alertEvent.targetId = "1";
        alertEvent.sourceId = "SYSTEM";
        if (!target.id.isEmpty()) {
            alertEvent.lon = target.lon;
            alertEvent.lat = target.lat;
        }
        addEvent(alertEvent);
        // Logger::info("Alert event added: Missile alert for Red Ship 1 (id=1)");
    }
}
