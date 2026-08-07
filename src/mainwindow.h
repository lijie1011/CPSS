/**
 * @file mainwindow.h
 * @brief 主窗口类定义
 * @details 顶层主窗口，负责菜单栏、工具栏、状态栏以及插件 Dock 管理。
 *          海图功能由内置的 ChartPlugin 提供，主窗口通过 IPluginHost 接口
 *          为已加载的插件提供宿主服务（获取数据管理器、显示状态消息等）。
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMap>
#include <QAction>
#include <QCloseEvent>
#include "plugin/IPlugin.h"
#include "plugin/IPluginHost.h"
#include "plugin/PluginManager.h"

namespace Ui {
class MainWindow;
}

class ChartPlugin;

/**
 * @class MainWindow
 * @brief 主应用窗口类
 * @details 继承自 QMainWindow 并实现 IPluginHost 接口。
 *          内部维护插件管理器，将每个已加载插件包装为 QDockWidget
 *          并添加到主窗口布局中。海图视图通过 ChartPlugin 创建，
 *          默认停靠在左侧，其他插件默认停靠在右侧。
 */
class MainWindow : public QMainWindow, public IPluginHost
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

    // ---- IPluginHost 接口实现 ----

    /**
     * @brief 获取海图视图部件
     * @return 海图视图指针，未创建则返回 nullptr
     */
    QWidget* getViewWidget() const override;

    /**
     * @brief 获取数据管理器
     * @return 数据管理器指针
     */
    DataManager* getDataManager() override;

    /**
     * @brief 获取应用版本号
     * @return 版本字符串
     */
    QString getAppVersion() const override;

    /**
     * @brief 获取应用所在目录
     * @return 目录路径字符串
     */
    QString getAppPath() const override;

    /**
     * @brief 在状态栏显示消息
     * @param message 消息内容
     */
    void showStatusMessage(const QString &message) override;

    /**
     * @brief 弹出通知
     * @param title 通知标题
     * @param message 通知内容
     */
    void showNotification(const QString &title, const QString &message) override;

    /**
     * @brief 向宿主注册插件工具栏按钮
     * @param pluginId 插件唯一标识符
     * @param buttonText 按钮显示文本
     * @return 注册成功返回 true
     */
    bool registerPluginButton(const QString &pluginId, const QString &buttonText) override;

    /**
     * @brief 注销插件工具栏按钮
     * @param pluginId 插件唯一标识符
     */
    void unregisterPluginButton(const QString &pluginId) override;

    /**
     * @brief 激活指定部件
     * @param widget 要激活的部件
     * @return 激活成功返回 true
     */
    bool setActiveWidget(QWidget *widget) override;

    /**
     * @brief 显示海图视图
     */
    void showMapView() override;

private:
    /**
     * @brief 初始化子系统
     * @details 设置工具栏连接、创建插件管理器、注册 ChartPlugin、
     *          加载外部 DLL 插件、添加工具栏和状态栏。
     */
    void init();

    /**
     * @brief 创建状态栏
     */
    void createStatusBar();

    /**
     * @brief 从 plugins/ 目录加载外部插件
     */
    void loadPlugins();

    /**
     * @brief 插件加载完成回调
     * @param plugin 已加载的插件
     */
    void onPluginLoaded(IPlugin *plugin);

    /**
     * @brief 将插件包装为 QDockWidget
     * @param plugin 要包装的插件
     * @return 创建的 Dock 指针，失败返回 nullptr
     */
    QDockWidget* createPluginDock(IPlugin *plugin);

    /**
     * @brief 显示指定插件的界面
     * @param plugin 目标插件
     */
    void showPluginWidget(IPlugin *plugin);

    /**
     * @brief 创建并显示海图 Dock
     */
    void createChartDock();

    /**
     * @brief 获取海图内置插件实例
     * @return ChartPlugin 指针
     */
    ChartPlugin* chartPlugin() const;

    /**
     * @brief 首次显示事件处理
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;

    /**
     * @brief 关闭事件处理
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    /** @brief 放大海图 */
    void zoomIn();

    /** @brief 缩小海图 */
    void zoomOut();

    /** @brief 重置海图视图 */
    void resetView();

    /**
     * @brief 更新状态栏地理坐标
     * @param pos 鼠标坐标点
     */
    void updateGeoPosition(QPoint pos);

    /** @brief 显示事件图例对话框 */
    void showEventLegend();

    /** @brief 插件工具栏按钮点击处理 */
    void onPluginActionTriggered();

    /** @brief 显示海图显示设置对话框 */
    void showDisplaySetting();

    /** @brief 显示水深/等高线设置对话框 */
    void showDepthAndContour();

    /** @brief 显示事件历史对话框 */
    void showEventHistory();

    /** @brief 显示平台控制面板 */
    void showPlatformControl();

private:
    Ui::MainWindow *ui;                    ///< Qt Designer 生成的 UI 对象
    PluginManager* m_pluginManager;        ///< 插件管理器
    QMap<QString, QAction*> m_pluginActions;   ///< 插件工具栏按钮映射（key=pluginId）
    QMap<QString, QWidget*> m_pluginWidgets;  ///< 已创建的插件界面映射
    QMap<QString, QDockWidget*> m_pluginDocks;   ///< 插件 Dock 映射（key=pluginId）
    IPlugin* m_currentPlugin;             ///< 当前活跃的插件
};

#endif
