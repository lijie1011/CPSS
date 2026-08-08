/**
 * @file datacache.cpp
 * @brief 数据缓存类实现
 * @details 本类实现单例模式，负责缓存所有动态数据（平台、事件等）。
 *          它提供线程安全的数据访问接口，并在数据变更时发射信号通知。
 *          同时支持测试数据生成和过期数据清理。
 * @date 2026-07-28
 */

#include "datacache.h"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include "logger.h"
#include "datamanager.h"

// 静态成员变量初始化
DataCache* DataCache::s_instance = nullptr;
QMutex DataCache::s_mutex;

/**
 * @brief 获取数据缓存单例实例
 * @return DataCache指针
 */
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

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
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

/**
 * @brief 析构函数
 */
DataCache::~DataCache()
{
    stopDataPush();
    stopTestDataTimer();
}

/**
 * @brief 更新平台数据
 * @param data 平台数据
 * @return true 表示更新成功
 */
bool DataCache::updatePlatform(const PlatformData &data)
{
    QWriteLocker locker(&m_dataLock);
    
    PlatformData newData = data;
    if (m_dynamicData.platforms.contains(data.id)) {
        // 保留已有航迹点
        PlatformData existing = m_dynamicData.platforms[data.id];
        newData.trackPoints = existing.trackPoints;
        // 若位置发生变化，则追加旧位置到航迹
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
    return true;
}

/**
 * @brief 移除平台
 * @param id 平台ID
 * @return true 表示已移除，false 表示未找到
 */
bool DataCache::removePlatform(const QString &id)
{
    QWriteLocker locker(&m_dataLock);
    if (m_dynamicData.platforms.contains(id)) {
        m_dynamicData.platforms.remove(id);
        m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
        locker.unlock();
        
        emit platformsUpdated(getAllPlatforms());
        emit dynamicDataChanged(getAllData());
        return true;
    }
    return false;
}

/**
 * @brief 获取指定平台
 * @param id 平台ID
 * @return 平台数据
 */
PlatformData DataCache::getPlatform(const QString &id) const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.platforms.value(id);
}

/**
 * @brief 获取所有平台
 * @return 平台列表
 */
QList<PlatformData> DataCache::getAllPlatforms() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.platforms.values();
}

/**
 * @brief 获取有效平台（未过期）
 * @return 有效平台列表
 */
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

/**
 * @brief 添加事件
 * @param event 事件数据
 * @return true 表示已添加
 */
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
    return true;
}

/**
 * @brief 移除事件
 * @param eventId 事件ID
 * @return true 表示已移除，false 表示未找到
 */
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
            return true;
        }
    }
    return false;
}

/**
 * @brief 清空所有事件
 */
void DataCache::clearEvents()
{
    QWriteLocker locker(&m_dataLock);
    m_dynamicData.events.clear();
    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    emit dynamicDataChanged(getAllData());
}

/**
 * @brief 获取所有事件
 * @return 事件列表
 */
QList<SpecialEvent> DataCache::getAllEvents() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.events;
}

/**
 * @brief 获取事件历史记录
 * @return 事件历史列表
 */
QList<SpecialEvent> DataCache::getEventHistory() const
{
    QReadLocker locker(&m_dataLock);
    return m_eventHistory;
}

/**
 * @brief 清空事件历史记录
 */
void DataCache::clearEventHistory()
{
    QWriteLocker locker(&m_dataLock);
    m_eventHistory.clear();
}

/**
 * @brief 获取最大历史记录数量
 * @return 最大历史大小
 */
int DataCache::getMaxHistorySize() const
{
    return m_maxHistorySize;
}

/**
 * @brief 设置最大历史记录数量
 * @param size 最大历史大小
 */
void DataCache::setMaxHistorySize(int size)
{
    QWriteLocker locker(&m_dataLock);
    m_maxHistorySize = size;
    while (m_eventHistory.size() > m_maxHistorySize) {
        m_eventHistory.removeLast();
    }
}

/**
 * @brief 获取所有数据
 * @return 动态对象集合
 */
DynamicObjects DataCache::getAllData() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData;
}

/**
 * @brief 清理过期数据
 */
