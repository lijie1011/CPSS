/**
 * @file datamanager.cpp
 * @brief 数据管理器类实现
 * @details 该类采用单例模式，作为数据层的核心管理组件，负责管理多个协议适配器，
 *          接收和分发平台、事件数据，提供统一的数据访问接口，并支持回调机制。
 * @date 2026-07-28
 */

#include "datamanager.h"
#include <QJsonArray>
#include "logger.h"

// 静态成员变量初始化
DataManager* DataManager::s_instance = nullptr;
QMutex DataManager::s_mutex;

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
DataManager::DataManager(QObject *parent)
    : QObject(parent),
      m_defaultValidDuration(5000)
{
    m_priorityMap[Protocol_HTTP] = 5;
    m_priorityMap[Protocol_Unknown] = 0;
    
    DataCache* cache = DataCache::instance();
    connect(cache, &DataCache::dynamicDataChanged,
            this, &DataManager::dynamicDataChanged);
    connect(cache, &DataCache::platformUpdated,
            this, &DataManager::platformUpdated);
    connect(cache, &DataCache::platformsUpdated,
            this, &DataManager::platformsUpdated);
}

/**
 * @brief 注册平台更新回调
 * @param callback 回调函数
 */
void DataManager::registerPlatformUpdateCallback(PlatformUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformUpdateCallback = callback;
}

/**
 * @brief 注册数据更新回调
 * @param callback 回调函数
 */
void DataManager::registerDataUpdateCallback(DataUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataUpdateCallback = callback;
}

/**
 * @brief 注册事件更新回调
 * @param callback 回调函数
 */
void DataManager::registerEventUpdateCallback(EventUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_eventUpdateCallback = callback;
}

/**
 * @brief 注册平台过期回调
 * @param callback 回调函数
 */
void DataManager::registerPlatformExpiredCallback(PlatformExpiredCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformExpiredCallback = callback;
}

/**
 * @brief 注销平台更新回调
 */
void DataManager::unregisterPlatformUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformUpdateCallback = nullptr;
}

/**
 * @brief 注销数据更新回调
 */
void DataManager::unregisterDataUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataUpdateCallback = nullptr;
}

/**
 * @brief 注销事件更新回调
 */
void DataManager::unregisterEventUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_eventUpdateCallback = nullptr;
}

/**
 * @brief 注销平台过期回调
 */
void DataManager::unregisterPlatformExpiredCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformExpiredCallback = nullptr;
}

/**
 * @brief 平台过期处理
 * @param id 过期平台的ID
 */
void DataManager::onPlatformExpired(const QString &id)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    if (m_platformExpiredCallback) {
        m_platformExpiredCallback(id);
    }
}

/**
 * @brief 析构函数
 */
DataManager::~DataManager()
{
    stopAllAdapters();
}

/**
 * @brief 获取数据管理器单例实例
 * @return DataManager指针
 */
DataManager* DataManager::instance()
{
    if (!s_instance) {
        QMutexLocker locker(&s_mutex);
        if (!s_instance) {
            s_instance = new DataManager();
        }
    }
    return s_instance;
}

/**
 * @brief 添加协议适配器
 * @param adapter 协议适配器指针
 */
void DataManager::addAdapter(IProtocolAdapter *adapter)
{
    if (!m_adapters.contains(adapter)) {
        m_adapters.append(adapter);
        connect(adapter, &IProtocolAdapter::dataReceived,
                this, &DataManager::onDataReceived);
    }
}

/**
 * @brief 移除协议适配器
 * @param adapter 协议适配器指针
 */
void DataManager::removeAdapter(IProtocolAdapter *adapter)
{
    m_adapters.removeOne(adapter);
    disconnect(adapter, &IProtocolAdapter::dataReceived,
               this, &DataManager::onDataReceived);
}

/**
 * @brief 启动所有适配器
 */
void DataManager::startAllAdapters()
{
    for (auto adapter : m_adapters) {
        adapter->start();
    }
}

/**
 * @brief 停止所有适配器
 */
void DataManager::stopAllAdapters()
{
    for (auto adapter : m_adapters) {
        adapter->stop();
    }
}

/**
 * @brief 设置数据源优先级
 * @param type 协议类型
 * @param priority 优先级值
 */
void DataManager::setDataSourcePriority(ProtocolType type, int priority)
{
    m_priorityMap[type] = priority;
}

