/**
 * @file dynamicdata.h
 * @brief 动态数据结构定义
 * @details 定义了平台数据、事件数据、传感器/武器信息等核心数据结构，
 *          用于在系统中传递和存储动态对象信息。
 * @date 2026-07-28
 */

#ifndef DYNAMICDATA_H
#define DYNAMICDATA_H

#ifdef CPSS_DATA_EXPORT
#   define CPSS_DATA_API __declspec(dllexport)
#else
#   define CPSS_DATA_API __declspec(dllimport)
#endif

#include <QString>
#include <QList>
#include <QDateTime>
#include <QMap>
#include <QJsonObject>
#include <QPointF>

/**
 * @enum DataStatus
 * @brief 数据状态枚举
 */
enum DataStatus {
    DataStatus_Normal,   ///< 正常状态
    DataStatus_Expired,  ///< 已过期
    DataStatus_Invalid   ///< 无效
};

/**
 * @enum ProtocolType
 * @brief 协议类型枚举
 */
enum ProtocolType {
    Protocol_Unknown,    ///< 未知协议
    Protocol_HTTP        ///< HTTP协议
};

/**
 * @enum CampType
 * @brief 阵营类型枚举
 */
enum CampType {
    Camp_Unknown,        ///< 未知阵营
    Camp_Friendly,       ///< 友方
    Camp_Enemy,          ///< 敌方
    Camp_Red,            ///< 红方
    Camp_Purple,         ///< 紫方
    Camp_Neutral         ///< 中立
};

/**
 * @enum SpecialEventType
 * @brief 特殊事件类型枚举
 */
enum SpecialEventType {
    Event_Unknown,       ///< 未知事件
    Event_Attack,        ///< 攻击事件
    Event_Defense,       ///< 防御事件
    Event_Alert,         ///< 警报事件
    Event_MissionStart,  ///< 任务开始
    Event_MissionEnd,    ///< 任务结束
    Event_Contact,       ///< 接触事件
    Event_Lost,          ///< 丢失事件
    Event_Damage,        ///< 受损事件
    Event_Repair,        ///< 修复事件
    Event_Custom         ///< 自定义事件
};

/**
 * @struct WeaponInfo
 * @brief 武器信息结构
 */
struct WeaponInfo {
    QString type;        ///< 武器类型
    int count;          ///< 数量
    double range;       ///< 射程（海里）
};

/**
 * @struct SensorInfo
 * @brief 传感器信息结构
 */
struct SensorInfo {
    QString type;        ///< 传感器类型
    int count;          ///< 数量
    double range;       ///< 探测范围（海里）
};

/**
 * @struct PlatformData
 * @brief 平台数据结构
 */
struct PlatformData {
    QString id;          ///< 平台ID
    QString name;        ///< 平台名称
    double lon;          ///< 经度
    double lat;          ///< 纬度
    double altitude;     ///< 高度
    double speed;        ///< 速度（节）
    double heading;      ///< 航向（度，0度为正北）
    QString type;        ///< 平台类型（ship/plane/missile等）
    QString category;    ///< 平台类别
    CampType camp;       ///< 阵营
    QString targetId;    ///< 目标ID

    QList<WeaponInfo> weapons;   ///< 武器列表
    QList<SensorInfo> sensors;   ///< 传感器列表

    QList<QPointF> trackPoints;  ///< 航迹点列表
    static const int MAX_TRACK_POINTS = 1000;  ///< 最大航迹点数

    DataStatus dataStatus;       ///< 数据状态
    qint64 validUntil;          ///< 有效期截止时间戳
    qint64 updateTime;          ///< 更新时间戳
    ProtocolType sourceProtocol; ///< 来源协议

    /**
     * @brief 检查平台是否过期
     * @return 过期返回true
     */
    bool isExpired() const {
        return dataStatus == DataStatus_Expired || 
               (validUntil > 0 && validUntil < QDateTime::currentMSecsSinceEpoch());
    }

    /**
     * @brief 添加航迹点
     * @param lon 经度
     * @param lat 纬度
     */
    void addTrackPoint(double lon, double lat) {
        trackPoints.append(QPointF(lon, lat));
        if (trackPoints.size() > MAX_TRACK_POINTS) {
            trackPoints.removeFirst();
        }
    }
};

/**
 * @struct SpecialEvent
 * @brief 特殊事件结构
 */
struct SpecialEvent {
    QString eventId;       ///< 事件ID
    SpecialEventType eventType;  ///< 事件类型
    QString eventName;     ///< 事件名称
    QString description;   ///< 事件描述
    qint64 timestamp;      ///< 时间戳
    QString targetId;      ///< 目标ID
    QString sourceId;      ///< 来源ID
    QJsonObject extraData; ///< 额外数据
    double lon;            ///< 经度
    double lat;            ///< 纬度
};

/**
 * @struct DynamicObjects
 * @brief 动态对象集合结构
 */
struct CPSS_DATA_API DynamicObjects {
    QMap<QString, PlatformData> platforms;  ///< 平台集合（key为平台ID）
    QList<SpecialEvent> events;              ///< 事件列表
    qint64 timestamp;                        ///< 时间戳
};

#endif