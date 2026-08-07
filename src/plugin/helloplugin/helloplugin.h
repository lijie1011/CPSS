/**
 * @file helloplugin.h
 * @brief Hello插件类定义
 * @details 本插件是一个简单的演示插件，用于显示"Hello"消息。
 */

#ifndef HELLOPLUGIN_H
#define HELLOPLUGIN_H

#include <QObject>
#include "IPlugin.h"
#include "HelloWidget.h"

/**
 * @class HelloPlugin
 * @brief Hello插件类
 * @details 继承自QObject和IPlugin接口，实现简单的Hello消息显示功能
 */
class HelloPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    /**
     * @brief 构造函数
     */
    HelloPlugin();
    
    /**
     * @brief 析构函数
     */
    ~HelloPlugin() override;

    /**
     * @brief 获取插件名称
     * @return 插件的显示名称
     */
    QString pluginName() const override;
    
    /**
     * @brief 获取插件ID
     * @return 插件的唯一标识符
     */
    QString pluginId() const override;
    
    /**
     * @brief 获取插件版本号
     * @return 版本号字符串
     */
    QString pluginVersion() const override;

    /**
     * @brief 初始化插件
     * @param host 插件宿主接口
     * @return 初始化成功返回true
     */
    bool init(IPluginHost *host) override;
    
    /**
     * @brief 关闭插件
     * @details 清理资源
     */
    void shutdown() override;

    /**
     * @brief 创建插件界面
     * @param parent 父界面
     * @return 插件界面
     */
    QWidget* createWidget(QWidget *parent = nullptr) override;
    
    /**
     * @brief 销毁插件界面
     * @param widget 要销毁的界面
     */
    void destroyWidget(QWidget *widget) override;

private:
    QList<HelloWidget*> m_widgets;  ///< 已创建的界面列表
    IPluginHost *m_host;             ///< 插件宿主接口
};

#endif
