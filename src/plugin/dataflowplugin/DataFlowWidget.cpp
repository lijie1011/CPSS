/**
 * @file DataFlowWidget.cpp
 * @brief 数据流主界面实现 — DataFlowPlugin 的主可视化界面
 *
 * 构建分割面板 UI，连接 DataManager 信号到槽，实现三种流模式
 * 构建器（SourcePipeline、PlatformData、EventAlert），并提供
 * 高亮链步进动画，以可视化方式重放数据/事件如何在图中传播。
 */

#include "DataFlowWidget.h"

#include "common/logger.h"
#include "datamanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QVBoxLayout>

namespace
{

/**
 * @brief 为节点挑选六种预定义颜色之一
 * @param index 调色板的索引；使用模运算，因此任何非负输入都有效。
 * @return 从调色板中选择的 QColor。
 */
QColor nodeColor(int index)
{
    switch (index % 6) {
    case 1: return QColor("#1F9D63");
    case 2: return QColor("#D87516");
    case 3: return QColor("#7A5CFA");
    case 4: return QColor("#E9573F");
    case 5: return QColor("#6F7FA6");
    default: return QColor("#2E7DD1");
    }
}

/**
 * @brief 便捷工厂，填充 NodeFlowWidget::Node 结构体
 *
 * 集中节点描述符的构建，使每个调用位置都能提供
 * id/title/subtitle/color/position，而无需重复 168×78
 * 矩形分配。
 *
 * @param id      唯一节点标识符。
 * @param title   节点内显示的粗体标题。
 * @param subtitle 标题下方显示的较小次行。
 * @param color   节点矩形的填充色。
 * @param x       画布左坐标。
 * @param y       画布上坐标。
 * @return 完全填充的 NodeFlowWidget::Node（矩形为 168×78）。
 */
NodeFlowWidget::Node node(int id, const QString &title, const QString &subtitle, const QColor &color, double x, double y)
{
    NodeFlowWidget::Node item;
    item.id = id; item.title = title; item.subtitle = subtitle;
    item.color = color; item.rect = QRectF(x, y, 168, 78);
    return item;
}

} // namespace

/**
 * @brief 构造 DataFlowWidget
 *
 * 初始化高亮定时器（200 毫秒，非单次触发），通过 createUI()
 * 构建分割面板 UI，加载默认流模式（Mode_PlatformData），
 * 并连接驱动实时更新的 DataManager 信号。
 */
DataFlowWidget::DataFlowWidget(DataManager *dataManager, QWidget *parent)
    : QWidget(parent)
    , m_dataManager(dataManager)
    , m_highlightTimer(new QTimer(this))
{
    setStyleSheet("background-color: #f5f5f5;");

    createUI();
    loadFlowMode(Mode_PlatformData);

    if (m_dataManager) {
        // 信号1：当新采集的数据被推送到管理器时发射
        connect(m_dataManager, &DataManager::dataPushed, this, &DataFlowWidget::onDataPushed);
        // 信号2：单条平台记录变化 → 平台节点增量刷新（用于 PlatformData 模式）
        connect(m_dataManager, &DataManager::platformUpdated, this, &DataFlowWidget::onPlatformUpdated);
    }

    m_highlightTimer->setSingleShot(false);
    m_highlightTimer->setInterval(200);
    connect(m_highlightTimer, &QTimer::timeout, this, &DataFlowWidget::onHighlightTimer);
    m_highlightTimer->start();
}

/** @brief 析构函数（定时器作为子对象所有，无需显式清理） */
DataFlowWidget::~DataFlowWidget()
{
}