void DataCache::invalidateExpiredData()
{
    QWriteLocker locker(&m_dataLock);
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    QStringList expiredIds;
    // 遍历所有平台，标记已过期的平台
    for (auto it = m_dynamicData.platforms.begin(); it != m_dynamicData.platforms.end(); ++it) {
        // 正常状态、有有效期、且已超过有效期
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
    }
}

/**
 * @brief 获取时间戳
 * @return 最后更新时间戳
 */
qint64 DataCache::getTimestamp() const
{
    QReadLocker locker(&m_dataLock);
    return m_dynamicData.timestamp;
}

/**
 * @brief 注册数据推送回调
 * @param callback 回调函数
 */
void DataCache::registerDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.push_back(callback);
}

/**
 * @brief 注销数据推送回调
 * @param callback 回调函数
 */
void DataCache::unregisterDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.clear();
}

/**
 * @brief 启动数据推送
 * @param intervalMs 推送间隔（毫秒）
 */
void DataCache::startDataPush(int intervalMs)
{
    if (m_pushTimer.isActive()) {
        m_pushTimer.stop();
    }
    disconnect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    connect(&m_pushTimer, &QTimer::timeout, this, &DataCache::pushData);
    m_pushTimer.start(intervalMs);
}

/**
 * @brief 停止数据推送
 */
void DataCache::stopDataPush()
{
    m_pushTimer.stop();
}

/**
 * @brief 检查推送是否正在运行
 * @return true 表示正在运行
 */
bool DataCache::isPushRunning() const
{
    return m_pushTimer.isActive();
}

/**
 * @brief 启动测试数据定时器
 * @param intervalMs 间隔时间（毫秒）
 */
void DataCache::startTestDataTimer(int intervalMs)
{
    initTestData();
    if (m_testDataTimer.isActive()) {
        m_testDataTimer.stop();
    }
    disconnect(&m_testDataTimer, &QTimer::timeout, this, &DataCache::updateTestData);
    connect(&m_testDataTimer, &QTimer::timeout, this, &DataCache::updateTestData);
    m_testDataTimer.start(intervalMs);
}

/**
 * @brief 停止测试数据定时器
 */
void DataCache::stopTestDataTimer()
{
    m_testDataTimer.stop();
}

/**
 * @brief 推送数据
 */
void DataCache::pushData()
{
    static int pushCount = 0;
    pushCount++;
    DynamicObjects data = getAllData();
    for (const auto &callback : m_pushCallbacks) {
        callback(data);
    }
    emit dataPushed(data);
}

/**
 * @brief 初始化测试数据
 */
