/**
 * @file NodeFlowWidget.cpp
 * @brief 节点流图编辑控件实现 — 交互式节点-边图可视化
 *
 * @details 该文件包含 NodeFlowWidget 类的具体渲染、事件处理和图
 *          操作逻辑。场景/控件坐标变换、三次贝塞尔边路径以及基于
 *          QPainterPathStroker 的点击测试都在此实现。匿名命名空间中的
 *          常量驱动默认的缩放限制和节点尺寸。
 */

#include "NodeFlowWidget.h"

#include "common/logger.h"

#include <QContextMenuEvent>
#include <QFile>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLineF>
#include <QLineEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace
{
constexpr double MinZoom = 0.25;   /**< 最小缩放 */
constexpr double MaxZoom = 3.5;    /**< 最大缩放 */
constexpr double NodeWidth = 168.0;  /**< 节点宽度 */
constexpr double NodeHeight = 78.0;  /**< 节点高度 */
}

/**
 * @brief 构造控件并启用鼠标跟踪和强键盘焦点
 */
NodeFlowWidget::NodeFlowWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

/**
 * @brief 替换节点列表，重置选择/ID计数器，并调度一次初始适应视图
 */
void NodeFlowWidget::setNodes(const QVector<Node> &nodes)
{
    m_nodes = nodes;
    m_selectedNodeId = -1;
    m_selectedEdgeIndex = -1;
    m_nextNodeId = 1;
    for (const Node &node : m_nodes) {
        m_nextNodeId = std::max(m_nextNodeId, node.id + 1);
    }
    m_needInitialFit = true;
    emitGraphChanged();
    update();
}

/**
 * @brief 替换边列表并清除当前边选择
 */
void NodeFlowWidget::setEdges(const QVector<Edge> &edges)
{
    m_edges = edges;
    m_selectedEdgeIndex = -1;
    emitGraphChanged();
    update();
}

/** @brief 返回当前所有节点的副本 */
QVector<NodeFlowWidget::Node> NodeFlowWidget::nodes() const { return m_nodes; }
/** @brief 返回当前所有边的副本 */
QVector<NodeFlowWidget::Edge> NodeFlowWidget::edges() const { return m_edges; }

/**
 * @brief 分配具有下一个顺序 ID 的新节点并将其追加到图中
 */
int NodeFlowWidget::addNode(const QString &title, const QString &subtitle, const QColor &color, const QPointF &position)
{
    Node node;
    node.id = m_nextNodeId++;
    node.title = title;
    node.subtitle = subtitle;
    node.color = color.isValid() ? color : QColor("#2E7DD1");
    node.rect = QRectF(position, QSizeF(NodeWidth, NodeHeight));
    m_nodes.push_back(node);
    setSelectedNode(node.id);
    emitGraphChanged();
    update();
    return node.id;
}

/**
 * @brief 追加新的有向边，拒绝自环、重复边和悬空端点
 */
void NodeFlowWidget::addEdge(int from, int to, const QString &label)
{
    if (from == to || !findNode(from) || !findNode(to)) return;
    for (const Edge &edge : m_edges) {
        if (edge.from == from && edge.to == to) return;
    }
    m_edges.push_back({from, to, label});
    setSelectedEdge(static_cast<int>(m_edges.size()) - 1);
    emitGraphChanged();
    update();
}

/**
 * @brief 如果有选中的边则移除，否则委托给 removeSelectedNode()
 */
void NodeFlowWidget::removeSelectedItem()
{
    if (m_selectedEdgeIndex >= 0) { removeSelectedEdge(); return; }
    removeSelectedNode();
}

/**
 * @brief 删除选中节点及其所有关联边
 */
