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
      m_testEventsGenerated(false),
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
    //emit platformsUpdated(getAllPlatforms());
    //emit dynamicDataChanged(getAllData());
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
        
        // 单目标增量删除通知（供海图直接删除对应图元，无需全量对账）
        emit platformRemoved(id);
        emit platformsUpdated(getAllPlatforms());
        // 已停用全量广播：改由订阅者接收定向增量信号（platformRemoved 等）
        // emit dynamicDataChanged(getAllData());
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
 * @details 广播列表 m_dynamicData.events 按 eventId 去重：同一 id 只保留最新
 *          一条（覆盖旧值），避免测试/推送反复添加同 id 事件导致列表无限增长
 *          （该列表随每次信号深拷贝广播并用于画海图，是内存膨胀的根源）。
 *          历史列表 m_eventHistory 不设上限，完整保留所有事件供历史界面展示。
 */
bool DataCache::addEvent(const SpecialEvent &event)
{
    QWriteLocker locker(&m_dataLock);

    // 广播列表按 eventId 去重：若已存在同 id 事件则原地更新为最新数据，否则追加
    bool replaced = false;
    for (int i = 0; i < m_dynamicData.events.size(); ++i) {
        if (m_dynamicData.events[i].eventId == event.eventId) {
            m_dynamicData.events[i] = event;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        m_dynamicData.events.append(event);
    }

    // 历史列表完整保留（不设上限）：每次都记录，供事件历史界面展示全部记录
    m_eventHistory.prepend(event);

    m_dynamicData.timestamp = QDateTime::currentMSecsSinceEpoch();
    locker.unlock();
    
    emit eventAdded(event);
    // 已停用全量广播：事件新增改由订阅者接收定向 eventAdded 信号刷新
    // emit dynamicDataChanged(getAllData());
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
            // 已停用全量广播：事件移除改由订阅者接收定向 eventRemoved 信号刷新
            // emit dynamicDataChanged(getAllData());
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
    
    // 已停用全量广播：如需通知清空，应改用定向信号（当前无调用方）
    // emit dynamicDataChanged(getAllData());
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
    // 遍历所有平台，收集已过期平台ID（正常状态、有有效期、且已超过有效期）
    for (auto it = m_dynamicData.platforms.begin(); it != m_dynamicData.platforms.end(); ++it) {
        if (it.value().dataStatus == DataStatus_Normal &&
            it.value().validUntil > 0 &&
            it.value().validUntil < now) {
            expiredIds.append(it.key());
        }
    }
    // 真正从 map 删除过期平台数据，避免陈旧数据驻留并被后续 .values() 反复深拷贝
    for (const QString& id : expiredIds) {
        m_dynamicData.platforms.remove(id);
    }
    if (!expiredIds.isEmpty()) {
        m_dynamicData.timestamp = now;
        locker.unlock();

        for (const QString& id : expiredIds) {
            DataManager::instance()->onPlatformExpired(id);
            // 过期即视为该目标从海图消失，发出单目标增量删除通知
            emit platformRemoved(id);
        }

        emit platformsUpdated(getAllPlatforms());
        // 已停用全量广播：过期删除改由订阅者接收定向 platformRemoved 信号处理
        // emit dynamicDataChanged(getAllData());
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
    m_testEventsGenerated = false;
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
        // 每拍（1 秒）位移量：定时器为 1000ms（1Hz），每秒只更新一次。
        // 为使目标在海图上移动明显、变动够快，采用“较大的每拍经纬度位移”，
        // 并按航速做温和缩放（高速目标步长更大，但整体量级仍可见）。
        // 基准步长约 0.004 度/拍，各平台方向系数不同以形成不同航向。
        double moveSpeed = 0.02 + platform.speed * 0.00015;   // 舰船≈0.022，飞机≈0.155

        if (platform.camp == Camp_Mine) {
            if (platform.id == "1") {
                platform.lon += 0.20 * moveSpeed;
                platform.lat += 0.10 * moveSpeed;
            } else if (platform.id == "2") {
                platform.lon += 0.15 * moveSpeed;
                platform.lat += 0.15 * moveSpeed;
            } else if (platform.id == "3") {
                platform.lon += 0.30 * moveSpeed;
                platform.lat -= 0.05 * moveSpeed;
            }
        } else if (platform.camp == Camp_Enemy) {
            if (platform.id == "9") {
                platform.lon -= 0.18 * moveSpeed;
                platform.lat += 0.08 * moveSpeed;
            } else if (platform.id == "10") {
                platform.lon -= 0.12 * moveSpeed;
                platform.lat -= 0.10 * moveSpeed;
            } else if (platform.id == "13") {
                platform.lon -= 0.25 * moveSpeed;
                platform.lat += 0.10 * moveSpeed;
            }
        }

        platform.updateTime = now;
        // 演示级联删除：敌方潜艇（id=="10"）停止续期，约 10 秒后由
        // invalidateExpiredData 自动标记过期并发出 platformRemoved，
        // 海图据此级联删除其平台图元、传感器/武器范围、航迹、事件标记、
        // 属性框与连接线。其余平台正常续期，保持存活。
        if (platform.id != "9") {
            platform.validUntil = now + 10000;
        }

        updatePlatform(platform);
    }

    // 事件生成节奏：定时器为 1000ms（1Hz），每拍 m_eventCounter 加 1（≈1 秒）。
    // 仅在首周期（m_testEventsGenerated 为 false）生成事件，每个目标最多触发两次历史事件；
    // 首周期结束后置位 m_testEventsGenerated，后续周期只重置计数器，不再重复入历史，
    // 避免 m_eventHistory 无限增长。广播列表 m_dynamicData.events 首周期已建好，
    // 后续靠海图本地缓存继续显示。
    //   5/8/10/12/15/17 tick 各触发一次（≈对应秒），20 tick（≈20 秒）整轮重置。
    m_eventCounter++;
    if (!m_testEventsGenerated) {
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
        } else if (m_eventCounter == 8) {
            // 方式2 演示：地理标绘事件（targetId 留空，不关联任何平台）。
            // 事件按自身固定 lon/lat 在海图上以菱形标志 + 文字标签标绘，
            // 位置固定、不跟随平台，即便相关平台被删除也不受影响。
            SpecialEvent mineEvent;
            mineEvent.eventId = "geo_event1";
            mineEvent.eventType = Event_Alert;
            mineEvent.eventName = QString::fromLocal8Bit("Mine Detected");
            mineEvent.description = QString::fromLocal8Bit("Suspected mine field in this area");
            mineEvent.timestamp = now;
            mineEvent.targetId = "";                 // 关键：无目标平台 → 走地理标绘（方式2）
            mineEvent.sourceId = "";
            // 固定在红蓝双方之间的一处海域（介于 redBase 121.5 与 purpleBase 122.55 之间）
            mineEvent.lon = 122.0;
            mineEvent.lat = 31.25;
            addEvent(mineEvent);
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
        } else if (m_eventCounter == 12) {
            // 演示“受损”徽标（X）：给我方平台 "2" 发受损事件（Event_Damage）。
            // 海图 updatePlatformStatusStrip 会据该平台最新事件类型派生受损状态，
            // 在其图标上方显示橙红色 "X" 徽标。
            SpecialEvent damageEvent;
            damageEvent.eventId = "event_damage_2";
            damageEvent.eventType = Event_Damage;
            damageEvent.eventName = QString::fromLocal8Bit("Hull Damage");
            damageEvent.description = QString::fromLocal8Bit("Platform sustained battle damage");
            damageEvent.timestamp = now;
            damageEvent.targetId = "2";
            damageEvent.sourceId = "13";
            PlatformData target = getPlatform("2");
            damageEvent.lon = target.lon;
            damageEvent.lat = target.lat;
            addEvent(damageEvent);
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
        } else if (m_eventCounter == 17) {
            // 演示“通信中断”徽标（N）：给敌方平台 "10" 发失联事件（Event_Lost）。
            // 海图据该平台最新事件类型派生通信中断状态，在其图标上方显示品红 "N" 徽标。
            SpecialEvent lostEvent;
            lostEvent.eventId = "event_lost_10";
            lostEvent.eventType = Event_Lost;
            lostEvent.eventName = QString::fromLocal8Bit("Comms Lost");
            lostEvent.description = QString::fromLocal8Bit("Lost communication with platform");
            lostEvent.timestamp = now;
            lostEvent.targetId = "10";
            lostEvent.sourceId = "";
            PlatformData target = getPlatform("10");
            lostEvent.lon = target.lon;
            lostEvent.lat = target.lat;
            addEvent(lostEvent);
        }
    }
    // 计数器整轮重置（始终执行，保证周期循环）；首周期结束时置位“已生成”标志
    if (m_eventCounter >= 20) {
        m_eventCounter = 0;
        m_testEventsGenerated = true;
    }
}
