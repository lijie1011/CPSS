/**
 * @file SituationLayer.cpp
 * @brief 公共态势层实现（完整功能，chart 与 htmapplugin 共用）
 * @date 2026-11-25
 */

#include "SituationLayer.h"
#include "platformitem.h"
#include "propertyboxitem.h"
#include "datamanager.h"
#include "datacache.h"
#include "PoseAtlasManager.h"
#include "BoundingBoxItem.h"
#include "logger.h"

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QColor>
#include <QtMath>

SituationLayer::SituationLayer(QGraphicsScene *scene, IMapCanvas *canvas, QObject *parent)
    : QObject(parent), m_scene(scene), m_canvas(canvas)
{
    PoseAtlasManager::instance()->initialize();
    DataManager *dm = DataManager::instance();
    connect(dm, &DataManager::platformUpdated, this, &SituationLayer::onPlatformUpdated);
    connect(dm, &DataManager::platformRemoved, this, &SituationLayer::onPlatformRemoved);

    for (const PlatformData &p : dm->getAllPlatforms()) onPlatformUpdated(p);
}

SituationLayer::~SituationLayer()
{
    for (auto it = m_propertyBoxes.begin(); it != m_propertyBoxes.end(); ++it) {
        delete it->box;
        delete it->line;
    }
    m_propertyBoxes.clear();
    for (PlatformItem *item : m_platformItems) delete item;
    m_platformItems.clear();
    if (m_selectedBox) { delete m_selectedBox; m_selectedBox = nullptr; }
    for (QGraphicsPathItem *t : m_trackItems) delete t;
    m_trackItems.clear();
    for (auto it = m_rangeItems.begin(); it != m_rangeItems.end(); ++it)
        qDeleteAll(it.value());
    m_rangeItems.clear();
}

void SituationLayer::updateTrack(const QString &id)
{
    if (!m_scene || !m_canvas) return;
    PlatformItem *item = m_platformItems.value(id, nullptr);
    if (!item) return;
    const PlatformData &p = item->platformData();
    const PlatformDisplayState st = m_displayStates.value(id);
    const bool show = st.showTrack;

    QGraphicsPathItem *pathItem = m_trackItems.value(id, nullptr);
    if (!show || p.trackPoints.isEmpty() || p.isExpired()) {
        if (pathItem) pathItem->setVisible(false);
        return;
    }

    QPainterPath path;
    bool first = true;
    for (const QPointF &pt : p.trackPoints) {
        const QPointF scene = m_canvas->lonLatToScene(pt.x(), pt.y());
        if (first) { path.moveTo(scene); first = false; }
        else path.lineTo(scene);
    }

    if (!pathItem) {
        pathItem = new QGraphicsPathItem();
        pathItem->setZValue(-10);
        m_scene->addItem(pathItem);
        m_trackItems.insert(id, pathItem);
    }
    pathItem->setPath(path);
    pathItem->setPen(QPen(campColor(p.camp), 2, Qt::DashLine));
    pathItem->setVisible(true);
}

void SituationLayer::updateRanges(const QString &id)
{
    if (!m_scene || !m_canvas) return;
    PlatformItem *item = m_platformItems.value(id, nullptr);
    if (!item) return;
    const PlatformData &p = item->platformData();
    const PlatformDisplayState st = m_displayStates.value(id);

    // 清理旧的
    if (m_rangeItems.contains(id)) {
        for (QGraphicsEllipseItem *e : m_rangeItems.take(id)) delete e;
    }

    if (p.isExpired()) return;
    const QPointF center = m_canvas->lonLatToScene(p.lon, p.lat);
    QList<QGraphicsEllipseItem*> circles;

    auto addCircle = [&](double range, const QColor &color) {
        if (range <= 0) return;
        const double rangeDeg = range * 1.852 / 111.0;   // 海里 -> 度
        const QPointF edge = m_canvas->lonLatToScene(p.lon + rangeDeg, p.lat);
        const double r = qAbs(edge.x() - center.x());
        if (r <= 0) return;
        QGraphicsEllipseItem *c = new QGraphicsEllipseItem(center.x()-r, center.y()-r, 2*r, 2*r);
        c->setPen(QPen(color, 1, Qt::DashLine));
        c->setBrush(Qt::NoBrush);
        c->setZValue(-20);
        m_scene->addItem(c);
        circles.append(c);
    };

    if (st.showSensors) {
        for (const ComponentState &s : st.sensors) addCircle(s.range, Qt::cyan);
    }
    if (st.showWeapons) {
        for (const ComponentState &w : st.weapons) addCircle(w.range, QColor(255,165,0));
    }
    if (!circles.isEmpty()) m_rangeItems.insert(id, circles);
}

