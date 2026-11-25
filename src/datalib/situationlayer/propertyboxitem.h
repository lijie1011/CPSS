/**
 * @file propertyboxitem.h
 * @brief 场景内属性框图元类定义
 * @details 本类定义了在 QGraphicsScene 中显示的属性/事件信息框图元。
 *          取代原先基于 QLabel 顶层浮动窗口的实现，使属性框统一进入
 *          场景坐标体系，随海图缩放/平移自动跟随，并支持拖动、双击删除、
 *          虚线连接三项交互。
 *          - 默认附着（跟随平台），一旦被拖动即自动切为脱离/固定；
 *          - 双击触发删除信号（由 view 负责实际清理）。
 *          交互通过信号抛给 view 处理，避免 item 反向直接调用 view。
 */

#ifndef PROPERTYBOXITEM_H
#define PROPERTYBOXITEM_H

#include <QGraphicsObject>
#include "dynamicdata.h"   // CPSS_DATA_API
#include <QColor>
#include <QFont>
#include <QPointF>
#include <QString>

/**
 * @class PropertyBoxItem
 * @brief 属性框图元类
 * @details 继承自 QGraphicsObject（QObject + QGraphicsItem），支持信号槽。
 *          绘制为一个带背景色与边框的多行文本框，局部原点 (0,0) 为左上角。
 */
class CPSS_DATA_API PropertyBoxItem : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param id 归属对象 ID（平台 ID 或事件 ID）
     * @param isEvent 是否为事件框（true 事件框 / false 平台属性框）
     * @param parent 父图元
     */
    explicit PropertyBoxItem(const QString &id, bool isEvent, QGraphicsItem *parent = nullptr);

    /**
     * @brief 获取边界矩形
     * @return 图元的边界矩形（含内边距）
     */
    QRectF boundingRect() const override;

    /**
     * @brief 绘制图元
     * @param painter 绘制器
     * @param option 样式选项
     * @param widget 父界面
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief 设置显示文本
     * @param text 多行文本
     * @details 内部调用 prepareGeometryChange() 后 update()，
     *          以便根据文本重新测量 boundingRect，避免残影/命中错位。
     */
    void setText(const QString &text);

    /**
     * @brief 设置背景与边框颜色
     * @param bg 背景色
     * @param border 边框色
     */
    void setColors(const QColor &bg, const QColor &border);

    /**
     * @brief 获取归属对象 ID
     * @return ID
     */
    QString boxId() const { return m_id; }

    /**
     * @brief 是否为事件框
     * @return true 表示事件框
     */
    bool isEventBox() const { return m_isEvent; }

    /**
     * @brief 是否处于附着状态（跟随平台）
     * @return true 表示附着
     */
    bool isAttached() const { return m_attached; }

    /**
     * @brief 设置附着状态
     * @param attached true 附着（跟随），false 脱离（固定）
     */
    void setAttached(bool attached) { m_attached = attached; }

    /**
     * @brief 获取相对锚点的偏移量
     * @return 偏移（场景坐标）
     */
    QPointF offset() const { return m_offset; }

    /**
     * @brief 设置相对锚点的偏移量
     * @param off 偏移（场景坐标）
     */
    void setOffset(const QPointF &off) { m_offset = off; }

signals:
    /**
     * @brief 拖动中信号
     * @param item 被拖动的属性框图元
     */
    void boxMoved(PropertyBoxItem *item);

    /**
     * @brief 双击信号
     * @param item 被双击的属性框图元
     */
    void boxDoubleClicked(PropertyBoxItem *item);

protected:
    /**
     * @brief 鼠标按下事件
     * @param event 鼠标事件
     * @details 记录按下时的场景坐标与图元位置，用于拖动增量计算。
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件（拖动）
     * @param event 鼠标事件
     * @details 拖动即将 m_attached 置为 false（脱离/固定），并发 boxMoved 信号。
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标双击事件
     * @param event 鼠标事件
     * @details 发 boxDoubleClicked 信号，由 view 执行删除。
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QString m_id;            ///< 归属对象 ID
    bool m_isEvent;          ///< 是否为事件框
    QString m_text;          ///< 显示文本（多行）
    QColor m_bgColor;        ///< 背景色
    QColor m_borderColor;    ///< 边框色
    QFont m_font;            ///< 文本字体
    qreal m_padding;         ///< 内边距
    bool m_attached;         ///< 是否附着（跟随平台）
    QPointF m_offset;        ///< 相对锚点偏移

    QPointF m_pressScenePos; ///< 按下时的场景坐标（拖动增量基准）
    QPointF m_pressItemPos;  ///< 按下时的图元位置（拖动增量基准）
};

#endif // PROPERTYBOXITEM_H
