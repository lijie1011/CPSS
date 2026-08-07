/**
 * @file datamanager.h
 * @brief 数据管理器类定义
 * @details 本类是数据管理系统的核心，采用单例模式。
 *          它协调多个协议适配器，负责数据的接收、解析、存储和分发。
 * @date 2026-07-28
 */

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <functional>
#include <mutex>
#include "dynamicdata.h"
#include "protocoladapter.h"
#include "datacache.h"

/**
 * @brief 平台更新回调类型
 */
typedef std::function<void(const PlatformData&)> PlatformUpdateCallback;

/**
 * @brief 数据更新回调类型
 */
typedef std::function<void(const DynamicObjects&)> DataUpdateCallback;

/**
 * @brief 事件更新回调类型
 */
typedef std::function<void(const SpecialEvent&)> EventUpdateCallback;

/**
 * @brief 平台过期回调类型
 */
typedef std::function<void(const QString&)> PlatformExpiredCallback;

/**
 * @class DataManager
 * @brief 数据管理器类
 * @details 继承自QObject，实现单例模式，负责统一的数据管理与分发
 */
class CPSS_DATA_API DataManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取数据管理器单例实例
     * @return DataManager指针
     */
    static DataManager* instance();

    /**
     * @brief 添加协议适配器
     * @param adapter 协议适配器指针
     */
    void addAdapter(IProtocolAdapter *adapter);
    
    /**
     * @brief 移除协议适配器
     * @param adapter 协议适配器指针
     */
    void removeAdapter(IProtocolAdapter *adapter);
    
    /**
     * @brief 启动所有适配器
     */
    void startAllAdapters();
    
    /**
     * @brief 停止所有适配器
     */
    void stopAllAdapters();

    /**
     * @brief 设置数据源优先级
     * @param type 协议类型
     * @param priority 优先级值
     */
    void setDataSourcePriority(ProtocolType type, int priority);
    
    /**
     * @brief 获取数据源优先级
     * @param type 协议类型
     * @return 优先级值
     */
    int getDataSourcePriority(ProtocolType type) const;

    /**
     * @brief 设置默认数据有效期
     * @param ms 有效期（毫秒）
     */
    void setDefaultValidDuration(qint64 ms);
    
    /**
     * @brief 获取默认数据有效期
     * @return 有效期（毫秒）
     */
    qint64 defaultValidDuration() const;

    /**
     * @brief 获取指定平台的数据
     * @param id 平台ID
     * @return 平台数据
     */
    PlatformData getPlatform(const QString &id) const;
    
    /**
     * @brief 获取所有平台数据
     * @return 平台数据列表
     */
    QList<PlatformData> getAllPlatforms() const;
    
    /**
     * @brief 获取有效平台数据
     * @return 有效平台数据列表
     */
    QList<PlatformData> getValidPlatforms() const;
    
    /**
     * @brief 获取所有数据
     * @return 动态对象集合
     */
    DynamicObjects getAllData() const;
    
    /**
     * @brief 获取所有事件
     * @return 事件列表
     */
    QList<SpecialEvent> getAllEvents() const;
    
    /**
     * @brief 获取事件历史记录
     * @return 事件列表
     */
    QList<SpecialEvent> getEventHistory() const;

    /**
     * @brief 数据推送回调类型
     */
    typedef std::function<void(const DynamicObjects &data)> DataPushCallback;

    /**
     * @brief 注册数据推送回调
     * @param callback 回调函数
     */
    void registerDataPushCallback(DataPushCallback callback);
    
    /**
     * @brief 注销数据推送回调
     * @param callback 回调函数
     */
    void unregisterDataPushCallback(DataPushCallback callback);

    /**
     * @brief 启动数据推送
     * @param intervalMs 推送间隔（毫秒）
     */
    void startDataPush(int intervalMs = 1000);
    
    /**
     * @brief 停止数据推送
     */
    void stopDataPush();

    /**
     * @brief 启动测试数据定时器
     * @param intervalMs 间隔（毫秒）
     */
    void startTestDataTimer(int intervalMs = 1000);
    
    /**
     * @brief 停止测试数据定时器
     */
    void stopTestDataTimer();

    /**
     * @brief 注册平台更新回调
     * @param callback 回调函数
     */
    void registerPlatformUpdateCallback(PlatformUpdateCallback callback);
    
    /**
     * @brief 注册数据更新回调
     * @param callback 回调函数
     */
    void registerDataUpdateCallback(DataUpdateCallback callback);
    
    /**
     * @brief 注册事件更新回调
     * @param callback 回调函数
     */
    void registerEventUpdateCallback(EventUpdateCallback callback);
    
    /**
     * @brief 注册平台过期回调
     * @param callback 回调函数
     */
    void registerPlatformExpiredCallback(PlatformExpiredCallback callback);

    /**
     * @brief 注销平台更新回调
     */
    void unregisterPlatformUpdateCallback();
    
    /**
     * @brief 注销数据更新回调
     */
    void unregisterDataUpdateCallback();
    
    /**
     * @brief 注销事件更新回调
     */
    void unregisterEventUpdateCallback();
    
    /**
     * @brief 注销平台过期回调
     */
    void unregisterPlatformExpiredCallback();

    /**
     * @brief 处理平台过期
     * @param id 平台ID
     */
    void onPlatformExpired(const QString &id);

