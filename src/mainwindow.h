/**
 * @file mainwindow.h
 * @brief 主窗口类定义
 * @details 该类是应用程序的主窗口，负责管理海图显示、工具栏、状态栏、插件系统和各种对话框。
 *          主要功能包括海图初始化、平台控制、事件历史显示、显示设置等。
 * @date 2026-07-28
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QHBoxLayout>
#include <QMap>
#include <QAction>
#include "graphicsviewwidget.h"
#include "dynamicdata.h"
#include "eventhistorydialog.h"
#include "plugin/IPlugin.h"
#include "plugin/IPluginHost.h"
#include "plugin/PluginManager.h"

namespace Ui {
class MainWindow;
}

/**
 * @class MainWindow
 * @brief 主窗口类
 * @details 继承自QMainWindow和IPluginHost，作为应用程序的主界面和插件宿主
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

    /**
     * @brief 获取视图部件
     * @return 图形视图部件指针
     */
    QWidget* getViewWidget() const override { return m_viewWidget; }
    
    /**
     * @brief 获取数据管理器
     * @return 数据管理器指针
     */
    DataManager* getDataManager() override;
    
    /**
     * @brief 获取应用程序版本号
     * @return 版本号字符串
     */
    QString getAppVersion() const override;
    
    /**
     * @brief 获取应用程序路径
     * @return 应用程序运行目录路径
     */
    QString getAppPath() const override;

    /**
     * @brief 显示状态栏消息
     * @param message 消息内容
     */
    void showStatusMessage(const QString &message) override;
    
    /**
     * @brief 显示通知对话框
     * @param title 对话框标题
     * @param message 消息内容
     */
    void showNotification(const QString &title, const QString &message) override;

    /**
     * @brief 注册插件按钮到工具栏
     * @param pluginId 插件ID
     * @param buttonText 按钮显示文本
     * @return 注册成功返回true
     */
    bool registerPluginButton(const QString &pluginId, const QString &buttonText) override;
    
    /**
     * @brief 注销插件按钮
     * @param pluginId 插件ID
     */
    void unregisterPluginButton(const QString &pluginId) override;

    /**
     * @brief 设置活动插件界面
     * @param widget 插件界面组件
     * @return 设置成功返回true
     */
    bool setActiveWidget(QWidget *widget) override;
    
    /**
     * @brief 显示海图视图
     * @details 隐藏插件界面，显示海图
     */
    void showMapView() override;

private:
    /**
     * @brief 初始化主窗口
     * @details 初始化Enclib海图库、视图组件、数据管理器和插件系统
     */
    void init();
    
    /**
     * @brief 创建状态栏
     */
    void createStatusBar();
    
    /**
     * @brief 加载插件
     * @details 从应用程序目录下的plugins文件夹加载所有插件
     */
    void loadPlugins();
    
    /**
     * @brief 插件加载完成处理
     * @param plugin 加载完成的插件
     */
    void onPluginLoaded(IPlugin *plugin);
    
    /**
     * @brief 显示插件界面
     * @param plugin 要显示的插件
     */
    void showPluginWidget(IPlugin *plugin);
    
    /**
     * @brief 检查Enclib许可证是否过期
     * @param enclibPath Enclib库路径
     * @return 过期返回true，有效返回false
     */
    bool checkLicenseExpired(const QString &enclibPath);
    
    /**
     * @brief 显示事件处理
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;

private slots:
    /**
     * @brief 放大视图
     */
    void zoomIn();
    
    /**
     * @brief 缩小视图
     */
    void zoomOut();
    
    /**
     * @brief 重置视图到默认位置
     */
    void resetView();
    
    /**
     * @brief 更新地理坐标显示
     * @param pos 鼠标位置
     */
    void updateGeoPosition(QPoint pos);
    
    /**
     * @brief 显示事件图例对话框
     * @details 显示各类事件和阵营的颜色、图标说明
     */
    void showEventLegend();
    
    /**
     * @brief 插件按钮点击处理
     * @details 切换插件界面和海图视图的显示
     */
    void onPluginActionTriggered();
    
    /**
     * @brief 显示海图显示设置对话框
     */
    void showDisplaySetting();
    
    /**
     * @brief 显示水深和等深线设置对话框
     */
    void showDepthAndContour();
    
    /**
     * @brief 显示事件历史对话框
     */
    void showEventHistory();
    
    /**
     * @brief 显示平台控制面板
     */
    void showPlatformControl();

private:
    Ui::MainWindow *ui;                        ///< UI界面指针
    GraphicsViewWidget* m_viewWidget;          ///< 图形视图部件
    PluginManager* m_pluginManager;            ///< 插件管理器
    QMap<QString, QAction*> m_pluginActions;   ///< 插件动作映射
    QMap<QString, QWidget*> m_pluginWidgets;   ///< 插件界面映射
    QMap<QString, int> m_pluginPageIndices;    ///< 插件页面索引映射
    IPlugin* m_currentPlugin;                  ///< 当前活动插件
};

#endif