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
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QPen>
#include <QBrush>
#include <QFont>

#include "encl.h"
#include "common/logger.h"
#include "common/IconManager.h"
#include "datamanager.h"

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

    for (auto &items : m_statusStripItems) {
        for (auto item : items) {
            delete item;
        }
    }
    m_statusStripItems.clear();

    for (auto &box : m_propertyBoxes) {
        if (box.item) {
            m_scene->removeItem(box.item);
            delete box.item;
        }
        if (box.connectingLine) {
            m_scene->removeItem(box.connectingLine);
            delete box.connectingLine;
        }
    }
    m_propertyBoxes.clear();

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
        if (box && box->item && box->item->isVisible()) {
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

            box->item->setText(
                QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6°\n俯仰: %7°\n横滚: %8°\n速度: %9 节%10")
                    .arg(platform.name)
                    .arg(platform.id)
                    .arg(campStr)
                    .arg(platform.lon, 0, 'f', 6)
                    .arg(platform.lat, 0, 'f', 6)
                    .arg(platform.heading, 0, 'f', 8)
                    .arg(platform.pitch, 0, 'f', 8)
                    .arg(platform.roll, 0, 'f', 8)
                    .arg(platform.speed)
                    .arg(eventsStr)
            );
        }
    }

    updatePlatformItems();
    updateSensorWeaponRanges();
    updateTracks();
    updateEventMarkers();
    updateAllConnectingLines();
}

/**
 * @brief 事件新增时刷新海图事件标记
 * @param event 新增的事件
 * @details 全量 dynamicDataChanged 停发后，海图本地缓存 m_dynamicData.events
 *          不再被整体更新。此处先把新增事件按 eventId 去重并入本地事件列表
 *          （与 DataCache::addEvent 的去重逻辑保持一致），再调用
 *          updateEventMarkers() 重绘事件标记，使新事件图标立即显示。
 */
void GraphicsViewWidget::refreshEventMarkers(const SpecialEvent &event)
{
    // 按 eventId 去重并入本地事件列表：已存在则原地更新为最新，否则追加
    bool replaced = false;
    for (int i = 0; i < m_dynamicData.events.size(); ++i) {
        if (m_dynamicData.events[i].eventId == event.eventId) {
            m_dynamicData.events[i] = event;
            replaced = true;
            break;
        }
    }
    if (!replaced) {
        m_dynamicData.events.append(event);
    }

    updateEventMarkers();
}

/**
 * @brief 单平台增量更新（新增或更新）
 * @param platform 单个平台数据
 * @details 仅处理该 id 对应的图元：已过期则转为删除；否则复用或新建图元并
 *          更新位置/数据/显示状态。同时把该平台写回 m_dynamicData，保证鼠标
 *          命中测试、传感器/武器范围、航迹、属性框等依赖全量数据的逻辑仍可用。
 */
void GraphicsViewWidget::updateSinglePlatform(const PlatformData &platform)
{
    // 过期平台按删除处理，避免图上残留
    if (platform.isExpired()) {
        removeSinglePlatform(platform.id);
        return;
    }

    // 同步维护本地全量数据（供命中测试/控制面板等使用）
    m_dynamicData.platforms[platform.id] = platform;

    int x, y;
    if (!geoToScreen(platform.lon, platform.lat, x, y)) {
        return;
    }

    // 复用或新建该 id 的平台图元
    PlatformItem *item = m_platformItems.value(platform.id, nullptr);
    if (!item) {
        item = new PlatformItem(platform);
        item->setPos(x, y);
        m_scene->addItem(item);
        m_platformItems[platform.id] = item;
    } else {
        item->updateData(platform);
        item->setPos(x, y);
    }

    // 应用该平台的显示状态
    auto stateIt = m_displayStates.find(platform.id);
    if (stateIt != m_displayStates.end()) {
        item->updateDisplayState(stateIt.value());
    }
    item->setVisible(true);

    // 回写对应属性框的地理坐标/航向/速度，并刷新文本，
    // 使附着框在 updateAllConnectingLines 中按新锚点跟随、显示最新数据
    PropertyBox *box = findPropertyBoxById(platform.id, platform.id == "SHIP_001");
    if (box && box->item) {
        box->lon = platform.lon;
        box->lat = platform.lat;
        box->heading = platform.heading;
        box->speed = platform.speed;

        if (box->item->isVisible()) {
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

            box->item->setText(
                QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6°\n俯仰: %7°\n横滚: %8°\n速度: %9 节%10")
                    .arg(platform.name)
                    .arg(platform.id)
                    .arg(campStr)
                    .arg(platform.lon, 0, 'f', 6)
                    .arg(platform.lat, 0, 'f', 6)
                    .arg(platform.heading, 0, 'f', 8)
                    .arg(platform.pitch, 0, 'f', 8)
                    .arg(platform.roll, 0, 'f', 8)
                    .arg(platform.speed)
                    .arg(eventsStr)
            );
        }
    }

    // 单平台差量刷新：仅处理本平台的传感器/武器范围、航迹、事件徽标条与连接线，
    // 不再调用 updateSensorWeaponRanges/updateTracks/updateEventMarkers/updateAllConnectingLines
    // 等全量遍历函数，避免每平台更新都触发 m_dynamicData.platforms.values() 全量深拷贝。
    // 事件标记的“方式1（跟随平台）”即设备状态徽标条，直接用本平台已算得的屏幕坐标更新。
    updateSingleSensorWeaponRanges(platform.id, platform);
    updateSingleTrack(platform.id, platform);
    updatePlatformStatusStrip(platform, x, y);
    updateSingleConnectingLine(platform.id, platform);
}

