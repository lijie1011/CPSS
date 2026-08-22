/**
 * @file PluginManager.cpp
 * @brief 插件管理器实现
 * @details 负责从指定目录动态加载插件 DLL、注册内置插件、管理插件的生命周期。
 *          支持通过 qt_plugin_instance 符号解析插件实例，并将其转换为 IPlugin 接口。
 * @date 2026-07-28
 */

#include "PluginManager.h"
#include "common/logger.h"
#include <QDir>
#include <QFileInfo>
#include <QLibrary>

// 日志辅助函数：通过 Logger 输出信息级别日志，统一以 [plug] 为前缀标识插件加载/卸载日志
static inline void logInfo(const QString &msg) {
    Logger::info("[plug] %s", msg.toUtf8().constData());
}

// 插件实例工厂函数类型：无参数、返回 QObject*
typedef QObject* (*InstanceFunc)();

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PluginManager::PluginManager(QObject *parent)
    : QObject(parent),
      m_host(nullptr)
{
}

/**
 * @brief 析构函数
 * @details 卸载所有已加载的插件 DLL 并释放资源，同时在日志中记录每个插件的卸载信息
 */
PluginManager::~PluginManager()
{
    // 记录每个已加载插件的卸载日志（退出清理阶段）
    for (auto it = m_plugins.constBegin(); it != m_plugins.constEnd(); ++it) {
        logInfo(QString("PluginManager: UNLOADING id=%1 (destructor)")
                .arg(it.key()));
    }
    for (auto lib : m_libraries.values()) {
        lib->unload();
        delete lib;
    }
    m_libraries.clear();
    m_plugins.clear();
    logInfo("PluginManager: destructor unload all done");
}

/**
 * @brief 设置插件宿主接口
 * @param host 插件宿主接口指针，用于插件与主程序通信
 */
void PluginManager::setPluginHost(IPluginHost *host)
{
    m_host = host;
}

/**
 * @brief 从指定目录加载所有插件
 * @param pluginDir 插件目录路径，扫描其中的 .dll / .so 文件
 * @return 只要有插件成功加载即返回 true；若全部失败返回 false
 */
bool PluginManager::loadPlugins(const QString &pluginDir)
{
    logInfo("PluginManager::loadPlugins enter");

    if (!m_host) {
        logInfo("PluginManager: ERROR no host set");
        return false;
    }

    QDir dir(pluginDir);
    if (!dir.exists()) {
        logInfo("PluginManager: ERROR dir not exist: " + pluginDir);
        return false;
    }

    QStringList filters;
    filters << "*.dll" << "*.so";
    dir.setNameFilters(filters);

    QFileInfoList files = dir.entryInfoList(QDir::Files);
    logInfo(QString("PluginManager: found %1 files").arg(files.size()));

    for (const QFileInfo &file : files) {
        QString absPath = file.absoluteFilePath();
        logInfo("PluginManager: trying " + file.fileName());

        QLibrary *lib = new QLibrary(absPath);
        if (!lib->load()) {
            logInfo("PluginManager: QLibrary load FAILED: " + lib->errorString());
            delete lib;
            continue;
        }

        InstanceFunc instFunc = (InstanceFunc)lib->resolve("qt_plugin_instance");
        if (!instFunc) {
            logInfo("PluginManager: resolve qt_plugin_instance FAILED");
            lib->unload();
            delete lib;
            continue;
        }

        QObject *pluginObj = instFunc();
        if (!pluginObj) {
            logInfo("PluginManager: qt_plugin_instance returned NULL");
            lib->unload();
            delete lib;
            continue;
        }

        IPlugin *plugin = qobject_cast<IPlugin*>(pluginObj);
        if (!plugin) {
            logInfo("PluginManager: not IPlugin interface: " + file.fileName());
            delete pluginObj;
            lib->unload();
            delete lib;
            continue;
        }

        if (!plugin->init(m_host)) {
            logInfo("PluginManager: init FAILED: " + plugin->pluginId());
            delete pluginObj;
            lib->unload();
            delete lib;
            continue;
        }

        m_plugins[plugin->pluginId()] = plugin;
        m_libraries[plugin->pluginId()] = lib;
        logInfo(QString("PluginManager: LOADED id=%1")
                .arg(plugin->pluginId()));
        emit pluginLoaded(plugin);
    }

    logInfo(QString("PluginManager: total loaded=%1").arg(m_plugins.size()));
    return true;
}

