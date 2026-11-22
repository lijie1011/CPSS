/**
 * @file OutputPlugin.h
 * @brief 输出插件类定义
 * @details 提供全局输出面板，显示其他模块通过 ut::log 输出的消息。
 * @date 2026-11-22
 */

#ifndef OUTPUTPLUGIN_H
#define OUTPUTPLUGIN_H

#include <QObject>
#include <QList>
#include "IPlugin.h"
#include "OutputWidget.h"

class OutputPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    explicit OutputPlugin();
    ~OutputPlugin() override;

    QString pluginName() const override;
    QString pluginId() const override;
    QString pluginVersion() const override;
    bool init(IPluginHost *host) override;
    void shutdown() override;
    QObject *object() override { return this; }
    QWidget* createWidget(QWidget *parent = nullptr) override;
    void destroyWidget(QWidget *widget) override;

private:
    QList<OutputWidget*> m_widgets;
    IPluginHost *m_host = nullptr;
};

#endif // OUTPUTPLUGIN_H
