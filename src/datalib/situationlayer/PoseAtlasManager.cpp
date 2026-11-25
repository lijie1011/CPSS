/**
 * @file PoseAtlasManager.cpp
 * @brief 姿态图集管理器实现
 * @date 2026-11-25
 */

#include "PoseAtlasManager.h"
#include "logger.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <cmath>

PoseAtlasManager* PoseAtlasManager::s_instance = nullptr;

PoseAtlasManager::PoseAtlasManager(QObject *parent) : QObject(parent)
{
}

PoseAtlasManager* PoseAtlasManager::instance()
{
    if (!s_instance) s_instance = new PoseAtlasManager();
    return s_instance;
}

void PoseAtlasManager::initialize()
{
    const QString ini = QCoreApplication::applicationDirPath() + "/cpss.ini";
    QSettings settings(ini, QSettings::IniFormat);
    m_enabled = settings.value("Display/Use3DModel", true).toBool();

    m_atlasDir = settings.value("Display/AtlasDir", "3dParty/models/atlases").toString();
    QFileInfo fi(m_atlasDir);
    if (fi.isRelative()) {
        QDir exe(QCoreApplication::applicationDirPath());
        QString cand = exe.filePath(m_atlasDir);
        if (!QDir(cand).exists()) {
            QDir up = exe; up.cdUp(); up.cdUp(); up.cdUp();
            cand = up.filePath(m_atlasDir);
        }
        m_atlasDir = cand;
    }
    m_atlasDir = QDir::cleanPath(m_atlasDir);

    m_headingSteps = settings.value("Display/AtlasHeadingSteps", 36).toInt();
    m_rollSteps = settings.value("Display/AtlasRollSteps", 7).toInt();
    m_tileSize = settings.value("Display/AtlasTileSize", 128).toInt();
    m_iconSize = settings.value("Display/ModelIconSize", 64).toInt();

    // 类型 -> 模型（6种程序化模型）
    m_typeModels["aircraft"]  = settings.value("Display/ModelAircraft",  "fighter").toString();
    m_typeModels["surface"]   = settings.value("Display/ModelSurface",   "destroyer").toString();
    m_typeModels["submarine"] = settings.value("Display/ModelSubmarine", "carrier").toString();
    m_typeModels["land"]      = settings.value("Display/ModelLand",      "missile").toString();
    m_typeModels["space"]     = settings.value("Display/ModelSpace",     "recon").toString();

    Logger::info("[PoseAtlas] enabled=%d dir=%s models=%d",
                 (int)m_enabled, m_atlasDir.toUtf8().constData(), m_typeModels.size());
}

QString PoseAtlasManager::modelForType(const QString &type) const
{
    return m_typeModels.value(type, m_typeModels.value("aircraft"));
}

bool PoseAtlasManager::loadAtlas(const QString &key)
{
    if (m_atlases.contains(key)) return true;
    const QString path = m_atlasDir + "/" + key + ".png";
    QImage img(path);
    if (img.isNull()) {
        Logger::warn("[PoseAtlas] load failed: %s", path.toUtf8().constData());
        return false;
    }
    m_atlases.insert(key, img);
    return true;
}

QImage PoseAtlasManager::colorize(const QImage &gray, const QColor &campColor)
{
    // 灰度图 → 阵营色：保留原图明暗层次（灰度值决定亮度，阵营色决定色相）
    QImage result(gray.size(), QImage::Format_ARGB32);
    result.fill(Qt::transparent);
    const int cr = campColor.red(), cg = campColor.green(), cb = campColor.blue();
    for (int y = 0; y < gray.height(); ++y) {
        for (int x = 0; x < gray.width(); ++x) {
            const QRgb pix = gray.pixel(x, y);
            const int a = qAlpha(pix);
            if (a == 0) continue;
            // 原图灰度强度（0~255）→ 亮度系数（0.35~1.0，保留暗部可见）
            const int intensity = qGray(pix);
            const double k = 0.35 + 0.65 * (intensity / 255.0);
            const int r = qMin(255, (int)(cr * k));
            const int g = qMin(255, (int)(cg * k));
            const int b = qMin(255, (int)(cb * k));
            result.setPixel(x, y, qRgba(r, g, b, a));
        }
    }
    return result;
}

QString PoseAtlasManager::borderForCamp(const QColor &campColor) const
{
    // 按阵营色的红/蓝分量选边框版本（灰白阵营用 gray）
    const int r = campColor.red(), b = campColor.blue();
    if (r > b + 40) return "red";
    if (b > r + 40) return "blue";
    return "gray";
}

QImage PoseAtlasManager::poseImage(const QString &type, double heading, double roll, const QColor &campColor)
{
    if (!m_enabled) return QImage();
    const QString model = modelForType(type);
    if (model.isEmpty()) return QImage();
    const QString border = borderForCamp(campColor);
    const QString atlasKey = model + "_" + border;
    if (!loadAtlas(atlasKey)) return QImage();

    // 航向/横滚离散化
    int hi = (int)(std::fmod(heading, 360.0) / 360.0 * m_headingSteps) % m_headingSteps;
    if (hi < 0) hi += m_headingSteps;
    const double rollMin = -45.0, rollMax = 45.0;
    int ri = (int)((roll - rollMin) / (rollMax - rollMin) * (m_rollSteps - 1) + 0.5);
    if (ri < 0) ri = 0;
    if (ri >= m_rollSteps) ri = m_rollSteps - 1;

    // 缓存键（含阵营边框版本）
    const QString key = atlasKey + "|" + QString::number(hi) + "|" + QString::number(ri);
    if (m_colored.contains(key)) return m_colored.value(key);

    const QImage &atlas = m_atlases.value(atlasKey);
    const int tw = atlas.width() / m_headingSteps;
    const int th = atlas.height() / m_rollSteps;
    // 行 = roll 档（生成器: r=0 是 -45°, r=6 是 +45°）
    // 列 = heading 档（生成器: h=0 是 0°）
    QImage cell = atlas.copy(hi * tw, ri * th, tw, th);
    m_colored.insert(key, cell);
    return cell;
}

double PoseAtlasManager::simulateRoll(double curHeading, double prevHeading, double dtMs)
{
    if (dtMs <= 0) return 0.0;
    double diff = curHeading - prevHeading;
    while (diff > 180) diff -= 360;
    while (diff < -180) diff += 360;
    const double rate = diff / (dtMs / 1000.0);   // 度/秒
    const double roll = rate * 0.6;               // 系数可调
    return qBound(-45.0, roll, 45.0);
}