/**
 * @brief 创建界面，构建分割面板 UI
 *
 * 布局层级：
 *   - 根 QVBoxLayout 包含一个 QSplitter(Qt::Horizontal)
 *     - 左面板（"画布面板"）：QVBoxLayout → NodeFlowWidget m_flow
 *     - 右面板（"控制面板"）：QVBoxLayout → 标题标签、图信息标签、
 *       选中标签、缩放标签、状态标签、FlowMode 组合框、
 *       网格复选框、缩放按钮行
 *
 * 信号连接：
 *   - m_flow → graphChanged  → updateGraphInfo()
 *   - m_flow → nodeSelected  → updateSelectedInfo()
 *   - m_flow → viewChanged   → updateZoomInfo()
 *   - m_modeCombo → currentIndexChanged → loadFlowMode()
 *   - gridCheck   → toggled             → m_flow->setGridVisible()
 *   - fit/zoom buttons                  → m_flow 导航方法
 */
void DataFlowWidget::createUI()
{
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    // --- 左面板：画布面板 ---
    auto *canvasPanel = new QFrame(splitter);
    canvasPanel->setObjectName("DataFlowCanvasPanel");
    canvasPanel->setStyleSheet(
        "QFrame#DataFlowCanvasPanel{background:#f7f9fb;border:1px solid #dce3e8;border-radius:8px;}");
    auto *canvasLayout = new QVBoxLayout(canvasPanel);
    canvasLayout->setContentsMargins(8, 8, 8, 8);

    m_flow = new NodeFlowWidget(canvasPanel);
    canvasLayout->addWidget(m_flow, 1);

    connect(m_flow, &NodeFlowWidget::graphChanged, this, &DataFlowWidget::updateGraphInfo);
    connect(m_flow, &NodeFlowWidget::nodeSelected, this, &DataFlowWidget::updateSelectedInfo);
    connect(m_flow, &NodeFlowWidget::viewChanged, this, &DataFlowWidget::updateZoomInfo);

    splitter->addWidget(canvasPanel);

    // --- 右面板：控制面板 ---
    auto *panel = new QFrame(splitter);
    panel->setObjectName("DataFlowControlPanel");
    panel->setMinimumWidth(260);
    panel->setStyleSheet(
        "QFrame#DataFlowControlPanel{background:#ffffff;border:1px solid #dce3e8;border-radius:8px;}");
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *panelTitle = new QLabel(QStringLiteral("数据流监控"), panel);
    QFont titleFont = panelTitle->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    panelTitle->setFont(titleFont);

    m_graphLabel = new QLabel(panel);
    m_selectedLabel = new QLabel(panel);
    m_zoomLabel = new QLabel(panel);
    m_statusLabel = new QLabel(panel);
    m_statusLabel->setStyleSheet("color:#E9573F; font-weight:bold;");
    for (QLabel *label : {m_graphLabel, m_selectedLabel, m_zoomLabel}) {
        label->setWordWrap(true);
        label->setStyleSheet("color:#56636d;");
    }

    m_modeCombo = new QComboBox(panel);
    m_modeCombo->addItems({
        QStringLiteral("平台数据流"),
        QStringLiteral("监控平台")
    });

    auto *gridCheck = new QCheckBox(QStringLiteral("显示网格"), panel);
    gridCheck->setChecked(true);

    auto *fitButton = new QPushButton(QStringLiteral("适应窗口"), panel);
    auto *zoomInButton = new QPushButton(QStringLiteral("放大"), panel);
    auto *zoomOutButton = new QPushButton(QStringLiteral("缩小"), panel);

    fitButton->setMinimumHeight(28);
    zoomInButton->setMinimumHeight(28);
    zoomOutButton->setMinimumHeight(28);

    auto *zoomLayout = new QHBoxLayout;
    zoomLayout->setSpacing(6);
    zoomLayout->addWidget(fitButton);
    zoomLayout->addWidget(zoomInButton);
    zoomLayout->addWidget(zoomOutButton);

    layout->addWidget(panelTitle);
    layout->addWidget(m_graphLabel);
    layout->addWidget(m_selectedLabel);
    layout->addWidget(m_zoomLabel);
    layout->addWidget(m_statusLabel);
    layout->addSpacing(4);
    layout->addWidget(new QLabel(QStringLiteral("流程模式"), panel));
    layout->addWidget(m_modeCombo);
    layout->addWidget(gridCheck);
    layout->addLayout(zoomLayout);

    auto *importButton = new QPushButton(QStringLiteral("导入 JSON"), panel);
    importButton->setMinimumHeight(32);
    importButton->setStyleSheet(
        "QPushButton{background:#6F7FA6;color:white;border:none;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#5C6A8C;}");

    auto *exportButton = new QPushButton(QStringLiteral("导出 JSON"), panel);
    exportButton->setMinimumHeight(32);
    exportButton->setStyleSheet(
        "QPushButton{background:#2E7DD1;color:white;border:none;border-radius:4px;font-weight:bold;}"
        "QPushButton:hover{background:#256DB8;}");

    layout->addWidget(importButton);
    layout->addWidget(exportButton);
    layout->addStretch(1);

    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) { loadFlowMode(static_cast<FlowMode>(idx)); });
    connect(gridCheck, &QCheckBox::toggled, m_flow, &NodeFlowWidget::setGridVisible);
    connect(fitButton, &QPushButton::clicked, m_flow, &NodeFlowWidget::fitToView);
    connect(zoomInButton, &QPushButton::clicked, m_flow, &NodeFlowWidget::zoomIn);
    connect(zoomOutButton, &QPushButton::clicked, m_flow, &NodeFlowWidget::zoomOut);
    connect(exportButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(
            this, QStringLiteral("导出图为 JSON"),
            QString(), QStringLiteral("JSON 文件 (*.json)"));
        if (fileName.isEmpty()) return;
        if (!fileName.endsWith(".json", Qt::CaseInsensitive)) fileName += ".json";
        bool ok = m_flow->exportJson(fileName);
        if (!ok) {
            QMessageBox::warning(this, QStringLiteral("导出失败"),
                                 QStringLiteral("无法写入文件：%1").arg(fileName));
        }
    });
    connect(importButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(
            this, QStringLiteral("从 JSON 导入图"),
            QString(), QStringLiteral("JSON 文件 (*.json)"));
        if (fileName.isEmpty()) return;
        bool ok = m_flow->importJson(fileName);
        if (!ok) {
            QMessageBox::warning(this, QStringLiteral("导入失败"),
                                 QStringLiteral("文件格式无效或无法读取：%1").arg(fileName));
        }
    });

    splitter->addWidget(panel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 12, 12, 12);
    rootLayout->setSpacing(8);
    rootLayout->addWidget(splitter);

    updateGraphInfo(0, 0);
    updateSelectedInfo(-1, QString());
    updateZoomInfo(1.0);
    m_statusLabel->setText(QStringLiteral("等待数据..."));
}

