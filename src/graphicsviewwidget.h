#ifndef GRAPHICSVIEWWIDGET_H
#define GRAPHICSVIEWWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QString>
#include <QSet>
#include <QLabel>
#include <QRect>
#include <QMap>

#include "dynamicdata.h"
#include "displaystate.h"
#include "platformitem.h"

struct PropertyBox {
    QString id;
    QString name;
    bool isOwnShip;
    double lon;
    double lat;
    double heading;
    double speed;
    QLabel *label;
    QGraphicsLineItem *connectingLine;
    bool isDragging;
    QPoint dragOffset;

    bool operator==(const PropertyBox &other) const {
        return id == other.id && isOwnShip == other.isOwnShip;
    }
};

class GraphicsViewWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsViewWidget(QWidget *parent = nullptr);
    ~GraphicsViewWidget();

    void updateDynamicData(const DynamicObjects &data);
    const DynamicObjects& getDynamicData() const { return m_dynamicData; }
    const DisplayStateMap& getDisplayStates() const { return m_displayStates; }
    void setEnclibReady(bool ready);
    bool isEnclibReady() const { return m_enclibReady; }

signals:
    void updateGeoPosition(QPoint pos);

public slots:
    void updateChart();
    void zoomIn();
    void zoomOut();
    void setChartCenter(double lon, double lat);
    void updateDisplayState(const DisplayStateMap &stateMap);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void updateMapBackground();
    void updatePlatformItems();
    void updateSensorWeaponRanges();
    void updateTracks();
    void updateEventMarkers();

    bool geoToScreen(double lon, double lat, int &x, int &y);
    void updateConnectingLine(PropertyBox &box);
    void updateAllConnectingLines();
    PropertyBox* findPropertyBoxByLabel(QLabel *label);
    PropertyBox* findPropertyBoxById(const QString &id, bool isOwnShip);
    bool isPointInPlatform(int x, int y, PlatformItem *item);
    bool isPointInShip(int x, int y, int shipX, int shipY);
    bool isPointInEvent(int x, int y, const SpecialEvent &event);
    void createPropertyBox(const PlatformData &platform);
    void createEventInfoBox(const SpecialEvent &event);
    void destroyPropertyBox(PropertyBox *box);
    void initOverviewMap();
    void updateOverviewMap();
    void drawOverviewMapContent();
    void loadIcons();
    void drawConnectingLines();

    QPoint m_leftMousePressPt;
    QPoint m_lastLeftMousePt;
    QImage m_storedViewImg;
    QImage m_overviewImage;
    double m_rotAngle;
    bool m_drawOthers;
    bool m_enclibReady;
    DynamicObjects m_dynamicData;
    DisplayStateMap m_displayStates;

    QGraphicsScene *m_scene;
    QMap<QString, PlatformItem*> m_platformItems;
    QMap<QString, QList<QGraphicsItem*>> m_sensorRangeItems;
    QMap<QString, QList<QGraphicsItem*>> m_weaponRangeItems;
    QMap<QString, QGraphicsItem*> m_trackItems;
    QMap<QString, QList<QGraphicsItem*>> m_eventMarkerItems;

    QList<PropertyBox> m_propertyBoxes;
    QLabel *m_overviewLabel;
    QRect m_overviewViewport;

    QImage m_redBoatIcon;
    QImage m_redPlaneIcon;
    QImage m_purpleBoatIcon;
    QImage m_purplePlaneIcon;
};

#endif
