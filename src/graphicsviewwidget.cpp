/**
 * @file graphicsviewwidget.cpp
 * @brief 海图视图部件实现
 * @details 本类是基于 QGraphicsView 的海图显示部件，负责渲染平台、
 *          传感器/武器作用范围、航迹、事件标记等动态元素。
 *          支持与 Enclib 海图库集成，提供地图缩放、平移、鹰眼概览等功能。
 */

#include "graphicsviewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <cmath>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QFont>

#include "encl.h"
#include "common/logger.h"
#include "common/IconManager.h"

/**
 * @brief 构造函数
 * @param parent 父部件
 */
GraphicsViewWidget::GraphicsViewWidget(QWidget *parent)
    : QGraphicsView(parent),
      m_enclibReady(false),
      m_overviewLabel(nullptr),
      m_scene(nullptr)
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setMouseTracking(true);
    setInteractive(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    initOverviewMap();

    // 初始化图标管理器（模板加载/转换），平台图元将按需着色取图
    IconManager::instance().init();
}

/**
 * @brief 析构函数
 */
GraphicsViewWidget::~GraphicsViewWidget()
{
    for (auto item : m_platformItems.values()) {
        delete item;
    }
    m_platformItems.clear();

    for (auto &items : m_sensorRangeItems) {
        for (auto item : items) {
            delete item;
        }
    }
    m_sensorRangeItems.clear();

    for (auto &items : m_weaponRangeItems) {
        for (auto item : items) {
            delete item;
        }
    }
    m_weaponRangeItems.clear();

    for (auto item : m_trackItems.values()) {
        delete item;
    }
    m_trackItems.clear();

    for (auto &items : m_eventMarkerItems) {
        for (auto item : items) {
            delete item;
        }
    }
    m_eventMarkerItems.clear();

    for (auto &box : m_propertyBoxes) {
        if (box.label) {
            delete box.label;
        }
    }

    delete m_overviewLabel;
    delete m_scene;
}

/**
 * @brief 设置 Enclib 海图库就绪状态
 * @param ready 就绪标志
 */
void GraphicsViewWidget::setEnclibReady(bool ready)
{
    m_enclibReady = ready;
}

/**
 * @brief 更新动态数据
 * @param data 动态目标数据
 */
void GraphicsViewWidget::updateDynamicData(const DynamicObjects &data)
{
    m_dynamicData = data;

    for (const auto &platform : data.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }

        PropertyBox *box = findPropertyBoxById(platform.id, platform.id == "SHIP_001");
        if (box && box->label && box->label->isVisible() && !box->isDragging) {
            box->lon = platform.lon;
            box->lat = platform.lat;
            box->heading = platform.heading;
            box->speed = platform.speed;

            QString campStr = campName(platform.camp);

            QString eventsStr;
            const SpecialEvent *latestEvent = nullptr;
            qint64 latestTimestamp = 0;
            for (const auto &event : data.events) {
                if (event.targetId == platform.id && event.timestamp > latestTimestamp) {
                    latestTimestamp = event.timestamp;
                    latestEvent = &event;
                }
            }
            if (latestEvent) {
                eventsStr = QStringLiteral("\n事件: %1").arg(latestEvent->eventName);
            }

            box->label->setText(
                QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6\n速度: %7 节%8")
                    .arg(platform.name)
                    .arg(platform.id)
                    .arg(campStr)
                    .arg(platform.lon, 0, 'f', 6)
                    .arg(platform.lat, 0, 'f', 6)
                    .arg(platform.heading > 0 ? platform.heading : 0)
                    .arg(platform.speed)
                    .arg(eventsStr)
            );
            box->label->adjustSize();
        }
    }

    updatePlatformItems();
    updateSensorWeaponRanges();
    updateTracks();
    updateEventMarkers();
    updateAllConnectingLines();
}

/**
 * @brief 更新地图背景
 */
void GraphicsViewWidget::updateMapBackground()
{
    if (!m_enclibReady) {
        QImage img(width(), height(), QImage::Format_RGB32);
        img.fill(Qt::darkGray);
        QPainter painter(&img);
        painter.setPen(Qt::white);
        painter.drawText(img.rect(), Qt::AlignCenter, QStringLiteral("海图未初始化"));
        m_storedViewImg = img;
        m_scene->setBackgroundBrush(QBrush(m_storedViewImg));
        return;
    }

    unsigned char *pPixBuf = EnclDrawChart();
    if (pPixBuf) {
        QImage img(pPixBuf, width(), height(), QImage::Format_RGB32);
        m_storedViewImg = QImage(img.constBits(), img.width(), img.height(), img.bytesPerLine(), img.format()).copy();
        m_scene->setBackgroundBrush(QBrush(m_storedViewImg));
    } else {
        m_scene->setBackgroundBrush(QBrush(m_storedViewImg));
    }
}

/**
 * @brief 更新平台图元
 */
