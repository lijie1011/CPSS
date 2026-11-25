/**
 * @file HtMapPlugin.h
 * @brief HT 地图插件（本地离线瓦片 / GeoServer WMTS 双模式）
 * @date 2026-11-25
 */

#ifndef HTMAPPLUGIN_H
#define HTMAPPLUGIN_H

#include <QObject>
#include <QList>
#include "IPlugin.h"
#include "IMapView.h"
#include "MapWidget.h"

class HtMapPlugin : public QObject, public IPlugin, public IMapView
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin IMapView)

public:
    explicit HtMapPlugin();
    ~HtMapPlugin() override;

    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;
    bool init(IPluginHost *host) override;
    void shutdown() override;
    QObject *object() override { return this; }
    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

    // ---- IMapView 通用地图接口 ----
    QWidget* mapWidget() override;
    DynamicObjects getDynamicData() const override;
    DisplayStateMap getDisplayStates() const override;
    void setDisplayStates(const DisplayStateMap &states) override;

private:
    QList<MapWidget*> m_widgets;
    IPluginHost *m_host = nullptr;
};

#endif // HTMAPPLUGIN_H