/**
 * @brief 加载流模式，分发到模式特定的 setup*() 辅助函数然后适应视图
 * @param mode 用户选择或启动时的 FlowMode。
 */
void DataFlowWidget::loadFlowMode(FlowMode mode)
{
    switch (mode) {
    case Mode_PlatformData: setupPlatformDataFlow(); break;
    case Mode_MonitorPlatform: setupMonitorPlatform(); break;
    }
    m_flow->fitToView();
}

/**
 * @brief 初始化平台数据动态流
 *
 * 重置 ID→节点映射和计数器，清空画布，然后遍历 DataManager
 * 已知的每个平台，使画布初始化为实时平台列表。
 */
void DataFlowWidget::setupPlatformDataFlow()
{
    m_platformNodeMap.clear();
    m_platformNodeCounter = 0;
    m_flow->clearGraph();

    if (m_dataManager) {
        for (const PlatformData &platform : m_dataManager->getAllPlatforms()) {
            addPlatformNode(platform);
        }
    }
}

/**
 * @brief 用监控平台树形结构图填充画布
 *
 * 自上而下的树形层级（连线方向全部为上层指向下层）：
 *   - 第一排：传感器1 ~ 传感器5（数据采集末端）
 *   - 第二排：态势（汇总传感器信息）
 *   - 第三排：本指（本级指挥）
 *   - 第四排：对空分系统 / 对水分系统
 *   - 第五排：对空分系统下挂 H10 / jp / 电子对抗；
 *             对水分系统下挂 声纳 / 鱼雷
 */
