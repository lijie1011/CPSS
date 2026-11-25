/**
 * @file TileProvider.cpp
 * @brief 瓦片提供者实现
 * @date 2026-11-25
 */

#include "TileProvider.h"
#include "logger.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QQueue>
#include <QMutexLocker>
#include <algorithm>

// ============ TileLoadTask ============
TileLoadTask::TileLoadTask(TileProvider *provider, const TileKey &key, const QString &path)
    : m_provider(provider), m_key(key), m_path(path)
{
}

void TileLoadTask::run()
{
    QImage img(m_path);
    if (!img.isNull()) {
        emit m_provider->tileLoadedInternal(m_key, img);   // QueuedConnection 切主线程
    }
}

// ============ TileProvider ============
TileProvider::TileProvider(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<TileKey>("TileKey");
    m_threadPool.setMaxThreadCount(4);
    connect(this, &TileProvider::tileLoadedInternal, this, &TileProvider::onTileLoaded, Qt::QueuedConnection);
}

TileProvider::~TileProvider()
{
    m_threadPool.waitForDone();
    qDeleteAll(m_netManagers);
    m_netManagers.clear();
}

void TileProvider::initialize()
{
    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);

    const QString mode = settings.value("HtMap/Mode", "Local").toString();
    m_mode = (mode.compare("GeoServer", Qt::CaseInsensitive) == 0)
             ? TileSourceMode::GeoServer : TileSourceMode::Local;

    m_localRoot = settings.value("HtMap/LocalRoot",
                                 "3dParty/mapdata").toString();
    // 相对路径解析：优先 exe 目录，找不到则上溯查找（bin/Release → src）
    if (QDir(m_localRoot).isRelative()) {
        QDir exeDir(QCoreApplication::applicationDirPath());
        QString candidate = exeDir.filePath(m_localRoot);
        if (!QDir(candidate).exists()) {
            // 上溯 3 级：bin/Release/../../.. -> src
            QDir up = exeDir;
            up.cdUp(); up.cdUp(); up.cdUp();   // Release -> bin -> build -> src
            candidate = up.filePath(m_localRoot);
        }
        m_localRoot = candidate;
    }
    m_localRoot = QDir::cleanPath(m_localRoot);

    m_wmtsUrl = settings.value("HtMap/GeoServerUrl",
                               "http://localhost:8082/geoserver/gwc/service/wmts").toString();
    m_wmtsLayer = settings.value("HtMap/WmtsLayer", "world_dom").toString();
    m_gridSet = settings.value("HtMap/GridSet", "EPSG:4326").toString();
    m_maxZoom = settings.value("HtMap/MaxZoom", 21).toInt();
    m_cacheCapacity = settings.value("HtMap/CacheSize", 512).toInt();

    // 创建 4 个网络管理器轮询（每个最多6并发 → 总计约24并发）
    for (int i = 0; i < 4; ++i) {
        QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
        connect(mgr, &QNetworkAccessManager::finished,
                this, &TileProvider::onWmtsReplyFinished);
        m_netManagers.append(mgr);
    }

    Logger::info("[TileProvider] mode=%s root=%s wmts=%s layer=%s maxZoom=%d cache=%d",
                 (m_mode == TileSourceMode::Local ? "Local" : "GeoServer"),
                 m_localRoot.toUtf8().constData(), m_wmtsUrl.toUtf8().constData(),
                 m_wmtsLayer.toUtf8().constData(), m_maxZoom, m_cacheCapacity);

    if (m_mode == TileSourceMode::Local) {
        buildLocalIndex();
    } else {
        Logger::info("[TileProvider] WMTS mode ready");
    }
}