/**
 * @brief 获取数据源优先级
 * @param type 协议类型
 * @return 优先级值
 */
int DataManager::getDataSourcePriority(ProtocolType type) const
{
    return m_priorityMap.value(type, 0);
}

/**
 * @brief 设置默认数据有效期
 * @param ms 有效期（毫秒）
 */
void DataManager::setDefaultValidDuration(qint64 ms)
{
    m_defaultValidDuration = ms;
}

/**
 * @brief 获取默认数据有效期
 * @return 有效期（毫秒）
 */
qint64 DataManager::defaultValidDuration() const
{
    return m_defaultValidDuration;
}

/**
 * @brief 注册数据推送回调
 * @param callback 回调函数
 */
void DataManager::registerDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.push_back(callback);
}

/**
 * @brief 注销数据推送回调
 * @param callback 回调函数
 */
void DataManager::unregisterDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.clear();
}

/**
 * @brief 启动数据推送
 * @param intervalMs 推送间隔（毫秒）
 */
void DataManager::startDataPush(int intervalMs)
{
    if (m_pushTimer.isActive()) {
        m_pushTimer.stop();
    }
    connect(&m_pushTimer, &QTimer::timeout, this, &DataManager::pushData);
    m_pushTimer.start(intervalMs);
}

/**
 * @brief 停止数据推送
 */
void DataManager::stopDataPush()
{
    m_pushTimer.stop();
}

/**
 * @brief 处理接收到的数据
 * @param data JSON格式的数据
 * @param source 数据来源协议类型
 */
void DataManager::onDataReceived(const QJsonObject &data, ProtocolType source)
{
    parseAndUpdate(data, source);
}

/**
 * @brief 推送数据
 */
void DataManager::pushData()
{
    static int pushCount = 0;
    pushCount++;
    
    DynamicObjects data = DataCache::instance()->getAllData();
    
    for (const auto &callback : m_pushCallbacks) {
        callback(data);
    }
    
    emit dataPushed(data);
}

/**
 * @brief 解析并更新数据
 * @param data JSON格式的数据
 * @param source 数据来源协议类型
 */
void DataManager::parseAndUpdate(const QJsonObject &data, ProtocolType source)
{
    QString type = data["type"].toString();

    if (type == "platform") {
        updatePlatform(data, source);
    } else if (type == "event") {
        updateEvent(data, source);
    } else if (type == "batch") {
        QJsonArray items = data["items"].toArray();
        for (const auto &item : items) {
            if (item.isObject()) {
                parseAndUpdate(item.toObject(), source);
            }
        }
    }
}

/**
 * @brief 更新平台数据
 * @param obj JSON格式的平台对象
 * @param source 数据来源协议类型
 */
void DataManager::updatePlatform(const QJsonObject &obj, ProtocolType source)
{
    QString id = obj["id"].toString();
    if (id.isEmpty()) {
        return;
    }

    PlatformData platform;
    platform.id = id;
    platform.name = obj["name"].toString();
    platform.lon = obj["lon"].toDouble();
    platform.lat = obj["lat"].toDouble();
    platform.altitude = obj["altitude"].toDouble();
    platform.speed = obj["speed"].toDouble();
    platform.type = obj["platformType"].toString();
    if (platform.type.isEmpty()) platform.type = obj["type"].toString();
    platform.category = obj["category"].toString();
    
    QString campStr = obj["camp"].toString().toLower();
    if (campStr == "friendly") platform.camp = Camp_Friendly;
    else if (campStr == "red") platform.camp = Camp_Red;
    else if (campStr == "purple") platform.camp = Camp_Purple;
    else if (campStr == "enemy") platform.camp = Camp_Enemy;
    else if (campStr == "neutral") platform.camp = Camp_Neutral;
    else platform.camp = Camp_Unknown;

    QJsonArray weaponsArray = obj["weapons"].toArray();
    for (const auto &weapon : weaponsArray) {
        QJsonObject w = weapon.toObject();
        WeaponInfo wi = {w["type"].toString(), w["count"].toInt(), w["range"].toDouble()};
        platform.weapons.append(wi);
    }

    QJsonArray sensorsArray = obj["sensors"].toArray();
    for (const auto &sensor : sensorsArray) {
        QJsonObject s = sensor.toObject();
        SensorInfo si = {s["type"].toString(), s["count"].toInt(), s["range"].toDouble()};
        platform.sensors.append(si);
    }

    platform.dataStatus = DataStatus_Normal;
    platform.updateTime = QDateTime::currentMSecsSinceEpoch();
    platform.validUntil = platform.updateTime + 
        (obj.contains("validDuration") ? obj["validDuration"].toInt() : m_defaultValidDuration);
    platform.sourceProtocol = source;

    DataCache::instance()->updatePlatform(platform);

    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_platformUpdateCallback) {
            m_platformUpdateCallback(platform);
        }
        if (m_dataUpdateCallback) {
            m_dataUpdateCallback(getAllData());
        }
    }

    emit platformUpdated(platform);
    emit platformsUpdated(getAllPlatforms());
    emit dynamicDataChanged(getAllData());
}

