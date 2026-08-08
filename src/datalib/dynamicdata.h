/**
 * @file dynamicdata.h
 * @brief 动态数据结构定义
 * @details 定义了平台数据、事件数据、传感器/武器信息等核心数据结构，
 *          用于在系统中传递和存储动态目标信息。
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
#include <QColor>

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
    Camp_Neutral,        ///< 中立
    Camp_Mine,           ///< 我立
};

/**
 * 颜色表
 */
namespace Color {
//透明
const QColor Transparent("#00000000");

//紫色
const QColor Purple("#A020F0");
const QColor Magenta("#FF00FF");//*

//蓝色
const QColor Blue("#0000FF");
const QColor DarkBlue("#00008B");
const QColor RoyalBlue("#4169E1");
const QColor MediumBlue("#0000CD");

//青色
const QColor Cyan("#00FFFF");//*
const QColor Turquoise("#40E0D0");
const QColor MediumTurquoise("#48D1CC");
const QColor DarkTurquoise("#00CED1");

//绿色
const QColor Green("#00FF00");
const QColor LightGreen("#90EE90");
const QColor DarkGreen("#006400");
const QColor MediumAquamarine("#66CDAA");
const QColor ForestGreen("#228B22");
const QColor OliveDrab("#6B8E23");
const QColor YellowGreen("#9ACD32");
const QColor LimeGreen("#32CD32");    //*文字
const QColor GreenYellow("#ADFF2F");
const QColor MedSpringGreen("#00FA9A");
const QColor SpringGreen("#00FF7F");
const QColor LightSeaGreen("#20B2AA");
const QColor MediumSeaGreen("#3CB371"); //*控件
const QColor SeaGreen("#2E8B57");
const QColor DarkSeaGreen("#8FBC8F");

//黄色
const QColor Yellow("#FFFF00");
const QColor Gold("#FFD700");
const QColor LightYellow("#FFFFC8");

//红色
const QColor Red("#FF0000");
const QColor OrangeRed("#FF4500");
const QColor DarkRed("#800000");//*
const QColor PaleVioletRed("#DB7093");
const QColor LightCoral("#F08080");

//橙色
const QColor Orange("#FFA500");
const QColor DarkOrange("#FF8C00");

//灰色
const QColor Grey("#C0C0C0");
const QColor DarkGrey("#404040");
const QColor DarkGreyA("#22808082");

//黑色
const QColor Black("#000000");
const QColor BlackGrey("#222222");

//白色
const QColor White("#FFFFFF");


//状态颜色
const QColor Normal(LimeGreen);
const QColor Fault(DarkRed);
const QColor Warning(Gold);
const QColor ShutDown(DarkGrey);
const QColor Unknown(BlackGrey);
}

/**
 * @brief 获取阵营对应的绘制颜色（敌我方 → 颜色 综合绘制映射）
 * @param camp 阵营类型
 * @return 阵营颜色
 * @details 配色约定（我红敌蓝）：
 *          - 我方  Camp_Mine     → 红色
 *          - 友方  Camp_Friendly → 橙色
 *          - 敌方  Camp_Enemy    → 蓝色
 *          - 中立  Camp_Neutral  → 绿色
 *          - 未知  Camp_Unknown  → 黄色
 *          统一在此处收敛，供各绘制模块（海图/图元/控制面板）共用，避免多处重复实现。
 */
inline QColor campColor(CampType camp)
{
    switch (camp) {
    case Camp_Mine:     return Color::Red;     // 我方 → 红
    case Camp_Friendly: return Color::Orange;  // 友方 → 橙
    case Camp_Enemy:    return Color::Blue;     // 敌方 → 蓝
    case Camp_Neutral:  return Color::Green;    // 中立 → 绿
    case Camp_Unknown:  return Color::Yellow;   // 未知 → 黄
    default:            return Color::Yellow;   // 兜底 → 黄
    }
}

/**
 * @brief 获取阵营中文显示名称
 * @param camp 阵营类型
 * @return 阵营名称
 */
inline QString campName(CampType camp)
{
    switch (camp) {
    case Camp_Mine:     return QStringLiteral("我方");
    case Camp_Friendly: return QStringLiteral("友方");
    case Camp_Enemy:    return QStringLiteral("敌方");
    case Camp_Neutral:  return QStringLiteral("中立");
    case Camp_Unknown:  return QStringLiteral("未知");
    default:            return QStringLiteral("未知");
    }
}

