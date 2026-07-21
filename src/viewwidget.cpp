﻿#include "viewwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QRect>
#include <QCoreApplication>

#include "encl.h"
#include "common/logger.h"

ViewWidget::ViewWidget(QWidget *parent)
    : QWidget(parent),
      m_draggingBox(nullptr),
      m_enclibReady(false)
{
    setMouseTracking(true);
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setEnclibReady(bool ready)
{
    m_enclibReady = ready;
    if (ready) {
        EnclViewSetScale(4000000);
    }
}

void ViewWidget::updateDynamicData(const DynamicObjects &data)
{
    m_dynamicData = data;
    static int updateCount = 0;
    if (updateCount++ % 5 == 0) {
        Logger::info("updateDynamicData: %d targets, %d sensors, %d weapons, %d markers, ownShip visible=%d", 
                     data.aisTargets.size(), data.sensors.size(), data.weapons.size(), data.markers.size(), data.ownShip.visible);
    }

    for (PropertyBox &box : m_propertyBoxes) {
        if (box.isOwnShip) {
            box.lon = data.ownShip.lon;
            box.lat = data.ownShip.lat;
            box.heading = data.ownShip.heading;
            box.speed = data.ownShip.speed;
            box.name = data.ownShip.name;
        } else {
            for (const AisTarget &target : data.aisTargets) {
                if (target.mmsi == box.id) {
                    box.lon = target.lon;
                    box.lat = target.lat;
                    box.heading = target.heading;
                    box.speed = target.speed;
                    box.name = target.name;
                    break;
                }
            }
        }
    }

    Logger::info("updateDynamicData: calling update(), size=%dx%d", width(), height());
    this->repaint();
}

bool ViewWidget::geoToScreen(double lon, double lat, int &x, int &y)
{
    if (lon < -180 || lon > 180 || lat < -90 || lat > 90) {
        Logger::info("geoToScreen: invalid coords lon=%f, lat=%f", lon, lat);
        return false;
    }

    if (m_enclibReady && EnclTransformGeoToScrn(lon, lat, &x, &y)) {
        Logger::info("geoToScreen: lon=%f, lat=%f -> x=%d, y=%d, enclib=%d", lon, lat, x, y, m_enclibReady);
        return true;
    }

    Logger::info("geoToScreen fallback: lon=%f, lat=%f, enclib=%d", lon, lat, m_enclibReady);

    double scale = 50000.0;
    x = (lon - 121.0) * scale + width() / 2;
    y = (31.5 - lat) * scale + height() / 2;
    return true;
}

void ViewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    static int paintCount = 0;
    if (paintCount++ % 60 == 0) {
        Logger::info("paintEvent called, width=%d, height=%d", width(), height());
    }

    QPainter painter(this);

    if (m_enclibReady) {
        unsigned char *pPixBuf = EnclDrawChart();
        if (pPixBuf) {
            QImage img = QImage(pPixBuf, this->width(), this->height(), QImage::Format_RGB32);
            painter.drawImage(0, 0, img);
            m_storedViewImg = img.copy();
        } else {
            painter.fillRect(this->rect(), Qt::black);
            if (!m_storedViewImg.isNull()) {
                painter.drawImage(0, 0, m_storedViewImg);
            }
        }
    } else {
        painter.fillRect(this->rect(), Qt::black);
        if (!m_storedViewImg.isNull()) {
            painter.drawImage(0, 0, m_storedViewImg);
        }
    }

    QPainterPath clipPath;
    clipPath.addRect(this->rect());
    painter.setClipPath(clipPath);

    drawSensors(painter);
    drawWeapons(painter);
    drawAisTargets(painter);
    drawMarkers(painter);

    if (m_dynamicData.ownShip.visible) {
        drawShip(painter, m_dynamicData.ownShip);
    }

    drawPropertyBoxes(painter);
}