void NodeFlowWidget::removeSelectedNode()
{
    if (m_selectedNodeId < 0) return;
    const int id = m_selectedNodeId;
    m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(), [id](const Node &n) { return n.id == id; }), m_nodes.end());
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), [id](const Edge &e) { return e.from == id || e.to == id; }), m_edges.end());
    m_activeNodeIds.remove(id);
    m_selectedEdgeIndex = -1;
    setSelectedNode(-1);
    emitGraphChanged();
    update();
}

/**
 * @brief 按索引删除选中边，并相应调整活动边高亮集合
 */
void NodeFlowWidget::removeSelectedEdge()
{
    if (m_selectedEdgeIndex < 0 || m_selectedEdgeIndex >= m_edges.size()) return;
    m_edges.removeAt(m_selectedEdgeIndex);
    m_activeEdgeIndices.remove(m_selectedEdgeIndex);
    setSelectedEdge(-1);
    emitGraphChanged();
    update();
}

/**
 * @brief 清空所有容器和选择/高亮状态，然后发射取消选择信号
 */
void NodeFlowWidget::clearGraph()
{
    m_nodes.clear();
    m_edges.clear();
    m_selectedNodeId = -1;
    m_selectedEdgeIndex = -1;
    m_nextNodeId = 1;
    m_activeNodeIds.clear();
    m_activeEdgeIndices.clear();
    emitGraphChanged();
    emit nodeSelected(-1, QString());
    emit edgeSelected(-1, -1, QString());
    update();
}

/**
 * @brief 通过计算使边界框居中的缩放和平移，将整个图适配到控件中
 * @details 在边缘留出 96 像素余量。缩放限制在 [MinZoom, MaxZoom] 范围内。
 */
void NodeFlowWidget::fitToView()
{
    const QRectF bounds = graphBounds();
    if (bounds.isNull() || width() <= 0 || height() <= 0) return;
    const double xScale = (width() - 96.0) / std::max(1.0, bounds.width());
    const double yScale = (height() - 96.0) / std::max(1.0, bounds.height());
    m_zoom = std::max(MinZoom, std::min(MaxZoom, std::min(xScale, yScale)));
    const QPointF graphCenter = bounds.center();
    const QPointF widgetCenter(width() / 2.0, height() / 2.0);
    m_panOffset = widgetCenter - graphCenter * m_zoom;
    m_needInitialFit = false;
    emit viewChanged(m_zoom);
    update();
}

/**
 * @brief 将缩放乘以约 1.18 进行放大，然后限制在允许范围内
 */
void NodeFlowWidget::zoomIn()
{
    m_zoom *= 1.18;
    clampZoom();
    emit viewChanged(m_zoom);
    update();
}

/**
 * @brief 将缩放除以约 1.18 进行缩小，然后限制在允许范围内
 */
void NodeFlowWidget::zoomOut()
{
    m_zoom /= 1.18;
    clampZoom();
    emit viewChanged(m_zoom);
    update();
}

/**
 * @brief 将缩放恢复为 1.0，平移恢复为较小的默认偏移
 */
void NodeFlowWidget::resetView()
{
    m_zoom = 1.0;
    m_panOffset = QPointF(40.0, 40.0);
    emit viewChanged(m_zoom);
    update();
}

/**
 * @brief 切换网格可见性并重绘
 */
void NodeFlowWidget::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    update();
}

/**
 * @brief 将给定的节点 ID 插入到活动（高亮）集合中
 */
void NodeFlowWidget::highlightNode(int id)
{
    m_activeNodeIds.insert(id);
    update();
}

/**
 * @brief 将匹配 (from, to) 的边索引插入到活动集合中
 */
void NodeFlowWidget::highlightEdge(int from, int to)
{
    for (int i = 0; i < m_edges.size(); ++i) {
        if (m_edges[i].from == from && m_edges[i].to == to) {
            m_activeEdgeIndices.insert(i);
            break;
        }
    }
    update();
}

/**
 * @brief 高亮节点链以及给定顺序中连续节点对之间的边
 */
