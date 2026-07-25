#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QPluginLoader>
#include "IPlugin.h"

class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    void setPluginHost(IPluginHost *host);
    bool loadPlugins(const QString &pluginDir);
    QList<IPlugin*> getLoadedPlugins() const;
    IPlugin* getPlugin(const QString &pluginId) const;

signals:
    void pluginLoaded(IPlugin *plugin);
    void pluginUnloaded(IPlugin *plugin);

private:
    IPluginHost *m_host;
    QMap<QString, QPluginLoader*> m_loaders;
    QMap<QString, IPlugin*> m_plugins;
};

#endif