/**
 * @brief 根据平台类型字段获取对应的图标模板文件名（平台类型 → 图标 综合绘制映射）
 * @param type 平台类型字符串（PlatformData.type，支持英文关键字或中文名）
 * @return JB_Souce 目录下的图标文件名（如 "驱逐舰.png"）；未匹配返回默认舰船图标
 * @details 该映射把抽象的平台类型解析为具体的军标图标素材，
 *          再由 IconManager 按 campColor(camp) 着色后绘制。
 *          支持英文关键字（不区分大小写）与中文名两种键，便于协议兼容。
 */
inline QString iconFileForType(const QString &type)
{
    const QString t = type.toLower();

    // ---- 水面舰艇 ----
    if (t.contains("carrier") || type.contains(QStringLiteral("航母")) || type.contains(QStringLiteral("航空母舰")))
        return QStringLiteral("航空母舰.png");
    if (t.contains("destroyer") || type.contains(QStringLiteral("驱逐舰")))
        return QStringLiteral("驱逐舰.png");
    if (t.contains("frigate") || type.contains(QStringLiteral("护卫舰")))
        return QStringLiteral("护卫舰.png");
    if (t.contains("missileboat") || type.contains(QStringLiteral("导弹艇")) || type.contains(QStringLiteral("导弹舰")))
        return QStringLiteral("导弹舰.png");
    if (t.contains("escortboat") || type.contains(QStringLiteral("护卫艇")))
        return QStringLiteral("护卫艇.png");
    if (t.contains("landingship") || type.contains(QStringLiteral("登陆舰")))
        return QStringLiteral("登陆舰.png");
    if (t.contains("landingcraft") || type.contains(QStringLiteral("登陆艇")))
        return QStringLiteral("登陆艇.png");
    if (t.contains("service") || type.contains(QStringLiteral("勤务船")))
        return QStringLiteral("勤务船.png");
    if (t.contains("civilian") || type.contains(QStringLiteral("民用")))
        return QStringLiteral("民用船只.png");

    // ---- 水下 ----
    if (t.contains("submarine") || type.contains(QStringLiteral("潜艇")))
        return QStringLiteral("潜艇.png");

    // ---- 空中平台 ----
    if (t.contains("awacs") || type.contains(QStringLiteral("预警机")))
        return QStringLiteral("预警机.png");
    if (t.contains("bomber") || type.contains(QStringLiteral("轰炸机")))
        return QStringLiteral("轰炸机.png");
    if (t.contains("attacker") || type.contains(QStringLiteral("强击机")))
        return QStringLiteral("强击机.png");
    if (t.contains("recon") || type.contains(QStringLiteral("侦察机")))
        return QStringLiteral("侦察机.png");
    if (t.contains("transport") || type.contains(QStringLiteral("运输机")))
        return QStringLiteral("运输机.png");
    if (t.contains("uav") || type.contains(QStringLiteral("无人机")))
        return QStringLiteral("无人机.png");
    if (t.contains("fighter") || t == "aircraft" || type.contains(QStringLiteral("歼击机")))
        return QStringLiteral("歼击机.png");

    // ---- 导弹 / 防空系统 ----
    if (t.contains("manpads") || type.contains(QStringLiteral("便携")))
        return QStringLiteral("便携式低空导弹.png");
    if (t.contains("aaa") || type.contains(QStringLiteral("弹炮")))
        return QStringLiteral("炮弹结合防空系统.png");
    if (t.contains("sam") || type.contains(QStringLiteral("地空导弹")))
        return QStringLiteral("地空导弹.png");
    if (t.contains("cruise_air") || type.contains(QStringLiteral("空基巡航")))
        return QStringLiteral("空基巡航导弹.png");
    if (t.contains("cruise_sea") || type.contains(QStringLiteral("海基巡航")))
        return QStringLiteral("海基巡航导弹.png");
    if (t.contains("cruise_land") || type.contains(QStringLiteral("路基巡航")) || type.contains(QStringLiteral("陆基巡航")))
        return QStringLiteral("路基巡航导弹.png");
    if (t.contains("ballistic") || type.contains(QStringLiteral("弹道")))
        return QStringLiteral("路基弹道式常规导弹.png");
    // 泛化导弹类型（未指明具体种类）→ 使用通用导弹图标
    if (t.contains("missile") || type.contains(QStringLiteral("导弹")))
        return QStringLiteral("地空导弹.png");

    // ---- 兜底：默认舰船图标 ----
    return QStringLiteral("驱逐舰.png");
}