void NodeFlowWidget::highlightNodeChain(const QVector<int> &nodeIds)
{
    m_activeNodeIds.clear();
    for (int id : nodeIds) m_activeNodeIds.insert(id);
    m_activeEdgeIndices.clear();
    for (int i = 0; i < m_edges.size(); ++i) {
        const Edge &e = m_edges[i];
        if (m_activeNodeIds.contains(e.from) && m_activeNodeIds.contains(e.to)) {
            int fromIdx = nodeIds.indexOf(e.from);
            int toIdx = nodeIds.indexOf(e.to);
            if (fromIdx >= 0 && toIdx >= 0 && std::abs(toIdx - fromIdx) == 1) {
                m_activeEdgeIndices.insert(i);
            }
        }
    }
    update();
}

/**
 * @brief 清除所有活动节点和活动边的高亮
 */
void NodeFlowWidget::clearHighlights()
{
    m_activeNodeIds.clear();
    m_activeEdgeIndices.clear();
    update();
}

/**
 * @brief 将当前图导出为 JSON 文件
 * @param fileName 输出文件路径
 * @return 写入成功返回 true
 */
bool NodeFlowWidget::exportJson(const QString &fileName) const
{
    QJsonArray nodes;
    for (const Node &node : m_nodes) {
        QJsonObject item;
        item.insert(QStringLiteral("id"), node.id);
        item.insert(QStringLiteral("title"), node.title);
        item.insert(QStringLiteral("subtitle"), node.subtitle);
        item.insert(QStringLiteral("color"), node.color.name(QColor::HexRgb).toUpper());
        item.insert(QStringLiteral("x"), node.rect.x());
        item.insert(QStringLiteral("y"), node.rect.y());
        item.insert(QStringLiteral("width"), node.rect.width());
        item.insert(QStringLiteral("height"), node.rect.height());
        nodes.append(item);
    }

    QJsonArray edges;
    for (const Edge &edge : m_edges) {
        QJsonObject item;
        item.insert(QStringLiteral("from"), edge.from);
        item.insert(QStringLiteral("to"), edge.to);
        item.insert(QStringLiteral("label"), edge.label);
        edges.append(item);
    }

    QJsonObject root;
    root.insert(QStringLiteral("nodes"), nodes);
    root.insert(QStringLiteral("edges"), edges);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

/** @brief 返回当前选中的节点 ID（-1 表示无选中） */
int NodeFlowWidget::selectedNodeId() const { return m_selectedNodeId; }
/** @brief 返回图中节点数量 */
int NodeFlowWidget::nodeCount() const { return m_nodes.size(); }
/** @brief 返回图中边数量 */
int NodeFlowWidget::edgeCount() const { return m_edges.size(); }

/**
 * @brief 绘制整个场景：背景、网格、边、连接预览和节点
 * @details 如果图从未被自动适配且存在节点，则延迟调用 fitToView()。
 */
void NodeFlowWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#F5F7FA"));
    if (m_gridVisible) drawGrid(&painter);
    if (m_needInitialFit && !m_nodes.isEmpty()) fitToView();

    for (int i = 0; i < m_edges.size(); ++i) {
        drawEdge(&painter, m_edges.at(i), m_activeEdgeIndices.contains(i), i == m_selectedEdgeIndex);
    }
    drawConnectionDraft(&painter);
    for (int i = 0; i < m_nodes.size(); ++i) {
        const Node &node = m_nodes.at(i);
        drawNode(&painter, node, node.id == m_selectedNodeId, m_activeNodeIds.contains(node.id));
    }
}

/**
 * @brief 当控件首次调整大小且图非空时触发一次初始适应视图
 */
void NodeFlowWidget::resizeEvent(QResizeEvent *)
{
    if (m_needInitialFit && !m_nodes.isEmpty()) fitToView();
}

/**
 * @brief 显示上下文菜单，内容取决于光标是在节点上、边上还是空白处
 */
