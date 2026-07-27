#ifndef PLATFORMITEM_H
#define PLATFORMITEM_H

#include <QGraphicsItem>
#include <QImage>
#include <QPainterPath>
#include <QColor>

#include "dynamicdata.h"
#include "displaystate.h"

class PlatformItem : public QGraphicsItem
{
public:
    PlatformItem(const PlatformData &platform, const QImage &redBoatIcon,
                 const QImage &redPlaneIcon, const QImage &purpleBoatIcon,
                 const QImage &purplePlaneIcon, QGraphicsItem *parent = nullptr);

    void updateData(const PlatformData &platform);
    void updateDisplayState(const PlatformDisplayState &state);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString id() const { return m_platform.id; }
    bool isExpired() const { return m_platform.isExpired(); }
    double lon() const { return m_platform.lon; }
    double lat() const { return m_platform.lat; }

private:
    void updateIcon();
    void updateBoundingRect();

    PlatformData m_platform;
    PlatformDisplayState m_displayState;

    QImage m_redBoatIcon;
    QImage m_redPlaneIcon;
    QImage m_purpleBoatIcon;
    QImage m_purplePlaneIcon;
    QImage m_currentIcon;
    QImage m_scaledIcon;

    QRectF m_boundingRect;
    QColor m_campColor;
    bool m_isMissile;
};

#endif
