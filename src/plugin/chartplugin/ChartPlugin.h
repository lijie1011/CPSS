/**
 * @file ChartPlugin.h
 * @brief Chart 内置插件定义
 * @details 将海图功能封装为插件，实现 IPlugin 接口，
 *          负责 Enclib 初始化和 GraphicsViewWidget 的创建与管理。
 *          本插件为内置插件（不编译为独立 DLL），在 MainWindow 中通过
 *          PluginManager::registerPlugin() 注册。
 */

#ifndef CHARTPLUGIN_H
#define CHARTPLUGIN_H

#include <QObject>
#include "IPlugin.h"

class GraphicsViewWidget;

/**
 * @class ChartPlugin
 * @brief 海图内置插件类
 * @details 封装 Enclib 初始化、海图视图创建和海图控制接口。
 *          通过 zoomIn/zoomOut/resetView/setChartCenter 暴露海图操作，
 *          通过 chartWidget() 暴露内部 GraphicsViewWidget 供外部连接信号。
 */
class ChartPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)

public:
    ChartPlugin();
    ~ChartPlugin() override;

    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;

    bool init(IPluginHost *host) override;
    void shutdown() override;

    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

    /**
     * @brief 获取已创建的海图视图（未创建则返回 nullptr）
     */
    GraphicsViewWidget* chartWidget() const { return m_viewWidget; }

    /**
     * @brief Enclib 是否已就绪
     */
    bool isEnclibReady() const { return m_enclibReady; }

    /**
     * @brief 放大海图
     */
    void zoomIn();

    /**
     * @brief 缩小海图
     */
    void zoomOut();

    /**
     * @brief 重置海图到默认位置
     */
    void resetView();

    /**
     * @brief 设置海图中心
     */
    void setChartCenter(double lon, double lat);

    /**
     * @brief 坐标转换（屏幕 -> 地理）
     */
    bool screenToGeo(int x, int y, double &lon, double &lat) const;

    /**
     * @brief 获取当前比例尺
     */
    double currentScale() const;

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
