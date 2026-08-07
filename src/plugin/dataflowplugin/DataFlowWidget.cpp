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
 * 构建分割面板 UI，加载默认流模式（Mode_SourcePipeline），
 * 并连接驱动实时更新的三个 DataManager 信号。
 */
DataFlowWidget::DataFlowWidget(DataManager *dataManager, QWidget *parent)
    : QWidget(parent)
    , m_dataManager(dataManager)
    , m_highlightTimer(new QTimer(this))
{
    setStyleSheet("background-color: #f5f5f5;");

    createUI();
    loadFlowMode(Mode_SourcePipeline);

    if (m_dataManager) {
        // 信号1：当新采集的数据被推送到管理器时发射
        connect(m_dataManager, &DataManager::dataPushed, this, &DataFlowWidget::onDataPushed);
        // 信号2：当管理器修改内存中的平台/事件记录时发射
        connect(m_dataManager, &DataManager::dynamicDataChanged, this, &DataFlowWidget::onDataChanged);
        // 信号3：当单条平台记录变化时发射（用于 PlatformData 模式）
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

    auto *panelTitle = new QLabel(QStringLiteral("Data Flow Monitor"), panel);
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
        QStringLiteral("Source Pipeline"),
        QStringLiteral("Platform Data Flow"),
        QStringLiteral("Event Alert Flow")
    });

    auto *gridCheck = new QCheckBox(QStringLiteral("Show Grid"), panel);
    gridCheck->setChecked(true);

    auto *fitButton = new QPushButton(QStringLiteral("Fit to View"), panel);
    auto *zoomInButton = new QPushButton(QStringLiteral("Zoom In"), panel);
    auto *zoomOutButton = new QPushButton(QStringLiteral("Zoom Out"), panel);

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
    layout->addWidget(new QLabel(QStringLiteral("Flow Mode"), panel));
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
    m_statusLabel->setText(QStringLiteral("Waiting for data..."));
}

/**
 * @brief 加载流模式，分发到模式特定的 setup*() 辅助函数然后适应视图
 * @param mode 用户选择或启动时的 FlowMode。
 */
void DataFlowWidget::loadFlowMode(FlowMode mode)
{
    switch (mode) {
    case Mode_SourcePipeline: setupSourcePipeline(); break;
    case Mode_PlatformData: setupPlatformDataFlow(); break;
    case Mode_EventAlert: setupEventAlertFlow(); break;
    }
    m_flow->fitToView();
}

/**
 * @brief 用静态源流水线图填充画布
 *
 * 六个节点水平排列在 y=100，单个分支节点（数据缓存 → 过期清理）
 * 放置在主流程下方。
 */