void DataFlowWidget::setupMonitorPlatform()
{
    QVector<NodeFlowWidget::Node> nodes = {
        // 第一排：五个传感器
        node(1, QStringLiteral("传感器1"), QStringLiteral("传感/采集"), nodeColor(0), 40, 40),
        node(2, QStringLiteral("传感器2"), QStringLiteral("传感/采集"), nodeColor(1), 238, 40),
        node(3, QStringLiteral("传感器3"), QStringLiteral("传感/采集"), nodeColor(2), 436, 40),
        node(4, QStringLiteral("传感器4"), QStringLiteral("传感/采集"), nodeColor(3), 634, 40),
        node(5, QStringLiteral("传感器5"), QStringLiteral("传感/采集"), nodeColor(4), 832, 40),
        // 第二排：态势
        node(6, QStringLiteral("态势"), QStringLiteral("综合态势"), nodeColor(5), 436, 210),
        // 第三排：本指
        node(7, QStringLiteral("本指"), QStringLiteral("本级指挥"), nodeColor(0), 436, 380),
        // 第四排：对空分系统 / 对水分系统
        node(8, QStringLiteral("对空分系统"), QStringLiteral("防空/对空"), nodeColor(1), 196, 550),
        node(9, QStringLiteral("对水分系统"), QStringLiteral("反潜/对水"), nodeColor(2), 696, 550),
        // 第五排：对空下挂 H10 / jp / 电子对抗；对水下挂 声纳 / 鱼雷
        node(10, QStringLiteral("H10"), QStringLiteral("装备/H10"), nodeColor(3), 4, 720),
        node(11, QStringLiteral("jp"), QStringLiteral("装备/jp"), nodeColor(4), 196, 720),
        node(12, QStringLiteral("电子对抗"), QStringLiteral("电子战"), nodeColor(5), 388, 720),
        node(13, QStringLiteral("声纳"), QStringLiteral("水声探测"), nodeColor(0), 600, 720),
        node(14, QStringLiteral("鱼雷"), QStringLiteral("水声武器"), nodeColor(1), 792, 720),
    };
    QVector<NodeFlowWidget::Edge> edges = {
        // 传感器 → 态势（上下连接：源底 → 目标顶）
        {1, 6, QStringLiteral("传感"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {2, 6, QStringLiteral("传感"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {3, 6, QStringLiteral("传感"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {4, 6, QStringLiteral("传感"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {5, 6, QStringLiteral("传感"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        // 态势 → 本指
        {6, 7, QStringLiteral("汇总"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        // 本指 → 对空/对水分系统
        {7, 8, QStringLiteral("对空"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {7, 9, QStringLiteral("对水"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        // 对空分系统 → 下挂装备
        {8, 10, QStringLiteral("装备"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {8, 11, QStringLiteral("装备"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {8, 12, QStringLiteral("装备"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        // 对水分系统 → 下挂装备
        {9, 13, QStringLiteral("探测"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
        {9, 14, QStringLiteral("武器"), NodeFlowWidget::AnchorSide::Bottom, NodeFlowWidget::AnchorSide::Top},
    };
    m_flow->setNodes(nodes);
    m_flow->setEdges(edges);
}

/**
 * @brief 格式化 "Nodes: X, Edges: Y" 行
 * @param nodeCount 当前画布上绘制的节点数量。
 * @param edgeCount 当前画布上绘制的边数量。
 */
void DataFlowWidget::updateGraphInfo(int nodeCount, int edgeCount)
{
    m_graphLabel->setText(QStringLiteral("节点数: %1, 连线数: %2").arg(nodeCount).arg(edgeCount));
}

/**
 * @brief 更新 "Selected: #id title" 行（或 id < 0 时显示 "None"）
 * @param id    节点标识符；负数表示无选择。
 * @param title 选中节点的标题。
 */
void DataFlowWidget::updateSelectedInfo(int id, const QString &title)
{
    if (id < 0) {
        m_selectedLabel->setText(QStringLiteral("选中: 无"));
        return;
    }
    m_selectedLabel->setText(QStringLiteral("选中: #%1 %2").arg(id).arg(title));
}

/**
 * @brief 将缩放因子格式化为整数百分比
 * @param zoom 当前缩放因子（1.0 → "100%"）。
 */
void DataFlowWidget::updateZoomInfo(double zoom)
{
    m_zoomLabel->setText(QStringLiteral("缩放: %1%").arg(QString::number(zoom * 100.0, 'f', 0)));
}

/**
 * @brief DataManager::dataPushed 的槽
 *
 * 构建特定于模式的高亮链，使画布动画显示传播路径，并向
 * 状态标签写入摘要行。
 *   - Mode_PlatformData:   高亮默认平台数据路径。
 *   - Mode_MonitorPlatform: 高亮传感器 → 态势 → 本指 主链路。
 *
 * @param data 新推送的动态对象。
 */
void DataFlowWidget::onDataPushed(const DynamicObjects &data)
{
    Q_UNUSED(data);
    FlowMode mode = static_cast<FlowMode>(m_modeCombo->currentIndex());
    switch (mode) {
    case Mode_PlatformData:
        m_pendingHighlightChain = {1, 2, 3, 4, 5, 7};
        m_statusLabel->setText(QStringLiteral("平台已更新: %1 条").arg(data.platforms.size()));
        break;
    case Mode_MonitorPlatform:
        // 高亮 传感器 → 态势 → 本指 主链路，示意监控数据汇总上报
        m_pendingHighlightChain = {1, 2, 3, 4, 5, 6, 7};
        m_statusLabel->setText(QStringLiteral("监控平台数据已更新"));
        break;
    }
}

/**
 * @brief DataManager::dynamicDataChanged 的槽
 *
 * 在 Mode_PlatformData 中延迟添加尚未在画布上的任何平台 ID
 * （addPlatformNode 是幂等的）。在其他所有模式中将调用转发给
 * onDataPushed()，以便运行相同的高亮回放。
 *
 * @param data 变更后的动态对象。
 */
void DataFlowWidget::onDataChanged(const DynamicObjects &data)
{
    FlowMode mode = static_cast<FlowMode>(m_modeCombo->currentIndex());
    if (mode == Mode_PlatformData) {
        for (auto it = data.platforms.constBegin(); it != data.platforms.constEnd(); ++it) {
            addPlatformNode(it.value());
        }
        m_statusLabel->setText(QStringLiteral("平台数: %1").arg(m_platformNodeMap.size()));
        return;
    }
    onDataPushed(data);
}

/**
 * @brief DataManager::platformUpdated 的槽
 *
 * 仅在 Mode_PlatformData 中生效，确保平台节点存在
 * （addPlatformNode 跳过重复项）并刷新状态行。
 *
 * @param platform 平台记录的最新快照。
 */
void DataFlowWidget::onPlatformUpdated(const PlatformData &platform)
{
    FlowMode mode = static_cast<FlowMode>(m_modeCombo->currentIndex());
    if (mode == Mode_PlatformData) {
        addPlatformNode(platform);
        m_statusLabel->setText(QStringLiteral("平台 %1 已更新").arg(platform.id));
    }
}

/**
 * @brief 高亮动画定时器，每 200 毫秒步进 m_pendingHighlightChain
 *
 * 动画逻辑：
 *   1. 若链为空则返回（无动画内容）。
 *   2. 使用静态 chainId 检测何时安装了新链：
 *      reinterpret_cast<quintptr>(&m_pendingHighlightChain)
 *      提供稳定的基于指针的 ID。当指针变化（新批次数据到达）
 *      或 stepIndex 已超出链末尾时，步进索引重置为 0。
 *   3. 每次 tick，调用 highlightNodeChain 并传入前缀
 *      [0 .. stepIndex+1]，使每次 tick 多亮一个节点。
 *   4. 到达最后一个节点后，完整链保持点亮 600 毫秒，
 *      然后清除所有高亮并清空链。
 *
 * 这产生一个动画"传播"可视化，重放数据/事件在图中经过的路径。
 */
void DataFlowWidget::onHighlightTimer()
{
    if (m_pendingHighlightChain.isEmpty()) return;

    static int stepIndex = 0;
    static quintptr chainId = 0;

    quintptr currentId = reinterpret_cast<quintptr>(&m_pendingHighlightChain);
    if (chainId != currentId || stepIndex >= m_pendingHighlightChain.size()) {
        stepIndex = 0;
        chainId = currentId;
    }

    QVector<int> currentChain = m_pendingHighlightChain.mid(0, stepIndex + 1);
    m_flow->highlightNodeChain(currentChain);

    stepIndex++;
    if (stepIndex >= m_pendingHighlightChain.size()) {
        m_flow->highlightNodeChain(m_pendingHighlightChain);
        QTimer::singleShot(600, this, [this]() {
            m_flow->clearHighlights();
            m_pendingHighlightChain.clear();
        });
        stepIndex = 0;
    }
}

/**
 * @brief 计算平台网格槽的左上角画布坐标
 *
 * 网格数学（行优先，4 列）：
 *   - 列 = index % 4
 *   - 行 = index / 4 （整数除法）
 *   - 节点尺寸        = 168 × 78
 *   - 水平间距        = 24
 *   - 垂直间距        = 32
 *   - 左上角外边距     = (40, 40)
 *
 *   x = marginX + col * (nodeW + gapX)
 *   y = marginY + row * (nodeH + gapY)
 *
 * @param index 网格中从零开始的位置。
 * @return 节点左上角的画布坐标。
 */
QPointF DataFlowWidget::platformGridPosition(int index) const
{
    constexpr double nodeW = 168.0;
    constexpr double nodeH = 78.0;
    constexpr double gapX = 24.0;
    constexpr double gapY = 32.0;
    constexpr double marginX = 40.0;
    constexpr double marginY = 40.0;
    constexpr int cols = 4;

    int col = index % cols;
    int row = index / cols;
    return QPointF(marginX + col * (nodeW + gapX),
                   marginY + row * (nodeH + gapY));
}

/**
 * @brief 添加平台节点到画布，跳过已绘制的 ID
 *
 * 去重检查：如果 platform.id 已存在于 m_platformNodeMap 中，
 * 立即返回 — 该平台已可视化，更新不会在 PlatformData 画布上
 * 动画显示。否则：
 *   1. 通过 platformGridPosition(m_platformNodeCounter) 获取下一个网格槽。
 *   2. 调用 NodeFlowWidget::addNode 实际创建画布节点。
 *   3. 将 platform.id → 返回的画布节点 ID 记录到 m_platformNodeMap，
 *      以便后续相同 ID 的调用命中提前返回。
 *   4. 递增 m_platformNodeCounter，使下一个新 ID 获得后续槽。
 *
 * @param platform 要可视化的 PlatformData 记录。
 */
void DataFlowWidget::addPlatformNode(const PlatformData &platform)
{
    // 去重：如果此平台 ID 已在画布上则跳过
    if (m_platformNodeMap.contains(platform.id)) {
        return;
    }

    // 分配下一个网格槽（行优先，4 列）并创建节点
    QPointF pos = platformGridPosition(m_platformNodeCounter++);
    int nodeId = m_flow->addNode(platform.id, platform.name,
                                 nodeColor(m_platformNodeCounter), pos);
    m_platformNodeMap[platform.id] = nodeId;
}
