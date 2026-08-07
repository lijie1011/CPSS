/**
 * @file NodeFlowWidget.h
 * @brief 节点流图编辑控件
 *
 * @details 该控件提供了一个轻量级、自包含的画布，用于渲染和操作
 *          由矩形节点和贝塞尔曲线边组成的有向图。支持平移
 *          （Alt+拖拽或中键）、缩放（滚轮、缩放按钮或适应视图）、
 *          节点拖拽、上下文菜单编辑（重命名/删除节点、编辑/删除
 *          边、添加节点）、选择跟踪，以及对节点、边和节点链的
 *          任意高亮。它是 DataFlowPlugin 用来展示管道式数据/
 *          控制流的主要视图组件。
 */

#pragma once

#include <QColor>
#include <QEvent>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QContextMenuEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QResizeEvent;
class QWheelEvent;

/**
 * @class NodeFlowWidget
 * @brief 节点流图编辑控件
 *
 * @details 该控件维护一个内部场景坐标系，通过缩放因子和平移偏移
 *          变换为控件坐标。节点为固定大小矩形（NodeWidth × NodeHeight），
 *          放置在场景空间中；边为三次贝塞尔曲线，锚定在端点节点的
 *          左/右中点。所有的点击测试、高亮和选择状态都由该类的
 *          成员变量驱动。
 */
class NodeFlowWidget final : public QWidget
{
    Q_OBJECT

public:
    struct Node
    {
        int id = 0;            /**< 唯一整数标识符，由控件顺序分配 */
        QString title;         /**< 以粗体显示在节点标题行的主文本 */
        QString subtitle;      /**< 以较小字体显示在标题下方的次文本 */
        QColor color;          /**< 用于左侧条纹和状态点的强调色，默认"#2E7DD1" */
        QRectF rect;           /**< 节点在场景空间中的边界矩形（位置 + 固定大小） */
    };

    struct Edge
    {
        int from = 0;          /**< 源节点标识符（箭头尾部） */
        int to = 0;            /**< 目标节点标识符（箭头头部） */
        QString label;         /**< 可选的短标签，渲染在边的中点处 */
    };

    /**
     * @brief 构造函数
     * @param parent 父 QWidget，传递给基类 QWidget 构造函数。
     */
    explicit NodeFlowWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置节点
     * @param nodes 新的节点列表。控件不接管任何数据所有权；
     *              列表在内部被复制。选择状态和下一ID计数器会被重置，
     *              并调度一次自动适应视图。
     */
    void setNodes(const QVector<Node> &nodes);

    /**
     * @brief 设置边
     * @param edges 新的边列表。已有的边选择状态会被清除。
     */
    void setEdges(const QVector<Edge> &edges);

    /** @return 当前节点列表的副本。 */
    QVector<Node> nodes() const;

    /** @return 当前边列表的副本。 */
    QVector<Edge> edges() const;

    /**
     * @brief 添加节点
     * @param title    节点的主标签。
     * @param subtitle 节点的次标签。
     * @param color    强调色；无效 QColor 回退为 "#2E7DD1"。
     * @param position 节点矩形在场景空间中的左上角位置。
     * @return 分配给新节点的唯一整数 ID。
     */
    int addNode(const QString &title, const QString &subtitle, const QColor &color, const QPointF &position);

    /**
     * @brief 添加边
     * @param from  源节点 ID。
     * @param to    目标节点 ID。
     * @param label 可选的边标签，默认为空。
     * @note 重复边（相同的 from/to 对）和自环会被静默忽略。
     */
    void addEdge(int from, int to, const QString &label = QString());

    /**
     * @brief 移除当前选中项，优先边然后节点
     */
    void removeSelectedItem();

    /**
     * @brief 移除当前选中节点及其所有关联边
     */
    void removeSelectedNode();

    /**
     * @brief 仅移除当前选中边
     */
    void removeSelectedEdge();

    /**
     * @brief 清空画布，重置所有节点、边、选择和高亮状态
     */
    void clearGraph();

    /**
     * @brief 适应视图，计算缩放和平移使所有节点可见并留有余量
     */
    void fitToView();

    /**
     * @brief 放大，缩放级别增加约1.18倍
     */
    void zoomIn();

    /**
     * @brief 缩小，缩放级别减少约1.18倍
     */
    void zoomOut();

    /**
     * @brief 重置视图，将缩放恢复为1.0，平移恢复为默认偏移
     */
    void resetView();

    /**
     * @brief 切换网格显示
     * @param visible @c true 绘制网格，@c false 省略网格。
     */
    void setGridVisible(bool visible);

    /**
     * @brief 高亮节点
     * @param id 要高亮的节点标识符。
     */
    void highlightNode(int id);

    /**
     * @brief 高亮链路
     * @param from 边的源节点 ID。
     * @param to   边的目标节点 ID。
     */
    void highlightEdge(int from, int to);

