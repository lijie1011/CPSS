/**
 * @file displaystate.h
 * @brief 显示状态相关结构体定义
 * @details 定义了平台显示状态及组件状态的数据结构，用于控制
 *          平台各类元素的可见性。
 */

#ifndef DISPLAYSTATE_H
#define DISPLAYSTATE_H

#include <QString>
#include <QList>
#include <QMap>

/**
 * @struct ComponentState
 * @brief 组件状态结构体
 * @details 描述传感器或武器组件的状态
 */
struct ComponentState {
    QString componentId;   ///< 组件 ID
    QString componentName; ///< 组件名称
    QString componentType; ///< 组件类型（传感器/武器）
    double range;          ///< 探测/交战距离
    bool enabled;          ///< 是否启用
};

/**
 * @struct PlatformDisplayState
 * @brief 平台显示状态结构体
 * @details 控制平台各类元素的可见性
 */
struct PlatformDisplayState {
    QString platformId;   ///< 平台 ID
    QString platformName; ///< 平台名称
    bool showShip;        ///< 是否显示平台图标
    bool showName;        ///< 是否显示平台名称
    bool showTrack;       ///< 是否显示航迹
    bool showSensors;     ///< 是否显示传感器作用范围
    bool showWeapons;     ///< 是否显示武器作用范围
    bool showEvents;      ///< 是否显示事件标记

    QList<ComponentState> sensors; ///< 传感器状态列表
    QList<ComponentState> weapons; ///< 武器状态列表

    /**
     * @brief 构造函数
     * @details 默认初始化显示状态，航迹及传感器/武器默认隐藏
     */
    PlatformDisplayState()
        : showShip(true), showName(true), showTrack(false),
          showSensors(false), showWeapons(false), showEvents(true) {}
};

/**
 * @typedef DisplayStateMap
 * @brief 显示状态映射类型
 * @details 键为平台 ID，值为平台显示状态
 */
typedef QMap<QString, PlatformDisplayState> DisplayStateMap;

#endif
