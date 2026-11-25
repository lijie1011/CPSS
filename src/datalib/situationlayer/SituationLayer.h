/**
 * @file SituationLayer.h
 * @brief 公共态势层：目标图元 + 属性框 + 连接线 + 显示状态 + 数据接入
 * @details chart 与 htmapplugin 共用此层，保证两边行为完全一致。
 * @date 2026-11-25
 */

#ifndef SITUATIONLAYER_H
#define SITUATIONLAYER_H

#include "IMapCanvas.h"
#include "dynamicdata.h"
#include "platformdisplaystate.h"
#include <QObject>
#include <QHash>
#include <QPointF>

class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsPathItem;
class QGraphicsEllipseItem;
class PlatformItem;
class PropertyBoxItem;

/**
 * @struct PropertyBoxEntry
 * @brief 属性框及其连接线
 */
struct PropertyBoxEntry {
    PropertyBoxItem *box = nullptr;
    QGraphicsLineItem *line = nullptr;
};

class CPSS_DATA_API SituationLayer : public QObject
{
    Q_OBJECT

public:
    explicit SituationLayer(QGraphicsScene *scene, IMapCanvas *canvas, QObject *parent = nullptr);
    ~SituationLayer();

    /** @brief 底图坐标变化后重新定位所有图元与连接线 */
    void refreshPositions();

    /** @brief 视图点击（命中平台则弹/关属性框）；返回是否命中 */
    bool handleSceneClick(const QPointF &scenePos);

    /** @brief 隐藏所有属性框 */
    void hideAllPropertyBoxes();

    /** @brief 更新事件数据（属性框显示最新事件用） */
    void setEvents(const QList<SpecialEvent> &events);

    // ===== 显示状态（平台控制面板）=====
    void setDisplayStates(const DisplayStateMap &states);
    DisplayStateMap displayStates() const { return m_displayStates; }

    int platformCount() const { return m_platformItems.size(); }
    /** @brief 获取当前所有平台数据 */
    QList<PlatformData> platforms() const;

public slots:
    void onPlatformUpdated(const PlatformData &platform);
    void onPlatformRemoved(const QString &id);

private slots:
    void onPropertyBoxDoubleClicked(PropertyBoxItem *box);
    void onPropertyBoxMoved(PropertyBoxItem *box);

private:
    void createPropertyBox(const QString &id);
    void destroyPropertyBox(const QString &id);
    void updatePropertyBoxText(const QString &id);
    void updateConnectingLine(const QString &id);
    void applyDisplayState(const QString &id);
    void updateTrack(const QString &id);
    void updateRanges(const QString &id);
    QString campName(CampType camp) const;

    QGraphicsScene *m_scene = nullptr;
    IMapCanvas *m_canvas = nullptr;
    QHash<QString, PlatformItem*> m_platformItems;
    QHash<QString, PropertyBoxEntry> m_propertyBoxes;
    QHash<QString, QGraphicsPathItem*> m_trackItems;              ///< 航迹
    QHash<QString, QList<QGraphicsEllipseItem*>> m_rangeItems;    ///< 传感器/武器范围圆
    QList<SpecialEvent> m_events;
    DisplayStateMap m_displayStates;
};

#endif // SITUATIONLAYER_H
