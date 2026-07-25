#include "PluginManager.h"
#include <QDir>
#include <QFileInfo>
#include "common/logger.h"

PluginManager::PluginManager(QObject *parent)
    : QObject(parent),
      m_host(nullptr)
{
}

PluginManager::~PluginManager()
{
    for (auto loader : m_loaders.values()) {
        loader->unload();
        delete loader;
    }
}

void PluginManager::setPluginHost(IPluginHost *host)
{
    m_host = host;
}

bool PluginManager::loadPlugins(const QString &pluginDir)
{
    Logger::info("PluginManager::loadPlugins - loading plugins from: %s", pluginDir.toStdString().c_str());

    if (!m_host) {
        Logger::error("PluginManager::loadPlugins - no host set");
        return false;
    }

    QDir dir(pluginDir);
    if (!dir.exists()) {
        Logger::warn("Plugin directory does not exist: %s", pluginDir.toStdString().c_str());
        return false;
    }

    QStringList filters;
    filters << "*.dll" << "*.so";
    dir.setNameFilters(filters);

    QFileInfoList files = dir.entryInfoList(QDir::Files);
    Logger::info("Found %d potential plugin files", files.size());

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
                    Logger::info("Loaded plugin: %s (%s) v%s", 
                                 plugin->pluginName().toStdString().c_str(),
                                 plugin->pluginId().toStdString().c_str(),
                                 plugin->pluginVersion().toStdString().c_str());
                    emit pluginLoaded(plugin);
                } else {
                    Logger::warn("Plugin init failed: %s", plugin->pluginName().toStdString().c_str());
                    loader->unload();
                    delete loader;
                }
            } else {
                Logger::warn("Plugin file does not implement IPlugin interface: %s", 
                            file.fileName().toStdString().c_str());
                loader->unload();
                delete loader;
            }
        } else {
            Logger::warn("Failed to load plugin: %s - %s", 
                        file.fileName().toStdString().c_str(),
                        loader->errorString().toStdString().c_str());
            delete loader;
        }
    }

    Logger::info("Total loaded plugins: %d", m_plugins.size());
    return true;
}

QList<IPlugin*> PluginManager::getLoadedPlugins() const
{
    return m_plugins.values();
}

IPlugin* PluginManager::getPlugin(const QString &pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}