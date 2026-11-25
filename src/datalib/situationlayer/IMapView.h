/**
 * @file IMapView.h
 * @brief 通用地图视图接口（所有地图插件实现，供宿主统一控制）
 * @details 与 IChartPlugin 区别：本接口不含 Enclib 专有方法（zoom/screenToGeo 等），
 *          只定义态势显示相关的通用能力。主窗口的平台控制面板通过此接口广播。
 * @date 2026-11-25
 */

#ifndef IMAPVIEW_H
#define IMAPVIEW_H

#include "dynamicdata.h"
#include "platformdisplaystate.h"
#include "IPlugin.h"
#include <QWidget>

#define MAP_VIEW_INTERFACE_IID "com.cpss.plugin.IMapView/1.0"

/**
 * @class IMapView
 * @brief 通用地图视图接口
 */
class IMapView
{
public:
    virtual ~IMapView() = default;

    /** @brief 获取地图视图控件 */
    virtual QWidget* mapWidget() = 0;
    /** @brief 获取当前动态目标数据 */
    virtual DynamicObjects getDynamicData() const = 0;
    /** @brief 获取当前显示状态 */
    virtual DisplayStateMap getDisplayStates() const = 0;
    /** @brief 更新显示状态（平台控制面板调用） */
    virtual void setDisplayStates(const DisplayStateMap &states) = 0;
};

Q_DECLARE_INTERFACE(IMapView, MAP_VIEW_INTERFACE_IID)

#endif // IMAPVIEW_H
