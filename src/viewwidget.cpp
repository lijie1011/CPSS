#include "viewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDateTime>
#include <cmath>
#include "encl.h"
#include "common/logger.h"

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget(parent),
      m_enclibReady(false),
      m_overviewLabel(nullptr)
{
    // Logger::info("ViewWidget constructor: entering");
    setMouseTracking(true);
    initOverviewMap();
    loadIcons();
    // Logger::info("ViewWidget constructor: done");
}

void ViewWidget::loadIcons()
{
    QString resourcePath = QCoreApplication::applicationDirPath() + "/../resource";
    
    m_redBoatIcon = QImage(resourcePath + "/red/boat.png");
    m_redPlaneIcon = QImage(resourcePath + "/red/plane.png");
    m_purpleBoatIcon = QImage(resourcePath + "/purple/boat.png");
    m_purplePlaneIcon = QImage(resourcePath + "/purple/plane.png");
    
    // Logger::info("Icons loaded: redBoat=%dx%d, redPlane=%dx%d, purpleBoat=%dx%d, purplePlane=%dx%d",
                 // m_redBoatIcon.width(), m_redBoatIcon.height(),
                 // m_redPlaneIcon.width(), m_redPlaneIcon.height(),
                 // m_purpleBoatIcon.width(), m_purpleBoatIcon.height(),
                 // m_purplePlaneIcon.width(), m_purplePlaneIcon.height());
}

ViewWidget::~ViewWidget()
{
    // Logger::info("ViewWidget destructor called");
    for (auto &box : m_propertyBoxes) {
        if (box.label) {
            delete box.label;
        }
    }
    delete m_overviewLabel;
}

void ViewWidget::setEnclibReady(bool ready)
{
    m_enclibReady = ready;
}

void ViewWidget::updateDynamicData(const DynamicObjects &data)
{
    m_dynamicData = data;
    // Logger::info("ViewWidget::updateDynamicData received, platforms: %d", data.platforms.size());
    for (const auto &platform : data.platforms.values()) {
        if (platform.isExpired()) {
            // Logger::info("  Platform: id=%s is expired, skipping", platform.id.toStdString().c_str());
            continue;
        }
        // Logger::info("  Platform: id=%s, lon=%f, lat=%f", platform.id.toStdString().c_str(), platform.lon, platform.lat);
        PropertyBox *box = findPropertyBoxById(platform.id, platform.id == "SHIP_001");
        if (box && box->label && box->label->isVisible() && !box->isDragging) {
            box->lon = platform.lon;
            box->lat = platform.lat;
            box->speed = platform.speed;
            
            QString campStr;
            switch (platform.camp) {
            case Camp_Friendly: campStr = "Friendly"; break;
            case Camp_Red: campStr = "Red"; break;
            case Camp_Purple: campStr = "Purple"; break;
            case Camp_Enemy: campStr = "Enemy"; break;
            case Camp_Neutral: campStr = "Neutral"; break;
            default: campStr = "Unknown"; break;
            }
            
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
                eventsStr = QString("\nEvent: %1").arg(latestEvent->eventName);
            }
            
            box->label->setText(
                QString("Property\nName: %1\nID: %2\nCamp: %3\nLongitude: %4\nLatitude: %5\nSpeed: %6 kn%7")
                    .arg(platform.name)
                    .arg(platform.id)
                    .arg(campStr)
                    .arg(platform.lon, 0, 'f', 6)
                    .arg(platform.lat, 0, 'f', 6)
                    .arg(platform.speed)
                    .arg(eventsStr)
            );
            box->label->adjustSize();
        }
    }
    update();
}

void ViewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);

    if (m_enclibReady) {
        unsigned char *pPixBuf = EnclDrawChart();
        if (pPixBuf) {
            QImage img(pPixBuf, this->width(), this->height(), QImage::Format_RGB32);
            m_storedViewImg = QImage(img.constBits(), img.width(), img.height(), img.bytesPerLine(), img.format()).copy();
            painter.drawImage(0, 0, m_storedViewImg);
        } else {
            painter.drawImage(0, 0, m_storedViewImg);
        }
    } else {
        painter.fillRect(this->rect(), Qt::darkGray);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("Map not initialized"));
    }

    QPainterPath clipPath;
    clipPath.addRect(this->rect());
    painter.setClipPath(clipPath);

    // Logger::info("paintEvent: drawing %d platforms, enclibReady=%d", m_dynamicData.platforms.size(), m_enclibReady);
    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            // Logger::info("paintEvent: platform %s is expired, skipping", platform.id.toStdString().c_str());
            continue;
        }
        drawPlatform(painter, platform);
    }

    for (const SpecialEvent &event : m_dynamicData.events) {
        if (event.targetId.isEmpty() && event.lon != 0 && event.lat != 0) {
            drawStandaloneEvent(painter, event);
        }
    }

    drawConnectingLines(painter);
    updateOverviewMap();
}

void ViewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_enclibReady) {
        EnclViewSetSize(size().width(), size().height());
    }
    updateChart();
}

void ViewWidget::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        m_leftMousePressPt = event->pos();
        m_lastLeftMousePt = event->pos();
        break;
    default:
        break;
    }
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentMousePt = event->pos();
    Qt::MouseButtons btns = event->buttons();
    if (m_enclibReady && (Qt::LeftButton == (btns & Qt::LeftButton))) {
        EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), currentMousePt.x(), currentMousePt.y());
        m_lastLeftMousePt = currentMousePt;
        update();
    }

    emit updateGeoPosition(currentMousePt);
}

void ViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        if (m_enclibReady) {
            EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), event->x(), event->y());
        }

        QPoint releasePos = event->pos();
        QPoint dragDelta = releasePos - m_leftMousePressPt;
        if (dragDelta.manhattanLength() < 5) {
            int clickedX = releasePos.x();
            int clickedY = releasePos.y();

            for (const PlatformData &platform : m_dynamicData.platforms.values()) {
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
}

void ViewWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_enclibReady) return;

    double zoomFactor = 1;
    if (event->delta() > 0)
        zoomFactor = 1 / 1.25;
    else
        zoomFactor = 1.25;
    EnclViewZoom(event->x(), event->y(), zoomFactor);
    update();
    emit updateGeoPosition(event->pos());
}

void ViewWidget::updateChart()
{
    if (m_enclibReady) {
        EnclViewSetSize(width(), height());
    }
    update();
}

void ViewWidget::zoomIn()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 0.8);
    updateChart();
}

void ViewWidget::zoomOut()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 1.25);
    updateChart();
}

void ViewWidget::setChartCenter(double lon, double lat)
{
    if (!m_enclibReady) return;
    EnclViewCenter(lon, lat);
    EnclViewSetScale(4000000);
    updateChart();
}

void ViewWidget::updateDisplayState(const DisplayStateMap &stateMap)
{
    m_displayStates = stateMap;
    update();
}

void ViewWidget::drawPlatform(QPainter &painter, const PlatformData &platform)
{
    auto stateIt = m_displayStates.find(platform.id);
    bool hasState = (stateIt != m_displayStates.end());
    PlatformDisplayState state;
    if (hasState) {
        state = stateIt.value();
    }

    int x, y;
    if (!geoToScreen(platform.lon, platform.lat, x, y)) {
        // Logger::info("drawPlatform: geoToScreen failed for %s (lon=%f, lat=%f)", platform.id.toStdString().c_str(), platform.lon, platform.lat);
        return;
    }
    // Logger::info("drawPlatform: drawing %s at screen (%d, %d)", platform.id.toStdString().c_str(), x, y);

    bool isOwnShip = (platform.id == "1");

    QColor campColor;
    switch (platform.camp) {
    case Camp_Friendly: campColor = Qt::green; break;
    case Camp_Red: campColor = Qt::red; break;
    case Camp_Purple: campColor = QColor(148, 0, 211); break;
    case Camp_Enemy: campColor = Qt::darkRed; break;
    case Camp_Neutral: campColor = Qt::yellow; break;
    default: campColor = Qt::gray; break;
    }

    if ((!hasState || state.showTrack) && !platform.trackPoints.isEmpty()) {
        painter.save();
        QPen trackPen(campColor, 2, Qt::DashLine);
        painter.setPen(trackPen);

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
        painter.drawPath(trackPath);
        painter.restore();
    }

    if (!hasState || state.showShip) {
        painter.save();
        painter.translate(x, y);
        
        if (platform.heading >= 0) {
            painter.rotate(platform.heading);
        }

        bool isAircraft = (platform.type == "aircraft");
        bool isRed = (platform.camp == Camp_Red);
        bool isPurple = (platform.camp == Camp_Purple);
        bool isMissile = (platform.type == "missile");

        QImage icon;
        if (!isMissile) {
            if (isRed && isAircraft) {
                icon = m_redPlaneIcon;
            } else if (isRed && !isAircraft) {
                icon = m_redBoatIcon;
            } else if (isPurple && isAircraft) {
                icon = m_purplePlaneIcon;
            } else if (isPurple && !isAircraft) {
                icon = m_purpleBoatIcon;
            }
        }

        if (!icon.isNull()) {
            int iconSize = 32;
            QImage scaledIcon = icon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter.drawImage(-iconSize/2, -iconSize/2, scaledIcon);
        } else if (isMissile) {
            QColor missileColor(255, 165, 0);
            QPen pen(missileColor, 2);
            painter.setPen(pen);
            painter.setBrush(missileColor);
            painter.drawEllipse(-5, -5, 10, 10);
            painter.setFont(QFont("Arial", 7));
            painter.setPen(Qt::white);
            painter.drawText(QRect(-15, -15, 30, 10), Qt::AlignCenter, "M");
        } else if (isOwnShip) {
            QPen pen(campColor, 2);
            painter.setPen(pen);

            QPolygonF shipShape;
            shipShape << QPointF(15, 0)
                      << QPointF(-10, -8)
                      << QPointF(-5, 0)
                      << QPointF(-10, 8);
            painter.drawPolygon(shipShape);
        } else {
            QPen pen(campColor, 2);
            painter.setPen(pen);
            painter.setBrush(campColor);
            painter.drawEllipse(-8, -8, 16, 16);
        }

        painter.restore();
    }

    if (!hasState || state.showName) {
        painter.setFont(QFont("Arial", 8));
        painter.setPen(Qt::white);
        painter.drawText(x + 18, y + 4, platform.id);
    }

    if (!hasState || state.showEvents) {
        const SpecialEvent *latestEvent = nullptr;
        qint64 latestTimestamp = 0;
        for (const auto &event : m_dynamicData.events) {
            if (event.targetId == platform.id && event.timestamp > latestTimestamp) {
                latestTimestamp = event.timestamp;
                latestEvent = &event;
            }
        }
        if (latestEvent) {
            drawEventMarker(painter, x, y, latestEvent->eventType);
        }
    }
    
    if (state.showSensors) {
        drawSensorRanges(painter, x, y, platform);
    }
    if (state.showWeapons) {
        drawWeaponRanges(painter, x, y, platform);
    }
}

