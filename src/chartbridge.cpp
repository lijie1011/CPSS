#include "chartbridge.h"
#include "viewwidget.h"
#include "encl.h"
#include <QPainter>
#include <QJsonArray>
#include <QBuffer>
#include <QDebug>
#include <QDateTime>

#include "common/logger.h"

ChartBridge::ChartBridge(QObject *parent)
    : QObject(parent),
      m_view(nullptr)
{
}

void ChartBridge::setViewWidget(ViewWidget *view)
{
    m_view = view;
}

QJsonObject ChartBridge::handleRequest(const QString &action, const QJsonObject &data)
{
    Logger::info("ChartBridge handleRequest: %s", action.toStdString().c_str());

    if (action == "getChartImage")    return getChartImage();
    if (action == "zoomAt")           return zoomAt(data);
    if (action == "panChart")         return panChart(data);
    if (action == "setScale")         return setScale(data);
    if (action == "setCenter")        return setCenter(data);
    if (action == "getGeoPosition")   return getGeoPosition(data);
    if (action == "queryObjects")     return queryObjects(data);
    if (action == "getChartList")     return getChartList();
    if (action == "setDisplayCategory") return setDisplayCategory(data);
    if (action == "getDynamicObjects") return getDynamicObjects();

    QJsonObject err;
    err["error"] = QString("Unknown action: %1").arg(action);
    Logger::error("Unknown action: %s", action.toStdString().c_str());
    return err;
}

QJsonObject ChartBridge::getChartImage()
{
    if (!m_view) {
        return QJsonObject{{"error", "view not set"}};
    }

    QImage img(m_view->size(), QImage::Format_RGB32);
    QPainter painter(&img);
    m_view->render(&painter);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "WEBP", 75);

    QJsonObject resp;
    resp["image"] = QString::fromLatin1(ba.toBase64());
    resp["format"] = "webp";
    resp["width"] = img.width();
    resp["height"] = img.height();

    return resp;
}

QJsonObject ChartBridge::getDynamicObjects()
{
    QJsonObject data;

    double lon = 121.5;
    double lat = 31.2;

    int scrnX = 500;
    int scrnY = 350;
    EnclTransformGeoToScrn(lon, lat, &scrnX, &scrnY);

    QJsonObject ship;
    ship["lon"] = lon;
    ship["lat"] = lat;
    ship["x"] = scrnX;
    ship["y"] = scrnY;
    ship["heading"] = 45.0;
    ship["speed"] = 12.5;

    data["ship"] = ship;
    data["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    data["aisTargets"] = QJsonArray();
    data["weapons"] = QJsonArray();
    data["sensors"] = QJsonArray();

    return data;
}

QJsonObject ChartBridge::zoomAt(const QJsonObject &data)
{
    int x = data.value("x").toInt();
    int y = data.value("y").toInt();
    double factor = data.value("factor").toDouble(1.0);

    double currentScale = EnclViewGetScale();
    double newScale = currentScale * factor;
    EnclViewSetScale(newScale);

    return QJsonObject{{"success", true}, {"scale", newScale}};
}

QJsonObject ChartBridge::panChart(const QJsonObject &data)
{
    int fromX = data.value("fromX").toInt();
    int fromY = data.value("fromY").toInt();
    int toX = data.value("toX").toInt();
    int toY = data.value("toY").toInt();

    EnclViewPan(fromX, fromY, toX, toY);

    return QJsonObject{{"success", true}};
}

QJsonObject ChartBridge::setScale(const QJsonObject &data)
{
    double scale = data.value("scale").toDouble();
    EnclViewSetScale(scale);
    return QJsonObject{{"success", true}};
}

QJsonObject ChartBridge::setCenter(const QJsonObject &data)
{
    double lon = data.value("lon").toDouble();
    double lat = data.value("lat").toDouble();
    EnclViewCenter(lon, lat);
    return QJsonObject{{"success", true}};
}

QJsonObject ChartBridge::getGeoPosition(const QJsonObject &data)
{
    int x = data.value("x").toInt();
    int y = data.value("y").toInt();

    double lon, lat;
    EnclTransformScrnToGeo(x, y, &lon, &lat);

    QJsonObject resp;
    resp["lon"] = lon;
    resp["lat"] = lat;
    return resp;
}

QJsonObject ChartBridge::queryObjects(const QJsonObject &data)
{
    int x = data.value("x").toInt();
    int y = data.value("y").toInt();
    int radius = data.value("radius").toInt(15);

    double lon, lat;
    EnclTransformScrnToGeo(x, y, &lon, &lat);

    int nCnt;
    EnclObjectPtr *pObjs = EnclQueryPickObjects(lon, lat, radius, &nCnt);

    QJsonArray objects;
    for (int i = 0; i < nCnt; ++i) {
        int len;
        char *pattrs = EnclQueryGetObjectAttributes(pObjs[i], &len);
        objects.append(QString::fromUtf8(pattrs));
        EnclQueryFreeAttributes(pattrs);
    }
    EnclQueryFreeObjectIDs(pObjs);

    QJsonObject resp;
    resp["objects"] = objects;
    resp["count"] = nCnt;
    return resp;
}

QJsonObject ChartBridge::getChartList()
{
    int count;
    char **pChartList = EnclQueryGetSENCList(&count, true);

    QJsonArray arr;
    for (int i = 0; i < count; ++i) {
        arr.append(QString(pChartList[i]));
    }
    EnclQueryFreeSENCList(pChartList, count);

    QJsonObject resp;
    resp["charts"] = arr;
    resp["count"] = count;
    return resp;
}

QJsonObject ChartBridge::setDisplayCategory(const QJsonObject &data)
{
    int category = data.value("category").toInt();
    EnclDrawSetDisplayCategory(static_cast<EnclDisplayCategory>(category));
    return QJsonObject{{"success", true}};
}