/**
 * @brief 注册内置插件（直接实例化，非 DLL 加载）
 * @param plugin 插件实例
 * @return 注册成功返回 true
 */
bool PluginManager::registerPlugin(IPlugin *plugin)
{
    if (!plugin) {
        logInfo("PluginManager: registerPlugin got null");
        return false;
    }
    if (!m_host) {
        logInfo("PluginManager: registerPlugin ERROR no host");
        return false;
    }
    if (m_plugins.contains(plugin->pluginId())) {
        logInfo("PluginManager: registerPlugin duplicate id=" + plugin->pluginId());
        return false;
    }
    if (!plugin->init(m_host)) {
        logInfo("PluginManager: registerPlugin init FAILED: " + plugin->pluginId());
        return false;
    }
    m_plugins[plugin->pluginId()] = plugin;
    logInfo(QString("PluginManager: REGISTERED id=%1 (built-in)")
            .arg(plugin->pluginId()));
    emit pluginLoaded(plugin);
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
 * @brief 根据 ID 获取插件
 * @param pluginId 插件 ID
 * @return 插件指针，未找到返回 nullptr
 */
IPlugin* PluginManager::getPlugin(const QString &pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}

/**
 * @brief 卸载指定插件
 * @param pluginId 插件 ID
 * @return 卸载成功返回 true，插件不存在返回 false
 * @details 调用插件 shutdown() 清理资源、从插件表中移除、卸载动态库，
 *          并在日志中记录卸载信息，最后发出 pluginUnloaded 信号。
 */
bool PluginManager::unloadPlugin(const QString &pluginId)
{
    // 查找待卸载插件，不存在则记录日志并返回失败
    IPlugin *plugin = m_plugins.value(pluginId, nullptr);
    if (!plugin) {
        logInfo("PluginManager: unloadPlugin not found id=" + pluginId);
        return false;
    }

    logInfo(QString("PluginManager: UNLOADING id=%1")
            .arg(pluginId));

    // 调用插件关闭接口，释放插件自身资源（界面、按钮等）
    plugin->shutdown();

    // 从插件映射表中移除该插件
    m_plugins.remove(pluginId);

    // 若为动态库插件，则卸载并释放对应的 QLibrary
    QLibrary *lib = m_libraries.take(pluginId);
    if (lib) {
        lib->unload();
        delete lib;
        logInfo(QString("PluginManager: UNLOADED id=%1 (library unloaded)")
                .arg(pluginId));
    }

    // 发出插件卸载信号，通知宿主清理对应的界面与按钮
    emit pluginUnloaded(plugin);

    logInfo(QString("PluginManager: UNLOADED id=%1")
            .arg(pluginId));
    return true;
}

/**
 * @brief 卸载所有已加载插件
 * @details 逐个卸载全部插件并记录日志，供程序退出或插件重置时调用
 */
void PluginManager::unloadAllPlugins()
{
    logInfo(QString("PluginManager::unloadAllPlugins start, count=%1")
            .arg(m_plugins.size()));
    // 复制 ID 列表，避免卸载过程中迭代器失效
    QList<QString> ids = m_plugins.keys();
    for (const QString &id : ids) {
        unloadPlugin(id);
    }
    logInfo(QString("PluginManager::unloadAllPlugins done, remain=%1")
            .arg(m_plugins.size()));
}
