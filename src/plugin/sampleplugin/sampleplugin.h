/**
 * @file sampleplugin.h
 * @brief 示例插件类定义
 * @details 该插件是CPSS插件系统的示例实现，展示了如何创建一个完整的插件。
 * @date 2026-07-28
 */

#ifndef SAMPLEPLUGIN_H
#define SAMPLEPLUGIN_H

#include <QObject>
#include "IPlugin.h"
#include "SampleWidget.h"

/**
 * @class SamplePlugin
 * @brief 示例插件类
 * @details 继承自QObject和IPlugin接口，实现基本的插件功能
 */
class SamplePlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    /**
     * @brief 构造函数
     */
    SamplePlugin();
    
    /**
     * @brief 析构函数
     */
    ~SamplePlugin() override;

    /**
     * @brief 获取插件名称
     * @return 插件显示名称
     */
    QString pluginName() const override;
    
    /**
     * @brief 获取插件ID
     * @return 插件唯一标识符
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
     * @param parent 父窗口部件
     * @return 插件界面部件
     */
    QWidget* createWidget(QWidget *parent = nullptr) override;
    
    /**
     * @brief 销毁插件界面
     * @param widget 要销毁的界面
     */
    void destroyWidget(QWidget *widget) override;

private:
    QList<SampleWidget*> m_widgets;  ///< 已创建的界面列表
    IPluginHost *m_host;             ///< 插件宿主接口
};

#endif