void GraphicsViewWidget::updatePlatformItems()
{
    QSet<QString> currentPlatformIds;

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }
        currentPlatformIds.insert(platform.id);

        PlatformItem *item = m_platformItems.value(platform.id, nullptr);

        int x, y;
        if (!geoToScreen(platform.lon, platform.lat, x, y)) {
            continue;
        }

        if (!item) {
            item = new PlatformItem(platform);
            item->setPos(x, y);
            m_scene->addItem(item);
            m_platformItems[platform.id] = item;
        } else {
            item->updateData(platform);
            item->setPos(x, y);
        }

        auto stateIt = m_displayStates.find(platform.id);
        if (stateIt != m_displayStates.end()) {
            item->updateDisplayState(stateIt.value());
        }

        item->setVisible(true);
    }

    QList<QString> toRemove;
    for (const QString &id : m_platformItems.keys()) {
        if (!currentPlatformIds.contains(id)) {
            toRemove.append(id);
        }
    }

    for (const QString &id : toRemove) {
        PlatformItem *item = m_platformItems.take(id);
        m_scene->removeItem(item);
        delete item;
    }
}

/**
 * @brief 更新传感器和武器作用范围
 */
void GraphicsViewWidget::updateSensorWeaponRanges()
{
    QSet<QString> currentPlatformIds;

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }
        currentPlatformIds.insert(platform.id);

        bool showSensors = m_displayStates.contains(platform.id) && m_displayStates[platform.id].showSensors;
        bool showWeapons = m_displayStates.contains(platform.id) && m_displayStates[platform.id].showWeapons;

        int x, y;
        if (!geoToScreen(platform.lon, platform.lat, x, y)) {
            if (m_sensorRangeItems.contains(platform.id)) {
                for (auto item : m_sensorRangeItems[platform.id]) {
                    item->setVisible(false);
                }
            }
            if (m_weaponRangeItems.contains(platform.id)) {
                for (auto item : m_weaponRangeItems[platform.id]) {
                    item->setVisible(false);
                }
            }
            continue;
        }

        if (showSensors) {
            int sensorIndex = 0;
            for (const SensorInfo &sensor : platform.sensors) {
                if (sensor.range <= 0) {
                    continue;
                }

                double rangeKm = sensor.range * 1.852;
                double rangeDegrees = rangeKm / 111.0;

                int edgeX, edgeY;
                if (geoToScreen(platform.lon + rangeDegrees, platform.lat, edgeX, edgeY)) {
                    int radius = edgeX - x;

                    QColor sensorColor;
                    if (sensor.type.contains("radar", Qt::CaseInsensitive)) {
                        sensorColor = Qt::cyan;
                    } else if (sensor.type.contains("sonar", Qt::CaseInsensitive)) {
                        sensorColor = Qt::blue;
                    } else {
                        sensorColor = Qt::green;
                    }

                    int ellipseIdx = sensorIndex * 2;
                    int textIdx = sensorIndex * 2 + 1;

                    if (m_sensorRangeItems.contains(platform.id) && 
                        m_sensorRangeItems[platform.id].size() > ellipseIdx) {
                        QGraphicsEllipseItem *ellipse = 
                            dynamic_cast<QGraphicsEllipseItem*>(m_sensorRangeItems[platform.id][ellipseIdx]);
                        if (ellipse) {
                            ellipse->setRect(x - radius, y - radius, radius * 2, radius * 2);
                            ellipse->setVisible(true);
                        }
                        if (m_sensorRangeItems[platform.id].size() > textIdx) {
                            QGraphicsTextItem *text = 
                                dynamic_cast<QGraphicsTextItem*>(m_sensorRangeItems[platform.id][textIdx]);
                            if (text) {
                                text->setPos(x + radius + 5, y + sensorIndex * 15);
                                text->setVisible(true);
                            }
                        }
                    } else {
                        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(x - radius, y - radius, radius * 2, radius * 2);
                        ellipse->setPen(QPen(sensorColor, 1, Qt::DashLine));
                        ellipse->setBrush(Qt::NoBrush);
                        m_scene->addItem(ellipse);
                        m_sensorRangeItems[platform.id].append(ellipse);

                        QGraphicsTextItem *text = new QGraphicsTextItem(QString("%1:S:%2(%3nm)").arg(platform.id).arg(sensor.type).arg(sensor.range));
                        text->setPos(x + radius + 5, y + sensorIndex * 15);
                        text->setDefaultTextColor(sensorColor);
                        text->setFont(QFont("Arial", 7));
                        m_scene->addItem(text);
                        m_sensorRangeItems[platform.id].append(text);
                    }
                    sensorIndex++;
                }
            }

            if (m_sensorRangeItems.contains(platform.id)) {
                int expectedSize = platform.sensors.size() * 2;
                while (m_sensorRangeItems[platform.id].size() > expectedSize) {
                    auto item = m_sensorRangeItems[platform.id].takeLast();
                    m_scene->removeItem(item);
                    delete item;
                }
            }
        } else if (m_sensorRangeItems.contains(platform.id)) {
            for (auto item : m_sensorRangeItems[platform.id]) {
                item->setVisible(false);
            }
        }

        if (showWeapons) {
            int weaponIndex = 0;
            for (const WeaponInfo &weapon : platform.weapons) {
                if (weapon.range <= 0) {
                    continue;
                }

                double rangeKm = weapon.range * 1.852;
                double rangeDegrees = rangeKm / 111.0;

                int edgeX, edgeY;
                if (geoToScreen(platform.lon + rangeDegrees, platform.lat, edgeX, edgeY)) {
                    int radius = edgeX - x;

                    QColor weaponColor;
                    if (weapon.type.contains("missile", Qt::CaseInsensitive)) {
                        weaponColor = Qt::red;
                    } else if (weapon.type.contains("gun", Qt::CaseInsensitive)) {
                        weaponColor = QColor(255, 165, 0);
                    } else {
                        weaponColor = Qt::darkRed;
                    }

                    int ellipseIdx = weaponIndex * 2;
                    int textIdx = weaponIndex * 2 + 1;

                    if (m_weaponRangeItems.contains(platform.id) && 
                        m_weaponRangeItems[platform.id].size() > ellipseIdx) {
                        QGraphicsEllipseItem *ellipse = 
                            dynamic_cast<QGraphicsEllipseItem*>(m_weaponRangeItems[platform.id][ellipseIdx]);
                        if (ellipse) {
                            ellipse->setRect(x - radius, y - radius, radius * 2, radius * 2);
                            ellipse->setVisible(true);
                        }
                        if (m_weaponRangeItems[platform.id].size() > textIdx) {
                            QGraphicsTextItem *text = 
                                dynamic_cast<QGraphicsTextItem*>(m_weaponRangeItems[platform.id][textIdx]);
                            if (text) {
                                text->setPos(x + radius + 5, y + weaponIndex * 15);
                                text->setVisible(true);
                            }
                        }
                    } else {
                        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem(x - radius, y - radius, radius * 2, radius * 2);
                        ellipse->setPen(QPen(weaponColor, 2, Qt::DotLine));
                        ellipse->setBrush(Qt::NoBrush);
                        m_scene->addItem(ellipse);
                        m_weaponRangeItems[platform.id].append(ellipse);

                        QGraphicsTextItem *text = new QGraphicsTextItem(QString("%1:W:%2(%3nm)").arg(platform.id).arg(weapon.type).arg(weapon.range));
                        text->setPos(x + radius + 5, y + weaponIndex * 15);
                        text->setDefaultTextColor(weaponColor);
                        text->setFont(QFont("Arial", 7));
                        m_scene->addItem(text);
                        m_weaponRangeItems[platform.id].append(text);
                    }
                    weaponIndex++;
                }
            }

            if (m_weaponRangeItems.contains(platform.id)) {
                int expectedSize = platform.weapons.size() * 2;
                while (m_weaponRangeItems[platform.id].size() > expectedSize) {
                    auto item = m_weaponRangeItems[platform.id].takeLast();
                    m_scene->removeItem(item);
                    delete item;
                }
            }
        } else if (m_weaponRangeItems.contains(platform.id)) {
            for (auto item : m_weaponRangeItems[platform.id]) {
                item->setVisible(false);
            }
        }
    }

    QList<QString> toRemove;
    for (const QString &id : m_sensorRangeItems.keys()) {
        if (!currentPlatformIds.contains(id)) {
            toRemove.append(id);
        }
    }
    for (const QString &id : toRemove) {
        for (auto item : m_sensorRangeItems.take(id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    toRemove.clear();
    for (const QString &id : m_weaponRangeItems.keys()) {
        if (!currentPlatformIds.contains(id)) {
            toRemove.append(id);
        }
    }
    for (const QString &id : toRemove) {
        for (auto item : m_weaponRangeItems.take(id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }
}

/**
 * @brief 更新航迹
 */
void GraphicsViewWidget::updateTracks()
{
    QSet<QString> currentPlatformIds;

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }
        currentPlatformIds.insert(platform.id);

        bool showTrack = m_displayStates.contains(platform.id) && m_displayStates[platform.id].showTrack;

        if (m_trackItems.contains(platform.id)) {
            m_trackItems[platform.id]->setVisible(showTrack);
        }

        if (!showTrack || platform.trackPoints.isEmpty()) {
            continue;
        }

        // 航迹颜色与阵营颜色保持一致（我红敌蓝）
        QColor trackColor = campColor(platform.camp);

        QPainterPath trackPath;
        bool firstPoint = true;
        for (const QPointF &point : platform.trackPoints) {
            int px, py;
            if (geoToScreen(point.x(), point.y(), px, py)) {
                if (firstPoint) {
                    trackPath.moveTo(px, py);
                    firstPoint = false;
                } else {
                    trackPath.lineTo(px, py);
                }
            }
        }

        if (m_trackItems.contains(platform.id)) {
            QGraphicsPathItem *pathItem = static_cast<QGraphicsPathItem*>(m_trackItems[platform.id]);
            pathItem->setPath(trackPath);
            pathItem->setPen(QPen(trackColor, 2, Qt::DashLine));
        } else {
            QGraphicsPathItem *pathItem = new QGraphicsPathItem(trackPath);
            pathItem->setPen(QPen(trackColor, 2, Qt::DashLine));
            m_scene->addItem(pathItem);
            m_trackItems[platform.id] = pathItem;
        }
    }

    QList<QString> toRemove;
    for (const QString &id : m_trackItems.keys()) {
        if (!currentPlatformIds.contains(id)) {
            toRemove.append(id);
        }
    }
    for (const QString &id : toRemove) {
        QGraphicsItem *item = m_trackItems.take(id);
        m_scene->removeItem(item);
        delete item;
    }
}

/**
 * @brief 更新事件标记
 */
void GraphicsViewWidget::updateEventMarkers()
{
    QSet<QString> currentPlatformIds;

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }
        currentPlatformIds.insert(platform.id);

        bool showEvents = !m_displayStates.contains(platform.id) || m_displayStates[platform.id].showEvents;

        if (m_eventMarkerItems.contains(platform.id)) {
            for (auto item : m_eventMarkerItems[platform.id]) {
                item->setVisible(showEvents);
            }
        }

        if (!showEvents) {
            continue;
        }

        const SpecialEvent *latestEvent = nullptr;
        qint64 latestTimestamp = 0;
        for (const auto &event : m_dynamicData.events) {
            if (event.targetId == platform.id && event.timestamp > latestTimestamp) {
                latestTimestamp = event.timestamp;
                latestEvent = &event;
            }
        }

        if (!latestEvent) {
            if (m_eventMarkerItems.contains(platform.id)) {
                for (auto item : m_eventMarkerItems[platform.id]) {
                    item->setVisible(false);
                }
            }
            continue;
        }

        int x, y;
        if (!geoToScreen(platform.lon, platform.lat, x, y)) {
            continue;
        }

        QColor markerColor;
        QString iconText;

        switch (latestEvent->eventType) {
        case Event_Alert: markerColor = Qt::red; iconText = "!"; break;
        case Event_Attack: markerColor = Qt::darkRed; iconText = "A"; break;
        case Event_Defense: markerColor = Qt::blue; iconText = "D"; break;
        case Event_Contact: markerColor = Qt::cyan; iconText = "C"; break;
        case Event_Damage: markerColor = QColor(255, 165, 0); iconText = "X"; break;
        case Event_MissionStart: markerColor = Qt::green; iconText = "M"; break;
        case Event_MissionEnd: markerColor = Qt::gray; iconText = "E"; break;
        case Event_Lost: markerColor = Qt::magenta; iconText = "?"; break;
        case Event_Repair: markerColor = QColor(144, 238, 144); iconText = "R"; break;
        default: markerColor = Qt::yellow; iconText = "*"; break;
        }

        if (m_eventMarkerItems.contains(platform.id)) {
            QList<QGraphicsItem*> items = m_eventMarkerItems[platform.id];
            if (items.size() >= 2) {
                QGraphicsRectItem *rect = static_cast<QGraphicsRectItem*>(items[0]);
                rect->setRect(x + 20 - 8, y - 15 - 8, 16, 16);
                rect->setPen(QPen(markerColor, 2));
                rect->setBrush(QBrush(markerColor, Qt::Dense4Pattern));
                rect->setVisible(true);

                QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(items[1]);
                text->setPlainText(iconText);
                QFont font("Arial", 9, QFont::Bold);
                text->setFont(font);
                text->setDefaultTextColor(Qt::white);
                text->setPos(x + 20 - 4, y - 15 - 5);
                text->setVisible(true);
            }
        } else {
            QGraphicsRectItem *rect = new QGraphicsRectItem(x + 20 - 8, y - 15 - 8, 16, 16);
            rect->setPen(QPen(markerColor, 2));
            rect->setBrush(QBrush(markerColor, Qt::Dense4Pattern));
            m_scene->addItem(rect);
            m_eventMarkerItems[platform.id].append(rect);

            QGraphicsTextItem *text = new QGraphicsTextItem(iconText);
            QFont font("Arial", 9, QFont::Bold);
            text->setFont(font);
            text->setDefaultTextColor(Qt::white);
            text->setPos(x + 20 - 4, y - 15 - 5);
            m_scene->addItem(text);
            m_eventMarkerItems[platform.id].append(text);
        }
    }

    QList<QString> toRemove;
    for (const QString &id : m_eventMarkerItems.keys()) {
        if (!currentPlatformIds.contains(id)) {
            toRemove.append(id);
        }
    }
    for (const QString &id : toRemove) {
        for (auto item : m_eventMarkerItems.take(id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }
}

/**
 * @brief 绘制事件
 * @param event 绘制事件指针
 */
void GraphicsViewWidget::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
    updateOverviewMap();
}

/**
 * @brief 调整大小事件
 * @param event 调整大小事件指针
 */
void GraphicsViewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_enclibReady) {
        EnclViewSetSize(size().width(), size().height());
    }
    m_scene->setSceneRect(0, 0, width(), height());
    updateChart();
}