/**
 * @brief 更新事件数据
 * @param obj JSON格式的事件对象
 * @param source 数据来源协议类型
 */
void DataManager::updateEvent(const QJsonObject &obj, ProtocolType source)
{
    Q_UNUSED(source);
    
    QString eventId = obj["eventId"].toString();
    if (eventId.isEmpty()) {
        return;
    }

    SpecialEvent event;
    event.eventId = eventId;

    QString eventTypeStr = obj["eventType"].toString().toLower();
    if (eventTypeStr == "attack") event.eventType = Event_Attack;
    else if (eventTypeStr == "defense") event.eventType = Event_Defense;
    else if (eventTypeStr == "alert") event.eventType = Event_Alert;
    else if (eventTypeStr == "contact") event.eventType = Event_Contact;
    else if (eventTypeStr == "damage") event.eventType = Event_Damage;
    else if (eventTypeStr == "missionstart") event.eventType = Event_MissionStart;
    else if (eventTypeStr == "missionend") event.eventType = Event_MissionEnd;
    else if (eventTypeStr == "lost") event.eventType = Event_Lost;
    else if (eventTypeStr == "repair") event.eventType = Event_Repair;
    else if (eventTypeStr == "custom") event.eventType = Event_Custom;
    else event.eventType = Event_Unknown;

    event.eventName = obj["eventName"].toString();
    event.description = obj["description"].toString();
    event.timestamp = static_cast<qint64>(obj["timestamp"].toDouble());
    if (event.timestamp == 0) {
        event.timestamp = QDateTime::currentMSecsSinceEpoch();
    }
    event.targetId = obj["targetId"].toString();
    event.sourceId = obj["sourceId"].toString();
    event.lon = obj["lon"].toDouble();
    event.lat = obj["lat"].toDouble();

    if (obj.contains("extraData") && obj["extraData"].isObject()) {
        event.extraData = obj["extraData"].toObject();
    }

    DataCache::instance()->addEvent(event);

    {
        std::lock_guard<std::mutex> lock(m_callbackMutex);
        if (m_eventUpdateCallback) {
            m_eventUpdateCallback(event);
        }
    }
}

/**
 * @brief 获取所有平台
 * @return 平台列表
 */
QList<PlatformData> DataManager::getAllPlatforms() const
{
    return DataCache::instance()->getAllPlatforms();
}

/**
 * @brief 获取有效平台
 * @return 有效平台列表
 */
QList<PlatformData> DataManager::getValidPlatforms() const
{
    return DataCache::instance()->getValidPlatforms();
}

/**
 * @brief 获取指定平台
 * @param id 平台ID
 * @return 平台数据
 */
PlatformData DataManager::getPlatform(const QString &id) const
{
    return DataCache::instance()->getPlatform(id);
}

/**
 * @brief 获取所有事件
 * @return 事件列表
 */
QList<SpecialEvent> DataManager::getAllEvents() const
{
    return DataCache::instance()->getAllEvents();
}

/**
 * @brief 获取事件历史
 * @return 事件历史列表
 */
QList<SpecialEvent> DataManager::getEventHistory() const
{
    return DataCache::instance()->getEventHistory();
}

/**
 * @brief 获取所有数据
 * @return 动态对象集合
 */
DynamicObjects DataManager::getAllData() const
{
    return DataCache::instance()->getAllData();
}

/**
 * @brief 启动测试数据定时器
 * @param intervalMs 间隔时间（毫秒）
 */
void DataManager::startTestDataTimer(int intervalMs)
{
    DataCache::instance()->startTestDataTimer(intervalMs);
}

/**
 * @brief 停止测试数据定时器
 */
void DataManager::stopTestDataTimer()
{
    DataCache::instance()->stopTestDataTimer();
}
