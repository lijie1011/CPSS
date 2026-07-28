/**
 * @file displaystate.h
 * @brief 显示状态相关结构定义
 * @details 定义了平台显示状态和组件状态的数据结构，用于控制平台各元素的显示/隐藏。
 * @date 2026-07-28
 */

#ifndef DISPLAYSTATE_H
#define DISPLAYSTATE_H

#include <QString>
#include <QList>
#include <QMap>

/**
 * @struct ComponentState
 * @brief 组件状态结构
 * @details 描述传感器或武器组件的状态
 */
struct ComponentState {
    QString componentId;   ///< 组件ID
    QString componentName; ///< 组件名称
    QString componentType; ///< 组件类型（sensor/weapon）
    double range;          ///< 探测/射程范围
    bool enabled;          ///< 是否启用
};

/**
 * @struct PlatformDisplayState
 * @brief 平台显示状态结构
 * @details 控制平台各元素的显示状态
 */
struct PlatformDisplayState {
    QString platformId;   ///< 平台ID
    QString platformName; ///< 平台名称
    bool showShip;        ///< 是否显示平台图标
    bool showName;        ///< 是否显示平台名称
    bool showTrack;       ///< 是否显示航迹
    bool showSensors;     ///< 是否显示传感器范围
    bool showWeapons;     ///< 是否显示武器范围
    bool showEvents;      ///< 是否显示事件标记

    QList<ComponentState> sensors; ///< 传感器状态列表
    QList<ComponentState> weapons; ///< 武器状态列表

    /**
     * @brief 构造函数
     * @details 默认初始化显示状态，航迹和传感器/武器默认不显示
     */
    PlatformDisplayState()
        : showShip(true), showName(true), showTrack(false),
          showSensors(false), showWeapons(false), showEvents(true) {}
};

/**
 * @typedef DisplayStateMap
 * @brief 显示状态映射类型
 * @details key为平台ID，value为平台显示状态
 */
typedef QMap<QString, PlatformDisplayState> DisplayStateMap;

#endif