signals:
    /**
     * @brief 平台更新信号
     * @param data 平台数据
     */
    void platformUpdated(const PlatformData &data);
    
    /**
     * @brief 平台列表更新信号
     * @param platforms 平台数据列表
     */
    void platformsUpdated(const QList<PlatformData> &platforms);
    
    /**
     * @brief 动态数据变更信号
     * @param data 动态对象集合
     */
    void dynamicDataChanged(const DynamicObjects &data);
    
    /**
     * @brief 数据过期信号
     * @param id 平台ID
     * @param source 数据来源
     */
    void dataExpired(const QString &id, ProtocolType source);
    
    /**
     * @brief 数据已推送信号
     * @param data 动态对象集合
     */
    void dataPushed(const DynamicObjects &data);

public slots:
    /**
     * @brief 处理接收到的数据
     * @param data JSON数据
     * @param source 数据来源
     */
    void onDataReceived(const QJsonObject &data, ProtocolType source);
    
    /**
     * @brief 推送数据
     */
    void pushData();

private:
    /**
     * @brief 私有构造函数
     * @param parent 父对象指针
     */
    explicit DataManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DataManager();

    /**
     * @brief 解析并更新数据
     * @param data JSON数据
     * @param source 数据来源
     */
    void parseAndUpdate(const QJsonObject &data, ProtocolType source);
    
    /**
     * @brief 更新平台数据
     * @param obj JSON对象
     * @param source 数据来源
     */
    void updatePlatform(const QJsonObject &obj, ProtocolType source);
    
    /**
     * @brief 更新事件数据
     * @param obj JSON对象
     * @param source 数据来源
     */
    void updateEvent(const QJsonObject &obj, ProtocolType source);

    static DataManager *s_instance;              ///< 单例实例
    static QMutex s_mutex;                        ///< 单例互斥锁

    QList<IProtocolAdapter*> m_adapters;          ///< 协议适配器列表
    QTimer m_pushTimer;                           ///< 数据推送定时器

    QMap<ProtocolType, int> m_priorityMap;        ///< 数据源优先级映射
    qint64 m_defaultValidDuration;                ///< 默认数据有效期

    std::vector<DataPushCallback> m_pushCallbacks;           ///< 数据推送回调列表

    PlatformUpdateCallback m_platformUpdateCallback;         ///< 平台更新回调
    DataUpdateCallback m_dataUpdateCallback;                 ///< 数据更新回调
    EventUpdateCallback m_eventUpdateCallback;               ///< 事件更新回调
    PlatformExpiredCallback m_platformExpiredCallback;       ///< 平台过期回调
    std::mutex m_callbackMutex;                              ///< 回调互斥锁
};

#endif
