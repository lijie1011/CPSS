/**
 * @file propertyboxitem.cpp
 * @brief 场景内属性框图元类实现
 * @details 实现属性/事件信息框在场景中的绘制与交互（拖动、双击）。
 *          交互均以信号形式抛给 view 处理，item 不直接调用 view。
 */

#include "propertyboxitem.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

/**
 * @brief 构造函数
 * @param id 归属对象 ID（平台 ID 或事件 ID）
 * @param isEvent 是否为事件框
 * @param parent 父图元
 */
PropertyBoxItem::PropertyBoxItem(const QString &id, bool isEvent, QGraphicsItem *parent)
    : QGraphicsObject(parent),
      m_id(id),
      m_isEvent(isEvent),
      m_bgColor(Qt::white),
      m_borderColor(Qt::black),
      m_font(QStringLiteral("Arial"), 9),
      m_padding(8.0),
      m_attached(true),
      m_offset(0.0, 0.0)
{
    // 属性框需要接收鼠标事件以支持拖动与双击
    setAcceptedMouseButtons(Qt::LeftButton);
}

/**
 * @brief 获取边界矩形
 * @return 由文本内容与内边距测量得到的矩形（局部原点为左上角）
 */
QRectF PropertyBoxItem::boundingRect() const
{
    QFontMetricsF fm(m_font);
    // 多行文本按最宽行计算宽度、按行数计算高度
    QRectF textRect = fm.boundingRect(QRectF(0, 0, 10000, 10000),
                                      Qt::AlignLeft | Qt::TextWordWrap,
                                      m_text);
    qreal w = textRect.width() + m_padding * 2.0;
    qreal h = textRect.height() + m_padding * 2.0;
    // 预留 1px 边框，避免绘制被裁剪
    return QRectF(-1.0, -1.0, w + 2.0, h + 2.0);
}

/**
 * @brief 绘制图元
 * @param painter 绘制器
 * @param option 样式选项（未使用）
 * @param widget 父界面（未使用）
 */
void PropertyBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QFontMetricsF fm(m_font);
    QRectF textRect = fm.boundingRect(QRectF(0, 0, 10000, 10000),
                                      Qt::AlignLeft | Qt::TextWordWrap,
                                      m_text);
    qreal w = textRect.width() + m_padding * 2.0;
    qreal h = textRect.height() + m_padding * 2.0;

    // 绘制背景与边框
    painter->setPen(QPen(m_borderColor, 1));
    painter->setBrush(m_bgColor);
    painter->drawRect(QRectF(0, 0, w, h));

    // 绘制黑色多行文本（左对齐、自动换行）
    painter->setPen(Qt::black);
    painter->setFont(m_font);
    QRectF contentRect(m_padding, m_padding, w - m_padding * 2.0, h - m_padding * 2.0);
    painter->drawText(contentRect, Qt::AlignLeft | Qt::TextWordWrap, m_text);
}

/**
 * @brief 设置显示文本
 * @param text 多行文本
 */
void PropertyBoxItem::setText(const QString &text)
{
    if (m_text == text) {
        return;
    }
    prepareGeometryChange();
    m_text = text;
    update();
}

/**
 * @brief 设置背景与边框颜色
 * @param bg 背景色
 * @param border 边框色
 */
void PropertyBoxItem::setColors(const QColor &bg, const QColor &border)
{
    m_bgColor = bg;
    m_borderColor = border;
    update();
}

/**
 * @brief 鼠标按下事件
 * @param event 鼠标事件
 */
void PropertyBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressScenePos = event->scenePos();
        m_pressItemPos = pos();
        event->accept();
        return;
    }
    QGraphicsObject::mousePressEvent(event);
}

/**
 * @brief 鼠标移动事件（拖动）
 * @param event 鼠标事件
 */
void PropertyBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 拖动增量：新位置 = 按下时位置 + 场景坐标位移
    QPointF delta = event->scenePos() - m_pressScenePos;
    setPos(m_pressItemPos + delta);
    // 一旦拖动即脱离/固定，不再跟随平台
    m_attached = false;
    emit boxMoved(this);
    event->accept();
}

/**
 * @brief 鼠标释放事件
 * @param event 鼠标事件
 */
void PropertyBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

/**
 * @brief 鼠标双击事件
 * @param event 鼠标事件
 */
void PropertyBoxItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit boxDoubleClicked(this);
    event->accept();
}