void NodeFlowWidget::contextMenuEvent(QContextMenuEvent *event)
{
    const QPointF scenePoint = widgetToScene(event->pos());
    const int nodeId = hitNode(scenePoint);
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #FFFFFF; border: 1px solid #D0D7DE; padding: 4px 0px; }"
        "QMenu::item { padding: 6px 24px; color: #1F2933; }"
        "QMenu::item:selected { background-color: #2E7DD1; color: #FFFFFF; }"
        "QMenu::separator { height: 1px; background: #E4E9EF; margin: 4px 8px; }"
    );
    if (nodeId >= 0) {
        setSelectedNode(nodeId);
        QAction *renameAction = menu.addAction(QStringLiteral("重命名节点"));
        QAction *connectAction = menu.addAction(QStringLiteral("从此节点开始连线"));
        QAction *deleteAction = menu.addAction(QStringLiteral("删除节点"));
        const QAction *chosen = menu.exec(event->globalPos());
        if (chosen == renameAction) {
            editNodeTitle(nodeId);
        } else if (chosen == connectAction) {
            m_connecting = true;
            m_connectionFromId = nodeId;
            m_connectionEnd = scenePoint;
            update();
        } else if (chosen == deleteAction) {
            removeSelectedNode();
        }
        return;
    }
    const int edgeIndex = hitEdge(scenePoint);
    if (edgeIndex >= 0) {
        setSelectedEdge(edgeIndex);
        QAction *editAction = menu.addAction(QStringLiteral("编辑连线标签"));
        QAction *deleteAction = menu.addAction(QStringLiteral("删除连线"));
        const QAction *chosen = menu.exec(event->globalPos());
        if (chosen == editAction) editEdgeLabel(edgeIndex);
        else if (chosen == deleteAction) removeSelectedEdge();
        return;
    }
    QAction *addAction = menu.addAction(QStringLiteral("在此处添加节点"));
    QAction *fitAction = menu.addAction(QStringLiteral("适应窗口"));
    const QAction *chosen = menu.exec(event->globalPos());
    if (chosen == addAction) {
        addNode(QStringLiteral("节点 %1").arg(m_nextNodeId), QStringLiteral("右键添加"),
                QColor("#2E7DD1"), scenePoint - QPointF(NodeWidth / 2.0, NodeHeight / 2.0));
    } else if (chosen == fitAction) {
        fitToView();
    }
}

/**
 * @brief 双击时重命名光标下方的节点或编辑边标签
 */
void NodeFlowWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QPointF scenePoint = widgetToScene(event->pos());
    const int nodeId = hitNode(scenePoint);
    if (nodeId >= 0) { setSelectedNode(nodeId); editNodeTitle(nodeId); return; }
    const int edgeIndex = hitEdge(scenePoint);
    if (edgeIndex >= 0) { setSelectedEdge(edgeIndex); editEdgeLabel(edgeIndex); return; }
    QWidget::mouseDoubleClickEvent(event);
}

/**
 * @brief 开始平移（Alt 或中键）、节点拖拽、连线完成或选择
 * @details 当 m_connecting 为 true 时（右键菜单触发的单次连线模式）：
 *          左键点目标节点 → 完成连线并重置状态；
 *          左键点空白处或起点自身 → 取消连线。
 */
void NodeFlowWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    m_lastWidgetPos = event->pos();
    m_lastScenePos = widgetToScene(event->pos());
    if (event->button() == Qt::MiddleButton || event->modifiers().testFlag(Qt::AltModifier)) {
        m_panning = true;
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        // 单次连线模式中：左键决定连线终点或取消
        if (m_connecting) {
            const int id = hitNode(m_lastScenePos);
            if (id >= 0 && id != m_connectionFromId) {
                addEdge(m_connectionFromId, id, QStringLiteral("next"));
                setSelectedNode(id);
            }
            m_connecting = false;
            m_connectionFromId = -1;
            update();
            return;
        }
        const int id = hitNode(m_lastScenePos);
        if (id >= 0) {
            m_draggingNode = true;
            setSelectedNode(id);
        } else {
            const int edgeIndex = hitEdge(m_lastScenePos);
            if (edgeIndex >= 0) { setSelectedEdge(edgeIndex); }
            else { setSelectedNode(-1); setSelectedEdge(-1); }
        }
    }
}