    /**
     * @brief 高亮节点链，按给定顺序高亮连续节点和它们之间的边
     * @param nodeIds 形成链的有序节点 ID 列表。仅当 from/to 位置
     *                在该列表中是连续的边才会被高亮。
     */
    void highlightNodeChain(const QVector<int> &nodeIds);

    /**
     * @brief 清除高亮，移除所有活动节点和边的高亮状态
     */
    void clearHighlights();

    /** @return 当前选中节点的 ID，若无可选中项则返回 -1。 */
    int selectedNodeId() const;

    /** @return 当前图中的节点数量。 */
    int nodeCount() const;

    /** @return 当前图中的边数量。 */
    int edgeCount() const;

signals:
    /**
     * @brief 图变化信号，在节点或边集合改变后发射
     * @param nodeCount 当前节点数量。
     * @param edgeCount 当前边数量。
     */
    void graphChanged(int nodeCount, int edgeCount);

    /**
     * @brief 节点选择变化信号
     * @param id    新选中节点的 ID，或取消选中时为 -1。
     * @param title 选中节点的标题，或取消选中时为空字符串。
     */
    void nodeSelected(int id, const QString &title);

    /**
     * @brief 边选择变化信号
     * @param from  选中边的源节点 ID，或取消选中时为 -1。
     * @param to    选中边的目标节点 ID，或取消选中时为 -1。
     * @param label 选中边的标签，或取消选中时为空字符串。
     */
    void edgeSelected(int from, int to, const QString &label);

    /**
     * @brief 视图变化信号，在缩放级别或平移偏移改变后发射
     * @param zoom 当前缩放因子。
     */
    void viewChanged(double zoom);

protected:
    /**
     * @brief 绘制事件，渲染网格、边、连接预览和节点
     * @param event 未使用的绘制事件参数。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 尺寸变化事件，在首次调整大小时若图非空则调度一次初始适应视图
     * @param event 未使用的尺寸变化事件参数。
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief 上下文菜单事件，根据光标下方内容显示节点/边/全局操作菜单
     * @param event 携带控件空间光标位置的上下文菜单事件。
     */
    void contextMenuEvent(QContextMenuEvent *event) override;

    /**
     * @brief 鼠标双击事件，打开行内输入对话框重命名节点或编辑边标签
     * @param event 鼠标双击事件。
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标按下事件，开始平移（Alt 或中键）、节点拖拽或选择
     * @param event 鼠标按下事件。
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件，在光标移动期间执行平移或节点拖拽
     * @param event 鼠标移动事件。
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件，终止活动的平移或拖拽并恢复默认光标
     * @param event 鼠标释放事件。
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 滚轮事件，处理以光标位置为中心的滚轮缩放
     * @param event 携带光标位置和 delta 的滚轮事件。
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief 按键事件，按下 Delete 或 Backspace 时删除选中项
     * @param event 按键事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief 获取节点的场景空间边界矩形
     * @param node 要查询的节点。
     * @return 节点的 rect 成员。
     */
    QRectF nodeRect(const Node &node) const;

    /**
     * @brief 场景坐标转控件坐标
     * @param point 场景坐标中的点。
     * @return 控件坐标中的点（应用缩放和平移偏移）。
     */
    QPointF sceneToWidget(const QPointF &point) const;

    /**
     * @brief 控件坐标转场景坐标
     * @param point 控件坐标中的点。
     * @return 场景坐标中的点（sceneToWidget 的逆变换）。
     */
    QPointF widgetToScene(const QPointF &point) const;

    /**
     * @brief 场景矩形转控件矩形
     * @param rect 场景坐标中的矩形。
     * @return 控件坐标中的矩形（两个角都被变换）。
     */
    QRectF sceneToWidget(const QRectF &rect) const;

    /**
     * @brief 点击测试，判断哪个节点（如果有）包含给定的场景空间点
     * @param scenePoint 要测试的场景坐标中的点。
     * @return 包含该点的节点 ID，若无则返回 -1。
     */
    int hitNode(const QPointF &scenePoint) const;

    /**
     * @brief 边点击测试，判断哪条边（如果有）经过给定场景空间点附近
     * @param scenePoint 要测试的场景坐标中的点。
     * @return 匹配边的索引，若无则返回 -1。
     */
    int hitEdge(const QPointF &scenePoint) const;

    /**
     * @brief 根据 ID 查找节点（非常量重载）
     * @param id 要搜索的节点标识符。
     * @return 指向节点的指针，或未找到时为 @c nullptr。
     */
    Node *findNode(int id);

    /**
     * @brief 根据 ID 查找节点（常量重载）
     * @param id 要搜索的节点标识符。
     * @return 指向节点的常量指针，或未找到时为 @c nullptr。
     */
    const Node *findNode(int id) const;

