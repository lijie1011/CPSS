#ifndef DATACACHE_H
#define DATACACHE_H

#include <QObject>
#include <QReadWriteLock>
#include <QTimer>
#include <QMutex>
#include <functional>
#include "dynamicdata.h"

class CPSS_DATA_API DataCache : public QObject
{
    Q_OBJECT

public:
    static DataCache* instance();
    
    bool updatePlatform(const PlatformData &data);
    bool removePlatform(const QString &id);
    PlatformData getPlatform(const QString &id) const;
    QList<PlatformData> getAllPlatforms() const;
    QList<PlatformData> getValidPlatforms() const;
    
    bool addEvent(const SpecialEvent &event);
    bool removeEvent(const QString &eventId);
    void clearEvents();
    QList<SpecialEvent> getAllEvents() const;
    
    QList<SpecialEvent> getEventHistory() const;
    void clearEventHistory();
    int getMaxHistorySize() const;
    void setMaxHistorySize(int size);
    
    DynamicObjects getAllData() const;
    void invalidateExpiredData();
    qint64 getTimestamp() const;

    typedef std::function<void(const DynamicObjects &data)> DataPushCallback;
    
    void registerDataPushCallback(DataPushCallback callback);
    void unregisterDataPushCallback(DataPushCallback callback);
    
    void startDataPush(int intervalMs = 1000);
    void stopDataPush();
    bool isPushRunning() const;
    
    void startTestDataTimer(int intervalMs = 1000);
    void stopTestDataTimer();

signals:
    void platformUpdated(const PlatformData &data);
    void platformsUpdated(const QList<PlatformData> &platforms);
    void dynamicDataChanged(const DynamicObjects &data);
    void eventAdded(const SpecialEvent &event);
    void eventRemoved(const QString &eventId);
    void dataPushed(const DynamicObjects &data);

private slots:
    void pushData();
    void updateTestData();

private:
    DataCache(QObject *parent = nullptr);
    ~DataCache() override;
    
    void initTestData();
    
    static DataCache* s_instance;
    static QMutex s_mutex;
    
    mutable QReadWriteLock m_dataLock;
    DynamicObjects m_dynamicData;
    QList<SpecialEvent> m_eventHistory;
    int m_maxHistorySize;
    
    QTimer m_expireTimer;
    QTimer m_pushTimer;
    QTimer m_testDataTimer;
    
    std::vector<DataPushCallback> m_pushCallbacks;
    
    qint64 m_testStartTime;
    bool m_eventSecondPhase;
    int m_eventCounter;
};

#endif