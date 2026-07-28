/**
 * @file datacache.h
 * @brief 数据缓存类定义
 * @details 该类采用单例模式，负责缓存所有动态数据（平台、事件等），
 *          提供线程安全的数据访问接口，并在数据变化时发出信号通知。
 *          同时支持测试数据生成和过期数据清理功能。
 * @date 2026-07-28
 */

#ifndef DATACACHE_H
#define DATACACHE_H

#include <QObject>
#include <QReadWriteLock>
#include <QTimer>
#include <QMutex>
#include <functional>
#include "dynamicdata.h"

/**
 * @class DataCache
 * @brief 数据缓存类
 * @details 继承自QObject，提供线程安全的数据缓存和管理功能
 */
class CPSS_DATA_API DataCache : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取数据缓存单例实例
     * @return DataCache指针
     */
    static DataCache* instance();
    
    /**
     * @brief 更新平台数据
     * @param data 平台数据
     * @return 更新成功返回true
     */
    bool updatePlatform(const PlatformData &data);
    
    /**
     * @brief 移除平台
     * @param id 平台ID
     * @return 移除成功返回true，不存在返回false
     */
    bool removePlatform(const QString &id);
    
    /**
     * @brief 获取指定平台
     * @param id 平台ID
     * @return 平台数据
     */
    PlatformData getPlatform(const QString &id) const;
    
    /**
     * @brief 获取所有平台
     * @return 平台列表
     */
    QList<PlatformData> getAllPlatforms() const;
    
    /**
     * @brief 获取有效平台（未过期）
     * @return 有效平台列表
     */
    QList<PlatformData> getValidPlatforms() const;
    
    /**
     * @brief 添加事件
     * @param event 事件数据
     * @return 添加成功返回true
     */
    bool addEvent(const SpecialEvent &event);
    
    /**
     * @brief 移除事件
     * @param eventId 事件ID
     * @return 移除成功返回true，不存在返回false
     */
    bool removeEvent(const QString &eventId);
    
    /**
     * @brief 清空所有事件
     */
    void clearEvents();
    
    /**
     * @brief 获取所有事件
     * @return 事件列表
     */
    QList<SpecialEvent> getAllEvents() const;
    
    /**
     * @brief 获取事件历史
     * @return 事件历史列表
     */
    QList<SpecialEvent> getEventHistory() const;
    
    /**
     * @brief 清空事件历史
     */
    void clearEventHistory();
    
    /**
     * @brief 获取最大历史记录数
     * @return 最大历史记录数
     */
    int getMaxHistorySize() const;
    
    /**
     * @brief 设置最大历史记录数
     * @param size 最大历史记录数
     */
    void setMaxHistorySize(int size);
    
    /**
     * @brief 获取所有数据
     * @return 动态对象集合
     */
    DynamicObjects getAllData() const;
    
    /**
     * @brief 清理过期数据
     */
    void invalidateExpiredData();
    
    /**
     * @brief 获取时间戳
     * @return 最后更新时间戳
     */
    qint64 getTimestamp() const;

    /**
     * @brief 数据推送回调类型定义
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
     * @brief 检查推送是否运行
     * @return 运行中返回true
     */
    bool isPushRunning() const;
    
    /**
     * @brief 启动测试数据定时器
     * @param intervalMs 间隔时间（毫秒）
     */
    void startTestDataTimer(int intervalMs = 1000);
    
    /**
     * @brief 停止测试数据定时器
     */
    void stopTestDataTimer();

signals:
    /**
     * @brief 平台数据更新信号
     * @param data 更新的平台数据
     */
    void platformUpdated(const PlatformData &data);
    
    /**
     * @brief 平台列表更新信号
     * @param platforms 更新后的平台列表
     */
    void platformsUpdated(const QList<PlatformData> &platforms);
    
    /**
     * @brief 动态数据变化信号
     * @param data 变化后的动态对象集合
     */
    void dynamicDataChanged(const DynamicObjects &data);
    
    /**
     * @brief 事件添加信号
     * @param event 添加的事件
     */
    void eventAdded(const SpecialEvent &event);
    
    /**
     * @brief 事件移除信号
     * @param eventId 移除的事件ID
     */
    void eventRemoved(const QString &eventId);
    
    /**
     * @brief 数据推送信号
     * @param data 推送的数据
     */
    void dataPushed(const DynamicObjects &data);

private slots:
    /**
     * @brief 推送数据
     */
    void pushData();
    
    /**
     * @brief 更新测试数据
     */
    void updateTestData();

private:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    DataCache(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DataCache() override;
    
    /**
     * @brief 初始化测试数据
     */
    void initTestData();
    
    static DataCache* s_instance;      ///< 单例实例指针
    static QMutex s_mutex;              ///< 单例互斥锁
    
    mutable QReadWriteLock m_dataLock;  ///< 数据读写锁
    DynamicObjects m_dynamicData;       ///< 动态对象数据
    QList<SpecialEvent> m_eventHistory; ///< 事件历史记录
    int m_maxHistorySize;               ///< 最大历史记录数
    
    QTimer m_expireTimer;               ///< 过期检查定时器
    QTimer m_pushTimer;                 ///< 数据推送定时器
    QTimer m_testDataTimer;             ///< 测试数据更新定时器
    
    std::vector<DataPushCallback> m_pushCallbacks; ///< 数据推送回调列表
    
    qint64 m_testStartTime;             ///< 测试开始时间
    bool m_eventSecondPhase;             ///< 事件第二阶段标志
    int m_eventCounter;                 ///< 事件计数器
};

#endif