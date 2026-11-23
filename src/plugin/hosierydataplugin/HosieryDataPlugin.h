/**
 * @file HosieryDataPlugin.h
 * @brief 历史数据回放插件（本地文件版）
 * @date 2026-11-23
 */

#ifndef HOSIERYDATAPLUGIN_H
#define HOSIERYDATAPLUGIN_H

#include <QObject>
#include <QList>
#include "IPlugin.h"
#include "ReplayWidget.h"

class HosieryDataPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    explicit HosieryDataPlugin();
    ~HosieryDataPlugin() override;

    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;
    bool init(IPluginHost *host) override;
    void shutdown() override;
    QObject *object() override { return this; }
    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

private:
    QList<ReplayWidget*> m_widgets;
    IPluginHost *m_host = nullptr;
};

#endif // HOSIERYDATAPLUGIN_H
