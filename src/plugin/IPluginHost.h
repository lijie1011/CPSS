#ifndef IPLUGINHOST_H
#define IPLUGINHOST_H

#include <QObject>
#include <QString>
#include "viewwidget.h"
#include "datamanager.h"

class IPluginHost
{
public:
    virtual ~IPluginHost() = default;

    virtual ViewWidget* getViewWidget() const = 0;
    virtual DataManager* getDataManager() = 0;
    virtual QString getAppVersion() const = 0;
    virtual QString getAppPath() const = 0;

    virtual void showStatusMessage(const QString &message) = 0;
    virtual void showNotification(const QString &title, const QString &message) = 0;

    virtual bool registerPluginButton(const QString &pluginId, const QString &buttonText) = 0;
    virtual void unregisterPluginButton(const QString &pluginId) = 0;

    virtual bool setActiveWidget(QWidget *widget) = 0;
    virtual void showMapView() = 0;
};

#endif