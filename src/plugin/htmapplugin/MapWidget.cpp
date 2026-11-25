/**
 * @file MapWidget.cpp
 * @brief HT 地图显示控件实现
 * @date 2026-11-25
 */

#include "MapWidget.h"
#include "propertyboxitem.h"
#include "logger.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QSettings>
#include <QCoreApplication>
#include <QDateTime>
#include <QTransform>
#include <QScrollBar>
#include <QtMath>
#include <cmath>

// ===== 瓦片背景图元 =====
class TileBackgroundItem : public QGraphicsItem
{
public:
    explicit TileBackgroundItem(MapWidget *widget)
        : QGraphicsItem(nullptr), m_widget(widget) {}

    QRectF boundingRect() const override {
        // 一个很大的世界矩形（瓦片世界坐标）
        const int z = m_widget->m_zoom;
        const double n = (double)(1 << z);
        return QRectF(0, 0, 256.0 * 2.0 * n, 256.0 * n);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override {
        const int z = m_widget->m_zoom;
        const int cols = 1 << (z + 1);
        const int rows = 1 << z;

        // 可见场景矩形 → 瓦片范围
        QRectF visible = m_widget->sceneRect();
        // 用 view 的视口反算更精确，这里用整个场景范围裁剪到合理区域
        QRectF viewRect = m_widget->mapToScene(m_widget->viewport()->rect()).boundingRect();
        int cMin = qMax(0, (int)(viewRect.left() / 256.0) - 1);
        int cMax = qMin(cols - 1, (int)(viewRect.right() / 256.0) + 1);
        int rMin = qMax(0, (int)(viewRect.top() / 256.0) - 1);
        int rMax = qMin(rows - 1, (int)(viewRect.bottom() / 256.0) + 1);

        for (int r = rMin; r <= rMax; ++r) {
            for (int c = cMin; c <= cMax; ++c) {
                TileKey key; key.z = z; key.col = c; key.row = r;
                QImage img = m_widget->m_tiles.cachedTile(key);
                if (img.isNull()) {
                    if (m_widget->m_tiles.hasTile(key)) m_widget->m_tiles.requestTile(key);
                    continue;
                }
                painter->drawImage(QRectF(c * 256.0, r * 256.0, 256, 256), img);
            }
        }
    }

private:
    MapWidget *m_widget;
};

// ===== MapWidget =====
MapWidget::MapWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    qRegisterMetaType<PlatformData>("PlatformData");

    m_tiles.initialize();
    connect(&m_tiles, &TileProvider::tileLoaded, this, &MapWidget::onTileLoaded);

    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);
    m_fixedZoom = settings.value("HtMap/FixedZoom", -1).toInt();
    m_zoom = settings.value("HtMap/DefaultZoom", 9).toInt();
    m_centerLon = settings.value("HtMap/CenterLon", 94.0).toDouble();
    m_centerLat = settings.value("HtMap/CenterLat", 21.0).toDouble();
    m_zoom = qBound(0, m_zoom, m_tiles.maxZoom());
    if (m_fixedZoom >= 0) m_zoom = m_fixedZoom;

    // 本地模式：自动定位到有瓦片的数据区域（ini 的固定中心可能不在数据区）
    if (m_tiles.mode() == TileSourceMode::Local && m_tiles.hasZoom(m_zoom)) {
        const QPointF dataCenter = m_tiles.dataCenter(m_zoom);
        if (dataCenter.x() != 0.0 || dataCenter.y() != 0.0) {
            m_centerLon = dataCenter.x();
            m_centerLat = dataCenter.y();
        }
    }

    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing, false);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // 底图背景 item
    m_bgItem = new TileBackgroundItem(this);
    m_bgItem->setZValue(-1000);
    m_scene->addItem(m_bgItem);

    // 公共态势层（平台图元 + 属性框）
    m_situation = new SituationLayer(m_scene, this, this);

    // 合并重绘
    m_repaintTimer.setSingleShot(true);
    m_repaintTimer.setInterval(16);
    connect(&m_repaintTimer, &QTimer::timeout, this, [this]() { m_scene->invalidate(); });

    updateVisibleTiles();
    setWindowTitle(QStringLiteral("HT地图"));
}

