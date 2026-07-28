/**
 * @file IPlugin.h
 * @brief 插件接口定义
 * @details 该文件定义了插件系统的核心接口，所有插件必须实现此接口才能被插件管理器加载和管理。
 * @date 2026-07-28
 */

#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QWidget>
#include "IPluginHost.h"

/**
 * @def PLUGIN_INTERFACE_IID
 * @brief 插件接口唯一标识符
 */
#define PLUGIN_INTERFACE_IID "com.cpss.plugin.IPlugin/1.0"

/**
 * @class IPlugin
 * @brief 插件接口类
 * @details 定义插件必须实现的基本方法，包括插件信息、生命周期管理和界面创建
 */
class IPlugin
{
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IPlugin() = default;

    /**
     * @brief 获取插件名称
     * @return 插件显示名称
     */
    virtual QString pluginName() const = 0;
    
    /**
     * @brief 获取插件ID
     * @return 插件唯一标识符
     */
    virtual QString pluginId() const = 0;
    
    /**
     * @brief 获取插件版本号
     * @return 插件版本字符串
     */
    virtual QString pluginVersion() const = 0;

    /**
     * @brief 初始化插件
     * @param host 插件宿主接口指针
     * @return 初始化成功返回true，失败返回false
     */
    virtual bool init(IPluginHost *host) = 0;
    
    /**
     * @brief 关闭插件
     * @details 清理插件资源，释放内存
     */
    virtual void shutdown() = 0;

    /**
     * @brief 创建插件界面
     * @param parent 父窗口部件
     * @return 插件界面部件指针
     */
    virtual QWidget* createWidget(QWidget *parent = nullptr) = 0;
    
    /**
     * @brief 销毁插件界面
     * @param widget 要销毁的界面部件
     */
    virtual void destroyWidget(QWidget *widget) = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(IPlugin, PLUGIN_INTERFACE_IID)
QT_END_NAMESPACE

#endif