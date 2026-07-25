#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <functional>
#include "dynamicdata.h"
#include "protocoladapter.h"
#include "datacache.h"

class CPSS_DATA_API DataManager : public QObject
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

    // 数据查询直接委托给 DataCache
    PlatformData getPlatform(const QString &id) {
        return DataCache::instance()->getPlatform(id);
    }
    QList<PlatformData> getAllPlatforms() {
        return DataCache::instance()->getAllPlatforms();
    }
    QList<PlatformData> getValidPlatforms() {
        return DataCache::instance()->getValidPlatforms();
    }
    DynamicObjects getAllData() {
        return DataCache::instance()->getAllData();
    }
    QList<SpecialEvent> getEventHistory() {
        return DataCache::instance()->getEventHistory();
    }

    typedef std::function<void(const DynamicObjects &data)> DataPushCallback;

    void registerDataPushCallback(DataPushCallback callback);
    void unregisterDataPushCallback(DataPushCallback callback);

    void startDataPush(int intervalMs = 1000);
    void stopDataPush();

    void startTestDataTimer(int intervalMs = 1000) {
        DataCache::instance()->startTestDataTimer(intervalMs);
    }
    void stopTestDataTimer() {
        DataCache::instance()->stopTestDataTimer();
    }

signals:
    void platformUpdated(const PlatformData &data);
    void platformsUpdated(const QList<PlatformData> &platforms);
    void dynamicDataChanged(const DynamicObjects &data);
    void dataExpired(const QString &id, ProtocolType source);
    void dataPushed(const DynamicObjects &data);

public slots:
    void onDataReceived(const QJsonObject &data, ProtocolType source);
    void pushData();

private:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

    void parseAndUpdate(const QJsonObject &data, ProtocolType source);
    void updatePlatform(const QJsonObject &obj, ProtocolType source);
    void updateEvent(const QJsonObject &obj, ProtocolType source);

    static DataManager *s_instance;
    static QMutex s_mutex;

    QList<IProtocolAdapter*> m_adapters;
    QTimer m_pushTimer;

    QMap<ProtocolType, int> m_priorityMap;
    qint64 m_defaultValidDuration;

    std::vector<DataPushCallback> m_pushCallbacks;
};

#endif