/**
 * @brief 鼠标按下事件
 * @param event 鼠标事件指针
 */
void GraphicsViewWidget::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        m_leftMousePressPt = event->pos();
        m_lastLeftMousePt = event->pos();
        break;
    default:
        break;
    }

    QGraphicsView::mousePressEvent(event);
}

/**
 * @brief 鼠标移动事件
 * @param event 鼠标事件指针
 */
void GraphicsViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentMousePt = event->pos();
    Qt::MouseButtons btns = event->buttons();
    if (m_enclibReady && (Qt::LeftButton == (btns & Qt::LeftButton))) {
        EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), currentMousePt.x(), currentMousePt.y());
        m_lastLeftMousePt = currentMousePt;
        updateMapBackground();
        update();
    }

    emit updateGeoPosition(currentMousePt);
    QGraphicsView::mouseMoveEvent(event);
}

/**
 * @brief 鼠标释放事件
 * @param event 鼠标事件指针
 */
void GraphicsViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        if (m_enclibReady) {
            EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), event->x(), event->y());
        }

        QPoint releasePos = event->pos();
        QPoint dragDelta = releasePos - m_leftMousePressPt;
        if (dragDelta.manhattanLength() < 5) {
            QPointF scenePos = mapToScene(releasePos);
            int clickedX = scenePos.x();
            int clickedY = scenePos.y();

            for (const PlatformData &platform : m_dynamicData.platforms.values()) {
                if (platform.isExpired()) {
                    continue;
                }
                int shipX, shipY;
                if (geoToScreen(platform.lon, platform.lat, shipX, shipY)) {
                    if (isPointInShip(clickedX, clickedY, shipX, shipY)) {
                        PropertyBox *existingBox = findPropertyBoxById(platform.id, platform.id == "SHIP_001");
                        if (existingBox) {
                            destroyPropertyBox(existingBox);
                        } else {
                            createPropertyBox(platform);
                        }
                        update();
                        return;
                    }
                }
            }

            for (const SpecialEvent &event : m_dynamicData.events) {
                if (event.targetId.isEmpty() && event.lon != 0 && event.lat != 0) {
                    if (isPointInEvent(clickedX, clickedY, event)) {
                        createEventInfoBox(event);
                        update();
                        return;
                    }
                }
            }

            for (auto &box : m_propertyBoxes) {
                if (box.label) {
                    box.label->hide();
                }
            }
            update();
        }

        break;
    }
    default:
        break;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief 滚轮缩放事件
 * @param event 滚轮事件指针
 */
void GraphicsViewWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_enclibReady) {
        QGraphicsView::wheelEvent(event);
        event->accept();
        return;
    }

    double zoomFactor = 1;
    if (event->delta() > 0)
        zoomFactor = 1 / 1.25;
    else
        zoomFactor = 1.25;
    EnclViewZoom(event->x(), event->y(), zoomFactor);
    updateMapBackground();
    update();
    emit updateGeoPosition(event->pos());

    QGraphicsView::wheelEvent(event);
    event->accept();
}

/**
 * @brief 更新海图
 */
void GraphicsViewWidget::updateChart()
{
    if (m_enclibReady) {
        EnclViewSetSize(width(), size().height());
        updateMapBackground();
    }
    update();
}

/**
 * @brief 放大视图
 */
void GraphicsViewWidget::zoomIn()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 0.8);
    updateChart();
}

/**
 * @brief 缩小视图
 */
void GraphicsViewWidget::zoomOut()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 1.25);
    updateChart();
}

/**
 * @brief 设置海图中心
 * @param lon 目标经度
 * @param lat 目标纬度
 */
void GraphicsViewWidget::setChartCenter(double lon, double lat)
{
    if (!m_enclibReady) return;
    EnclViewCenter(lon, lat);
    EnclViewSetScale(4000000);
    updateChart();
}