void DataCache::initTestData()
{
    double redBaseLon = 121.5;
    double redBaseLat = 31.2;
    double purpleBaseLon = redBaseLon + 1.05;
    double purpleBaseLat = redBaseLat;

    PlatformData redShip1;
    redShip1.id = "1";
    redShip1.name = "Red Ship 1";
    redShip1.lon = redBaseLon;
    redShip1.lat = redBaseLat;
    redShip1.altitude = 0.0;
    redShip1.speed = 12.0;
    redShip1.type = "destroyer";
    redShip1.category = "battleship";
    redShip1.camp = Camp_Mine;
    redShip1.dataStatus = DataStatus_Normal;
    redShip1.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip1.validUntil = redShip1.updateTime + 10000;

    WeaponInfo redWeapon1 = {"missile", 8, 50.0};
    WeaponInfo redWeapon2 = {"gun", 16, 15.0};
    redShip1.weapons.append(redWeapon1);
    redShip1.weapons.append(redWeapon2);

    SensorInfo redSensor1 = {"radar", 1, 80.0};
    SensorInfo redSensor2 = {"sonar", 1, 30.0};
    redShip1.sensors.append(redSensor1);
    redShip1.sensors.append(redSensor2);

    updatePlatform(redShip1);

    PlatformData redShip2;
    redShip2.id = "2";
    redShip2.name = "Red Ship 2";
    redShip2.lon = redBaseLon + 0.1;
    redShip2.lat = redBaseLat + 0.05;
    redShip2.altitude = 0.0;
    redShip2.speed = 15.0;
    redShip2.type = "destroyer";
    redShip2.category = "destroyer";
    redShip2.camp = Camp_Mine;
    redShip2.dataStatus = DataStatus_Normal;
    redShip2.updateTime = QDateTime::currentMSecsSinceEpoch();
    redShip2.validUntil = redShip2.updateTime + 10000;
    updatePlatform(redShip2);

    PlatformData redFighter1;
    redFighter1.id = "3";
    redFighter1.name = "Red Fighter 1";
    redFighter1.lon = redBaseLon + 0.05;
    redFighter1.lat = redBaseLat + 0.2;
    redFighter1.altitude = 8000.0;
    redFighter1.speed = 900.0;
    redFighter1.type = "fighter";
    redFighter1.category = "fighter";
    redFighter1.camp = Camp_Mine;
    redFighter1.dataStatus = DataStatus_Normal;
    redFighter1.updateTime = QDateTime::currentMSecsSinceEpoch();
    redFighter1.validUntil = redFighter1.updateTime + 10000;
    updatePlatform(redFighter1);

    PlatformData purpleShip1;
    purpleShip1.id = "9";
    purpleShip1.name = "Purple Ship 1";
    purpleShip1.lon = purpleBaseLon;
    purpleShip1.lat = purpleBaseLat;
    purpleShip1.altitude = 0.0;
    purpleShip1.speed = 18.0;
    purpleShip1.type = "frigate";
    purpleShip1.category = "cruiser";
    purpleShip1.camp = Camp_Enemy;
    purpleShip1.dataStatus = DataStatus_Normal;
    purpleShip1.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip1.validUntil = purpleShip1.updateTime + 10000;

    WeaponInfo purpleWeapon1 = {"missile", 12, 60.0};
    purpleShip1.weapons.append(purpleWeapon1);

    SensorInfo purpleSensor1 = {"radar", 1, 100.0};
    purpleShip1.sensors.append(purpleSensor1);

    updatePlatform(purpleShip1);

    PlatformData purpleShip2;
    purpleShip2.id = "10";
    purpleShip2.name = "Purple Ship 2";
    purpleShip2.lon = purpleBaseLon - 0.08;
    purpleShip2.lat = purpleBaseLat + 0.06;
    purpleShip2.altitude = 0.0;
    purpleShip2.speed = 14.0;
    purpleShip2.type = "submarine";
    purpleShip2.category = "submarine";
    purpleShip2.camp = Camp_Enemy;
    purpleShip2.dataStatus = DataStatus_Normal;
    purpleShip2.updateTime = QDateTime::currentMSecsSinceEpoch();
    purpleShip2.validUntil = purpleShip2.updateTime + 10000;
    updatePlatform(purpleShip2);

    PlatformData purplePlane1;
    purplePlane1.id = "13";
    purplePlane1.name = "Purple Plane 1";
    purplePlane1.lon = purpleBaseLon - 0.1;
    purplePlane1.lat = purpleBaseLat - 0.15;
    purplePlane1.altitude = 6000.0;
    purplePlane1.speed = 850.0;
    purplePlane1.type = "bomber";
    purplePlane1.category = "bomber";
    purplePlane1.camp = Camp_Enemy;
    purplePlane1.dataStatus = DataStatus_Normal;
    purplePlane1.updateTime = QDateTime::currentMSecsSinceEpoch();
    purplePlane1.validUntil = purplePlane1.updateTime + 10000;
    updatePlatform(purplePlane1);

    // 敌方巡航导弹平台：导弹同样是一个平台，应按 type 显示导弹图标而非小圆点
    PlatformData enemyMissile1;
    enemyMissile1.id = "20";
    enemyMissile1.name = "Enemy Missile 1";
    enemyMissile1.lon = purpleBaseLon - 0.05;      // 位于敌方阵营区域附近
    enemyMissile1.lat = purpleBaseLat + 0.12;
    enemyMissile1.altitude = 1200.0;               // 巡航高度
    enemyMissile1.speed = 1000.0;                  // 高速飞行
    enemyMissile1.heading = 250.0;                 // 朝我方方向飞行
    enemyMissile1.type = "cruise_sea";             // 海基巡航导弹 → 海基巡航导弹.png
    enemyMissile1.category = "missile";
    enemyMissile1.camp = Camp_Enemy;               // 敌方（蓝色着色）
    enemyMissile1.dataStatus = DataStatus_Normal;
    enemyMissile1.updateTime = QDateTime::currentMSecsSinceEpoch();
    enemyMissile1.validUntil = enemyMissile1.updateTime + 10000;
    updatePlatform(enemyMissile1);

    SpecialEvent contactEvent;
    contactEvent.eventId = "event1";
    contactEvent.eventType = Event_Contact;
    contactEvent.eventName = QString::fromLocal8Bit("Purple Detected");
    contactEvent.description = QString::fromLocal8Bit("Purple forces detected in sector");
    contactEvent.timestamp = QDateTime::currentMSecsSinceEpoch();
    contactEvent.targetId = "1";
    contactEvent.sourceId = "9";
    contactEvent.lon = redBaseLon;
    contactEvent.lat = redBaseLat;
    addEvent(contactEvent);

    m_testStartTime = QDateTime::currentMSecsSinceEpoch();
    m_eventSecondPhase = false;
    m_eventCounter = 0;
}