void TileProvider::buildLocalIndex()
{
    QMutexLocker locker(&m_indexMutex);
    m_index.clear();
    m_availableZooms.clear();

    QDir root(m_localRoot);
    if (!root.exists()) {
        Logger::error("[TileProvider] tile root not exist: %s", m_localRoot.toUtf8().constData());
        return;
    }

    // 遍历所有 EPSG_4326_{z:02d} 目录
    const QStringList grids = root.entryList(QStringList() << "EPSG_4326_*", QDir::Dirs | QDir::NoDotAndDotDot);
    int count = 0;
    for (const QString &grid : grids) {
        // 解析层级：EPSG_4326_09 -> 9
        const QString zStr = grid.mid(QStringLiteral("EPSG_4326_").length());
        bool ok = false;
        int z = zStr.toInt(&ok);
        if (!ok || z < 0) continue;

        const QString gridPath = root.filePath(grid);
        // 只要该层级目录下有 PNG 就标记为有效层级
        QDirIterator chk(gridPath, QStringList() << "*.png", QDir::Files, QDirIterator::Subdirectories);
        if (chk.hasNext()) m_availableZooms.insert(z);
        // 遍历所有 PNG：目录 x_y / 文件 CCCC_RRRR.png
        QDirIterator it(gridPath, QStringList() << "*.png", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString path = it.next();
            const QFileInfo fi(path);
            const QString fileName = fi.completeBaseName();   // "0660_0332"
            const int sep = fileName.indexOf('_');
            if (sep <= 0) continue;
            bool cOk = false, rOk = false;
            const int col = fileName.left(sep).toInt(&cOk);
            const int row = fileName.mid(sep + 1).toInt(&rOk);
            if (!cOk || !rOk) continue;

            TileKey key;
            key.z = z; key.col = col; key.row = row;
            m_index.insert(key.hash(), path);
            ++count;
        }
    }
    Logger::info("[TileProvider] local index built: %d tiles", count);
}

bool TileProvider::hasZoom(int z) const
{
    QMutexLocker locker(&m_indexMutex);
    return m_availableZooms.contains(z);
}

int TileProvider::nearestZoom(int z, bool higher) const
{
    QMutexLocker locker(&m_indexMutex);
    if (m_availableZooms.isEmpty()) return z;
    if (m_availableZooms.contains(z)) return z;
    int best = -1;
    if (higher) {
        // 找 > z 的最小
        for (int i = z + 1; i <= m_maxZoom; ++i) {
            if (m_availableZooms.contains(i)) { best = i; break; }
        }
        if (best < 0) {
            for (int i = z - 1; i >= 0; --i) {
                if (m_availableZooms.contains(i)) { best = i; break; }
            }
        }
    } else {
        for (int i = z - 1; i >= 0; --i) {
            if (m_availableZooms.contains(i)) { best = i; break; }
        }
        if (best < 0) {
            for (int i = z + 1; i <= m_maxZoom; ++i) {
                if (m_availableZooms.contains(i)) { best = i; break; }
            }
        }
    }
    return best < 0 ? z : best;
}

QPointF TileProvider::dataCenter(int z) const
{
    QMutexLocker locker(&m_indexMutex);
    QList<int> cols, rows;
    for (auto it = m_index.constBegin(); it != m_index.constEnd(); ++it) {
        const quint64 h = it.key();
        const int iz = (int)(h >> 48);
        if (iz != z) continue;
        cols.append((int)((h >> 16) & 0xFFFF));
        rows.append((int)(h & 0xFFFF));
    }
    if (cols.isEmpty()) return QPointF(0, 0);
    std::sort(cols.begin(), cols.end());
    std::sort(rows.begin(), rows.end());
    const int col = cols[cols.size() / 2];
    const int row = rows[rows.size() / 2];
    const double n = (double)(1 << z);
    const double lon = col / (2.0 * n) * 360.0 - 180.0;
    const double lat = row / n * 180.0 - 90.0;   // TMS
    return QPointF(lon, lat);
}

bool TileProvider::hasTile(const TileKey &key) const
{
    Q_UNUSED(key);
    // WMTS 模式：瓦片由服务端生成，任意坐标都可用
    if (m_mode == TileSourceMode::GeoServer) return true;
    QMutexLocker locker(&m_indexMutex);
    return m_index.contains(key.hash());
}

QString TileProvider::tilePath(const TileKey &key) const
{
    QMutexLocker locker(&m_indexMutex);
    return m_index.value(key.hash());
}

QImage TileProvider::cachedTile(const TileKey &key)
{
    const quint64 h = key.hash();
    QMutexLocker locker(&m_cacheMutex);
    auto it = m_cache.find(h);
    if (it == m_cache.end()) return QImage();
    it->lastUsed = ++m_tick;   // 更新 LRU 时间
    return it->image;
}