    /**
     * @brief 计算所有节点矩形外加余量的轴对齐边界框
     * @return 场景空间中的边界矩形，若无节点则返回空矩形。
     */
    QRectF graphBounds() const;

    /**
     * @brief 返回节点上用于边端点的锚点
     * @param node   请求锚点的节点。
     * @param output @c true 表示右侧（源）锚点，@c false 表示左侧（目标）锚点。
     * @return 节点矩形左或右边的中点。
     */
    QPointF nodeAnchor(const Node &node, bool output) const;

    /**
     * @brief 构建边的三次贝塞尔 QPainterPath（场景空间）
     * @param edge 引用两个有效节点的边定义。
     * @return 适合用于描边渲染或点击测试的绘制路径。
     */
    QPainterPath edgePath(const Edge &edge) const;

    /**
     * @brief 设置选中节点 ID，发射 nodeSelected 信号并调度重绘
     * @param id 要选中的节点 ID，或 -1 清除选择。
     */
    void setSelectedNode(int id);

    /**
     * @brief 设置选中边索引，发射 edgeSelected 信号并调度重绘
     * @param index 要选中的边索引，或 -1 清除选择。
     */
    void setSelectedEdge(int index);

    /**
     * @brief 通过 QInputDialog 提示用户重命名节点
     * @param id 要重命名的节点 ID。
     */
    void editNodeTitle(int id);

    /**
     * @brief 通过 QInputDialog 提示用户编辑边的标签
     * @param index 要编辑的边索引。
     */
    void editEdgeLabel(int index);

    /**
     * @brief 发射 graphChanged 信号，携带当前节点/边计数
     */
    void emitGraphChanged();

    /**
     * @brief 将 m_zoom 限制在 [MinZoom, MaxZoom] 范围内
     */
    void clampZoom();

    /**
     * @brief 绘制点状网格背景
     * @param painter 控件当前的绘制器。
     */
    void drawGrid(QPainter *painter) const;

    /**
     * @brief 绘制单条边曲线、箭头和可选标签
     * @param painter 控件当前的绘制器。
     * @param edge    要绘制的边。
     * @param active  是否以高亮（红色）样式绘制。
     * @param selected 边是否被选中（较粗描边）。
     */
    void drawEdge(QPainter *painter, const Edge &edge, bool active, bool selected) const;

    /**
     * @brief 绘制单个节点矩形，包含标题、副标题和锚点圆点
     * @param painter 控件当前的绘制器。
     * @param node    要绘制的节点。
     * @param selected 节点是否被选中（深色边框）。
     * @param active  是否以高亮（红色）样式绘制。
     */
    void drawNode(QPainter *painter, const Node &node, bool selected, bool active) const;

    /**
     * @brief 在用户从节点拖拽连接时绘制虚线预览曲线
     * @param painter 控件当前的绘制器。
     */
    void drawConnectionDraft(QPainter *painter) const;

    QVector<Node> m_nodes;                 /**< 图中所有节点的列表，按插入顺序排列 */
    QVector<Edge> m_edges;                 /**< 图中所有边的列表，按插入顺序排列 */
    double m_zoom = 1.0;                    /**< 当前缩放因子，限制在 [MinZoom, MaxZoom] */
    QPointF m_panOffset;                    /**< 从场景空间到控件空间的平移量 */
    bool m_gridVisible = true;              /**< 是否绘制背景网格 */
    bool m_draggingNode = false;            /**< 用户用左键拖拽节点时为 @c true */
    bool m_panning = false;                 /**< 用户平移时（Alt 或中键）为 @c true */
    bool m_connecting = false;              /**< 用户正在拖拽连接时为 @c true（预留给未来使用） */
    bool m_needInitialFit = true;           /**< 在设置新图后 fitToView 至少被调用一次之前为 @c true */
    QPointF m_lastWidgetPos;                /**< 上一次鼠标在控件坐标中的位置，用于拖拽/平移时的增量计算 */
    QPointF m_lastScenePos;                 /**< 上一次鼠标在场景坐标中的位置，用于节点拖拽的增量计算 */
    QPointF m_connectionEnd;                /**< 连接拖拽时当前的终点（场景坐标） */
    int m_selectedNodeId = -1;              /**< 当前选中节点的 ID，若无则为 -1 */
    int m_selectedEdgeIndex = -1;           /**< 当前选中边的索引，若无则为 -1 */
    int m_connectionFromId = -1;            /**< 连接拖拽时源节点的 ID，若无则为 -1 */
    int m_nextNodeId = 1;                   /**< 单调递增计数器，用于给新节点分配唯一 ID */

    QSet<int> m_activeNodeIds;              /**< 当前作为链或显式高亮的一部分被高亮的节点 ID 集合 */
    QSet<int> m_activeEdgeIndices;          /**< 当前作为链或显式高亮的一部分被高亮的边索引集合 */
};