/**
 * @brief 更新全部显示状态
 * @param stateMap 平台显示状态映射表
 */
void GraphicsViewWidget::updateDisplayState(const DisplayStateMap &stateMap)
{
    m_displayStates = stateMap;
    updatePlatformItems();
    updateSensorWeaponRanges();
    updateTracks();
    updateEventMarkers();
    update();
}

/**
 * @brief 更新单个属性框的连接线
 * @param box 属性框引用
 */
void GraphicsViewWidget::updateConnectingLine(PropertyBox &box)
{
    if (!box.label || !box.label->isVisible()) {
        if (box.connectingLine) {
            m_scene->removeItem(box.connectingLine);
            delete box.connectingLine;
            box.connectingLine = nullptr;
        }
        return;
    }

    int shipX, shipY;
    if (!geoToScreen(box.lon, box.lat, shipX, shipY)) {
        if (box.connectingLine) {
            m_scene->removeItem(box.connectingLine);
            delete box.connectingLine;
            box.connectingLine = nullptr;
        }
        return;
    }

    QPoint shipCenter(shipX, shipY);
    QPoint boxCenter = box.label->geometry().center();
    boxCenter = mapFromGlobal(boxCenter);

    if (!box.connectingLine) {
        box.connectingLine = new QGraphicsLineItem(QLineF(shipCenter, boxCenter));
        QPen pen(Qt::white, 1, Qt::DashLine);
        box.connectingLine->setPen(pen);
        m_scene->addItem(box.connectingLine);
    } else {
        box.connectingLine->setLine(QLineF(shipCenter, boxCenter));
    }
}

