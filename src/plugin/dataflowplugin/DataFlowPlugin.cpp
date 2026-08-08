/**
 * @file DataFlowPlugin.cpp
 * @brief 数据流插件实现
 *
 * @details 实现 DataFlowPlugin 插件接口：通过宿主注入共享的
 *          DataManager，创建并跟踪 DataFlowWidget 可视化界面，
 *          在插件生命周期内负责界面的创建与销毁。
 */

#include "DataFlowPlugin.h"
#include "IPluginHost.h"
#include "datamanager.h"

/**
 * @brief 构造函数，将宿主指针初始化为 nullptr
 *
 * 插件宿主在构造时不可用，稍后通过 init() 注入。
 */
DataFlowPlugin::DataFlowPlugin()
    : m_host(nullptr)
{
}

/**
 * @brief 析构函数，显式删除所有已跟踪的界面
 *
 * 无论析构前是否调用了 shutdown()，都能确定性清理 m_widgets 中的所有界面。
 */
DataFlowPlugin::~DataFlowPlugin()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
}

/**
 * @brief 返回插件显示名称
 * @return 插件的人类可读名称 "Data Flow Monitor"
 */
QString DataFlowPlugin::pluginName() const
{
    return QStringLiteral("数据流监控");
}

/**
 * @brief 返回插件唯一标识符
 * @return 插件的机器可读 ID "dataflow.plugin"
 */
QString DataFlowPlugin::pluginId() const
{
    return QStringLiteral("dataflow.plugin");
}

/**
 * @brief 返回插件版本号
 * @return 语义化版本字符串 "1.0.0"
 */
QString DataFlowPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

/**
 * @brief 初始化插件，注入宿主指针
 *
 * 保存宿主指针，以便后续 createWidget() 调用能通过它访问共享的 DataManager。
 *
 * @param host 插件宿主接口指针
 * @return 宿主非空返回 true，否则返回 false
 */
bool DataFlowPlugin::init(IPluginHost *host)
{
    m_host = host;
    if (m_host) {
        return true;
    }
    return false;
}

/**
 * @brief 关闭插件，释放所有界面并注销按钮
 *
 * 释放插件创建的所有界面。宿主可能在此调用后立即卸载库，
 * 因此不能依赖 Qt 事件循环的清理。随后使用插件 ID 作为查找键，
 * 注销之前与该插件关联的任何工具栏/菜单按钮。
 */
void DataFlowPlugin::shutdown()
{
    for (auto widget : m_widgets) {
        delete widget;
    }
    m_widgets.clear();

    if (m_host) {
        m_host->unregisterPluginButton(pluginId());
    }
}

/**
 * @brief 创建数据流监控界面
 *
 * 安全地从宿主获取 DataManager（当没有宿主可用时回退到 nullptr，
 * 使界面在独立测试中仍可以被实例化），创建 DataFlowWidget 并将其
 * 加入跟踪列表，以便 destroyWidget() 和析构函数能清理它。
 *
 * @param parent 界面父对象
 * @return 新创建的 DataFlowWidget 实例
 */
QWidget* DataFlowPlugin::createWidget(QWidget *parent)
{
    DataManager *dm = m_host ? m_host->getDataManager() : nullptr;
    DataFlowWidget *widget = new DataFlowWidget(dm, parent);

    m_widgets.append(widget);
    return widget;
}

/**
 * @brief 销毁指定的界面
 *
 * 先从跟踪列表中移除，避免析构函数重复删除，然后释放界面对象。
 *
 * @param widget 要销毁的界面，必须是由 createWidget() 创建的 DataFlowWidget
 */
void DataFlowPlugin::destroyWidget(QWidget *widget)
{
    m_widgets.removeOne(static_cast<DataFlowWidget*>(widget));
    delete widget;
}