void ViewWidget::drawEventMarker(QPainter &painter, int x, int y, SpecialEventType eventType)
{
    painter.save();
    painter.translate(x + 20, y - 15);

    QColor markerColor;
    QString iconText;
    
    switch (eventType) {
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

    QPen pen(markerColor, 2);
    painter.setPen(pen);
    painter.setBrush(QBrush(markerColor, Qt::Dense4Pattern));
    painter.drawRect(-8, -8, 16, 16);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(QRect(-8, -8, 16, 16), Qt::AlignCenter, iconText);

    painter.restore();
}

void ViewWidget::drawSensorRanges(QPainter &painter, int x, int y, const PlatformData &platform)
{
    if (platform.sensors.isEmpty()) {
        return;
    }
    
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
            
            QPen pen(sensorColor, 1, Qt::DashLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(sensorColor, Qt::NoBrush));
            painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2);
            
            painter.setFont(QFont("Arial", 7));
            painter.setPen(sensorColor);
            QString label = QString("%1:S:%2(%3nm)").arg(platform.id).arg(sensor.type).arg(sensor.range);
            painter.drawText(x + radius + 5, y, label);
        }
    }
}

void ViewWidget::drawWeaponRanges(QPainter &painter, int x, int y, const PlatformData &platform)
{
    if (platform.weapons.isEmpty()) {
        return;
    }
    
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
            
            QPen pen(weaponColor, 2, Qt::DotLine);
            painter.setPen(pen);
            painter.setBrush(QBrush(weaponColor, Qt::NoBrush));
            painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2);
            
            painter.setFont(QFont("Arial", 7));
            painter.setPen(weaponColor);
            QString label = QString("%1:W:%2(%3nm)").arg(platform.id).arg(weapon.type).arg(weapon.range);
            painter.drawText(x + radius + 5, y + 10, label);
        }
    }
}

void ViewWidget::drawStandaloneEvent(QPainter &painter, const SpecialEvent &event)
{
    int x, y;
    if (!geoToScreen(event.lon, event.lat, x, y)) {
        return;
    }

    painter.save();
    painter.translate(x, y);

    QColor markerColor = Qt::darkMagenta;
    QString iconText = "e";

    QPen pen(markerColor, 2);
    painter.setPen(pen);
    painter.setBrush(QBrush(markerColor, Qt::Dense4Pattern));
    painter.drawRect(-10, -10, 20, 20);
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(-10, -10, 20, 20), Qt::AlignCenter, iconText);

    painter.restore();
}

bool ViewWidget::isPointInEvent(int x, int y, const SpecialEvent &event)
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

void ViewWidget::createEventInfoBox(const SpecialEvent &event)
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
    case Event_Alert: eventTypeStr = "Alert"; break;
    case Event_Attack: eventTypeStr = "Attack"; break;
    case Event_Defense: eventTypeStr = "Defense"; break;
    case Event_Contact: eventTypeStr = "Contact"; break;
    case Event_Damage: eventTypeStr = "Damage"; break;
    case Event_MissionStart: eventTypeStr = "Mission Start"; break;
    case Event_MissionEnd: eventTypeStr = "Mission End"; break;
    case Event_Lost: eventTypeStr = "Lost"; break;
    case Event_Repair: eventTypeStr = "Repair"; break;
    case Event_Custom: eventTypeStr = "Custom"; break;
    default: eventTypeStr = "Unknown"; break;
    }

    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(event.timestamp);

    label->setText(
        QString("Event\nName: %1\nType: %2\nID: %3\nLongitude: %4\nLatitude: %5\nTime: %6\nDescription: %7")
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
    m_propertyBoxes.append(box);
    label->show();
}

