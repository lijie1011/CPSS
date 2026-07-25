#ifndef SAMPLEPLUGIN_H
#define SAMPLEPLUGIN_H

#include <QObject>
#include "IPlugin.h"
#include "SampleWidget.h"

class SamplePlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    SamplePlugin();
    ~SamplePlugin() override;

    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;

    bool init(IPluginHost *host) override;
    void shutdown() override;

    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

private:
    QList<SampleWidget*> m_widgets;
    IPluginHost *m_host;
};

#endif