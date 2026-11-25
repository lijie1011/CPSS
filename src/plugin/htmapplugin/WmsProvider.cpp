/**
 * @file WmsProvider.cpp
 * @brief GeoServer WMS 地图提供者实现
 * @date 2026-11-25
 */

#include "WmsProvider.h"
#include "logger.h"

#include <QSettings>
#include <QCoreApplication>
#include <QUrlQuery>

WmsProvider::WmsProvider(QObject *parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    connect(&m_debounceTimer, &QTimer::timeout, this, &WmsProvider::onDebounceTimer);
    connect(&m_manager, &QNetworkAccessManager::finished, this, &WmsProvider::onReplyFinished);
}

WmsProvider::~WmsProvider()
{
    if (m_currentReply) m_currentReply->abort();
}

void WmsProvider::initialize()
{
    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);
    m_url = settings.value("HtMap/GeoServerUrl",
                           "http://localhost:8082/geoserver/wms").toString();
    m_layer = settings.value("HtMap/Layer", "world:world").toString();
    m_debounceMs = settings.value("HtMap/WmsDebounceMs", 200).toInt();
    Logger::info("[WmsProvider] url=%s layer=%s debounce=%d",
                 m_url.toUtf8().constData(), m_layer.toUtf8().constData(), m_debounceMs);
}

void WmsProvider::requestMap(double lonMin, double latMin, double lonMax, double latMax, int w, int h)
{
    // 存参数，启动防抖定时器
    m_pLonMin = lonMin; m_pLatMin = latMin;
    m_pLonMax = lonMax; m_pLatMax = latMax;
    m_pW = w; m_pH = h;
    m_debounceTimer.start(m_debounceMs);
}

void WmsProvider::onDebounceTimer()
{
    // 取消上一个未完成的请求
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply = nullptr;
    }

    // 构造 WMS GetMap URL
    QUrl url(m_url);
    QUrlQuery q;
    q.addQueryItem("SERVICE", "WMS");
    q.addQueryItem("VERSION", "1.1.1");
    q.addQueryItem("REQUEST", "GetMap");
    q.addQueryItem("LAYERS", m_layer);
    q.addQueryItem("STYLES", "");
    q.addQueryItem("FORMAT", "image/png");
    q.addQueryItem("SRS", "EPSG:4326");
    q.addQueryItem("BBOX", QString("%1,%2,%3,%4")
                   .arg(m_pLonMin, 0, 'f', 6).arg(m_pLatMin, 0, 'f', 6)
                   .arg(m_pLonMax, 0, 'f', 6).arg(m_pLatMax, 0, 'f', 6));
    q.addQueryItem("WIDTH", QString::number(m_pW));
    q.addQueryItem("HEIGHT", QString::number(m_pH));
    url.setQuery(q);

    m_lastUrl = url.toString();
    Logger::info("[WmsProvider] GET %s", m_lastUrl.toUtf8().constData());
    m_currentReply = m_manager.get(QNetworkRequest(url));
}

void WmsProvider::onReplyFinished(QNetworkReply *reply)
{
    if (reply != m_currentReply) {
        reply->deleteLater();
        return;
    }
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        m_lastError = reply->errorString();
        Logger::warn("[WmsProvider] request failed: %s", m_lastError.toUtf8().constData());
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    QImage img;
    if (!img.loadFromData(data, "PNG")) {
        m_lastError = QStringLiteral("PNG 解析失败");
        Logger::warn("[WmsProvider] PNG decode failed, bytes=%d body=%s",
                     data.size(), QString::fromUtf8(data.left(300)).toUtf8().constData());
        return;
    }

    // 记录请求时的 BBOX（用当前存的参数，与图片对应）
    Logger::info("[WmsProvider] map loaded %dx%d bbox=%.2f,%.2f,%.2f,%.2f",
                 img.width(), img.height(), m_pLonMin, m_pLatMin, m_pLonMax, m_pLatMax);
    emit mapLoaded(img, m_pLonMin, m_pLatMin, m_pLonMax, m_pLatMax);
}
