/**
 * @file WmsProvider.h
 * @brief GeoServer WMS 地图提供者
 * @details 按视口 BBOX 请求 WMS GetMap 整图，异步加载 + 防抖 + 缓存。
 * @date 2026-11-25
 */

#ifndef WMSPROVIDER_H
#define WMSPROVIDER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class WmsProvider : public QObject
{
    Q_OBJECT

public:
    explicit WmsProvider(QObject *parent = nullptr);
    ~WmsProvider();

    /** @brief 读取配置（URL、图层、防抖间隔） */
    void initialize();

    /**
     * @brief 请求当前视口地图（异步，防抖）
     * @param lonMin/latMin/lonMax/latMax 视口经纬度范围
     * @param w/h 请求图片尺寸（像素）
     */
    void requestMap(double lonMin, double latMin, double lonMax, double latMax, int w, int h);

    /** @brief 最后错误 */
    QString lastError() const { return m_lastError; }
    /** @brief 是否正在加载 */
    bool isLoading() const { return m_currentReply != nullptr; }

signals:
    /** @brief 地图图片加载完成 */
    void mapLoaded(const QImage &image, double lonMin, double latMin, double lonMax, double latMax);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onDebounceTimer();

private:
    QNetworkAccessManager m_manager;
    QTimer m_debounceTimer;
    QNetworkReply *m_currentReply = nullptr;

    QString m_url;          ///< WMS 服务地址（如 http://localhost:8082/geoserver/wms）
    QString m_layer;        ///< 图层名（如 world:world）
    int m_debounceMs = 200; ///< 防抖间隔
    QString m_lastError;
    QString m_lastUrl;

    // 待请求参数（防抖用）
    double m_pLonMin = 0, m_pLatMin = 0, m_pLonMax = 0, m_pLatMax = 0;
    int m_pW = 0, m_pH = 0;
};

#endif // WMSPROVIDER_H
