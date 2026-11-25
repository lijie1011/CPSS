/**
 * @file TileProvider.h
 * @brief 瓦片提供者：本地离线（GWC 目录）/ GeoServer WMTS 双模式
 * @details 第一版实现本地离线加载。用目录扫描建索引（规避 GWC 块规则复杂性），
 *          QThreadPool 异步加载，LRU 缓存。
 * @date 2026-11-25
 */

#ifndef TILEPROVIDER_H
#define TILEPROVIDER_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QHash>
#include <QList>
#include <QSet>
#include <QMutex>
#include <QRunnable>
#include <QThreadPool>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

/**
 * @enum TileSourceMode
 * @brief 瓦片来源模式
 */
enum class TileSourceMode {
    Local = 0,    ///< 本地离线瓦片
    GeoServer = 1 ///< GeoServer WMTS 服务
};

/**
 * @struct TileKey
 * @brief 瓦片标识（层级 + 列 + 行）
 */
struct TileKey {
    int z = 0;
    int col = 0;
    int row = 0;

    bool operator==(const TileKey &o) const { return z==o.z && col==o.col && row==o.row; }
    quint64 hash() const {
        // z最多16, col/row最多65535 → 打包成 quint64
        return ((quint64)z << 48) | ((quint64)(col & 0xFFFF) << 16) | (quint64)(row & 0xFFFF);
    }
};

/**
 * @class TileLoadTask
 * @brief 异步加载任务
 */
class TileLoadTask : public QRunnable
{
public:
    TileLoadTask(class TileProvider *provider, const TileKey &key, const QString &path);
    void run() override;

private:
    TileProvider *m_provider;
    TileKey m_key;
    QString m_path;
};

/**
 * @class TileProvider
 * @brief 瓦片加载管理器（索引 + 异步加载 + LRU缓存）
 */
class TileProvider : public QObject
{
    Q_OBJECT

public:
    explicit TileProvider(QObject *parent = nullptr);
    ~TileProvider();

    /** @brief 初始化（读取配置，本地模式扫描目录建索引） */
    void initialize();

    /** @brief 请求加载瓦片（异步）；命中缓存则立即返回缓存图像 */
    QImage cachedTile(const TileKey &key);
    /** @brief 发起异步加载（未缓存时） */
    void requestTile(const TileKey &key);

    /** @brief 瓦片是否可用（索引中存在） */
    bool hasTile(const TileKey &key) const;
    /** @brief 获取本地瓦片文件路径（本地模式） */
    QString tilePath(const TileKey &key) const;

    /** @brief 当前模式 */
    TileSourceMode mode() const { return m_mode; }
    /** @brief 最大层级 */
    int maxZoom() const { return m_maxZoom; }
    /** @brief 层级是否有数据 */
    bool hasZoom(int z) const;
    /** @brief 取最接近的有效层级（缩放时用） */
    int nearestZoom(int z, bool higher) const;
    /** @brief 计算指定层级瓦片索引的中心经纬度（自动定位数据区域） */
    QPointF dataCenter(int z) const;

    /** @brief 缓存容量（瓦片数） */
    void setCacheCapacity(int n) { m_cacheCapacity = n; }

signals:
    /** @brief 瓦片加载完成（供 UI 重绘） */
    void tileLoaded(const TileKey &key, const QImage &image);
    /** @brief 内部：加载任务完成（工作线程发出，切主线程） */
    void tileLoadedInternal(const TileKey &key, const QImage &image);

public slots:
    /** @brief 加载任务完成回调（主线程） */
    void onTileLoaded(const TileKey &key, const QImage &image);

private slots:
    /** @brief WMTS 网络瓦片返回 */
    void onWmtsReplyFinished(QNetworkReply *reply);

private:
    void buildLocalIndex();
    void addToCache(const TileKey &key, const QImage &image);
    QString wmtsUrl(const TileKey &key) const;   ///< 构造 WMTS GetTile URL

    TileSourceMode m_mode = TileSourceMode::Local;
    QString m_localRoot;
    QString m_wmtsUrl;
    QString m_wmtsLayer = QStringLiteral("world_dom");
    QString m_gridSet = QStringLiteral("EPSG:4326");
    int m_maxZoom = 10;

    QList<QNetworkAccessManager*> m_netManagers;   ///< 多网络管理器轮询（提高并发）
    int m_nextManager = 0;
    QHash<QNetworkReply*, TileKey> m_netPending;   ///< 网络请求 -> 瓦片key

    // 索引：hash(key) -> 文件路径
    QHash<quint64, QString> m_index;
    QSet<int> m_availableZooms;   ///< 实际有数据的层级
    mutable QMutex m_indexMutex;

    // LRU 缓存：hash(key) -> image，及访问顺序
    struct CacheEntry { QImage image; quint64 lastUsed; };
    QHash<quint64, CacheEntry> m_cache;
    quint64 m_tick = 0;
    int m_cacheCapacity = 512;
    mutable QMutex m_cacheMutex;

    QThreadPool m_threadPool;
    QSet<quint64> m_pending;   ///< 正在加载中的瓦片，避免重复任务
    mutable QMutex m_pendingMutex;
};

#endif // TILEPROVIDER_H