/**
 * @brief 平移时按控件空间增量平移 m_panOffset；节点拖拽时按场景空间增量平移节点矩形；
 *        连线模式下实时更新预览终点坐标
 */
void NodeFlowWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF widgetPos = event->pos();
    const QPointF scenePos = widgetToScene(widgetPos);
    if (m_panning) {
        m_panOffset += widgetPos - m_lastWidgetPos;
        emit viewChanged(m_zoom);
        update();
    } else if (m_draggingNode && m_selectedNodeId >= 0) {
        Node *node = findNode(m_selectedNodeId);
        if (node) {
            const QPointF delta = scenePos - m_lastScenePos;
            node->rect = node->rect.translated(delta);
            update();
        }
    } else if (m_connecting) {
        m_connectionEnd = scenePos;
        update();
    }
    m_lastWidgetPos = widgetPos;
    m_lastScenePos = scenePos;
    QWidget::mouseMoveEvent(event);
}

/**
 * @brief 结束平移/拖拽状态并恢复箭头光标
 */
void NodeFlowWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_panning = false;
    m_draggingNode = false;
    setCursor(Qt::ArrowCursor);
    QWidget::mouseReleaseEvent(event);
}

/**
 * @brief 以光标为中心缩放：计算缩放前的场景点，应用缩放，然后调整平移使同一场景点仍处于光标下方
 */
void NodeFlowWidget::wheelEvent(QWheelEvent *event)
{
    const QPointF pos = event->pos();
    const QPointF sceneBefore = widgetToScene(pos);
    if (event->delta() > 0) m_zoom *= 1.15; else m_zoom /= 1.15;
    clampZoom();
    const QPointF sceneAfter = widgetToScene(pos);
    m_panOffset += (sceneBefore - sceneAfter) * m_zoom;
    emit viewChanged(m_zoom);
    update();
    event->accept();
}

/**
 * @brief 处理 Delete 和 Backspace 键删除选中项，Esc 键取消连线模式
 */
void NodeFlowWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        removeSelectedItem();
    } else if (event->key() == Qt::Key_Escape) {
        if (m_connecting) {
            m_connecting = false;
            m_connectionFromId = -1;
            update();
        }
    }
}

/** @brief 返回节点在场景空间中的矩形 */
QRectF NodeFlowWidget::nodeRect(const Node &node) const { return node.rect; }

/**
 * @brief 场景到控件的变换：先按缩放缩放，再加上平移偏移
 */
QPointF NodeFlowWidget::sceneToWidget(const QPointF &point) const
{ return point * m_zoom + m_panOffset; }

/**
 * @brief 控件到场景的变换：先减去平移偏移，再除以缩放
 */
QPointF NodeFlowWidget::widgetToScene(const QPointF &point) const
{ return (point - m_panOffset) / m_zoom; }

/**
 * @brief 将矩形的两个角从场景空间变换到控件空间
 */
QRectF NodeFlowWidget::sceneToWidget(const QRectF &rect) const
{
    return QRectF(sceneToWidget(rect.topLeft()), sceneToWidget(rect.bottomRight()));
}

/**
 * @brief 在场景空间中对每个节点矩形进行简单的点在矩形内测试
 */
int NodeFlowWidget::hitNode(const QPointF &scenePoint) const
{
    for (const Node &node : m_nodes) {
        if (node.rect.contains(scenePoint)) return node.id;
    }
    return -1;
}