void DataFlowWidget::setupSourcePipeline()
{
    QVector<NodeFlowWidget::Node> nodes = {
        node(1, QStringLiteral("Data Source"), QStringLiteral("HTTP/Raw"), nodeColor(0), 40, 100),
        node(2, QStringLiteral("Protocol Adapter"), QStringLiteral("Parse/Validate"), nodeColor(1), 270, 100),
        node(3, QStringLiteral("Data Cache"), QStringLiteral("Store/Dedupe"), nodeColor(2), 500, 100),
        node(4, QStringLiteral("Data Push"), QStringLiteral("Distribute/Callback"), nodeColor(3), 730, 100),
        node(5, QStringLiteral("Situation Display"), QStringLiteral("Render/Interact"), nodeColor(4), 960, 100),
        node(6, QStringLiteral("Expire Cleanup"), QStringLiteral("Timer/TTL"), nodeColor(5), 500, 270),
    };
    QVector<NodeFlowWidget::Edge> edges = {
        {1, 2, QStringLiteral("raw")},
        {2, 3, QStringLiteral("parsed")},
        {3, 4, QStringLiteral("cached")},
        {4, 5, QStringLiteral("display")},
        {3, 6, QStringLiteral("expire")},
    };
    m_flow->setNodes(nodes);
    m_flow->setEdges(edges);
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
 * @brief 用静态事件告警图填充画布
 *
 * 七个节点：直线路径 武器发射 → 事件创建 → 导弹飞行 →
 * 告警事件，加上三个侧分支分别指向属性框、事件历史和态势图标。
 */
void DataFlowWidget::setupEventAlertFlow()
{
    QVector<NodeFlowWidget::Node> nodes = {
        node(1, QStringLiteral("Weapon Launch"), QStringLiteral("Attack Event"), nodeColor(0), 40, 120),
        node(2, QStringLiteral("Event Creation"), QStringLiteral("Missile Platform"), nodeColor(1), 270, 120),
        node(3, QStringLiteral("Missile Flight"), QStringLiteral("Pos Update"), nodeColor(2), 500, 120),
        node(4, QStringLiteral("Alert Event"), QStringLiteral("Alert Generated"), nodeColor(3), 730, 120),
        node(5, QStringLiteral("Property Box"), QStringLiteral("Details"), nodeColor(4), 960, 120),
        node(6, QStringLiteral("Event History"), QStringLiteral("Record List"), nodeColor(5), 500, 290),
        node(7, QStringLiteral("Situation Icon"), QStringLiteral("A/!/D Marker"), nodeColor(0), 730, 290),
    };
    QVector<NodeFlowWidget::Edge> edges = {
        {1, 2, QStringLiteral("launch")},
        {2, 3, QStringLiteral("fly")},
        {3, 4, QStringLiteral("alert")},
        {4, 5, QStringLiteral("show")},
        {4, 6, QStringLiteral("record")},
        {4, 7, QStringLiteral("marker")},
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
    m_graphLabel->setText(QStringLiteral("Nodes: %1, Edges: %2").arg(nodeCount).arg(edgeCount));
}

/**
 * @brief 更新 "Selected: #id title" 行（或 id < 0 时显示 "None"）
 * @param id    节点标识符；负数表示无选择。
 * @param title 选中节点的标题。
 */
void DataFlowWidget::updateSelectedInfo(int id, const QString &title)
{
    if (id < 0) {
        m_selectedLabel->setText(QStringLiteral("Selected: None"));
        return;
    }
    m_selectedLabel->setText(QStringLiteral("Selected: #%1 %2").arg(id).arg(title));
}

/**
 * @brief 将缩放因子格式化为整数百分比
 * @param zoom 当前缩放因子（1.0 → "100%"）。
 */
void DataFlowWidget::updateZoomInfo(double zoom)
{
    m_zoomLabel->setText(QStringLiteral("Zoom: %1%").arg(QString::number(zoom * 100.0, 'f', 0)));
}

/**
 * @brief DataManager::dataPushed 的槽
 *
 * 构建特定于模式的高亮链，使画布动画显示传播路径，并向
 * 状态标签写入摘要行。
 *   - Mode_SourcePipeline: 高亮完整管道 [1..5]。
 *   - Mode_PlatformData:   高亮默认平台数据路径。
 *   - Mode_EventAlert:     根据是否包含事件进行分支。
 *
 * @param data 新推送的动态对象。
 */
void DataFlowWidget::onDataPushed(const DynamicObjects &data)
{
    Q_UNUSED(data);
    FlowMode mode = static_cast<FlowMode>(m_modeCombo->currentIndex());
    switch (mode) {
    case Mode_SourcePipeline:
        m_pendingHighlightChain = {1, 2, 3, 4, 5};
        m_statusLabel->setText(QStringLiteral("Data pushed for %1 platforms").arg(data.platforms.size()));
        break;
    case Mode_PlatformData:
        m_pendingHighlightChain = {1, 2, 3, 4, 5, 7};
        m_statusLabel->setText(QStringLiteral("Platform updated: %1 entries").arg(data.platforms.size()));
        break;
    case Mode_EventAlert:
        if (!data.events.isEmpty()) {
            m_pendingHighlightChain = {4, 5, 6, 7};
            m_statusLabel->setText(QStringLiteral("New events: %1").arg(data.events.size()));
        } else {
            m_pendingHighlightChain = {3, 7};
            m_statusLabel->setText(QStringLiteral("Situation updated"));
        }
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
        m_statusLabel->setText(QStringLiteral("Platforms: %1").arg(m_platformNodeMap.size()));
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
        m_statusLabel->setText(QStringLiteral("Platform %1 updated").arg(platform.id));
    }
}

/**
 * @brief DataManager::specialEventProduced 的槽
 *
 * 构建特定于事件类型的高亮链，并用事件名称更新状态标签。
 * 分支：
 *   - Event_Attack → [1,2,3,4,7]  （创建路径 + 态势标记）
 *   - Event_Alert  → [4,5,6,7]    （告警路径 + 详情/历史/标记）
 *   - default      → [4,6]        （仅告警节点 + 历史）
 *
 * @param event 新产生的特殊事件。
 */
void DataFlowWidget::onEventUpdated(const SpecialEvent &event)
{
    FlowMode mode = static_cast<FlowMode>(m_modeCombo->currentIndex());
    if (mode == Mode_EventAlert) {
        switch (event.eventType) {
        case Event_Attack:
            m_pendingHighlightChain = {1, 2, 3, 4, 7};
            m_statusLabel->setText(QStringLiteral("Attack event: %1").arg(event.eventName));
            break;
        case Event_Alert:
            m_pendingHighlightChain = {4, 5, 6, 7};
            m_statusLabel->setText(QStringLiteral("Alert event: %1").arg(event.eventName));
            break;
        default:
            m_pendingHighlightChain = {4, 6};
            m_statusLabel->setText(QStringLiteral("Event: %1").arg(event.eventName));
            break;
        }
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
