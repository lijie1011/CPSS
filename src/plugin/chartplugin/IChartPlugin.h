/**
 * @file IChartPlugin.h
 * @brief 海图插件扩展接口定义
 * @details 在通用 IPlugin 接口基础上扩展海图专有操作（缩放、中心点、坐标转换等）。
 *          宿主（主窗口）通过 qobject_cast 获取该接口，以调用海图插件的
 *          专有功能；本接口为纯虚接口，不依赖具体实现类，跨 DLL 可用。
 * @date 2026-08-22
 */

#ifndef ICHARTPLUGIN_H
#define ICHARTPLUGIN_H

#include "IPlugin.h"
#include "dynamicdata.h"
#include "displaystate.h"

/**
 * @def CHART_PLUGIN_INTERFACE_IID
 * @brief 海图插件接口唯一标识符
 */
#define CHART_PLUGIN_INTERFACE_IID "com.cpss.plugin.IChartPlugin/1.0"

/**
 * @class IChartPlugin
 * @brief 海图插件扩展接口
 * @details 继承自 IPlugin，新增海图专有的控制方法。
 *          所有方法均为纯虚函数，由具体海图插件（ChartPlugin）实现。
 */
class IChartPlugin : public IPlugin
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IChartPlugin() = default;

    /**
     * @brief 获取已创建的海图视图控件（未创建则返回 nullptr）
     * @return 海图视图控件指针（以通用 QWidget 形式返回）
     */
    virtual QWidget* chartWidget() const = 0;

    /**
     * @brief 判断 Enclib 海图库是否已就绪
     * @return 就绪返回 true
     */
    virtual bool isEnclibReady() const = 0;

    /**
     * @brief 放大海图视图
     */
    virtual void zoomIn() = 0;

    /**
     * @brief 缩小海图视图
     */
    virtual void zoomOut() = 0;

    /**
     * @brief 重置海图视图到默认中心点
     */
    virtual void resetView() = 0;

    /**
     * @brief 设置海图中心点
     * @param lon 经度
     * @param lat 纬度
     */
    virtual void setChartCenter(double lon, double lat) = 0;

    /**
     * @brief 屏幕坐标转换为地理坐标
     * @param x 屏幕 X 坐标
     * @param y 屏幕 Y 坐标
     * @param lon 输出经度
     * @param lat 输出纬度
     * @return 转换成功返回 true
     */
    virtual bool screenToGeo(int x, int y, double &lon, double &lat) const = 0;

    /**
     * @brief 获取当前海图比例尺
     * @return 当前比例尺
     */
    virtual double currentScale() const = 0;

    /**
     * @brief 刷新海图视图
     * @details 等价于 GraphicsViewWidget::updateChart()，用于设置对话框
     *          修改显示选项后实时刷新海图。
     */
    virtual void updateChartView() = 0;

    /**
     * @brief 更新海图显示状态
     * @param stateMap 显示状态映射
     */
    virtual void updateDisplayState(const DisplayStateMap &stateMap) = 0;

    /**
     * @brief 获取海图视图当前的动态目标数据
     * @return 动态目标数据
     */
    virtual DynamicObjects getDynamicData() const = 0;

    /**
     * @brief 获取海图视图当前的显示状态映射
     * @return 显示状态映射
     */
    virtual DisplayStateMap getDisplayStates() const = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(IChartPlugin, CHART_PLUGIN_INTERFACE_IID)
QT_END_NAMESPACE

#endif // ICHARTPLUGIN_H
