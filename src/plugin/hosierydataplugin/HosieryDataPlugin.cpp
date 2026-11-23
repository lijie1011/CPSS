/**
 * @file HosieryDataPlugin.cpp
 * @brief 历史数据回放插件实现
 * @date 2026-11-23
 */

#include "HosieryDataPlugin.h"
#include "logger.h"

HosieryDataPlugin::HosieryDataPlugin()
{
}

HosieryDataPlugin::~HosieryDataPlugin()
{
    qDeleteAll(m_widgets);
    m_widgets.clear();
}

QString HosieryDataPlugin::pluginName() const
{
    return QStringLiteral("历史数据回放");
}

QString HosieryDataPlugin::pluginId() const
{
    return QStringLiteral("hosierydata.plugin");
}

QString HosieryDataPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

bool HosieryDataPlugin::init(IPluginHost *host)
{
    m_host = host;
    Logger::info("[HosieryDataPlugin] init done");
    return true;
}

void HosieryDataPlugin::shutdown()
{
    Logger::info("[HosieryDataPlugin] shutdown");
}

QWidget* HosieryDataPlugin::createWidget(QWidget *parent)
{
    ReplayWidget *widget = new ReplayWidget(parent);
    m_widgets.append(widget);
    Logger::info("[HosieryDataPlugin] widget created");
    return widget;
}

void HosieryDataPlugin::destroyWidget(QWidget *widget)
{
    ReplayWidget *w = qobject_cast<ReplayWidget*>(widget);
    if (w) m_widgets.removeAll(w);
    delete widget;
}
