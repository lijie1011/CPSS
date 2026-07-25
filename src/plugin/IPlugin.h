#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QWidget>
#include "IPluginHost.h"

#define PLUGIN_INTERFACE_IID "com.cpss.plugin.IPlugin/1.0"

class IPlugin
{
public:
    virtual ~IPlugin() = default;

    virtual QString pluginName() const = 0;
    virtual QString pluginId() const = 0;
    virtual QString pluginVersion() const = 0;

    virtual bool init(IPluginHost *host) = 0;
    virtual void shutdown() = 0;

    virtual QWidget* createWidget(QWidget *parent = nullptr) = 0;
    virtual void destroyWidget(QWidget *widget) = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(IPlugin, PLUGIN_INTERFACE_IID)
QT_END_NAMESPACE

#endif