void ViewWidget::drawConnectingLines(QPainter &painter)
{
    for (const PropertyBox &box : m_propertyBoxes) {
        if (box.label && box.label->isVisible()) {
            int shipX, shipY;
            if (geoToScreen(box.lon, box.lat, shipX, shipY)) {
                QPoint shipCenter(shipX, shipY);
                QPoint boxCenter = box.label->geometry().center();
                boxCenter = mapFromGlobal(boxCenter);

                QPen pen(Qt::white, 1, Qt::DashLine);
                painter.setPen(pen);
                painter.drawLine(shipCenter, boxCenter);
            }
        }
    }
}

bool ViewWidget::geoToScreen(double lon, double lat, int &x, int &y)
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

PropertyBox* ViewWidget::findPropertyBoxByLabel(QLabel *label)
{
    for (auto &box : m_propertyBoxes) {
        if (box.label == label) {
            return &box;
        }
    }
    return nullptr;
}

PropertyBox* ViewWidget::findPropertyBoxById(const QString &id, bool isOwnShip)
{
    for (auto &box : m_propertyBoxes) {
        if (box.id == id && box.isOwnShip == isOwnShip) {
            return &box;
        }
    }
    return nullptr;
}

bool ViewWidget::isPointInShip(int x, int y, int shipX, int shipY)
{
    int radius = 15;
    int dx = x - shipX;
    int dy = y - shipY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

void ViewWidget::createPropertyBox(const PlatformData &platform)
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
    
    QString campStr;
    switch (platform.camp) {
    case Camp_Friendly: campStr = "Friendly"; break;
    case Camp_Red: campStr = "Red"; break;
    case Camp_Purple: campStr = "Purple"; break;
    case Camp_Enemy: campStr = "Enemy"; break;
    case Camp_Neutral: campStr = "Neutral"; break;
    default: campStr = "Unknown"; break;
    }
    
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
        eventsStr = QString("\nEvent: %1").arg(latestEvent->eventName);
    }
    
    label->setText(
        QString("Property\nName: %1\nID: %2\nCamp: %3\nLongitude: %4\nLatitude: %5\nHeading: %6°\nSpeed: %7 kn%8")
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
    m_propertyBoxes.append(box);
    label->show();
}

void ViewWidget::destroyPropertyBox(PropertyBox *box)
{
    if (box && box->label) {
        box->label->hide();
        delete box->label;
        box->label = nullptr;
    }
    m_propertyBoxes.removeOne(*box);
}

bool ViewWidget::eventFilter(QObject *obj, QEvent *event)
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
                    update();
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseButtonRelease) {
                box.isDragging = false;
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ViewWidget::initOverviewMap()
{
    m_overviewLabel = new QLabel(this);
    m_overviewLabel->setFixedSize(200, 150);
    m_overviewLabel->move(10, 10);
    m_overviewLabel->setStyleSheet("background-color: rgba(200, 200, 200, 180); border: 1px solid gray;");
    m_overviewLabel->setScaledContents(true);
    m_overviewLabel->installEventFilter(this);
}

void ViewWidget::updateOverviewMap()
{
    if (!m_overviewLabel) return;

    drawOverviewMapContent();
    m_overviewLabel->setPixmap(QPixmap::fromImage(m_overviewImage));
}

void ViewWidget::drawOverviewMapContent()
{
    int w = m_overviewLabel->width();
    int h = m_overviewLabel->height();
    m_overviewImage = QImage(w, h, QImage::Format_ARGB32);
    m_overviewImage.fill(QColor(220, 220, 220, 255));

    if (!m_enclibReady) {
        QPainter painter(&m_overviewImage);
        painter.setPen(Qt::gray);
        painter.drawText(m_overviewImage.rect(), Qt::AlignCenter, "Map not ready");
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

        QColor campColor;
        switch (platform.camp) {
        case Camp_Friendly: campColor = Qt::green; break;
        case Camp_Red: campColor = Qt::red; break;
        case Camp_Purple: campColor = QColor(148, 0, 211); break;
        case Camp_Enemy: campColor = Qt::darkRed; break;
        case Camp_Neutral: campColor = Qt::yellow; break;
        default: campColor = Qt::gray; break;
        }

        QPen p(campColor, 2);
        overviewPainter.setPen(p);
        overviewPainter.setBrush(campColor);
        overviewPainter.drawEllipse(x - 3, y - 3, 6, 6);
    }
}