/**
 * @brief 边点击测试：构建每条边的场景空间路径，用 QPainterPathStroker 创建 12 单位宽的描边，并测试包含性
 * @details 这提供了宽松的"接近即可"容差，使边尽管视觉宽度较窄也易于点击。
 */
int NodeFlowWidget::hitEdge(const QPointF &scenePoint) const
{
    for (int i = 0; i < m_edges.size(); ++i) {
        const QPainterPath path = edgePath(m_edges.at(i));
        QPainterPathStroker stroker;
        stroker.setWidth(12.0);
        if (stroker.createStroke(path).contains(scenePoint)) return i;
    }
    return -1;
}

/**
 * @brief 按 ID 查找节点（非 const 版本，返回可修改指针）
 * @param id 要查找的节点 ID
 * @return 找到的节点指针，未找到返回 nullptr
 */
NodeFlowWidget::Node *NodeFlowWidget::findNode(int id)
{
    for (Node &node : m_nodes) if (node.id == id) return &node;
    return nullptr;
}

/**
 * @brief 按 ID 查找节点（const 版本，返回只读指针）
 * @param id 要查找的节点 ID
 * @return 找到的节点只读指针，未找到返回 nullptr
 */
const NodeFlowWidget::Node *NodeFlowWidget::findNode(int id) const
{
    for (const Node &node : m_nodes) if (node.id == id) return &node;
    return nullptr;
}

/**
 * @brief 计算所有节点矩形的边界矩形，每侧扩展 40 单位余量
 */
QRectF NodeFlowWidget::graphBounds() const
{
    if (m_nodes.isEmpty()) return QRectF();
    QRectF bounds = m_nodes.first().rect;
    for (const Node &node : m_nodes) bounds = bounds.united(node.rect);
    return bounds.adjusted(-40, -40, 40, 40);
}

/**
 * @brief 返回节点右边缘中点作为输出锚点，或左边缘中点作为输入锚点
 */
QPointF NodeFlowWidget::nodeAnchor(const Node &node, bool output) const
{
    const QRectF r = node.rect;
    if (output) return QPointF(r.right(), r.center().y());
    return QPointF(r.left(), r.center().y());
}

/**
 * @brief 构建边的三次贝塞尔路径
 * @details 控制点从起始/结束锚点水平偏移，偏移量取 80 像素和锚点间水平距离 45% 中较大者。
 */
QPainterPath NodeFlowWidget::edgePath(const Edge &edge) const
{
    const Node *from = findNode(edge.from);
    const Node *to = findNode(edge.to);
    QPainterPath path;
    if (!from || !to) return path;
    const QPointF start = nodeAnchor(*from, true);
    const QPointF end = nodeAnchor(*to, false);
    path.moveTo(start);
    const double handle = std::max(80.0, std::abs(end.x() - start.x()) * 0.45);
    path.cubicTo(QPointF(start.x() + handle, start.y()),
                 QPointF(end.x() - handle, end.y()), end);
    return path;
}

/**
 * @brief 设置节点选择，更新重绘，并用新标题发射 nodeSelected
 */
void NodeFlowWidget::setSelectedNode(int id)
{
    if (m_selectedNodeId != id) { m_selectedNodeId = id; update(); }
    if (id >= 0) {
        const Node *node = findNode(id);
        if (node) emit nodeSelected(id, node->title);
    } else {
        emit nodeSelected(-1, QString());
    }
}

/**
 * @brief 设置边选择，更新重绘，并用 from/to/label 发射 edgeSelected
 */
void NodeFlowWidget::setSelectedEdge(int index)
{
    if (m_selectedEdgeIndex != index) { m_selectedEdgeIndex = index; update(); }
    if (index >= 0 && index < m_edges.size()) {
        const Edge &e = m_edges.at(index);
        emit edgeSelected(e.from, e.to, e.label);
    }
}

/**
 * @brief 通过 QInputDialog 提示用户输入新的节点标题并在接受时应用
 */