QList<PlatformData> SituationLayer::platforms() const
{
    QList<PlatformData> result;
    for (auto it = m_platformItems.constBegin(); it != m_platformItems.constEnd(); ++it)
        result.append(it.value()->platformData());
    return result;
}

QString SituationLayer::campName(CampType camp) const
{
    switch (camp) {
    case Camp_Mine:     return QStringLiteral("我方");
    case Camp_Friendly: return QStringLiteral("友方");
    case Camp_Enemy:    return QStringLiteral("敌方");
    case Camp_Neutral:  return QStringLiteral("中立");
    default:            return QStringLiteral("未知");
    }
}

void SituationLayer::onPlatformUpdated(const PlatformData &platform)
{
    if (!m_scene || !m_canvas || platform.id.isEmpty()) return;

    // 关键：DataManager 的原始信号不带航迹（航迹由 DataCache 累积生成）
    // 从 DataCache 取融合后的数据，保证航迹/状态与 chart 一致
    PlatformData data = platform;
    PlatformData cached = DataCache::instance()->getPlatform(platform.id);
    if (!cached.id.isEmpty()) data = cached;

    PlatformItem *item = m_platformItems.value(platform.id, nullptr);
    if (!item) {
        item = new PlatformItem(data);
        m_platformItems.insert(platform.id, item);
        m_scene->addItem(item);
    } else {
        item->updateData(data);
    }
    item->setPos(m_canvas->lonLatToScene(platform.lon, platform.lat));
    applyDisplayState(platform.id);
    updateTrack(platform.id);
    updateRanges(platform.id);

    if (m_propertyBoxes.contains(platform.id)) {
        updatePropertyBoxText(platform.id);
        updateConnectingLine(platform.id);
    }
}

void SituationLayer::onPlatformRemoved(const QString &id)
{
    if (m_propertyBoxes.contains(id)) destroyPropertyBox(id);
    if (m_trackItems.contains(id)) { delete m_trackItems.take(id); }
    if (m_rangeItems.contains(id)) {
        for (QGraphicsEllipseItem *e : m_rangeItems.take(id)) delete e;
    }
    PlatformItem *item = m_platformItems.take(id);
    if (item) delete item;
    m_displayStates.remove(id);
}

void SituationLayer::setEvents(const QList<SpecialEvent> &events)
{
    m_events = events;
    for (const QString &id : m_propertyBoxes.keys()) updatePropertyBoxText(id);
}

void SituationLayer::setDisplayStates(const DisplayStateMap &states)
{
    m_displayStates = states;
    for (const QString &id : m_platformItems.keys()) applyDisplayState(id);
    for (const QString &id : m_propertyBoxes.keys()) updateConnectingLine(id);
}

void SituationLayer::applyDisplayState(const QString &id)
{
    PlatformItem *item = m_platformItems.value(id, nullptr);
    if (!item) return;
    if (m_displayStates.contains(id)) {
        item->updateDisplayState(m_displayStates.value(id));
    } else {
        PlatformDisplayState st;
        st.platformId = id;
        item->updateDisplayState(st);
    }
    updateTrack(id);
    updateRanges(id);
}

void SituationLayer::refreshPositions()
{
    if (!m_canvas) return;
    for (auto it = m_platformItems.constBegin(); it != m_platformItems.constEnd(); ++it) {
        const PlatformData &p = it.value()->platformData();
        it.value()->setPos(m_canvas->lonLatToScene(p.lon, p.lat));
    }
    for (const QString &id : m_platformItems.keys()) {
        updateTrack(id);
        updateRanges(id);
    }
    // 选中包围盒跟随平台
    if (m_selectedBox) {
        m_selectedBox->setPos(m_selectedBox->platform()->pos());
        m_selectedBox->refresh();
    }
    for (const QString &id : m_propertyBoxes.keys()) {
        PropertyBoxEntry &e = m_propertyBoxes[id];
        if (e.box->isAttached()) {
            PlatformItem *item = m_platformItems.value(id, nullptr);
            if (item) e.box->setPos(item->pos() + e.box->offset());
        }
        updateConnectingLine(id);
    }
}

bool SituationLayer::handleSceneClick(const QPointF &scenePos)
{
    if (!m_canvas) return false;
    QString hitId;
    double best = 20.0;
    for (auto it = m_platformItems.constBegin(); it != m_platformItems.constEnd(); ++it) {
        // 尊重显示状态：隐藏的平台不响应
        PlatformDisplayState st = m_displayStates.value(it.key());
        if (!st.showShip) continue;
        const QPointF d = it.value()->pos() - scenePos;
        const double dist = qSqrt(d.x()*d.x() + d.y()*d.y());
        if (dist < best) { best = dist; hitId = it.key(); }
    }
    if (hitId.isEmpty()) {
        hideAllPropertyBoxes();
        // 点击空白：隐藏 3D 包围盒
        if (m_selectedBox) { delete m_selectedBox; m_selectedBox = nullptr; }
        return false;
    }

    // 显示/切换 3D 包围盒（作为平台子图元，自动跟随）
    if (m_selectedBox) { delete m_selectedBox; m_selectedBox = nullptr; }
    PlatformItem *selItem = m_platformItems.value(hitId);
    m_selectedBox = new BoundingBoxItem(selItem, 3, selItem);

    if (m_propertyBoxes.contains(hitId)) destroyPropertyBox(hitId);
    else createPropertyBox(hitId);
    return true;
}

