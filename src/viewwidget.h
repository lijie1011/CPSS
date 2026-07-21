#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QImage>
#include <QString>
#include <QSet>

#include "dynamicdata.h"

struct PropertyBox {
    QString id;
    QString name;
    bool isOwnShip;
    double lon;
    double lat;
    double heading;
    double speed;
    QPoint offset;
    bool isDragging;
    QRect rect;

    bool operator==(const PropertyBox &other) const {
        return id == other.id && isOwnShip == other.isOwnShip;
    }
};

class ViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewWidget(QWidget *parent = nullptr);
    ~ViewWidget();

    void updateDynamicData(const DynamicObjects &data);
    const DynamicObjects& getDynamicData() const { return m_dynamicData; }
    void setEnclibReady(bool ready);

signals:
    void updateGeoPosition(QPoint pos);

public slots:
    void updateChart();
    void zoomIn();
    void zoomOut();
    void setChartCenter(double lon, double lat);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void drawShip(QPainter &painter, const ShipData &ship);
    void drawAisTargets(QPainter &painter);
    void drawWeapons(QPainter &painter);
    void drawSensors(QPainter &painter);
    void drawMarkers(QPainter &painter);
    void drawPropertyBoxes(QPainter &painter);
    bool geoToScreen(double lon, double lat, int &x, int &y);
    PropertyBox* findPropertyBoxAt(int x, int y);
    PropertyBox* findPropertyBoxById(const QString &id, bool isOwnShip);
    bool isPointInShip(int x, int y, int shipX, int shipY);

    QPoint m_leftMousePressPt;
    QPoint m_lastLeftMousePt;
    QImage m_storedViewImg;
    double m_rotAngle;
    bool   m_drawOthers;
    bool   m_enclibReady;
    DynamicObjects m_dynamicData;

    QList<PropertyBox> m_propertyBoxes;
    PropertyBox *m_draggingBox;
    QPoint m_dragOffset;
};

#endif