/**
 * @brief 更新所有属性框的连接线
 */
void GraphicsViewWidget::updateAllConnectingLines()
{
    for (auto &box : m_propertyBoxes) {
        updateConnectingLine(box);
    }
}

/**
 * @brief 地理坐标转屏幕坐标
 * @param lon 经度
 * @param lat 纬度
 * @param x 输出屏幕 X 坐标
 * @param y 输出屏幕 Y 坐标
 * @return 转换成功返回 true，否则返回 false
 */
bool GraphicsViewWidget::geoToScreen(double lon, double lat, int &x, int &y)
{
    if (!m_enclibReady) {
        double scale = 500;
        x = (lon - 120.0) * scale + width() / 2;
        y = (31.5 - lat) * scale + height() / 2;
        return true;
    }

    if (!EnclTransformGeoToScrn(lon, lat, &x, &y)) {
        return false;
    }
    return true;
}

/**
 * @brief 通过标签指针查找属性框
 * @param label 标签指针
 * @return 找到返回属性框指针，否则返回 nullptr
 */
PropertyBox* GraphicsViewWidget::findPropertyBoxByLabel(QLabel *label)
{
    for (auto &box : m_propertyBoxes) {
        if (box.label == label) {
            return &box;
        }
    }
    return nullptr;
}

/**
 * @brief 通过平台 ID 查找属性框
 * @param id 平台 ID
 * @param isOwnShip 是否为己方舰船
 * @return 找到返回属性框指针，否则返回 nullptr
 */
PropertyBox* GraphicsViewWidget::findPropertyBoxById(const QString &id, bool isOwnShip)
{
    for (auto &box : m_propertyBoxes) {
        if (box.id == id && box.isOwnShip == isOwnShip) {
            return &box;
        }
    }
    return nullptr;
}

/**
 * @brief 检查点是否在平台图元内
 * @param x 屏幕 X 坐标
 * @param y 屏幕 Y 坐标
 * @param item 平台图元指针
 * @return 在图元内返回 true
 */
bool GraphicsViewWidget::isPointInPlatform(int x, int y, PlatformItem *item)
{
    QPointF itemPos = item->pos();
    QRectF boundingRect = item->boundingRect();
    QRectF itemRect(itemPos.x() + boundingRect.left(), itemPos.y() + boundingRect.top(),
                    boundingRect.width(), boundingRect.height());
    return itemRect.contains(x, y);
}

/**
 * @brief 检查点是否在舰船点击范围内
 * @param x 屏幕 X 坐标
 * @param y 屏幕 Y 坐标
 * @param shipX 舰船屏幕 X 坐标
 * @param shipY 舰船屏幕 Y 坐标
 * @return 在范围内返回 true
 */
