/**
 * @file viewwidget.cpp
 * @brief 视图部件类实现
 * @details 该类是基于QWidget的海图显示组件，负责绘制平台、航迹、事件标记等动态元素。
 *          支持与Enclib海图库的集成，提供地图缩放、平移、鹰眼图等功能。
 * @date 2026-07-28
 */

#include "viewwidget.h"
#include <QPainter>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDateTime>
#include <cmath>
#include "encl.h"
#include "common/logger.h"

/**
 * @brief 构造函数
 * @param parent 父组件
 */
ViewWidget::ViewWidget(QWidget *parent)
    : QWidget(parent),
      m_enclibReady(false),
      m_overviewLabel(nullptr)
{
    setMouseTracking(true);
    initOverviewMap();
    loadIcons();
}

/**
 * @brief 加载平台图标资源
 */
void ViewWidget::loadIcons()
{
    QString resourcePath = QCoreApplication::applicationDirPath() + "/../resource";
    
    m_redBoatIcon = QImage(resourcePath + "/red/boat.png");
    m_redPlaneIcon = QImage(resourcePath + "/red/plane.png");
    m_purpleBoatIcon = QImage(resourcePath + "/purple/boat.png");
    m_purplePlaneIcon = QImage(resourcePath + "/purple/plane.png");
}

/**
 * @brief 析构函数
 */
ViewWidget::~ViewWidget()
{
    for (auto &box : m_propertyBoxes) {
        if (box.label) {
            delete box.label;
        }
    }
    delete m_overviewLabel;
}

/**
 * @brief 设置Enclib海图库就绪状态
 * @param ready 就绪标志
 */
void ViewWidget::setEnclibReady(bool ready)
{
    m_enclibReady = ready;
}

/**
 * @brief 更新动态数据
 * @param data 动态对象数据
 */
void ViewWidget::updateDynamicData(const DynamicObjects &data)
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
            box->speed = platform.speed;
            
            QString campStr;
            switch (platform.camp) {
            case Camp_Friendly: campStr = QStringLiteral("友军"); break;
            case Camp_Red: campStr = QStringLiteral("红方"); break;
            case Camp_Purple: campStr = QStringLiteral("紫方"); break;
            case Camp_Enemy: campStr = QStringLiteral("敌方"); break;
            case Camp_Neutral: campStr = QStringLiteral("中立"); break;
            default: campStr = QStringLiteral("未知"); break;
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
                eventsStr = QStringLiteral("\n事件: %1").arg(latestEvent->eventName);
            }
            
            box->label->setText(
                QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n速度: %6 节%7")
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

/**
 * @brief 绘制海图及所有动态元素
 * @param event 绘制事件
 */
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
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("海图未初始化"));
    }

    QPainterPath clipPath;
    clipPath.addRect(this->rect());
    painter.setClipPath(clipPath);

    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
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

/**
 * @brief 调整大小事件
 * @param event 调整大小事件
 */
void ViewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_enclibReady) {
        EnclViewSetSize(size().width(), size().height());
    }
    updateChart();
}

/**
 * @brief 鼠标按下事件，记录拖拽起点
 * @param event 鼠标事件
 */
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

/**
 * @brief 鼠标移动事件，支持拖拽平移海图
 * @param event 鼠标事件
 */
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

/**
 * @brief 鼠标释放事件，处理点击选择平台或事件
 * @param event 鼠标事件
 */
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

/**
 * @brief 滚轮缩放海图
 * @param event 滚轮事件
 */
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

/**
 * @brief 更新海图
 */
void ViewWidget::updateChart()
{
    if (m_enclibReady) {
        EnclViewSetSize(width(), height());
    }
    update();
}

/**
 * @brief 放大视图
 */
void ViewWidget::zoomIn()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 0.8);
    updateChart();
}

/**
 * @brief 缩小视图
 */
void ViewWidget::zoomOut()
{
    if (!m_enclibReady) return;
    double currentScale = EnclViewGetScale();
    EnclViewSetScale(currentScale * 1.25);
    updateChart();
}

/**
 * @brief 设置海图中心
 * @param lon 经度
 * @param lat 纬度
 */
void ViewWidget::setChartCenter(double lon, double lat)
{
    if (!m_enclibReady) return;
    EnclViewCenter(lon, lat);
    EnclViewSetScale(4000000);
    updateChart();
}

/**
 * @brief 更新显示状态
 * @param stateMap 显示状态映射
 */
void ViewWidget::updateDisplayState(const DisplayStateMap &stateMap)
{
    m_displayStates = stateMap;
    update();
}

