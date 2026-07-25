#include "sampleplugin.h"

SamplePlugin::SamplePlugin()
    : m_host(nullptr)
{
}

SamplePlugin::~SamplePlugin()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
}

QString SamplePlugin::pluginName() const
{
    return QString("Sample Plugin");
}

QString SamplePlugin::pluginId() const
{
    return QString("sample.plugin");
}

QString SamplePlugin::pluginVersion() const
{
    return QString("1.0.0");
}

bool SamplePlugin::init(IPluginHost *host)
{
    m_host = host;
    if (m_host) {
        m_host->showStatusMessage(QString("Loaded plugin: %1").arg(pluginName()));
        return true;
    }
    return false;
}

void SamplePlugin::shutdown()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
    m_widgets.clear();
}

QWidget* SamplePlugin::createWidget(QWidget *parent)
{
    SampleWidget *widget = new SampleWidget(parent);
    m_widgets.append(widget);
    return widget;
}

void SamplePlugin::destroyWidget(QWidget *widget)
{
    m_widgets.removeOne(static_cast<SampleWidget*>(widget));
    delete widget;
}