bool GraphicsViewWidget::isPointInShip(int x, int y, int shipX, int shipY)
{
    int radius = 15;
    int dx = x - shipX;
    int dy = y - shipY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief 检查点是否在事件标记点击范围内
 * @param x 屏幕 X 坐标
 * @param y 屏幕 Y 坐标
 * @param event 事件数据
 * @return 在范围内返回 true
 */
bool GraphicsViewWidget::isPointInEvent(int x, int y, const SpecialEvent &event)
{
    int eventX, eventY;
    if (!geoToScreen(event.lon, event.lat, eventX, eventY)) {
        return false;
    }
    int radius = 15;
    int dx = x - eventX;
    int dy = y - eventY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief 创建平台属性信息框
 * @param platform 平台数据
 */
void GraphicsViewWidget::createPropertyBox(const PlatformData &platform)
{
    PropertyBox box;
    box.id = platform.id;
    box.name = platform.name;
    box.isOwnShip = (platform.id == "SHIP_001");
    box.lon = platform.lon;
    box.lat = platform.lat;
    box.heading = platform.heading;
    box.speed = platform.speed;
    box.isDragging = false;

    QLabel *label = new QLabel(this);
    label->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    label->setStyleSheet("background-color: white; border: 1px solid black; padding: 8px;");

    QString campStr = campName(platform.camp);

    QString eventsStr;
    const SpecialEvent *latestEvent = nullptr;
    qint64 latestTimestamp = 0;
    for (const auto &event : m_dynamicData.events) {
        if (event.targetId == platform.id && event.timestamp > latestTimestamp) {
            latestTimestamp = event.timestamp;
            latestEvent = &event;
        }
    }
    if (latestEvent) {
        eventsStr = QStringLiteral("\n事件: %1").arg(latestEvent->eventName);
    }

    label->setText(
        QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6°\n速度: %7 节%8")
            .arg(platform.name)
            .arg(platform.id)
            .arg(campStr)
            .arg(platform.lon, 0, 'f', 6)
            .arg(platform.lat, 0, 'f', 6)
            .arg(platform.heading > 0 ? platform.heading : 0)
            .arg(platform.speed)
            .arg(eventsStr)
    );
    label->adjustSize();
    label->installEventFilter(this);
    label->setMouseTracking(true);

    int shipX, shipY;
    if (geoToScreen(platform.lon, platform.lat, shipX, shipY)) {
        QPoint pos = mapToGlobal(QPoint(shipX, shipY));
        pos.setY(pos.y() - label->height() - 15);
        QRect screenGeometry = QApplication::desktop()->availableGeometry(this);
        if (pos.y() < screenGeometry.top())
            pos.setY(mapToGlobal(QPoint(shipX, shipY)).y() + 20);
        if (pos.x() + label->width() > screenGeometry.right())
            pos.setX(screenGeometry.right() - label->width());
        label->move(pos);
    }

    box.label = label;
    box.connectingLine = nullptr;
    m_propertyBoxes.append(box);
    label->show();
    updateConnectingLine(m_propertyBoxes.last());
}

/**
 * @brief 创建事件信息框
 * @param event 事件数据
 */
void GraphicsViewWidget::createEventInfoBox(const SpecialEvent &event)
{
    PropertyBox box;
    box.id = event.eventId;
    box.name = event.eventName;
    box.isOwnShip = false;
    box.lon = event.lon;
    box.lat = event.lat;
    box.heading = 0;
    box.speed = 0;
    box.isDragging = false;

    QLabel *label = new QLabel(this);
    label->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    label->setStyleSheet("background-color: #FFE4E1; border: 1px solid #DC143C; padding: 8px;");

    QString eventTypeStr;
    switch (event.eventType) {
    case Event_Alert: eventTypeStr = QStringLiteral("告警"); break;
    case Event_Attack: eventTypeStr = QStringLiteral("攻击"); break;
    case Event_Defense: eventTypeStr = QStringLiteral("防御"); break;
    case Event_Contact: eventTypeStr = QStringLiteral("接触"); break;
    case Event_Damage: eventTypeStr = QStringLiteral("损伤"); break;
    case Event_MissionStart: eventTypeStr = QStringLiteral("任务开始"); break;
    case Event_MissionEnd: eventTypeStr = QStringLiteral("任务结束"); break;
    case Event_Lost: eventTypeStr = QStringLiteral("失联"); break;
    case Event_Repair: eventTypeStr = QStringLiteral("修理"); break;
    case Event_Custom: eventTypeStr = QStringLiteral("自定义"); break;
    default: eventTypeStr = QStringLiteral("未知"); break;
    }

    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(event.timestamp);

    label->setText(
        QStringLiteral("事件\n名称: %1\n类型: %2\n编号: %3\n经度: %4\n纬度: %5\n时间: %6\n描述: %7")
            .arg(event.eventName)
            .arg(eventTypeStr)
            .arg(event.eventId)
            .arg(event.lon, 0, 'f', 6)
            .arg(event.lat, 0, 'f', 6)
            .arg(timestamp.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(event.description)
    );
    label->adjustSize();
    label->installEventFilter(this);
    label->setMouseTracking(true);

    int eventX, eventY;
    if (geoToScreen(event.lon, event.lat, eventX, eventY)) {
        QPoint pos = mapToGlobal(QPoint(eventX, eventY));
        pos.setY(pos.y() - label->height() - 15);
        QRect screenGeometry = QApplication::desktop()->availableGeometry(this);
        if (pos.y() < screenGeometry.top())
            pos.setY(mapToGlobal(QPoint(eventX, eventY)).y() + 20);
        if (pos.x() + label->width() > screenGeometry.right())
            pos.setX(screenGeometry.right() - label->width());
        label->move(pos);
    }

    box.label = label;
    box.connectingLine = nullptr;
    m_propertyBoxes.append(box);
    label->show();
    updateConnectingLine(m_propertyBoxes.last());
}

/**
 * @brief 销毁属性框
 * @param box 属性框指针
 */
void GraphicsViewWidget::destroyPropertyBox(PropertyBox *box)
{
    if (box && box->label) {
        box->label->hide();
        delete box->label;
        box->label = nullptr;
    }
    if (box && box->connectingLine) {
        m_scene->removeItem(box->connectingLine);
        delete box->connectingLine;
        box->connectingLine = nullptr;
    }
    m_propertyBoxes.removeOne(*box);
}

/**
 * @brief 事件过滤器，处理鹰眼图点击和属性框拖拽
 * @param obj 事件目标对象
 * @param event 事件指针
 * @return 处理了事件返回 true，否则返回 false
 */
bool GraphicsViewWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_overviewLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();

        double lon, lat;
        EnclEagleEyePixToGeo(pos.x(), pos.y(), lon, lat);
        EnclViewCenter(lon, lat);
        update();
        updateOverviewMap();
        return true;
    }

    for (auto &box : m_propertyBoxes) {
        if (obj == box.label) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                destroyPropertyBox(&box);
                update();
                return true;
            }
            if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    box.isDragging = true;
                    box.dragOffset = mouseEvent->globalPos() - box.label->pos();
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseMove) {
                if (box.isDragging) {
                    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                    QPoint newPos = mouseEvent->globalPos() - box.dragOffset;
                    QRect screenGeometry = QApplication::desktop()->availableGeometry(this);
                    if (newPos.x() < screenGeometry.left())
                        newPos.setX(screenGeometry.left());
                    if (newPos.y() < screenGeometry.top())
                        newPos.setY(screenGeometry.top());
                    if (newPos.x() + box.label->width() > screenGeometry.right())
                        newPos.setX(screenGeometry.right() - box.label->width());
                    if (newPos.y() + box.label->height() > screenGeometry.bottom())
                        newPos.setY(screenGeometry.bottom() - box.label->height());
                    box.label->move(newPos);
                    updateConnectingLine(box);
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseButtonRelease) {
                box.isDragging = false;
                return true;
            }
        }
    }
    return QGraphicsView::eventFilter(obj, event);
}

