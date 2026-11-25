/**
 * @file HtMapPlugin.cpp
 * @brief HT 地图插件实现
 * @date 2026-11-25
 */

#include "HtMapPlugin.h"
#include "logger.h"

HtMapPlugin::HtMapPlugin()
{
}

HtMapPlugin::~HtMapPlugin()
{
    qDeleteAll(m_widgets);
    m_widgets.clear();
}

QString HtMapPlugin::pluginName() const
{
    return QStringLiteral("HT地图");
}

QString HtMapPlugin::pluginId() const
{
    return QStringLiteral("htmapplugin.plugin");
}

QString HtMapPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

bool HtMapPlugin::init(IPluginHost *host)
{
    m_host = host;
    Logger::info("[HtMapPlugin] init done");
    return true;
}

void HtMapPlugin::shutdown()
{
    Logger::info("[HtMapPlugin] shutdown");
}

QWidget* HtMapPlugin::createWidget(QWidget *parent)
{
    MapWidget *w = new MapWidget(parent);
    m_widgets.append(w);
    Logger::info("[HtMapPlugin] widget created");
    return w;
}

QWidget* HtMapPlugin::mapWidget()
{
    return m_widgets.isEmpty() ? nullptr : m_widgets.first();
}

DynamicObjects HtMapPlugin::getDynamicData() const
{
    return m_widgets.isEmpty() ? DynamicObjects() : m_widgets.first()->dynamicData();
}

DisplayStateMap HtMapPlugin::getDisplayStates() const
{
    if (m_widgets.isEmpty() || !m_widgets.first()->situationLayer()) return DisplayStateMap();
    return m_widgets.first()->situationLayer()->displayStates();
}

void HtMapPlugin::setDisplayStates(const DisplayStateMap &states)
{
    if (m_widgets.isEmpty() || !m_widgets.first()->situationLayer()) return;
    m_widgets.first()->situationLayer()->setDisplayStates(states);
}

void HtMapPlugin::destroyWidget(QWidget *widget)
{
    MapWidget *w = qobject_cast<MapWidget*>(widget);
    if (w) m_widgets.removeAll(w);
    delete widget;
}
