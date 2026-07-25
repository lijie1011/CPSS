#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QImage>
#include <QString>
#include <QSet>
#include <QLabel>
#include <QRect>

#include "dynamicdata.h"

struct PropertyBox {
    QString id;
    QString name;
    bool isOwnShip;
    double lon;
    double lat;
    double heading;
    double speed;
    QLabel *label;
    bool isDragging;
    QPoint dragOffset;

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
    bool isEnclibReady() const { return m_enclibReady; }

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
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void drawPlatform(QPainter &painter, const PlatformData &platform);
    void drawEventMarker(QPainter &painter, int x, int y, SpecialEventType eventType);
    void drawStandaloneEvent(QPainter &painter, const SpecialEvent &event);
    void drawConnectingLines(QPainter &painter);
    bool geoToScreen(double lon, double lat, int &x, int &y);
    PropertyBox* findPropertyBoxByLabel(QLabel *label);
    PropertyBox* findPropertyBoxById(const QString &id, bool isOwnShip);
    bool isPointInShip(int x, int y, int shipX, int shipY);
    bool isPointInEvent(int x, int y, const SpecialEvent &event);
    void createPropertyBox(const PlatformData &platform);
    void createEventInfoBox(const SpecialEvent &event);
    void destroyPropertyBox(PropertyBox *box);
    void initOverviewMap();
    void updateOverviewMap();
    void drawOverviewMapContent();

    QPoint m_leftMousePressPt;
    QPoint m_lastLeftMousePt;
    QImage m_storedViewImg;
    QImage m_overviewImage;
    double m_rotAngle;
    bool   m_drawOthers;
    bool   m_enclibReady;
    DynamicObjects m_dynamicData;

    QList<PropertyBox> m_propertyBoxes;
    QLabel *m_overviewLabel;
    QRect m_overviewViewport;
};

#endif