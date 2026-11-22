/**
 * @file OutputPlugin.cpp
 * @brief 输出插件实现
 * @date 2026-11-22
 */

#include "OutputPlugin.h"
#include "logger.h"

OutputPlugin::OutputPlugin()
{
}

OutputPlugin::~OutputPlugin()
{
}

QString OutputPlugin::pluginName() const
{
    return QStringLiteral("控制台输出");
}

QString OutputPlugin::pluginId() const
{
    return QStringLiteral("output.plugin");
}

QString OutputPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

bool OutputPlugin::init(IPluginHost *host)
{
    m_host = host;
    Logger::info("[OutputPlugin] init done");
    return true;
}

void OutputPlugin::shutdown()
{
    Logger::info("[OutputPlugin] shutdown");
}

QWidget* OutputPlugin::createWidget(QWidget *parent)
{
    OutputWidget *widget = new OutputWidget(parent);
    m_widgets.append(widget);
    Logger::info("[OutputPlugin] widget created");
    return widget;
}

void OutputPlugin::destroyWidget(QWidget *widget)
{
    OutputWidget *w = qobject_cast<OutputWidget*>(widget);
    if (w) m_widgets.removeAll(w);
    delete widget;
}
