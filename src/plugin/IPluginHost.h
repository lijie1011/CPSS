/**
 * @file IPluginHost.h
 * @brief 插件宿主接口定义
 * @details 该文件定义了插件宿主必须提供的服务接口，插件通过此接口访问宿主应用程序的功能。
 * @date 2026-07-28
 */

#ifndef IPLUGINHOST_H
#define IPLUGINHOST_H

#include <QObject>
#include <QString>
#include <QWidget>
#include "datamanager.h"

/**
 * @class IPluginHost
 * @brief 插件宿主接口类
 * @details 定义宿主应用程序为插件提供的服务接口
 */
class IPluginHost
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IPluginHost() = default;

    /**
     * @brief 获取视图部件
     * @return 主视图部件指针
     */
    virtual QWidget* getViewWidget() const = 0;
    
    /**
     * @brief 获取数据管理器
     * @return 数据管理器指针
     */
    virtual DataManager* getDataManager() = 0;
    
    /**
     * @brief 获取应用程序版本号
     * @return 版本号字符串
     */
    virtual QString getAppVersion() const = 0;
    
    /**
     * @brief 获取应用程序路径
     * @return 应用程序运行目录
     */
    virtual QString getAppPath() const = 0;

    /**
     * @brief 显示状态栏消息
     * @param message 消息内容
     */
    virtual void showStatusMessage(const QString &message) = 0;
    
    /**
     * @brief 显示通知对话框
     * @param title 对话框标题
     * @param message 消息内容
     */
    virtual void showNotification(const QString &title, const QString &message) = 0;

    /**
     * @brief 注册插件按钮到工具栏
     * @param pluginId 插件ID
     * @param buttonText 按钮显示文本
     * @return 注册成功返回true
     */
    virtual bool registerPluginButton(const QString &pluginId, const QString &buttonText) = 0;
    
    /**
     * @brief 注销插件按钮
     * @param pluginId 插件ID
     */
    virtual void unregisterPluginButton(const QString &pluginId) = 0;

    /**
     * @brief 设置活动插件界面
     * @param widget 要激活的插件界面
     * @return 设置成功返回true
     */
    virtual bool setActiveWidget(QWidget *widget) = 0;
    
    /**
     * @brief 显示海图视图
     * @details 隐藏插件界面，切换回海图显示
     */
    virtual void showMapView() = 0;
};

#endif