/**
 * @brief 更新测试数据
 */
void DataCache::updateTestData()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double elapsed = (now - m_testStartTime) / 1000.0;

    QList<PlatformData> platforms = getAllPlatforms();
    for (PlatformData &platform : platforms) {
        double moveSpeed = platform.speed / 3600.0;

        if (platform.camp == Camp_Mine) {
            if (platform.id == "1") {
                platform.lon += 0.00002 * moveSpeed;
                platform.lat += 0.00001 * moveSpeed;
            } else if (platform.id == "2") {
                platform.lon += 0.000015 * moveSpeed;
                platform.lat += 0.000015 * moveSpeed;
            } else if (platform.id == "3") {
                platform.lon += 0.00003 * moveSpeed;
                platform.lat -= 0.000005 * moveSpeed;
            }
        } else if (platform.camp == Camp_Enemy) {
            if (platform.id == "9") {
                platform.lon -= 0.000018 * moveSpeed;
                platform.lat += 0.000008 * moveSpeed;
            } else if (platform.id == "10") {
                platform.lon -= 0.000012 * moveSpeed;
                platform.lat -= 0.00001 * moveSpeed;
            } else if (platform.id == "13") {
                platform.lon -= 0.000025 * moveSpeed;
                platform.lat += 0.00001 * moveSpeed;
            }
        }

        platform.updateTime = now;
        platform.validUntil = now + 10000;

        updatePlatform(platform);
    }

    m_eventCounter++;
    if (m_eventCounter == 5) {
        SpecialEvent shipAttackEvent;
        shipAttackEvent.eventId = "event2";
        shipAttackEvent.eventType = Event_Attack;
        shipAttackEvent.eventName = QString::fromLocal8Bit("Air-to-Surface Missile");
        shipAttackEvent.description = QString::fromLocal8Bit("Missile launched from aircraft");
        shipAttackEvent.timestamp = now;
        shipAttackEvent.targetId = "1";
        shipAttackEvent.sourceId = "13";
        PlatformData target = getPlatform("1");
        shipAttackEvent.lon = target.lon;
        shipAttackEvent.lat = target.lat;
        addEvent(shipAttackEvent);
    } else if (m_eventCounter == 10) {
        SpecialEvent planeAttackEvent;
        planeAttackEvent.eventId = "event3";
        planeAttackEvent.eventType = Event_Attack;
        planeAttackEvent.eventName = QString::fromLocal8Bit("Missile Attack");
        planeAttackEvent.description = QString::fromLocal8Bit("Anti-ship missile attack");
        planeAttackEvent.timestamp = now;
        planeAttackEvent.targetId = "9";
        planeAttackEvent.sourceId = "3";
        PlatformData target = getPlatform("9");
        planeAttackEvent.lon = target.lon;
        planeAttackEvent.lat = target.lat;
        addEvent(planeAttackEvent);
    } else if (m_eventCounter == 15) {
        SpecialEvent alertEvent;
        alertEvent.eventId = "event4";
        alertEvent.eventType = Event_Alert;
        alertEvent.eventName = QString::fromLocal8Bit("Threat Alert");
        alertEvent.description = QString::fromLocal8Bit("Incoming missile detected");
        alertEvent.timestamp = now;
        alertEvent.targetId = "9";
        alertEvent.sourceId = "1";
        PlatformData target = getPlatform("9");
        alertEvent.lon = target.lon;
        alertEvent.lat = target.lat;
        addEvent(alertEvent);
    } else if (m_eventCounter >= 20) {
        m_eventCounter = 0;
    }
}