void NodeFlowWidget::editNodeTitle(int id)
{
    Node *node = findNode(id);
    if (!node) return;
    bool ok = false;
    const QString text = QInputDialog::getText(this, QStringLiteral("Rename Node"),
        QStringLiteral("Node Name"), QLineEdit::Normal, node->title, &ok);
    if (ok && !text.isEmpty()) { node->title = text; update(); }
}

/**
 * @brief 通过 QInputDialog 提示用户输入新的边标签并在接受时应用
 */
void NodeFlowWidget::editEdgeLabel(int index)
{
    if (index < 0 || index >= m_edges.size()) return;
    Edge &edge = m_edges[index];
    bool ok = false;
    const QString text = QInputDialog::getText(this, QStringLiteral("Edit Edge Label"),
        QStringLiteral("Label"), QLineEdit::Normal, edge.label, &ok);
    if (ok) { edge.label = text; update(); }
}

/** @brief 发射 graphChanged 信号，携带当前节点数和边数 */
void NodeFlowWidget::emitGraphChanged() { emit graphChanged(m_nodes.size(), m_edges.size()); }

/**
 * @brief 将 m_zoom 限制在 [MinZoom, MaxZoom] 闭区间内
 */
void NodeFlowWidget::clampZoom()
{
    m_zoom = std::max(MinZoom, std::min(MaxZoom, m_zoom));
}

/**
 * @brief 绘制点状网格，间距为 40 场景单位，按当前缩放因子缩放
 */
void NodeFlowWidget::drawGrid(QPainter *painter) const
{
    painter->save();
    painter->setPen(QPen(QColor("#E4E9EF"), 1));
    const double gridSize = 40.0 * m_zoom;
    for (double x = std::fmod(m_panOffset.x(), gridSize); x < width(); x += gridSize)
        painter->drawLine(QPointF(x, 0), QPointF(x, height()));
    for (double y = std::fmod(m_panOffset.y(), gridSize); y < height(); y += gridSize)
        painter->drawLine(QPointF(0, y), QPointF(width(), y));
    painter->restore();
}

/**
 * @brief 渲染单条边曲线、箭头和其中点可选标签
 * @details 场景空间路径被逐元素重建并重新变换，以便控制点在当前缩放/平移下正确放置在控件空间中。
 */