void ViewWidget::drawShip(QPainter &painter, const ShipData &ship)
{
    int x, y;
    if (!geoToScreen(ship.lon, ship.lat, x, y)) {
        return;
    }

    bool isSelected = false;
    for (const PropertyBox &box : m_propertyBoxes) {
        if (box.isOwnShip) {
            isSelected = true;
            break;
        }
    }

    painter.save();
    painter.translate(x, y);
    painter.rotate(-ship.heading);

    QPen pen(Qt::white, 2);
    painter.setPen(pen);

    QPolygonF shipShape;
    shipShape << QPointF(15, 0)
              << QPointF(-10, -8)
              << QPointF(-5, 0)
              << QPointF(-10, 8);

    painter.drawPolygon(shipShape);

    if (isSelected) {
        QPen dashPen(Qt::white, 1, Qt::DashLine);
        painter.setPen(dashPen);
        painter.drawEllipse(-20, -20, 40, 40);
    }

    painter.restore();

    painter.setFont(QFont("Arial", 9));
    painter.setPen(Qt::white);
    painter.drawText(QRect(x + 18, y - 15, 80, 30), ship.name);
}

void ViewWidget::drawAisTargets(QPainter &painter)
{
    static int count = 0;
    if (count++ % 60 == 0) {
        Logger::info("drawAisTargets: %d targets, enclibReady=%d", m_dynamicData.aisTargets.size(), m_enclibReady);
    }
    
    for (const AisTarget &target : m_dynamicData.aisTargets) {
        if (!target.visible) {
            static int invisibleCount = 0;
            if (invisibleCount++ % 100 == 0) {
                Logger::info("drawAisTargets: target %s is not visible", target.mmsi.toStdString().c_str());
            }
            continue;
        }

        int x, y;
        if (!geoToScreen(target.lon, target.lat, x, y)) {
            Logger::info("drawAisTargets: geoToScreen failed for %s, lon=%f, lat=%f", 
                         target.mmsi.toStdString().c_str(), target.lon, target.lat);
            continue;
        }

        Logger::info("drawAisTargets: target %s at x=%d, y=%d, window=%dx%d", 
                     target.mmsi.toStdString().c_str(), x, y, width(), height());

        bool isSelected = false;
        for (const PropertyBox &box : m_propertyBoxes) {
            if (!box.isOwnShip && box.id == target.mmsi) {
                isSelected = true;
                break;
            }
        }

        QString iconPath;
        QString dir;

        if (target.shipType >= 4 && target.shipType <= 25) {
            dir = "red";
        } else if (target.shipType >= 26 && target.shipType <= 47) {
            dir = "purple";
        } else if (target.shipType >= 48 && target.shipType <= 69) {
            dir = "unknow";
        }

        QPixmap pixmap;
        if (!dir.isEmpty()) {
            QString resourcePath = QCoreApplication::applicationDirPath() + "/resource/" + dir + "/组 " + QString::number(target.shipType) + ".png";
            pixmap.load(resourcePath);
        }

        painter.save();
        painter.translate(x, y);
        painter.rotate(-target.heading);

        if (!pixmap.isNull()) {
            QPixmap scaled = pixmap.scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter.drawPixmap(-scaled.width()/2, -scaled.height()/2, scaled);
        } else {
            QColor color = target.isDanger ? Qt::red : Qt::yellow;
            QPen pen(color, 2);
            painter.setPen(pen);
            painter.setBrush(color);
            painter.drawEllipse(-8, -8, 16, 16);
        }

        if (isSelected) {
            QPen dashPen(Qt::white, 1, Qt::DashLine);
            painter.setPen(dashPen);
            painter.drawEllipse(-15, -15, 30, 30);
        }

        painter.restore();

        painter.setFont(QFont("Arial", 8));
        painter.drawText(x + 18, y + 4, target.mmsi);
    }
}

void ViewWidget::drawWeapons(QPainter &painter)
{
    for (const WeaponData &weapon : m_dynamicData.weapons) {
        if (!weapon.active) {
            continue;
        }

        int startX, startY, endX, endY;
        if (!geoToScreen(weapon.lon, weapon.lat, startX, startY)) {
            continue;
        }
        if (!geoToScreen(weapon.targetLon, weapon.targetLat, endX, endY)) {
            continue;
        }

        QPen pen(Qt::red, 3, Qt::DashLine);
        painter.setPen(pen);
        painter.drawLine(startX, startY, endX, endY);

        QBrush brush(Qt::red);
        painter.setBrush(brush);
        painter.drawEllipse(endX - 5, endY - 5, 10, 10);
    }
}

