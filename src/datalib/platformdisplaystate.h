/**
 * @file platformdisplaystate.h
 * @brief 显示状态相关结构体定义（从 core/displaystate.h 移到数据层共享）
 * @details 定义了平台显示状态及组件状态的数据结构，用于控制
 *          平台各类元素的可见性。
 * @date 2026-11-25
 */

#ifndef PLATFORMDISPLAYSTATE_H
#define PLATFORMDISPLAYSTATE_H

#include <QString>
#include <QList>
#include <QMap>

/**
 * @struct ComponentState
 * @brief 组件状态结构体
 */
struct ComponentState {
    QString componentId;
    QString componentName;
    QString componentType;
    double range = 0.0;
    bool enabled = false;
};

/**
 * @struct PlatformDisplayState
 * @brief 平台显示状态结构体
 */
struct PlatformDisplayState {
    QString platformId;
    QString platformName;
    bool showShip = true;
    bool showName = true;
    bool showTrack = false;
    bool showSensors = false;
    bool showWeapons = false;
    bool showEvents = true;

    QList<ComponentState> sensors;
    QList<ComponentState> weapons;

    PlatformDisplayState() = default;
};

typedef QMap<QString, PlatformDisplayState> DisplayStateMap;

#endif // PLATFORMDISPLAYSTATE_H
