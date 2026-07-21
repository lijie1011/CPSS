#ifndef CHARTBRIDGE_H
#define CHARTBRIDGE_H

#include <QObject>
#include <QJsonObject>
#include <QImage>

class ViewWidget;

class ChartBridge : public QObject
{
    Q_OBJECT

public:
    explicit ChartBridge(QObject *parent = nullptr);

    void setViewWidget(ViewWidget *view);
    QJsonObject handleRequest(const QString &action, const QJsonObject &data);

private:
    QJsonObject getChartImage();
    QJsonObject getDynamicObjects();
    QJsonObject zoomAt(const QJsonObject &data);
    QJsonObject panChart(const QJsonObject &data);
    QJsonObject setScale(const QJsonObject &data);
    QJsonObject setCenter(const QJsonObject &data);
    QJsonObject getGeoPosition(const QJsonObject &data);
    QJsonObject queryObjects(const QJsonObject &data);
    QJsonObject getChartList();
    QJsonObject setDisplayCategory(const QJsonObject &data);

    ViewWidget *m_view;
};

#endif