void SituationLayer::hideAllPropertyBoxes()
{
    for (auto it = m_propertyBoxes.begin(); it != m_propertyBoxes.end(); ++it) {
        it->box->setVisible(false);
        if (it->line) it->line->setVisible(false);
    }
}

void SituationLayer::createPropertyBox(const QString &id)
{
    if (!m_scene || !m_platformItems.contains(id)) return;

    PropertyBoxItem *box = new PropertyBoxItem(id, false);
    box->setColors(Qt::white, Qt::black);
    connect(box, &PropertyBoxItem::boxMoved, this, &SituationLayer::onPropertyBoxMoved);
    connect(box, &PropertyBoxItem::boxDoubleClicked, this, &SituationLayer::onPropertyBoxDoubleClicked);
    m_scene->addItem(box);

    QGraphicsLineItem *line = new QGraphicsLineItem();
    QPen pen(QColor(120, 120, 120), 1, Qt::DashLine);
    line->setPen(pen);
    line->setZValue(99);
    m_scene->addItem(line);

    PropertyBoxEntry entry;
    entry.box = box;
    entry.line = line;
    m_propertyBoxes.insert(id, entry);

    updatePropertyBoxText(id);

    // 附着：显示在平台上方
    const qreal boxH = box->boundingRect().height();
    box->setOffset(QPointF(0, -(boxH + 15)));
    box->setAttached(true);
    box->setZValue(100);
    PlatformItem *item = m_platformItems.value(id);
    if (item) box->setPos(item->pos() + box->offset());
    updateConnectingLine(id);
}

void SituationLayer::destroyPropertyBox(const QString &id)
{
    PropertyBoxEntry e = m_propertyBoxes.take(id);
    if (e.box) delete e.box;
    if (e.line) delete e.line;
}

void SituationLayer::updatePropertyBoxText(const QString &id)
{
    auto it = m_propertyBoxes.find(id);
    if (it == m_propertyBoxes.end()) return;
    PropertyBoxEntry *e = &it.value();
    PlatformItem *item = m_platformItems.value(id, nullptr);
    if (!e->box || !item) return;
    const PlatformData &p = item->platformData();

    // 与 chart 完全一致的格式
    QString eventsStr;
    qint64 latest = 0;
    const SpecialEvent *ev = nullptr;
    for (const SpecialEvent &x : m_events) {
        if (x.targetId == id && x.timestamp > latest) { latest = x.timestamp; ev = &x; }
    }
    if (ev) eventsStr = QStringLiteral("\n事件: %1").arg(ev->eventName);

    e->box->setText(
        QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6°\n速度: %7 节%8")
        .arg(p.name).arg(p.id).arg(campName(p.camp))
        .arg(p.lon, 0, 'f', 6).arg(p.lat, 0, 'f', 6)
        .arg(p.heading > 0 ? p.heading : 0)
        .arg(p.speed).arg(eventsStr));

    const qreal boxH = e->box->boundingRect().height();
    if (e->box->isAttached()) e->box->setOffset(QPointF(0, -(boxH + 15)));
}

void SituationLayer::updateConnectingLine(const QString &id)
{
    auto it = m_propertyBoxes.find(id);
    if (it == m_propertyBoxes.end()) return;
    PropertyBoxEntry *e = &it.value();
    PlatformItem *item = m_platformItems.value(id, nullptr);
    if (!e->line || !item) return;
    if (!e->box || !e->box->isVisible()) { e->line->setVisible(false); return; }
    const QPointF anchor = item->pos();
    const QPointF boxPos = e->box->pos();
    // 从平台到属性框底部中心
    const QPointF boxBottom(boxPos.x() + e->box->boundingRect().width() / 2.0,
                            boxPos.y() + e->box->boundingRect().height());
    e->line->setLine(QLineF(anchor, boxBottom));
    e->line->setVisible(true);
}

void SituationLayer::onPropertyBoxDoubleClicked(PropertyBoxItem *box)
{
    if (box) destroyPropertyBox(box->boxId());
}

void SituationLayer::onPropertyBoxMoved(PropertyBoxItem *box)
{
    if (!box) return;
    // PropertyBoxItem 内部已把 attached 置 false
    updateConnectingLine(box->boxId());
}
