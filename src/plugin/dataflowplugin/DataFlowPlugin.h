/**
 * @file DataFlowPlugin.h
 * @brief 数据流插件定义
 *
 * @details DataFlowPlugin 是一个基于 Qt 的插件，集成到应用程序的
 *          插件框架中。它提供一个监控界面，用于可视化系统中数据的
 *          实时流转。该插件实现了 IPlugin 接口，并向宿主应用程序
 *          注册自身，使用户可以创建一个或多个 DataFlowWidget 实例
 *          来检查运行中的数据管道。
 */

#pragma once

#include <QObject>
#include "IPlugin.h"
#include "DataFlowWidget.h"

/**
 * @class DataFlowPlugin
 * @brief 数据流监控插件类
 *
 * @details DataFlowPlugin 是 IPlugin 接口的具体实现类。
 *          负责提供元数据（名称、ID、版本）、管理插件生命周期
 *          （初始化 / 关闭），以及按需创建和销毁 DataFlowWidget 实例。
 *          插件会跟踪它创建的每一个界面，以便在宿主卸载插件或
 *          请求关闭时，所有资源都能被正确清理。
 *
 *          该类使用 Qt 的元对象系统，并通过 Q_PLUGIN_METADATA 和
 *          Q_INTERFACES 宏导出为 Qt 插件。
 */
class DataFlowPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_IID)
    Q_INTERFACES(IPlugin)

public:
    /**
     * @brief 构造函数
     *
     * @details 使用空的宿主指针初始化插件。实际的宿主引用将在稍后
     *          插件管理器调用 init() 时提供。
     */
    DataFlowPlugin();

    /**
     * @brief 析构函数
     *
     * @details 删除 m_widgets 列表中跟踪的所有 DataFlowWidget 实例。
     *          由于界面由插件所有，析构函数保证即使从未显式调用
     *          destroyWidget() 也不会泄漏任何界面。
     */
    ~DataFlowPlugin() override;

    /**
     * @brief 获取插件名称
     * @return 包含插件名称的 QString（"Data Flow Monitor"）。
     */
    QString pluginName() const override;

    /**
     * @brief 获取插件ID
     * @return 包含插件ID的 QString（"dataflow.plugin"）。
     *
     * @note 宿主使用该 ID 来注册/注销特定于插件的界面元素，
     *       例如工具栏按钮。
     */
    QString pluginId() const override;

    /**
     * @brief 获取插件版本
     * @return 包含插件语义版本字符串的 QString（例如 "1.0.0"）。
     */
    QString pluginVersion() const override;

    /**
     * @brief 初始化插件
     *
     * @param host 指向拥有此插件的 IPluginHost 的指针。不能为空指针
     *             才能初始化成功。
     * @return 如果提供了宿主且插件已准备就绪则返回 true；
     *         如果 @p host 为空则返回 false。
     *
     * @note 通常由插件管理器在插件加载后立即调用一次。
     */
    bool init(IPluginHost *host) override;

    /**
     * @brief 关闭插件
     *
     * @details 删除每个被跟踪的界面，清空界面列表，并请求宿主
     *          注销任何与该插件 ID 关联的插件按钮。即使 init()
     *          从未成功过也可以安全调用。
     */
    void shutdown() override;

    /**
     * @brief 创建界面
     *
     * @param parent 新 DataFlowWidget 的可选父界面。默认为
     *               nullptr（无父界面）。
     * @return 指向新创建 QWidget 的指针。所有权仍归插件所有；
     *         调用方应使用 destroyWidget() 来释放它。
     *
     * @note 该界面已连接到宿主的 DataManager，因此可以显示
     *       实时数据。如果没有可用的宿主，界面仍然会被创建，
     *       但将在没有数据源的情况下运行。
     */
    QWidget* createWidget(QWidget *parent = nullptr) override;

    /**
     * @brief 销毁界面
     *
     * @param widget 要销毁的界面指针。必须是由本插件产生的
     *               DataFlowWidget 实例。
     *
     * @details 从内部跟踪列表中移除该界面并删除它。
     *          传入非本插件创建的界面会导致未定义行为。
     */
    void destroyWidget(QWidget *widget) override;

private:
    QList<DataFlowWidget*> m_widgets;   /**< 插件持有的所有数据流界面实例列表 */
    IPluginHost *m_host;                /**< 宿主接口指针，由init()设置，可能为空 */
};