void TileProvider::requestTile(const TileKey &key)
{
    const quint64 h = key.hash();
    // 已在缓存或正在加载 → 跳过
    {
        QMutexLocker locker(&m_cacheMutex);
        if (m_cache.contains(h)) return;
    }
    {
        QMutexLocker locker(&m_pendingMutex);
        if (m_pending.contains(h)) return;
        m_pending.insert(h);
    }

    // ===== WMTS 模式：HTTP 请求瓦片 =====
    if (m_mode == TileSourceMode::GeoServer) {
        const QString url = wmtsUrl(key);
        // 轮询网络管理器，提高并发
        QNetworkAccessManager *mgr = m_netManagers.isEmpty() ? nullptr
                                     : m_netManagers[m_nextManager % m_netManagers.size()];
        if (mgr) ++m_nextManager;
        if (!mgr) {
            QMutexLocker locker(&m_pendingMutex);
            m_pending.remove(h);
            return;
        }
        QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(url)));
        m_netPending.insert(reply, key);
        return;
    }

    // ===== Local 模式：本地文件异步读取 =====
    const QString path = tilePath(key);
    if (path.isEmpty()) {
        QMutexLocker locker(&m_pendingMutex);
        m_pending.remove(h);
        return;
    }

    TileLoadTask *task = new TileLoadTask(this, key, path);
    task->setAutoDelete(true);
    m_threadPool.start(task);
}

void TileProvider::onTileLoaded(const TileKey &key, const QImage &image)
{
    {
        QMutexLocker locker(&m_pendingMutex);
        m_pending.remove(key.hash());
    }
    if (!image.isNull()) {
        addToCache(key, image);
        emit tileLoaded(key, image);   // 通知 MapWidget 重绘
    }
}

QString TileProvider::wmtsUrl(const TileKey &key) const
{
    // WMTS GetTile：TILEMATRIX=GridSet:z, TILEROW/TILECOL
    // 注意 WMTS EPSG:4326 的 row 从北到南（与屏幕方向一致）
    return QString("%1?SERVICE=WMTS&REQUEST=GetTile&VERSION=1.0.0"
                   "&LAYER=%2&TILEMATRIXSET=%3&TILEMATRIX=%4:%5"
                   "&TILEROW=%6&TILECOL=%7&FORMAT=image/png")
           .arg(m_wmtsUrl, m_wmtsLayer, m_gridSet, m_gridSet)
           .arg(key.z).arg(key.row).arg(key.col);
}

void TileProvider::onWmtsReplyFinished(QNetworkReply *reply)
{
    if (!m_netPending.contains(reply)) {
        reply->deleteLater();
        return;
    }
    const TileKey key = m_netPending.take(reply);
    {
        QMutexLocker locker(&m_pendingMutex);
        m_pending.remove(key.hash());
    }

    if (reply->error() != QNetworkReply::NoError) {
        Logger::warn("[TileProvider] WMTS tile failed z=%d c=%d r=%d: %s",
                     key.z, key.col, key.row, reply->errorString().toUtf8().constData());
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    QImage img;
    if (!img.loadFromData(data, "PNG")) {
        Logger::warn("[TileProvider] WMTS tile PNG decode failed z=%d c=%d r=%d bytes=%d",
                     key.z, key.col, key.row, data.size());
        return;
    }
    addToCache(key, img);
    emit tileLoaded(key, img);
}

void TileProvider::addToCache(const TileKey &key, const QImage &image)
{
    QMutexLocker locker(&m_cacheMutex);
    const quint64 h = key.hash();
    CacheEntry e;
    e.image = image;
    e.lastUsed = ++m_tick;
    m_cache.insert(h, e);

    // 超出容量：淘汰最久未用的
    while (m_cache.size() > m_cacheCapacity) {
        quint64 oldestKey = 0;
        quint64 oldest = UINT64_MAX;
        for (auto it = m_cache.constBegin(); it != m_cache.constEnd(); ++it) {
            if (it->lastUsed < oldest) { oldest = it->lastUsed; oldestKey = it.key(); }
        }
        if (oldestKey == 0) break;
        m_cache.remove(oldestKey);
    }
}
