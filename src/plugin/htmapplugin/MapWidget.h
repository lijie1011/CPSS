/**
 * @file MapWidget.h
 * @brief HT 地图显示控件（瓦片底图 + 公共态势层）
 * @details 继承 QGraphicsView：底图瓦片作为背景 item，平台/属性框由 SituationLayer 管理。
 * @date 2026-11-25
 */

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QPointF>
#include <QImage>
#include <QTimer>
#include "TileProvider.h"
#include "WmsProvider.h"
#include "IMapCanvas.h"
#include "SituationLayer.h"
#include "dynamicdata.h"

class TileBackgroundItem;

class MapWidget : public QGraphicsView, public IMapCanvas
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    // ===== IMapCanvas 实现 =====
    QPointF lonLatToScene(double lon, double lat) const override;
    QPointF sceneToLonLat(const QPointF &scene) const override;
    double zoomScale() const override { return 1.0; }

    /** @brief 获取态势层（供插件接口暴露显示状态） */
    SituationLayer* situationLayer() const { return m_situation; }
    /** @brief 当前动态目标数据 */
    DynamicObjects dynamicData() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTileLoaded(const TileKey &key, const QImage &image);

private:
    friend class TileBackgroundItem;
    void updateVisibleTiles();
    void requestTilesInView();

    TileProvider m_tiles;
    WmsProvider m_wms;

    QGraphicsScene *m_scene = nullptr;
    TileBackgroundItem *m_bgItem = nullptr;
    SituationLayer *m_situation = nullptr;

    int m_zoom = 9;
    int m_fixedZoom = -1;
    double m_centerLon = 94.0;
    double m_centerLat = 21.0;

    bool m_dragging = false;
    QPoint m_lastMousePos;
    int m_wheelDelta = 0;
    double m_pinchZoom = 0.0;
    QTimer m_repaintTimer;
};

#endif // MAPWIDGET_H