void NodeFlowWidget::drawEdge(QPainter *painter, const Edge &edge, bool active, bool selected) const
{
    const Node *from = findNode(edge.from);
    const Node *to = findNode(edge.to);
    if (!from || !to) return;

    QPainterPath path;
    const QPainterPath scenePath = edgePath(edge);
    for (int i = 0; i < scenePath.elementCount(); ++i) {
        const QPainterPath::Element element = scenePath.elementAt(i);
        if (element.isMoveTo()) {
            path.moveTo(sceneToWidget(QPointF(element.x, element.y)));
        } else if (element.isCurveTo()) {
            const QPainterPath::Element c1 = scenePath.elementAt(i);
            const QPainterPath::Element c2 = scenePath.elementAt(i + 1);
            const QPainterPath::Element endElement = scenePath.elementAt(i + 2);
            path.cubicTo(sceneToWidget(QPointF(c1.x, c1.y)),
                         sceneToWidget(QPointF(c2.x, c2.y)),
                         sceneToWidget(QPointF(endElement.x, endElement.y)));
            i += 2;
        }
    }
    const QPointF end = sceneToWidget(nodeAnchor(*to, false));

    painter->save();
    if (selected) {
        painter->setPen(QPen(QColor("#111827"), 6));
        painter->drawPath(path);
    }
    painter->setPen(QPen(active ? QColor("#E9573F") : QColor("#8EA0AD"), active || selected ? 4 : 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    const QLineF tail(QPointF(end.x() - 16, end.y() - 7), end);
    const QLineF head(QPointF(end.x() - 16, end.y() + 7), end);
    painter->drawLine(tail);
    painter->drawLine(head);

    if (!edge.label.isEmpty()) {
        const QPointF center = path.pointAtPercent(0.5);
        const QRectF labelRect(center.x() - 36, center.y() - 12, 72, 24);
        painter->setPen(Qt::NoPen);
        painter->setBrush(active ? QColor("#FFE8E3") : QColor("#FFFFFF"));
        painter->drawRoundedRect(labelRect, 4, 4);
        painter->setPen(active ? QColor("#C7442F") : QColor("#66727C"));
        painter->drawText(labelRect, Qt::AlignCenter, edge.label);
    }
    painter->restore();
}

/**
 * @brief 将节点绘制为圆角矩形，带有彩色左条纹、状态点、标题、副标题和锚点圆点
 */
void NodeFlowWidget::drawNode(QPainter *painter, const Node &node, bool selected, bool active) const
{
    const QRectF rect = sceneToWidget(nodeRect(node));
    painter->save();

    QColor border = selected ? QColor("#111827") : node.color.darker(115);
    if (active) border = QColor("#E9573F");

    painter->setPen(QPen(border, selected || active ? 3 : 2));
    painter->setBrush(QColor("#FFFFFF"));
    painter->drawRoundedRect(rect, 8, 8);

    if (active) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(233, 87, 63, 40));
        painter->drawRoundedRect(rect.adjusted(-3, -3, 3, 3), 10, 10);
    }

    const QRectF stripe(rect.left(), rect.top(), 9, rect.height());
    painter->setPen(Qt::NoPen);
    painter->setBrush(node.color);
    painter->drawRoundedRect(stripe, 4, 4);

    const QRectF dot(rect.right() - 26, rect.top() + 14, 12, 12);
    painter->setBrush(active ? QColor("#E9573F") : node.color.lighter(120));
    painter->drawEllipse(dot);

    painter->setPen(QColor("#1F2933"));
    QFont titleFont = painter->font();
    titleFont.setPointSizeF(std::max(8.0, 11.0 * m_zoom));
    titleFont.setBold(true);
    painter->setFont(titleFont);
    painter->drawText(rect.adjusted(20, 13, -34, -40), Qt::AlignLeft | Qt::AlignVCenter, node.title);

    painter->setPen(QColor("#66727C"));
    QFont subtitleFont = painter->font();
    subtitleFont.setPointSizeF(std::max(7.0, 9.0 * m_zoom));
    subtitleFont.setBold(false);
    painter->setFont(subtitleFont);
    painter->drawText(rect.adjusted(20, 38, -12, -10), Qt::AlignLeft | Qt::AlignVCenter, node.subtitle);

    painter->setPen(QPen(QColor("#CDD7DF"), 1));
    painter->setBrush(QColor("#FFFFFF"));
    painter->drawEllipse(sceneToWidget(nodeAnchor(node, false)), 4, 4);
    painter->drawEllipse(sceneToWidget(nodeAnchor(node, true)), 4, 4);
    painter->restore();
}

/**
 * @brief 在连接拖拽期间，从源节点的输出锚点到当前光标位置绘制蓝色虚线预览曲线
 */
void NodeFlowWidget::drawConnectionDraft(QPainter *painter) const
{
    if (m_connecting) {
        const Node *from = findNode(m_connectionFromId);
        if (from) {
            const QPointF start = sceneToWidget(nodeAnchor(*from, true));
            const QPointF end = sceneToWidget(m_connectionEnd);
            QPainterPath path(start);
            const double handle = std::max(80.0, std::abs(end.x() - start.x()) * 0.45);
            path.cubicTo(QPointF(start.x() + handle, start.y()),
                         QPointF(end.x() - handle, end.y()), end);
            painter->save();
            painter->setPen(QPen(QColor("#2E7DD1"), 2, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
            painter->restore();
        }
    }
}