/**
 * @brief 绘制平台
 * @param painter 绘制器
 * @param platform 平台数据
 */
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
        return;
    }

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

/**
 * @brief 绘制事件标记
 * @param painter 绘制器
 * @param x X坐标
 * @param y Y坐标
 * @param eventType 事件类型
 */
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

/**
 * @brief 绘制传感器范围
 * @param painter 绘制器
 * @param x X坐标
 * @param y Y坐标
 * @param platform 平台数据
 */
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

/**
 * @brief 绘制武器范围
 * @param painter 绘制器
 * @param x X坐标
 * @param y Y坐标
 * @param platform 平台数据
 */
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

/**
 * @brief 绘制独立事件
 * @param painter 绘制器
 * @param event 事件数据
 */
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

/**
 * @brief 检查点是否在事件范围内
 * @param x X坐标
 * @param y Y坐标
 * @param event 事件数据
 * @return 点在范围内返回true
 */
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

/**
 * @brief 创建事件信息框
 * @param event 事件数据
 */
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
    m_propertyBoxes.append(box);
    label->show();
}

/**
 * @brief 绘制连接线
 * @param painter 绘制器
 */
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

/**
 * @brief 地理坐标转屏幕坐标
 * @param lon 经度
 * @param lat 纬度
 * @param x 输出X坐标
 * @param y 输出Y坐标
 * @return 转换成功返回true
 */
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

/**
 * @brief 通过标签查找属性框
 * @param label 标签
 * @return 属性框指针
 */
PropertyBox* ViewWidget::findPropertyBoxByLabel(QLabel *label)
{
    for (auto &box : m_propertyBoxes) {
        if (box.label == label) {
            return &box;
        }
    }
    return nullptr;
}

/**
 * @brief 通过ID查找属性框
 * @param id 平台ID
 * @param isOwnShip 是否为己方舰船
 * @return 属性框指针
 */
PropertyBox* ViewWidget::findPropertyBoxById(const QString &id, bool isOwnShip)
{
    for (auto &box : m_propertyBoxes) {
        if (box.id == id && box.isOwnShip == isOwnShip) {
            return &box;
        }
    }
    return nullptr;
}

/**
 * @brief 检查点是否在舰船范围内
 * @param x X坐标
 * @param y Y坐标
 * @param shipX 舰船X坐标
 * @param shipY 舰船Y坐标
 * @return 点在范围内返回true
 */
bool ViewWidget::isPointInShip(int x, int y, int shipX, int shipY)
{
    int radius = 15;
    int dx = x - shipX;
    int dy = y - shipY;
    return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief 创建属性框
 * @param platform 平台数据
 */
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
    case Camp_Friendly: campStr = QStringLiteral("友军"); break;
    case Camp_Red: campStr = QStringLiteral("红方"); break;
    case Camp_Purple: campStr = QStringLiteral("紫方"); break;
    case Camp_Enemy: campStr = QStringLiteral("敌方"); break;
    case Camp_Neutral: campStr = QStringLiteral("中立"); break;
    default: campStr = QStringLiteral("未知"); break;
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
    m_propertyBoxes.append(box);
    label->show();
}

/**
 * @brief 销毁属性框
 * @param box 属性框指针
 */
void ViewWidget::destroyPropertyBox(PropertyBox *box)
{
    if (box && box->label) {
        box->label->hide();
        delete box->label;
        box->label = nullptr;
    }
    m_propertyBoxes.removeOne(*box);
}

/**
 * @brief 事件过滤器
 * @param obj 目标对象
 * @param event 事件
 * @return 处理了事件返回true
 */
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

/**
 * @brief 初始化鹰眼图
 */
void ViewWidget::initOverviewMap()
{
    m_overviewLabel = new QLabel(this);
    m_overviewLabel->setFixedSize(200, 150);
    m_overviewLabel->move(10, 10);
    m_overviewLabel->setStyleSheet("background-color: rgba(200, 200, 200, 180); border: 1px solid gray;");
    m_overviewLabel->setScaledContents(true);
    m_overviewLabel->installEventFilter(this);
}

/**
 * @brief 更新鹰眼图
 */
void ViewWidget::updateOverviewMap()
{
    if (!m_overviewLabel) return;

    drawOverviewMapContent();
    m_overviewLabel->setPixmap(QPixmap::fromImage(m_overviewImage));
}

/**
 * @brief 绘制鹰眼图内容
 */
void ViewWidget::drawOverviewMapContent()
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