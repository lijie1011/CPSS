/**
 * @file BoundingBoxItem.h
 * @brief 选中平台时显示的 3D 透明包围盒（随姿态变化）
 * @details 包围盒数据来自姿态图集生成器的模型归一化包围盒，
 *          渲染变换与图集完全一致。
 * @date 2026-11-25
 */

#ifndef BOUNDINGBOXITEM_H
#define BOUNDINGBOXITEM_H

#include "platformdisplaystate.h"
#include "dynamicdata.h"   // CPSS_DATA_API
#include <QGraphicsItem>
#include <QVector3D>

class PlatformItem;

class CPSS_DATA_API BoundingBoxItem : public QGraphicsItem
{
public:
    explicit BoundingBoxItem(PlatformItem *platform, int gridN = 3, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setGridN(int n) { m_gridN = n; update(); }
    void refresh() { update(); }
    PlatformItem* platform() const { return m_platform; }

private:
    /** @brief 取模型归一化包围盒 min/max（与 gen_models.py 一致） */
    void modelBounds(QVector3D &mn, QVector3D &mx) const;
    /** @brief 3D 点投影到 item 局部坐标（与图集生成变换一致） */
    QPointF project(const QVector3D &p, double heading, double roll) const;

    PlatformItem *m_platform = nullptr;
    int m_gridN = 3;
};

#endif // BOUNDINGBOXITEM_H