void ViewWidget::drawSensors(QPainter &painter)
{
    static int sensorCount = 0;
    if (sensorCount++ % 5 == 0) {
        Logger::info("drawSensors: %d sensors", m_dynamicData.sensors.size());
    }

    for (const SensorData &sensor : m_dynamicData.sensors) {
        if (!sensor.active) {
            continue;
        }

        int x, y;
        if (!geoToScreen(sensor.lon, sensor.lat, x, y)) {
            continue;
        }

        double radius = sensor.radius;

        int screenX1, screenY1;
        if (m_enclibReady && EnclTransformGeoToScrn(sensor.lon + radius / 111000.0, sensor.lat, &screenX1, &screenY1)) {
            // OK
        } else {
            double scale = 50000.0;
            screenX1 = (sensor.lon + radius / 111000.0 - 121.0) * scale + width() / 2;
            screenY1 = (31.5 - sensor.lat) * scale + height() / 2;
        }

        int screenRadius = abs(screenX1 - x);

        QPen pen(Qt::cyan, 1, Qt::DashLine);
        painter.setPen(pen);
        painter.drawEllipse(x - screenRadius, y - screenRadius,
                            screenRadius * 2, screenRadius * 2);
    }
}

void ViewWidget::drawMarkers(QPainter &painter)
{
    for (const UserMarker &marker : m_dynamicData.markers) {
        int x, y;
        if (!geoToScreen(marker.lon, marker.lat, x, y)) {
            continue;
        }

        QColor color(marker.color);
        QPen pen(color, 2);
        QBrush brush(color);

        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawEllipse(x - 5, y - 5, 10, 10);

        painter.setFont(QFont("Arial", 9));
        painter.setPen(Qt::white);
        painter.drawText(x + 8, y + 4, marker.label);
    }
}

void ViewWidget::drawPropertyBoxes(QPainter &painter)
{
    QFont font("Arial", 8);
    painter.setFont(font);

    for (PropertyBox &box : m_propertyBoxes) {
        int targetX, targetY;
        if (box.isOwnShip) {
            if (!geoToScreen(m_dynamicData.ownShip.lon, m_dynamicData.ownShip.lat, targetX, targetY)) {
                continue;
            }
        } else {
            bool found = false;
            for (const AisTarget &target : m_dynamicData.aisTargets) {
                if (target.mmsi == box.id) {
                    if (geoToScreen(target.lon, target.lat, targetX, targetY)) {
                        found = true;
                    }
                    break;
                }
            }
            if (!found) {
                continue;
            }
        }

        int boxX = targetX + 30 + box.offset.x();
        int boxY = targetY - 90 + box.offset.y();
        int boxWidth = 150;
        int boxHeight = 110;

        if (boxX + boxWidth > width()) {
            boxX = width() - boxWidth - 10;
        }
        if (boxY < 10) {
            boxY = targetY + 40 + box.offset.y();
        }

        box.rect = QRect(boxX, boxY, boxWidth, boxHeight);

        QColor bgColor(255, 255, 255, 230);
        QBrush bgBrush(bgColor);
        QPen borderPen(Qt::black, 1);
        painter.setBrush(bgBrush);
        painter.setPen(borderPen);
        painter.drawRoundedRect(boxX, boxY, boxWidth, boxHeight, 5, 5);

        QPen textPen(Qt::black);
        painter.setPen(textPen);

        int textY = boxY + 18;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"名称: %1").arg(box.name));
        textY += 16;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"ID: %1").arg(box.id));
        textY += 16;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"经度: %1").arg(box.lon, 0, 'f', 5));
        textY += 16;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"纬度: %1").arg(box.lat, 0, 'f', 5));
        textY += 16;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"航向: %1").arg(box.heading, 0, 'f', 1));
        textY += 16;
        painter.drawText(boxX + 8, textY, QString::fromUtf8(u8"航速: %1 kn").arg(box.speed, 0, 'f', 1));

        QPen linePen(Qt::black, 1, Qt::DashLine);
        painter.setPen(linePen);
        painter.drawLine(targetX, targetY, boxX, boxY + boxHeight / 2);
    }
}

bool ViewWidget::isPointInShip(int clickX, int clickY, int shipX, int shipY)
{
    int dx = clickX - shipX;
    int dy = clickY - shipY;
    return (dx * dx + dy * dy) < (25 * 25);
}

void ViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_leftMousePressPt = event->pos();
        m_lastLeftMousePt = event->pos();

        m_draggingBox = findPropertyBoxAt(event->pos().x(), event->pos().y());
        if (m_draggingBox) {
            m_dragOffset = event->pos() - QPoint(m_draggingBox->rect.x(), m_draggingBox->rect.y());
            return;
        }

        int x = event->pos().x();
        int y = event->pos().y();

        if (m_dynamicData.ownShip.visible) {
            int shipX, shipY;
            if (geoToScreen(m_dynamicData.ownShip.lon, m_dynamicData.ownShip.lat, shipX, shipY)) {
                if (isPointInShip(x, y, shipX, shipY)) {
                    PropertyBox *existing = findPropertyBoxById(m_dynamicData.ownShip.mmsi, true);
                    if (existing) {
                        m_propertyBoxes.removeOne(*existing);
                    } else {
                        PropertyBox box;
                        box.id = m_dynamicData.ownShip.mmsi;
                        box.name = m_dynamicData.ownShip.name;
                        box.isOwnShip = true;
                        box.lon = m_dynamicData.ownShip.lon;
                        box.lat = m_dynamicData.ownShip.lat;
                        box.heading = m_dynamicData.ownShip.heading;
                        box.speed = m_dynamicData.ownShip.speed;
                        box.offset = QPoint(0, 0);
                        box.isDragging = false;
                        m_propertyBoxes.append(box);
                    }
                    update();
                    return;
                }
            }
        }

        for (const AisTarget &target : m_dynamicData.aisTargets) {
            if (!target.visible) {
                continue;
            }

            int targetX, targetY;
            if (geoToScreen(target.lon, target.lat, targetX, targetY)) {
                if (isPointInShip(x, y, targetX, targetY)) {
                    PropertyBox *existing = findPropertyBoxById(target.mmsi, false);
                    if (existing) {
                        m_propertyBoxes.removeOne(*existing);
                    } else {
                        PropertyBox box;
                        box.id = target.mmsi;
                        box.name = target.name;
                        box.isOwnShip = false;
                        box.lon = target.lon;
                        box.lat = target.lat;
                        box.heading = target.heading;
                        box.speed = target.speed;
                        box.offset = QPoint(0, 0);
                        box.isDragging = false;
                        m_propertyBoxes.append(box);
                    }
                    update();
                    return;
                }
            }
        }
    } else if (event->button() == Qt::RightButton) {
        m_leftMousePressPt = event->pos();
        m_lastLeftMousePt = event->pos();
    }
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_draggingBox) {
            m_draggingBox->offset += event->pos() - m_lastLeftMousePt;
            m_lastLeftMousePt = event->pos();
            update();
            return;
        }

        if (!m_enclibReady) return;

        QPoint delta = event->pos() - m_lastLeftMousePt;
        m_lastLeftMousePt = event->pos();

        double currentLon, currentLat;
        if (EnclViewGetCenter(&currentLon, &currentLat)) {
            int screenCenterX, screenCenterY;
            if (EnclTransformGeoToScrn(currentLon, currentLat, &screenCenterX, &screenCenterY)) {
                int newCenterX = screenCenterX - delta.x();
                int newCenterY = screenCenterY - delta.y();

                double newLon, newLat;
                if (EnclTransformScrnToGeo(newCenterX, newCenterY, &newLon, &newLat)) {
                    EnclViewSetCenter(newLon, newLat);
                    updateChart();
                }
            }
        }
    }
}

void ViewWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggingBox = nullptr;
}

void ViewWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    double currentScale = EnclViewGetScale();
    double scaleFactor = (numSteps > 0) ? 0.9 : 1.1;

    for (int i = 0; i < abs(numSteps); ++i) {
        currentScale *= scaleFactor;
    }

    EnclViewSetScale(currentScale);
    updateChart();
}

void ViewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateChart();
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
    EnclViewSetCenter(lon, lat);
    updateChart();
}

PropertyBox* ViewWidget::findPropertyBoxAt(int x, int y)
{
    for (PropertyBox &box : m_propertyBoxes) {
        if (box.rect.contains(x, y)) {
            return &box;
        }
    }
    return nullptr;
}

PropertyBox* ViewWidget::findPropertyBoxById(const QString &id, bool isOwnShip)
{
    for (PropertyBox &box : m_propertyBoxes) {
        if (box.id == id && box.isOwnShip == isOwnShip) {
            return &box;
        }
    }
    return nullptr;
}
