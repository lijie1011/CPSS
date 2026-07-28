/**
 * @file PluginManager.cpp
 * @brief 插件管理器类实现
 * @details 该类负责从指定目录加载插件DLL/SO文件，管理插件生命周期，并提供插件访问接口。
 * @date 2026-07-28
 */

#include "PluginManager.h"
#include <QDir>
#include <QFileInfo>
#include "common/logger.h"

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
PluginManager::PluginManager(QObject *parent)
    : QObject(parent),
      m_host(nullptr)
{
}

/**
 * @brief 析构函数
 * @details 卸载所有已加载的插件并释放资源
 */
PluginManager::~PluginManager()
{
    for (auto loader : m_loaders.values()) {
        loader->unload();
        delete loader;
    }
}

/**
 * @brief 设置插件宿主
 * @param host 插件宿主接口指针
 */
void PluginManager::setPluginHost(IPluginHost *host)
{
    m_host = host;
}

/**
 * @brief 从指定目录加载所有插件
 * @param pluginDir 插件目录路径
 * @return 加载成功返回true
 */
bool PluginManager::loadPlugins(const QString &pluginDir)
{
    // Logger::info("PluginManager::loadPlugins - loading plugins from: %s", pluginDir.toStdString().c_str());

    if (!m_host) {
        // Logger::error("PluginManager::loadPlugins - no host set");
        return false;
    }

    QDir dir(pluginDir);
    if (!dir.exists()) {
        // Logger::warn("Plugin directory does not exist: %s", pluginDir.toStdString().c_str());
        return false;
    }

    QStringList filters;
    filters << "*.dll" << "*.so";
    dir.setNameFilters(filters);

    QFileInfoList files = dir.entryInfoList(QDir::Files);
    // Logger::info("Found %d potential plugin files", files.size());

    for (const QFileInfo &file : files) {
        QPluginLoader *loader = new QPluginLoader(file.absoluteFilePath(), this);
        QObject *pluginObj = loader->instance();

        if (pluginObj) {
            IPlugin *plugin = qobject_cast<IPlugin*>(pluginObj);
            if (plugin) {
                bool initSuccess = plugin->init(m_host);
                if (initSuccess) {
                    m_plugins[plugin->pluginId()] = plugin;
                    m_loaders[plugin->pluginId()] = loader;
                    // Logger::info("Loaded plugin: %s (%s) v%s", 
                                 // plugin->pluginName().toStdString().c_str(),
                                 // plugin->pluginId().toStdString().c_str(),
                                 // plugin->pluginVersion().toStdString().c_str());
                    emit pluginLoaded(plugin);
                } else {
                    // Logger::warn("Plugin init failed: %s", plugin->pluginName().toStdString().c_str());
                    loader->unload();
                    delete loader;
                }
            } else {
                // Logger::warn("Plugin file does not implement IPlugin interface: %s", 
                            // file.fileName().toStdString().c_str());
                loader->unload();
                delete loader;
            }
        } else {
            // Logger::warn("Failed to load plugin: %s - %s", 
            //             file.fileName().toStdString().c_str(),
            //             loader->errorString().toStdString().c_str());
            delete loader;
        }
    }

    // Logger::info("Total loaded plugins: %d", m_plugins.size());
    return true;
}

/**
 * @brief 获取所有已加载的插件
 * @return 插件指针列表
 */
QList<IPlugin*> PluginManager::getLoadedPlugins() const
{
    return m_plugins.values();
}

/**
 * @brief 根据插件ID获取插件
 * @param pluginId 插件ID
 * @return 插件指针，不存在返回nullptr
 */
IPlugin* PluginManager::getPlugin(const QString &pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}