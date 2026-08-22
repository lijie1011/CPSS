/**
 * @file PluginManager.h
 * @brief 插件管理器类定义
 * @details 该类负责插件的加载、管理和卸载，提供插件的统一管理接口。
 * @date 2026-07-28
 */

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QLibrary>
#include "IPlugin.h"

/**
 * @class PluginManager
 * @brief 插件管理器类
 * @details 继承自QObject，负责管理所有插件的生命周期
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit PluginManager(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~PluginManager();

    /**
     * @brief 设置插件宿主
     * @param host 插件宿主接口指针
     */
    void setPluginHost(IPluginHost *host);
    
    /**
     * @brief 从指定目录加载所有插件
     * @param pluginDir 插件目录路径
     * @return 加载成功返回true
     */
    bool loadPlugins(const QString &pluginDir);

    /**
     * @brief 注册内置插件（非 DLL 加载，直接实例化）
     * @param plugin 插件实例（PluginManager 接管所有权）
     * @return 注册成功返回true
     */
    bool registerPlugin(IPlugin *plugin);
    
    /**
     * @brief 获取所有已加载的插件
     * @return 插件指针列表
     */
    QList<IPlugin*> getLoadedPlugins() const;
    
    /**
     * @brief 根据插件ID获取插件
     * @param pluginId 插件ID
     * @return 插件指针，不存在返回nullptr
     */
    IPlugin* getPlugin(const QString &pluginId) const;

    /**
     * @brief 卸载指定插件
     * @param pluginId 插件ID
     * @return 卸载成功返回true，插件不存在返回false
     * @details 调用插件 shutdown() 清理资源、从插件表中移除、卸载动态库，
     *          并在日志中记录卸载信息，最后发出 pluginUnloaded 信号。
     */
    bool unloadPlugin(const QString &pluginId);

    /**
     * @brief 卸载所有已加载插件
     * @details 逐个卸载全部插件并记录日志，供程序退出或插件重置时调用
     */
    void unloadAllPlugins();

signals:
    /**
     * @brief 插件加载完成信号
     * @param plugin 已加载的插件
     */
    void pluginLoaded(IPlugin *plugin);
    
    /**
     * @brief 插件卸载完成信号
     * @param plugin 已卸载的插件
     */
    void pluginUnloaded(IPlugin *plugin);

private:
    IPluginHost *m_host;                          ///< 插件宿主接口
    QMap<QString, QLibrary*> m_libraries;         ///< 插件库映射
    QMap<QString, IPlugin*> m_plugins;            ///< 插件实例映射
};

#endif