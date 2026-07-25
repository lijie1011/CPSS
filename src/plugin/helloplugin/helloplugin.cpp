#include "helloplugin.h"

HelloPlugin::HelloPlugin()
    : m_host(nullptr)
{
}

HelloPlugin::~HelloPlugin()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
}

QString HelloPlugin::pluginName() const
{
    return QString("Hello");
}

QString HelloPlugin::pluginId() const
{
    return QString("hello.plugin");
}

QString HelloPlugin::pluginVersion() const
{
    return QString("1.0.0");
}

bool HelloPlugin::init(IPluginHost *host)
{
    m_host = host;
    if (m_host) {
        m_host->showStatusMessage(QString("Loaded plugin: %1").arg(pluginName()));
        return true;
    }
    return false;
}

void HelloPlugin::shutdown()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
    m_widgets.clear();
}

QWidget* HelloPlugin::createWidget(QWidget *parent)
{
    HelloWidget *widget = new HelloWidget(parent);
    m_widgets.append(widget);
    return widget;
}

void HelloPlugin::destroyWidget(QWidget *widget)
{
    m_widgets.removeOne(static_cast<HelloWidget*>(widget));
    delete widget;
}