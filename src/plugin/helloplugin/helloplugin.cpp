/**
 * @file helloplugin.cpp
 * @brief Hello插件类实现
 * @details 该文件实现了HelloPlugin类的所有方法，是一个简单的演示插件。
 * @date 2026-07-28
 */

#include "helloplugin.h"

/**
 * @brief 构造函数
 */
HelloPlugin::HelloPlugin()
    : m_host(nullptr)
{
}

/**
 * @brief 析构函数
 * @details 销毁所有已创建的界面部件
 */
HelloPlugin::~HelloPlugin()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
}

/**
 * @brief 获取插件名称
 * @return 插件显示名称
 */
QString HelloPlugin::pluginName() const
{
    return QString("Hello");
}

/**
 * @brief 获取插件ID
 * @return 插件唯一标识符
 */
QString HelloPlugin::pluginId() const
{
    return QString("hello.plugin");
}

/**
 * @brief 获取插件版本号
 * @return 版本号字符串
 */
QString HelloPlugin::pluginVersion() const
{
    return QString("1.0.0");
}

/**
 * @brief 初始化插件
 * @param host 插件宿主接口
 * @return 初始化成功返回true
 */
bool HelloPlugin::init(IPluginHost *host)
{
    m_host = host;
    if (m_host) {
        m_host->showStatusMessage(QString("Loaded plugin: %1").arg(pluginName()));
        return true;
    }
    return false;
}

/**
 * @brief 关闭插件
 * @details 清理所有资源
 */
void HelloPlugin::shutdown()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
    m_widgets.clear();
}

/**
 * @brief 创建插件界面
 * @param parent 父窗口部件
 * @return 插件界面部件
 */
QWidget* HelloPlugin::createWidget(QWidget *parent)
{
    HelloWidget *widget = new HelloWidget(parent);
    m_widgets.append(widget);
    return widget;
}

/**
 * @brief 销毁插件界面
 * @param widget 要销毁的界面
 */
void HelloPlugin::destroyWidget(QWidget *widget)
{
    m_widgets.removeOne(static_cast<HelloWidget*>(widget));
    delete widget;
}