MapWidget::~MapWidget()
{
}

DynamicObjects MapWidget::dynamicData() const
{
    DynamicObjects data;
    data.timestamp = QDateTime::currentMSecsSinceEpoch();
    if (m_situation) {
        for (const PlatformData &p : m_situation->platforms()) data.platforms.insert(p.id, p);
    }
    return data;
}

// ===== IMapCanvas =====
QPointF MapWidget::lonLatToScene(double lon, double lat) const
{
    const double n = (double)(1 << m_zoom);
    const double worldW = 256.0 * 2.0 * n;
    const double worldH = 256.0 * n;
    return QPointF((lon + 180.0) / 360.0 * worldW, (90.0 - lat) / 180.0 * worldH);
}

QPointF MapWidget::sceneToLonLat(const QPointF &scene) const
{
    const double n = (double)(1 << m_zoom);
    const double worldW = 256.0 * 2.0 * n;
    const double worldH = 256.0 * n;
    return QPointF(scene.x() / worldW * 360.0 - 180.0, 90.0 - scene.y() / worldH * 180.0);
}

void MapWidget::updateVisibleTiles()
{
    // 把视口中心对准当前经纬度
    const QPointF center = lonLatToScene(m_centerLon, m_centerLat);
    centerOn(center);
    m_scene->invalidate();
}

void MapWidget::requestTilesInView()
{
    // TileBackgroundItem::paint 内部按需请求，这里留空
}

// ===== 交互 =====
void MapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const QPointF scenePos = mapToScene(event->pos());
        // 先检查场景里是否命中了可交互图元（属性框/平台）
        QGraphicsItem *hit = m_scene->itemAt(scenePos, QTransform());
        const bool hitInteractive = (dynamic_cast<PropertyBoxItem*>(hit) != nullptr);

        if (hitInteractive) {
            // 属性框等可拖动图元：交给 QGraphicsView 正常派发，不启动底图拖动
            QGraphicsView::mousePressEvent(event);
            return;
        }

        // 空白处：交给态势层判断是否点中平台（弹/关属性框）
        const bool onPlatform = m_situation->handleSceneClick(scenePos);
        if (!onPlatform) {
            m_dragging = true;
            m_lastMousePos = event->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        const QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        const QPointF c = lonLatToScene(m_centerLon, m_centerLat);
        const QPointF ll = sceneToLonLat(c - QPointF(delta.x(), delta.y()));
        double lon = fmod(ll.x() + 180.0, 360.0);
        if (lon < 0) lon += 360.0;
        m_centerLon = lon - 180.0;
        m_centerLat = qBound(-85.0, ll.y(), 85.0);
        updateVisibleTiles();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
    QGraphicsView::mouseReleaseEvent(event);
}

void MapWidget::wheelEvent(QWheelEvent *event)
{
    if (m_fixedZoom >= 0) return;
    int delta = event->angleDelta().y();
    if (delta == 0) delta = event->delta();
    if (delta == 0) return;
    const bool zoomIn = delta > 0;
    const int target = zoomIn ? qMin(m_zoom + 1, m_tiles.maxZoom()) : qMax(m_zoom - 1, 0);
    const int effective = (m_tiles.mode() == TileSourceMode::Local)
                          ? m_tiles.nearestZoom(target, zoomIn) : target;
    if (effective != m_zoom) {
        m_zoom = effective;
        updateVisibleTiles();
        m_situation->refreshPositions();
    }
    event->accept();
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateVisibleTiles();
}

void MapWidget::onTileLoaded(const TileKey &key, const QImage &image)
{
    Q_UNUSED(key); Q_UNUSED(image);
    m_repaintTimer.start();
}