/**
 * @brief 初始化鹰眼概览图
 */
void GraphicsViewWidget::initOverviewMap()
{
    m_overviewLabel = new QLabel(this);
    m_overviewLabel->setFixedSize(200, 150);
    m_overviewLabel->move(10, 10);
    m_overviewLabel->setStyleSheet("background-color: rgba(200, 200, 200, 180); border: 1px solid gray;");
    m_overviewLabel->setScaledContents(true);
    m_overviewLabel->installEventFilter(this);
    m_overviewLabel->raise();
}

/**
 * @brief 更新鹰眼概览图显示
 */
void GraphicsViewWidget::updateOverviewMap()
{
    if (!m_overviewLabel) return;

    drawOverviewMapContent();
    m_overviewLabel->setPixmap(QPixmap::fromImage(m_overviewImage));
}

/**
 * @brief 绘制鹰眼概览图内容
 */
void GraphicsViewWidget::drawOverviewMapContent()
{
    int w = m_overviewLabel->width();
    int h = m_overviewLabel->height();
    m_overviewImage = QImage(w, h, QImage::Format_ARGB32);
    m_overviewImage.fill(QColor(220, 220, 220, 255));

    if (!m_enclibReady) {
        QPainter painter(&m_overviewImage);
        painter.setPen(Qt::gray);
        painter.drawText(m_overviewImage.rect(), Qt::AlignCenter, QStringLiteral("海图未就绪"));
        return;
    }

    unsigned char *pPixBuf = EnclEagleEyeGetImage(w, h);
    if (pPixBuf) {
        m_overviewImage = QImage(pPixBuf, w, h, QImage::Format_RGB32).copy();
    }

    QPainter overviewPainter(&m_overviewImage);

    double lon0, lat0;
    EnclTransformScrnToGeo(0, 0, &lon0, &lat0);
    int x0, y0;
    EnclEagleEyeGeoToPix(lon0, lat0, x0, y0);

    double lon1, lat1;
    EnclTransformScrnToGeo(width() - 1, height() - 1, &lon1, &lat1);
    int x1, y1;
    EnclEagleEyeGeoToPix(lon1, lat1, x1, y1);

    m_overviewViewport = QRect(QPoint(x0, y0), QPoint(x1, y1)).normalized();

    QPen pen(Qt::red, 2);
    overviewPainter.setPen(pen);
    overviewPainter.setBrush(Qt::NoBrush);
    overviewPainter.drawRect(m_overviewViewport);

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        int x, y;
        EnclEagleEyeGeoToPix(platform.lon, platform.lat, x, y);

        // 鹰眼图中平台点颜色与阵营颜色一致（我红敌蓝）
        QColor dotColor = campColor(platform.camp);

        QPen p(dotColor, 2);
        overviewPainter.setPen(p);
        overviewPainter.setBrush(dotColor);
        overviewPainter.drawEllipse(x - 3, y - 3, 6, 6);
    }
}