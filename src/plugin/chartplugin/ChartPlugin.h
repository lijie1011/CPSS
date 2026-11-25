/**
 * @file ChartPlugin.h
 * @brief Chart 插件类定义
 * @details 将海图功能封装为独立 DLL 插件，实现 IChartPlugin 扩展接口，
 *          负责 Enclib 初始化和 GraphicsViewWidget 的创建与管理。
 *          本插件编译为 chartplugin.dll，由 PluginManager 运行时扫描
 *          plugins/ 目录动态加载，通过 qt_plugin_instance 符号导出实例。
 */

#ifndef CHARTPLUGIN_H
#define CHARTPLUGIN_H

#include <QObject>
#include "IChartPlugin.h"
#include "IMapView.h"
#include "graphicsviewwidget.h"

/**
 * @class ChartPlugin
 * @brief 海图 DLL 插件类
 * @details 封装 Enclib 初始化、海图视图创建和海图控制接口。
 *          通过 zoomIn/zoomOut/resetView/setChartCenter 暴露海图操作，
 *          通过 chartWidget() 暴露内部 GraphicsViewWidget 供外部连接信号。
 */
class ChartPlugin : public QObject, public IChartPlugin, public IMapView
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin IChartPlugin IMapView)

public:
    ChartPlugin();
    ~ChartPlugin() override;

    // ---- IPlugin 接口 ----
    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;

    bool init(IPluginHost *host) override;
    void shutdown() override;

    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

    QObject *object() override { return this; }

    // ---- IChartPlugin 扩展接口 ----
    /**
     * @brief 获取已创建的海图视图（未创建则返回 nullptr）
     * @note 返回类型与接口 IChartPlugin 保持一致（QWidget*），
     *       避免跨 DLL 暴露具体类型。
     */
    QWidget* chartWidget() const override { return m_viewWidget; }

    /**
     * @brief Enclib 是否已就绪
     */
    bool isEnclibReady() const override { return m_enclibReady; }

    /**
     * @brief 放大海图
     */
    void zoomIn() override;

    /**
     * @brief 缩小海图
     */
    void zoomOut() override;

    /**
     * @brief 重置海图到默认位置
     */
    void resetView() override;

    /**
     * @brief 设置海图中心
     */
    void setChartCenter(double lon, double lat) override;

    /**
     * @brief 坐标转换（屏幕 -> 地理）
     */
    bool screenToGeo(int x, int y, double &lon, double &lat) const override;

    /**
     * @brief 获取当前比例尺
     */
    double currentScale() const override;

    /**
     * @brief 刷新海图视图
     */
    void updateChartView() override;

    /**
     * @brief 更新海图显示状态
     */
    void updateDisplayState(const DisplayStateMap &stateMap) override;

    /**
     * @brief 获取海图视图动态目标数据
     */
    DynamicObjects getDynamicData() const override;

    /**
     * @brief 获取海图视图显示状态映射
     */
    DisplayStateMap getDisplayStates() const override;

    // ---- IMapView 通用地图接口 ----
    QWidget* mapWidget() override { return m_viewWidget; }
    void setDisplayStates(const DisplayStateMap &states) override { updateDisplayState(states); }

signals:
    /**
     * @brief 地理坐标更新（鼠标在海图上移动时发出）
     */
    void geoPositionUpdated(QPoint pos);

private:
    /**
     * @brief 检查 Enclib license 是否过期
     */
    bool checkLicenseExpired(const QString &enclibPath);

    IPluginHost *m_host;             ///< 插件宿主接口
    GraphicsViewWidget *m_viewWidget; ///< 海图视图实例
    bool m_enclibReady;              ///< Enclib 是否就绪
};

#endif