/**
 * @brief 单平台增量删除
 * @param id 被移除（删除或过期）平台的 ID
 * @details 删除该 id 的平台图元及其传感器/武器范围、航迹、属性框等附属元素，
 *          并从 m_dynamicData 中移除，避免残留。
 */
void GraphicsViewWidget::removeSinglePlatform(const QString &id)
{
    // 从本地全量数据移除
    m_dynamicData.platforms.remove(id);

    // 删除平台图元
    if (m_platformItems.contains(id)) {
        PlatformItem *item = m_platformItems.take(id);
        if (item) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    // 删除传感器作用范围图元
    if (m_sensorRangeItems.contains(id)) {
        for (auto rItem : m_sensorRangeItems[id]) {
            m_scene->removeItem(rItem);
            delete rItem;
        }
        m_sensorRangeItems.remove(id);
    }

    // 删除武器作用范围图元
    if (m_weaponRangeItems.contains(id)) {
        for (auto rItem : m_weaponRangeItems[id]) {
            m_scene->removeItem(rItem);
            delete rItem;
        }
        m_weaponRangeItems.remove(id);
    }

    // 删除航迹图元
    if (m_trackItems.contains(id)) {
        QGraphicsItem *tItem = m_trackItems.take(id);
        if (tItem) {
            m_scene->removeItem(tItem);
            delete tItem;
        }
    }

    // 删除该平台的事件方块标记（方式1，按平台ID索引），避免残留
    if (m_eventMarkerItems.contains(id)) {
        for (auto item : m_eventMarkerItems.take(id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    // 删除该平台上方的设备状态徽标条，避免残留
    if (m_statusStripItems.contains(id)) {
        for (auto item : m_statusStripItems.take(id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    // 删除该平台对应的属性框（含连接线）
    PropertyBox *box = findPropertyBoxById(id, id == "SHIP_001");
    if (box) {
        destroyPropertyBox(box);
    }
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
        painter.drawText(img.rect(), Qt::AlignCenter, QStringLiteral("ZCHH海图未初始化"));
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
        // 委托单目标差量更新，航迹逻辑统一在 updateSingleTrack 维护，避免重复
        updateSingleTrack(platform.id, platform);
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
 * @details 统一处理两种事件呈现方式，均走同一套通信与容器 m_eventMarkerItems：
 *          - 方式1（跟随平台）：事件带 targetId，标记画在该平台当前位置，
 *            方形字母图标，随平台移动、随平台删除而清理，键为平台ID；
 *          - 方式2（地理标绘）：事件无 targetId，按事件自身 lon/lat 标绘，
 *            菱形标志 + 事件名文字，位置固定、不跟随平台，键为事件ID。
 *          末尾统一清理已不存在的标记（平台过期/事件移除）。
 */
void GraphicsViewWidget::updateEventMarkers()
{
    // 本轮仍有效的标记键集合（平台ID 或 事件ID），用于末尾清理
    QSet<QString> currentKeys;

    // ---- 方式1：带 targetId 的事件，作为“圆圈徽标”并入平台上方徽标条同一排 ----
    // 说明：事件不再单独画成右上方的方块，而是与传感器/武器/通信/受损等状态徽标
    //       统一由 updatePlatformStatusStrip 以圆圈形式在平台图标正上方横向一排显示。
    for (const PlatformData &platform : m_dynamicData.platforms.values()) {
        if (platform.isExpired()) {
            continue;
        }

        // 先计算平台屏幕坐标：徽标条以此为锚点
        int x, y;
        if (!geoToScreen(platform.lon, platform.lat, x, y)) {
            // 平台不在可视范围：隐藏其徽标条，避免残留错位
            if (m_statusStripItems.contains(platform.id)) {
                for (auto item : m_statusStripItems[platform.id]) {
                    item->setVisible(false);
                }
            }
            continue;
        }

        // 设备状态徽标条（横向一排圆圈：传感器/武器/通信/受损/事件），统一重建
        updatePlatformStatusStrip(platform, x, y);
    }

    // ---- 方式2：无 targetId 的事件，按事件自身经纬度地理标绘（菱形 + 文字） ----
    // 位置计算与图元更新统一委托 relocateGeoEventMarkers（含创建/复用/实时重定位），
    // 它返回本轮仍有效的方式2事件ID键，并入 currentKeys 供末尾清理，避免被误删。
    currentKeys += relocateGeoEventMarkers();

    // ---- 统一清理：删除已不存在的标记（平台过期 或 事件移除） ----
    QList<QString> toRemove;
    for (const QString &key : m_eventMarkerItems.keys()) {
        if (!currentKeys.contains(key)) {
            toRemove.append(key);
        }
    }
    for (const QString &key : toRemove) {
        for (auto item : m_eventMarkerItems.take(key)) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    // 同步清理已消失平台的设备状态徽标条（键为平台ID，故仅需比对平台键）
    QList<QString> stripToRemove;
    for (const QString &pid : m_statusStripItems.keys()) {
        if (!m_dynamicData.platforms.contains(pid)) {
            stripToRemove.append(pid);
        }
    }
    for (const QString &pid : stripToRemove) {
        for (auto item : m_statusStripItems.take(pid)) {
            m_scene->removeItem(item);
            delete item;
        }
    }
}

/**
 * @brief 重定位地理标绘事件标记（方式2）
 * @return 本轮仍有效的方式2事件ID集合（供 updateEventMarkers 末尾清理比对）
 * @details 仅处理无 targetId、按自身经纬度地理标绘的事件（菱形 + 文字）：
 *          按当前 Enclib 中心 geoToScreen 实时计算图元位置并复用/新建图元。
 *          海图平移（EnclViewPan）时调用，保证图标显示位置与鼠标命中测试
 *          （isPointInEvent 同样按实时 geoToScreen）一致。方式2事件数量极少，
 *          每帧调用开销可忽略。
 */
QSet<QString> GraphicsViewWidget::relocateGeoEventMarkers()
{
    QSet<QString> validKeys;
    for (const SpecialEvent &event : m_dynamicData.events) {
        // 仅处理不关联平台（targetId 为空）且有有效经纬度的事件
        if (!event.targetId.isEmpty()) {
            continue;
        }
        if (event.lon == 0 && event.lat == 0) {
            continue;
        }

        validKeys.insert(event.eventId);

        int x, y;
        if (!geoToScreen(event.lon, event.lat, x, y)) {
            continue;
        }

        QColor markerColor;
        switch (event.eventType) {
        case Event_Alert: markerColor = Qt::red; break;
        case Event_Attack: markerColor = Qt::darkRed; break;
        case Event_Defense: markerColor = Qt::blue; break;
        case Event_Contact: markerColor = Qt::cyan; break;
        case Event_Damage: markerColor = QColor(255, 165, 0); break;
        case Event_MissionStart: markerColor = Qt::green; break;
        case Event_MissionEnd: markerColor = Qt::gray; break;
        case Event_Lost: markerColor = Qt::magenta; break;
        case Event_Repair: markerColor = QColor(144, 238, 144); break;
        default: markerColor = Qt::yellow; break;
        }

        // 菱形顶点（以标绘点为中心，半径 10 像素的四个方向端点）
        const int r = 10;
        QPolygonF diamond;
        diamond << QPointF(x, y - r)   // 上
                << QPointF(x + r, y)   // 右
                << QPointF(x, y + r)   // 下
                << QPointF(x - r, y);  // 左

        if (m_eventMarkerItems.contains(event.eventId)) {
            QList<QGraphicsItem*> items = m_eventMarkerItems[event.eventId];
            if (items.size() >= 2) {
                QGraphicsPolygonItem *poly = static_cast<QGraphicsPolygonItem*>(items[0]);
                poly->setPolygon(diamond);
                poly->setPen(QPen(markerColor, 2));
                poly->setBrush(QBrush(markerColor, Qt::SolidPattern));
                poly->setVisible(true);

                QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(items[1]);
                text->setPlainText(event.eventName);
                text->setPos(x + r + 2, y - r);
                text->setVisible(true);
            }
        } else {
            QGraphicsPolygonItem *poly = new QGraphicsPolygonItem(diamond);
            poly->setPen(QPen(markerColor, 2));
            poly->setBrush(QBrush(markerColor, Qt::SolidPattern));
            poly->setZValue(60);   // 高于航迹/背景，便于醒目显示
            m_scene->addItem(poly);
            m_eventMarkerItems[event.eventId].append(poly);

            QGraphicsTextItem *text = new QGraphicsTextItem(event.eventName);
            QFont font("Arial", 9, QFont::Bold);
            text->setFont(font);
            text->setDefaultTextColor(markerColor);
            text->setPos(x + r + 2, y - r);
            text->setZValue(60);
            m_scene->addItem(text);
            m_eventMarkerItems[event.eventId].append(text);
        }
    }
    return validKeys;
}

/**
 * @brief 更新单个平台上方的设备状态徽标条（横向一排）
 * @param platform 平台数据
 * @param x 平台图标场景 X 坐标
 * @param y 平台图标场景 Y 坐标
 * @details 徽标数据来源：
 *          - 传感器/武器：复用 m_displayStates 中各 ComponentState.enabled，
 *            任一开启即视为该类设备工作中（S / W）；
 *          - 事件：取该平台最新一条事件，按事件类型映射为一个圆圈徽标（如 A/!/X/N…），
 *            受 showEvents 显示开关控制。
 *          所有徽标统一为“圆底 + 白色字母”，在平台图标正上方**同一排**水平居中排布。
 *          每拍先整条删除该平台旧徽标再重建，避免变长图元的索引复用问题。
 */
void GraphicsViewWidget::updatePlatformStatusStrip(const PlatformData &platform, int x, int y)
{
    // 整条重建：先清掉该平台旧徽标（1Hz 刷新，开销可忽略）
    if (m_statusStripItems.contains(platform.id)) {
        for (auto item : m_statusStripItems.take(platform.id)) {
            m_scene->removeItem(item);
            delete item;
        }
    }

    // 依据显示状态聚合“传感器开启 / 武器就绪”标志（复用控制面板写入的 enabled）
    bool sensorOn = false;
    bool weaponReady = false;
    if (m_displayStates.contains(platform.id)) {
        const PlatformDisplayState &st = m_displayStates[platform.id];
        for (const ComponentState &cs : st.sensors) {
            if (cs.enabled) { sensorOn = true; break; }
        }
        for (const ComponentState &cs : st.weapons) {
            if (cs.enabled) { weaponReady = true; break; }
        }
    }

    // 组装本轮要显示的徽标：{字母, 颜色}，按固定顺序排布（同一排圆圈）
    struct Badge { QString text; QColor color; };
    QList<Badge> badges;
    if (sensorOn)   badges.append({QStringLiteral("S"), Qt::cyan});               // S: 传感器开启
    if (weaponReady) badges.append({QStringLiteral("W"), QColor(255, 165, 0)});   // W: 武器就绪

    // 事件徽标：取该平台最新一条事件，按类型映射为一个圆圈徽标，并入同一排。
    // 受 showEvents 显示开关控制（未设置显示状态时默认显示）。
    bool showEvents = !m_displayStates.contains(platform.id) || m_displayStates[platform.id].showEvents;
    if (showEvents) {
        const SpecialEvent *latest = nullptr;
        qint64 latestTs = 0;
        for (const auto &event : m_dynamicData.events) {
            if (event.targetId == platform.id && event.timestamp > latestTs) {
                latestTs = event.timestamp;
                latest = &event;
            }
        }
        if (latest) {
            QColor evColor;
            QString evText;
            switch (latest->eventType) {
            case Event_Alert:        evColor = Qt::red;              evText = "!"; break;
            case Event_Attack:       evColor = Qt::darkRed;          evText = "A"; break;
            case Event_Defense:      evColor = Qt::blue;             evText = "D"; break;
            case Event_Contact:      evColor = Qt::cyan;             evText = "C"; break;
            case Event_Damage:       evColor = QColor(255, 69, 0);   evText = "X"; break;   // 受损
            case Event_MissionStart: evColor = Qt::green;            evText = "M"; break;
            case Event_MissionEnd:   evColor = Qt::gray;             evText = "E"; break;
            case Event_Lost:         evColor = Qt::magenta;          evText = "N"; break;   // 通信中断/失联
            case Event_Repair:       evColor = QColor(144, 238, 144);evText = "R"; break;
            default:                 evColor = Qt::yellow;           evText = "*"; break;
            }
            badges.append({evText, evColor});
        }
    }

    if (badges.isEmpty()) {
        return;   // 无任何状态则不绘制徽标条
    }

    // 布局：直径 14 的圆底，间距 4，整条在平台图标正上方水平居中
    const int d = 14;          // 徽标直径
    const int gap = 4;         // 徽标间距
    const int stripW = badges.size() * d + (badges.size() - 1) * gap;
    const int startX = x - stripW / 2;   // 整条水平居中于平台
    const int topY = y - 12 - d;         // 位于平台图标上方（贴近图标，减小间距）

    for (int i = 0; i < badges.size(); ++i) {
        const Badge &b = badges[i];
        int bx = startX + i * (d + gap);

        // 圆底：填充半透明底色 + 同色描边，便于在海图背景上辨识
        QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(bx, topY, d, d);
        QColor fill = b.color; fill.setAlpha(200);
        circle->setBrush(QBrush(fill));
        circle->setPen(QPen(b.color.darker(150), 1));
        circle->setZValue(61);   // 高于背景/航迹，确保徽标醒目
        m_scene->addItem(circle);
        m_statusStripItems[platform.id].append(circle);

        // 字母：白色加粗，按包围盒居中到圆内
        QGraphicsTextItem *text = new QGraphicsTextItem(b.text);
        QFont font("Arial", 8, QFont::Bold);
        text->setFont(font);
        text->setDefaultTextColor(Qt::white);
        QRectF tb = text->boundingRect();
        text->setPos(bx + d / 2.0 - tb.width() / 2.0, topY + d / 2.0 - tb.height() / 2.0);
        text->setZValue(62);
        m_scene->addItem(text);
        m_statusStripItems[platform.id].append(text);
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
    // 若有场景 item 正在被拖动（如属性框），则不平移海图，交由基类分发给 item
    if (m_scene && m_scene->mouseGrabberItem()) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    QPoint currentMousePt = event->pos();
    Qt::MouseButtons btns = event->buttons();
    if (m_enclibReady && (Qt::LeftButton == (btns & Qt::LeftButton))) {
        EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), currentMousePt.x(), currentMousePt.y());
        m_lastLeftMousePt = currentMousePt;
        updateMapBackground();
        // 拖动过程中不逐帧重定位事件标记（避免无谓开销），
        // 待 mouseReleaseEvent 拖动结束后一次性重定位，图标与命中测试保持一致
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
        // 若此次左键交互是在拖动某个场景 item（如属性框），则不触发海图平移与
        // 命中测试，直接交由基类把释放事件分发给该 item
        if (m_scene && m_scene->mouseGrabberItem()) {
            QGraphicsView::mouseReleaseEvent(event);
            return;
        }

        if (m_enclibReady) {
            EnclViewPan(m_lastLeftMousePt.x(), m_lastLeftMousePt.y(), event->x(), event->y());
            // 平移收尾后同样重定位地理标绘事件标记（方式2），确保位置与命中一致
            relocateGeoEventMarkers();
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
                // 平台图标被隐藏时（showShip=false），不参与命中测试，
                // 避免点击其原位置仍能选中不可见的平台。
                if (m_displayStates.contains(platform.id) &&
                    !m_displayStates[platform.id].showShip) {
                    continue;
                }
                int shipX, shipY;
                if (geoToScreen(platform.lon, platform.lat, shipX, shipY)) {
                    if (isPointInShip(clickedX, clickedY, shipX, shipY)) {
                        // 通知姿态面板（优先填空）
                        DataManager::instance()->notifyPlatformSelected(platform.id);
                        // 3D 包围盒（选中平台时显示）
                        if (m_selectedBox) { delete m_selectedBox; m_selectedBox = nullptr; }
                        PlatformItem *selItem = m_platformItems.value(platform.id, nullptr);
                        if (selItem && selItem->usePoseAtlas()) {
                            m_selectedBox = new BoundingBoxItem(selItem, 3, selItem);
                        }

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
                if (box.item) {
                    box.item->setVisible(false);
                }
            }
            // 点击空白：隐藏 3D 包围盒
            if (m_selectedBox) { delete m_selectedBox; m_selectedBox = nullptr; }
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
    // 缩放后海图中心/比例已变化：重定位地理标绘事件标记（方式2），
    // 使图标显示位置与鼠标命中测试（isPointInEvent 实时 geoToScreen）保持一致
    relocateGeoEventMarkers();
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
        // 缩放/平移后重算事件标记的屏幕位置（含跟随平台的方式1与地理标绘的方式2）
        updateEventMarkers();
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
    if (!box.item || !box.item->isVisible()) {
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

    // 场景坐标 ≡ 视图坐标：平台端直接用 geoToScreen 结果，框端用 item 中心的场景坐标
    QPointF shipCenter(shipX, shipY);
    QPointF boxCenter = box.item->mapToScene(box.item->boundingRect().center());

    if (!box.connectingLine) {
        box.connectingLine = new QGraphicsLineItem(QLineF(shipCenter, boxCenter));
        QPen pen(Qt::white, 1, Qt::DashLine);
        box.connectingLine->setPen(pen);
        box.connectingLine->setZValue(50);
        m_scene->addItem(box.connectingLine);
    } else {
        box.connectingLine->setLine(QLineF(shipCenter, boxCenter));
    }
}

/**
 * @brief 更新所有属性框的连接线
 * @details 对附着（attached）的属性框先按“平台锚点 + 偏移”重定位，再刷新连接线；
 *          脱离/固定的框保持用户放置位置不动，仅刷新其连接线（线仍连到平台锚点）。
 */
void GraphicsViewWidget::updateAllConnectingLines()
{
    for (auto &box : m_propertyBoxes) {
        if (box.item && box.item->isAttached()) {
            int shipX, shipY;
            if (geoToScreen(box.lon, box.lat, shipX, shipY)) {
                box.item->setPos(QPointF(shipX, shipY) + box.item->offset());
            }
        }
        updateConnectingLine(box);
    }
}

/**
 * @brief 单平台差量更新：传感器/武器作用范围
 * @param id 平台ID（用作 m_sensorRangeItems / m_weaponRangeItems 键）
 * @param platform 平台数据（提供 lon/lat/sensors/weapons）
 * @details 仅刷新该 id 平台的传感器/武器范围图元，复用或新建，不遍历其他平台。
 *          逻辑与 updateSensorWeaponRanges 的单平台分支一致，供 updateSinglePlatform
 *          调用以避免每平台更新都触发 m_dynamicData.platforms.values() 全量深拷贝。
 */
void GraphicsViewWidget::updateSingleSensorWeaponRanges(const QString &id, const PlatformData &platform)
{
    bool showSensors = m_displayStates.contains(id) && m_displayStates[id].showSensors;
    bool showWeapons = m_displayStates.contains(id) && m_displayStates[id].showWeapons;

    int x, y;
    if (!geoToScreen(platform.lon, platform.lat, x, y)) {
        if (m_sensorRangeItems.contains(id)) {
            for (auto item : m_sensorRangeItems[id]) {
                item->setVisible(false);
            }
        }
        if (m_weaponRangeItems.contains(id)) {
            for (auto item : m_weaponRangeItems[id]) {
                item->setVisible(false);
            }
        }
        return;
    }

    // ---- 传感器作用范围 ----
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

                if (m_sensorRangeItems.contains(id) &&
                    m_sensorRangeItems[id].size() > ellipseIdx) {
                    QGraphicsEllipseItem *ellipse =
                        dynamic_cast<QGraphicsEllipseItem*>(m_sensorRangeItems[id][ellipseIdx]);
                    if (ellipse) {
                        ellipse->setRect(x - radius, y - radius, radius * 2, radius * 2);
                        ellipse->setVisible(true);
                    }
                    if (m_sensorRangeItems[id].size() > textIdx) {
                        QGraphicsTextItem *text =
                            dynamic_cast<QGraphicsTextItem*>(m_sensorRangeItems[id][textIdx]);
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
                    m_sensorRangeItems[id].append(ellipse);

                    QGraphicsTextItem *text = new QGraphicsTextItem(QString("%1:S:%2(%3nm)").arg(id).arg(sensor.type).arg(sensor.range));
                    text->setPos(x + radius + 5, y + sensorIndex * 15);
                    text->setDefaultTextColor(sensorColor);
                    text->setFont(QFont("Arial", 7));
                    m_scene->addItem(text);
                    m_sensorRangeItems[id].append(text);
                }
                sensorIndex++;
            }
        }

        if (m_sensorRangeItems.contains(id)) {
            int expectedSize = platform.sensors.size() * 2;
            while (m_sensorRangeItems[id].size() > expectedSize) {
                auto item = m_sensorRangeItems[id].takeLast();
                m_scene->removeItem(item);
                delete item;
            }
        }
    } else if (m_sensorRangeItems.contains(id)) {
        for (auto item : m_sensorRangeItems[id]) {
            item->setVisible(false);
        }
    }

    // ---- 武器作用范围 ----
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

                if (m_weaponRangeItems.contains(id) &&
                    m_weaponRangeItems[id].size() > ellipseIdx) {
                    QGraphicsEllipseItem *ellipse =
                        dynamic_cast<QGraphicsEllipseItem*>(m_weaponRangeItems[id][ellipseIdx]);
                    if (ellipse) {
                        ellipse->setRect(x - radius, y - radius, radius * 2, radius * 2);
                        ellipse->setVisible(true);
                    }
                    if (m_weaponRangeItems[id].size() > textIdx) {
                        QGraphicsTextItem *text =
                            dynamic_cast<QGraphicsTextItem*>(m_weaponRangeItems[id][textIdx]);
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
                    m_weaponRangeItems[id].append(ellipse);

                    QGraphicsTextItem *text = new QGraphicsTextItem(QString("%1:W:%2(%3nm)").arg(id).arg(weapon.type).arg(weapon.range));
                    text->setPos(x + radius + 5, y + weaponIndex * 15);
                    text->setDefaultTextColor(weaponColor);
                    text->setFont(QFont("Arial", 7));
                    m_scene->addItem(text);
                    m_weaponRangeItems[id].append(text);
                }
                weaponIndex++;
            }
        }

        if (m_weaponRangeItems.contains(id)) {
            int expectedSize = platform.weapons.size() * 2;
            while (m_weaponRangeItems[id].size() > expectedSize) {
                auto item = m_weaponRangeItems[id].takeLast();
                m_scene->removeItem(item);
                delete item;
            }
        }
    } else if (m_weaponRangeItems.contains(id)) {
        for (auto item : m_weaponRangeItems[id]) {
            item->setVisible(false);
        }
    }
}

/**
 * @brief 单平台差量更新：航迹
 * @param id 平台ID（用作 m_trackItems 键）
 * @param platform 平台数据（提供 trackPoints / camp / lon / lat）
 * @details 仅用该 id 平台的 trackPoints 重建其航迹路径图元，不遍历其他平台。
 */
void GraphicsViewWidget::updateSingleTrack(const QString &id, const PlatformData &platform)
{
    bool showTrack = m_displayStates.contains(id) && m_displayStates[id].showTrack;

    if (m_trackItems.contains(id)) {
        m_trackItems[id]->setVisible(showTrack);
    }

    if (!showTrack || platform.trackPoints.isEmpty()) {
        return;
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

    if (m_trackItems.contains(id)) {
        QGraphicsPathItem *pathItem = static_cast<QGraphicsPathItem*>(m_trackItems[id]);
        pathItem->setPath(trackPath);
        pathItem->setPen(QPen(trackColor, 2, Qt::DashLine));
    } else {
        QGraphicsPathItem *pathItem = new QGraphicsPathItem(trackPath);
        pathItem->setPen(QPen(trackColor, 2, Qt::DashLine));
        m_scene->addItem(pathItem);
        m_trackItems[id] = pathItem;
    }
}

/**
 * @brief 单平台差量更新：属性框连接线
 * @param id 平台ID
 * @param platform 平台数据（提供 lon/lat，用于重定位附着属性框）
 * @details 仅查找并刷新该 id 平台对应的属性框（若存在）：先按新锚点重定位附着框，
 *          再更新其连接线。不遍历整个 m_propertyBoxes 列表。
 */
void GraphicsViewWidget::updateSingleConnectingLine(const QString &id, const PlatformData &platform)
{
    PropertyBox *box = findPropertyBoxById(id, id == "SHIP_001");
    if (!box) {
        return;
    }

    if (box->item && box->item->isAttached()) {
        int shipX, shipY;
        if (geoToScreen(platform.lon, platform.lat, shipX, shipY)) {
            box->item->setPos(QPointF(shipX, shipY) + box->item->offset());
        }
    }
    updateConnectingLine(*box);
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
 * @brief 通过图元指针查找属性框
 * @param item 属性框图元指针
 * @return 找到返回属性框指针，否则返回 nullptr
 */
PropertyBox* GraphicsViewWidget::findPropertyBoxByItem(PropertyBoxItem *item)
{
    for (auto &box : m_propertyBoxes) {
        if (box.item == item) {
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

    // 新建场景内属性框图元（平台框：白底黑框）
    PropertyBoxItem *item = new PropertyBoxItem(platform.id, false);
    item->setColors(Qt::white, Qt::black);

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

    item->setText(
        QStringLiteral("属性\n名称: %1\n编号: %2\n阵营: %3\n经度: %4\n纬度: %5\n航向: %6°\n俯仰: %7°\n横滚: %8°\n速度: %9 节%10")
            .arg(platform.name)
            .arg(platform.id)
            .arg(campStr)
            .arg(platform.lon, 0, 'f', 6)
            .arg(platform.lat, 0, 'f', 6)
            .arg(platform.heading, 0, 'f', 8)
            .arg(platform.pitch, 0, 'f', 8)
            .arg(platform.roll, 0, 'f', 8)
            .arg(platform.speed)
            .arg(eventsStr)
    );

    // 默认附着：初始偏移让框显示在平台上方
    qreal boxH = item->boundingRect().height();
    item->setOffset(QPointF(0, -(boxH + 15)));
    item->setAttached(true);
    item->setZValue(100);

    int shipX, shipY;
    if (geoToScreen(platform.lon, platform.lat, shipX, shipY)) {
        item->setPos(QPointF(shipX, shipY) + item->offset());
    }

    // 交互通过信号回到 view 处理（拖动脱离刷新连接线 / 双击删除）
    connect(item, &PropertyBoxItem::boxMoved, this, &GraphicsViewWidget::onPropertyBoxMoved);
    connect(item, &PropertyBoxItem::boxDoubleClicked, this, &GraphicsViewWidget::onPropertyBoxDoubleClicked);
    m_scene->addItem(item);

    box.item = item;
    box.connectingLine = nullptr;
    m_propertyBoxes.append(box);
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

    // 新建场景内事件信息框图元（事件框：#FFE4E1 底 / #DC143C 框）
    PropertyBoxItem *item = new PropertyBoxItem(event.eventId, true);
    item->setColors(QColor("#FFE4E1"), QColor("#DC143C"));

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

    item->setText(
        QStringLiteral("事件\n名称: %1\n类型: %2\n编号: %3\n经度: %4\n纬度: %5\n时间: %6\n描述: %7")
            .arg(event.eventName)
            .arg(eventTypeStr)
            .arg(event.eventId)
            .arg(event.lon, 0, 'f', 6)
            .arg(event.lat, 0, 'f', 6)
            .arg(timestamp.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(event.description)
    );

    // 默认附着：初始偏移让框显示在事件锚点上方
    qreal boxH = item->boundingRect().height();
    item->setOffset(QPointF(0, -(boxH + 15)));
    item->setAttached(true);
    item->setZValue(100);

    int eventX, eventY;
    if (geoToScreen(event.lon, event.lat, eventX, eventY)) {
        item->setPos(QPointF(eventX, eventY) + item->offset());
    }

    connect(item, &PropertyBoxItem::boxMoved, this, &GraphicsViewWidget::onPropertyBoxMoved);
    connect(item, &PropertyBoxItem::boxDoubleClicked, this, &GraphicsViewWidget::onPropertyBoxDoubleClicked);
    m_scene->addItem(item);

    box.item = item;
    box.connectingLine = nullptr;
    m_propertyBoxes.append(box);
    updateConnectingLine(m_propertyBoxes.last());
}

/**
 * @brief 销毁属性框
 * @param box 属性框指针
 */
void GraphicsViewWidget::destroyPropertyBox(PropertyBox *box)
{
    if (box && box->item) {
        m_scene->removeItem(box->item);
        // item 是 QObject，删除常由其自身信号（双击）触发，使用 deleteLater
        // 避免在其自身事件栈内析构导致崩溃
        box->item->deleteLater();
        box->item = nullptr;
    }
    if (box && box->connectingLine) {
        m_scene->removeItem(box->connectingLine);
        delete box->connectingLine;
        box->connectingLine = nullptr;
    }
    m_propertyBoxes.removeOne(*box);
}

/**
 * @brief 属性框被拖动的响应槽
 * @param item 被拖动的属性框图元
 * @details item 内部已置为脱离/固定，此处仅刷新其连接线。
 */
void GraphicsViewWidget::onPropertyBoxMoved(PropertyBoxItem *item)
{
    PropertyBox *box = findPropertyBoxByItem(item);
    if (box) {
        updateConnectingLine(*box);
    }
}

/**
 * @brief 属性框被双击的响应槽
 * @param item 被双击的属性框图元
 * @details 删除该属性框及其连接线（与原双击删除交互一致）。
 */
void GraphicsViewWidget::onPropertyBoxDoubleClicked(PropertyBoxItem *item)
{
    PropertyBox *box = findPropertyBoxByItem(item);
    if (box) {
        destroyPropertyBox(box);
        update();
    }
}

/**
 * @brief 事件过滤器，处理鹰眼图点击
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
        painter.drawText(m_overviewImage.rect(), Qt::AlignCenter, QStringLiteral("ZCHH海图未就绪"));
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