/**
 * @enum SpecialEventType
 * @brief 特殊事件类型枚举
 */
enum SpecialEventType {
    Event_Unknown,       ///< 未知事件
    Event_Attack,        ///< 攻击事件
    Event_Defense,       ///< 防御事件
    Event_Alert,         ///< 告警事件
    Event_MissionStart,  ///< 任务开始
    Event_MissionEnd,    ///< 任务结束
    Event_Contact,       ///< 接触事件
    Event_Lost,          ///< 丢失事件
    Event_Damage,        ///< 损伤事件
    Event_Repair,        ///< 修复事件
    Event_Custom         ///< 自定义事件
};

/**
 * @struct WeaponInfo
 * @brief 武器信息结构体
 * @details 描述平台搭载的单种武器的信息，包括类型、数量和射程
 */
struct WeaponInfo {
    QString type;        ///< 武器类型
    int count;          ///< 数量
    double range;       ///< 射程（海里）
};

/**
 * @struct SensorInfo
 * @brief 传感器信息结构体
 * @details 描述平台搭载的单种传感器的信息，包括类型、数量和探测距离
 */
struct SensorInfo {
    QString type;        ///< 传感器类型
    int count;          ///< 数量
    double range;       ///< 探测距离（海里）
};

/**
 * @struct PlatformData
 * @brief 平台数据结构体
 * @details 描述一个海上/空中平台（舰船、飞机、导弹等）的完整状态信息，
 *          包括位置、速度、阵营、武器、传感器、航迹等
 */
struct PlatformData {
    QString id;          ///< 平台ID
    QString name;        ///< 平台名称
    double lon;          ///< 经度
    double lat;          ///< 纬度
    double altitude;     ///< 高度
    double speed;        ///< 速度（节）
    double heading;      ///< 航向（度，0度为正北）
    QString type;        ///< 平台类型（舰船/飞机/导弹等）
    QString category;    ///< 平台类别
    CampType camp;       ///< 阵营
    QString targetId;    ///< 目标ID

    QList<WeaponInfo> weapons;   ///< 武器列表
    QList<SensorInfo> sensors;   ///< 传感器列表

    QList<QPointF> trackPoints;  ///< 航迹点列表
    static const int MAX_TRACK_POINTS = 1000;  ///< 最大航迹点数量

    DataStatus dataStatus;       ///< 数据状态
    qint64 validUntil;          ///< 有效期截止时间戳
    qint64 updateTime;          ///< 更新时间戳
    ProtocolType sourceProtocol; ///< 来源协议

    /**
     * @brief 检查平台是否已过期
     * @return true 表示已过期
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
 * @brief 特殊事件结构体
 * @details 描述一个战术事件（攻击、防御、告警、接触等）的信息，
 *          包括事件类型、目标/来源平台、时间、位置等
 */
struct SpecialEvent {
    QString eventId;       ///< 事件ID
    SpecialEventType eventType;  ///< 事件类型
    QString eventName;     ///< 事件名称
    QString description;   ///< 事件描述
    qint64 timestamp;      ///< 时间戳
    QString targetId;      ///< 目标ID
    QString sourceId;      ///< 来源ID
    QJsonObject extraData; ///< 附加数据
    double lon;            ///< 经度
    double lat;            ///< 纬度
};

/**
 * @struct DynamicObjects
 * @brief 动态对象集合结构体
 * @details 系统中所有动态目标的完整集合，用于在各模块之间传递整体态势数据
 */
struct CPSS_DATA_API DynamicObjects {
    QMap<QString, PlatformData> platforms;  ///< 平台集合（键为平台ID）
    QList<SpecialEvent> events;              ///< 事件列表
    qint64 timestamp;                        ///< 时间戳
};

#endif
