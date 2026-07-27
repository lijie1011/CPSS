#include "datamanager.h"
#include <QJsonArray>
#include "logger.h"

DataManager* DataManager::s_instance = nullptr;
QMutex DataManager::s_mutex;

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

void DataManager::registerPlatformUpdateCallback(PlatformUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformUpdateCallback = callback;
    // Logger::info("Platform update callback registered");
}

void DataManager::registerDataUpdateCallback(DataUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataUpdateCallback = callback;
    // Logger::info("Data update callback registered");
}

void DataManager::registerEventUpdateCallback(EventUpdateCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_eventUpdateCallback = callback;
    // Logger::info("Event update callback registered");
}

void DataManager::registerPlatformExpiredCallback(PlatformExpiredCallback callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformExpiredCallback = callback;
    // Logger::info("Platform expired callback registered");
}

void DataManager::unregisterPlatformUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformUpdateCallback = nullptr;
    // Logger::info("Platform update callback unregistered");
}

void DataManager::unregisterDataUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_dataUpdateCallback = nullptr;
    // Logger::info("Data update callback unregistered");
}

void DataManager::unregisterEventUpdateCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_eventUpdateCallback = nullptr;
    // Logger::info("Event update callback unregistered");
}

void DataManager::unregisterPlatformExpiredCallback()
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_platformExpiredCallback = nullptr;
    // Logger::info("Platform expired callback unregistered");
}

void DataManager::onPlatformExpired(const QString &id)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    if (m_platformExpiredCallback) {
        m_platformExpiredCallback(id);
    }
    // Logger::info("Platform expired callback triggered for: %s", id.toStdString().c_str());
}

DataManager::~DataManager()
{
    stopAllAdapters();
}

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

void DataManager::addAdapter(IProtocolAdapter *adapter)
{
    if (!m_adapters.contains(adapter)) {
        m_adapters.append(adapter);
        connect(adapter, &IProtocolAdapter::dataReceived,
                this, &DataManager::onDataReceived);
        // Logger::info("Adapter added: %s", typeid(*adapter).name());
    }
}

void DataManager::removeAdapter(IProtocolAdapter *adapter)
{
    m_adapters.removeOne(adapter);
    disconnect(adapter, &IProtocolAdapter::dataReceived,
               this, &DataManager::onDataReceived);
    // Logger::info("Adapter removed: %s", typeid(*adapter).name());
}

void DataManager::startAllAdapters()
{
    for (auto adapter : m_adapters) {
        adapter->start();
    }
    // Logger::info("All adapters started, count: %d", m_adapters.size());
}

void DataManager::stopAllAdapters()
{
    for (auto adapter : m_adapters) {
        adapter->stop();
    }
    // Logger::info("All adapters stopped");
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

void DataManager::registerDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.push_back(callback);
    // Logger::info("Data push callback registered, total: %d", m_pushCallbacks.size());
}

void DataManager::unregisterDataPushCallback(DataPushCallback callback)
{
    m_pushCallbacks.clear();
    // Logger::info("Data push callback unregistered");
}

void DataManager::startDataPush(int intervalMs)
{
    if (m_pushTimer.isActive()) {
        m_pushTimer.stop();
    }
    connect(&m_pushTimer, &QTimer::timeout, this, &DataManager::pushData);
    m_pushTimer.start(intervalMs);
    // Logger::info("Data push started with interval: %d ms", intervalMs);
}

void DataManager::stopDataPush()
{
    m_pushTimer.stop();
    // Logger::info("Data push stopped");
}

void DataManager::onDataReceived(const QJsonObject &data, ProtocolType source)
{
    parseAndUpdate(data, source);
}

void DataManager::pushData()
{
    static int pushCount = 0;
    pushCount++;
    
    DynamicObjects data = DataCache::instance()->getAllData();
    // Logger::info("pushData: count=%d, platforms=%d", pushCount, data.platforms.size());

    for (const auto &callback : m_pushCallbacks) {
        callback(data);
    }

    emit dataPushed(data);
}

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

void DataManager::updatePlatform(const QJsonObject &obj, ProtocolType source)
{
    QString id = obj["id"].toString();
    if (id.isEmpty()) {
        // Logger::warn("Platform update failed: id is empty");
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

    // Logger::info("Platform updated via DataManager: id=%s", id.toStdString().c_str());
}

void DataManager::updateEvent(const QJsonObject &obj, ProtocolType source)
{
    Q_UNUSED(source);
    
    QString eventId = obj["eventId"].toString();
    if (eventId.isEmpty()) {
        // Logger::warn("Event update failed: eventId is empty");
        return;
    }

    SpecialEvent event;
    event.eventId = eventId;
    
    QString eventTypeStr = obj["eventType"].toString().toLower();
    if (eventTypeStr == "attack") event.eventType = Event_Attack;
    else if (eventTypeStr == "defense") event.eventType = Event_Defense;
    else if (eventTypeStr == "alert") event.eventType = Event_Alert;
    else if (eventTypeStr == "missionstart") event.eventType = Event_MissionStart;
    else if (eventTypeStr == "missionend") event.eventType = Event_MissionEnd;
    else if (eventTypeStr == "contact") event.eventType = Event_Contact;
    else if (eventTypeStr == "lost") event.eventType = Event_Lost;
    else if (eventTypeStr == "damage") event.eventType = Event_Damage;
    else if (eventTypeStr == "repair") event.eventType = Event_Repair;
    else if (eventTypeStr == "custom") event.eventType = Event_Custom;
    else event.eventType = Event_Unknown;
    
    event.eventName = obj["eventName"].toString();
    event.description = obj["description"].toString();
    event.timestamp = static_cast<qint64>(obj["timestamp"].toDouble());
    if (event.timestamp == 0) event.timestamp = QDateTime::currentMSecsSinceEpoch();
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
        if (m_dataUpdateCallback) {
            m_dataUpdateCallback(getAllData());
        }
    }
}
