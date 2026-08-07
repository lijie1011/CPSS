/**
 * @file DataFlowWidget.h
 * @brief 数据流主界面
 *
 * 该界面以三种不同模式可视化数据流：
 *   - SourcePipeline : 内部处理管道的静态示意图
 *   - PlatformData   : 动态增长的矩形，每个平台 ID 一个
 *   - EventAlert      : 事件/告警传播链的静态示意图
 *
 * 右侧面板提供了流模式选择器、实时统计信息（节点/边计数、
 * 选中信息、缩放级别）、网格开关和缩放控件。
 */

#pragma once

#include <QEvent>
#include <QWidget>
#include <QTimer>
#include <QMap>
#include "NodeFlowWidget.h"
#include "dynamicdata.h"

class QComboBox;
class QLabel;

class DataManager;

/**
 * @class DataFlowWidget
 * @brief 数据流主界面
 *
 * 将节点/边画布（NodeFlowWidget）与侧边控制面板结合，
 * 并绑定到应用级 DataManager，以响应实时数据推送、平台更新
 * 和事件告警。
 */
class DataFlowWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param dataManager 后端数据管理器，其信号驱动更新。
     * @param parent      可选的父界面。
     */
    explicit DataFlowWidget(DataManager *dataManager, QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~DataFlowWidget();

    /**
     * @enum FlowMode
     * @brief 三种可视化模式选择器
     */
    enum FlowMode {
        Mode_SourcePipeline,  ///< 数据源处理管道静态示意图
        Mode_PlatformData,    ///< 动态矩形网格，每个已知平台 ID 一个
        Mode_EventAlert       ///< 事件创建 → 告警传播静态示意图
    };

private slots:
    /**
     * @brief 数据推送处理
     *
     * 当新的动态数据被推送到 DataManager 时触发。
     * 根据当前的 FlowMode，该槽调度画布上的高亮链以可视化方式
     * 重放数据如何流过流图并更新状态标签的计数。
     *
     * @param data 新推送的动态对象（平台 + 事件）。
     */
    void onDataPushed(const DynamicObjects &data);

    /**
     * @brief 数据变化处理
     *
     * 当现有动态数据被原地修改时触发。
     * 在 Mode_PlatformData 中重用 addPlatformNode() 延迟追加
     * 尚未绘制的任何平台 ID。在其他两种模式中委托给
     * onDataPushed()，以便执行相同的高亮链回放。
     *
     * @param data 更新后的动态对象。
     */
    void onDataChanged(const DynamicObjects &data);

    /**
     * @brief 平台更新处理
     *
     * 每当单条平台记录被更新时触发。
     * 仅在 Mode_PlatformData 中有意义，确保对应的节点存在于
     * 画布上（addPlatformNode()）并刷新状态标签。
     *
     * @param platform 平台的最新快照。
     */
    void onPlatformUpdated(const PlatformData &platform);

    /**
     * @brief 事件更新处理
     *
     * 每当 DataManager 产生特殊事件时触发。
     * 构建特定于事件类型（攻击、告警或通用）的高亮链，
     * 使画布动画显示正确的传播路径。
     *
     * @param event 新产生的特殊事件。
     */
    void onEventUpdated(const SpecialEvent &event);

    /**
     * @brief 高亮动画定时器
     *
     * 周期性触发（200 毫秒），逐步遍历待高亮链。
     * 每次触发额外高亮一个节点，产生动画传播效果。
     * 链完全点亮后短暂暂停，清除所有高亮，然后重置链
     * 以备下一次数据突发。
     */
    void onHighlightTimer();

private:
    /**
     * @brief 创建界面，构建分割面板 UI（左画布 + 右控制面板）
     *
     * 创建一个 QSplitter，左侧放 NodeFlowWidget，右侧放 QFrame
     * 控制面板，连接所有控件（模式切换、网格、缩放等的信号/槽），
     * 并设置默认统计信息显示。
     */
    void createUI();

    /**
     * @brief 加载流模式，切换可视化到请求的流模式
     * @param mode 要激活的模式。调用匹配的 setup*() 辅助函数
     *             然后将图适配到视口。
     */
    void loadFlowMode(FlowMode mode);

    /**
     * @brief 加载静态的源流水线示意图
     *
     * 节点表示 数据源 → 协议适配器 → 数据缓存 → 数据推送
     * → 态势显示，并有一个分支指向过期清理。
     */
    void setupSourcePipeline();

    /**
     * @brief 初始化平台数据流
     *
     * 清除之前的平台节点，遍历 DataManager 已知的每个平台，
     * 通过 addPlatformNode() 放置到画布上。后续更新通过
     * onDataChanged() / onPlatformUpdated() 到达。
     */
    void setupPlatformDataFlow();

    /**
     * @brief 加载静态的事件告警示意图
     *
     * 节点表示 武器发射 → 事件创建 → 导弹飞行 →
     * 告警事件，并有分支指向属性框、事件历史和态势图标。
     */
    void setupEventAlertFlow();

    /**
     * @brief 更新图信息，更新控制面板中的节点/边计数行
     * @param nodeCount 当前图中的节点数量。
     * @param edgeCount 当前图中的边数量。
     */
    void updateGraphInfo(int nodeCount, int edgeCount);

    /**
     * @brief 更新选中信息，更新控制面板中的"选中节点"行
     * @param id    节点标识符（若未选中则为 -1）。
     * @param title 选中节点的显示标题。
     */
    void updateSelectedInfo(int id, const QString &title);

    /**
     * @brief 更新缩放信息，更新控制面板中的缩放百分比行
     * @param zoom 当前缩放因子（1.0 = 100%）。
     */
    void updateZoomInfo(double zoom);

    /**
     * @brief 添加平台节点，若不存在则延迟添加到画布
     *
     * 使用 m_platformNodeMap 以平台 ID 为键进行去重。
     * 每个 ID 的首次出现被分配一个网格槽；对已存在平台的更新
     * 在视觉上被忽略。
     *
     * @param platform 要可视化的平台。
     */
    void addPlatformNode(const PlatformData &platform);

    /**
     * @brief 计算平台网格槽的画布坐标
     *
     * 网格固定为 4 列，每个单元格 168×78 像素，水平/垂直间距
     * 分别为 24 / 32 像素，外边距 40 像素。
     *
     * @param index 从零开始的槽索引（行优先顺序）。
     * @return 该槽中节点的左上角画布位置。
     */
    QPointF platformGridPosition(int index) const;

    /**
     * @brief 数据源管理器；所有数据流源自其信号
     */
    DataManager *m_dataManager;

    /**
     * @brief 画布控件；渲染当前活动的流图
     */
    NodeFlowWidget *m_flow;

    /**
     * @brief 模式下拉框；三种 FlowMode 值的选择器
     */
    QComboBox *m_modeCombo;

    /**
     * @brief 图信息标签；显示当前节点和边的计数
     */
    QLabel *m_graphLabel;

    /**
     * @brief 选中信息标签；显示当前选中节点的 ID 和标题
     */
    QLabel *m_selectedLabel;

    /**
     * @brief 缩放信息标签；以百分比显示当前画布缩放
     */
    QLabel *m_zoomLabel;

    /**
     * @brief 状态标签；显示最新的数据/状态摘要
     */
    QLabel *m_statusLabel;

    /**
     * @brief 高亮动画定时器；200 毫秒周期定时器，逐步遍历待高亮链
     */
    QTimer *m_highlightTimer;

    /**
     * @brief 待高亮节点链；按顺序每次 tick 高亮一个节点
     *
     * 由各种 on*() 槽填充；由 onHighlightTimer() 消费。
     * 整个链动画完成并短暂保留后清除。
     */
    QVector<int> m_pendingHighlightChain;

    /**
     * @brief 平台映射；平台 ID → 画布节点 ID，每个平台只绘制一次
     *
     * 由 addPlatformNode() 使用以跳过已可视化的 ID。
     */
    QMap<QString, int> m_platformNodeMap;

    /**
     * @brief 平台节点计数；已添加的平台节点运行计数器，驱动网格槽索引
     */
    int m_platformNodeCounter = 0;
};
