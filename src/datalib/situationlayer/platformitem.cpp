/**
 * @file platformitem.cpp
 * @brief 平台图元类实现
 * @details 本类继承自QGraphicsItem，负责在QGraphicsScene中绘制单个平台
 *          （舰船、飞机等）。支持根据平台类型和阵营显示不同图标，
 *          同时处理航迹点和显示状态。
 */

#include "platformitem.h"
#include <QPainter>
#include <QFont>
#include <QPolygonF>
#include <cmath>

#include "IconManager.h"
#include "PoseAtlasManager.h"
#include "logger.h"
#include <QDateTime>

/**
 * @brief 构造函数
 * @param platform 平台数据
 * @param parent 父图元
 * @details 图标根据平台类型与阵营通过 IconManager 按需着色获取
 */
PlatformItem::PlatformItem(const PlatformData &platform, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_platform(platform)
{
    // 先确定模式（3D 模式不回退）
    m_usePoseAtlas = PoseAtlasManager::instance()->enabled();
    m_campColor = campColor(platform.camp);
    m_lastHeading = platform.heading;
    m_lastUpdateMs = QDateTime::currentMSecsSinceEpoch();

    // 再加载图标（3D 直接取姿态图，不会先显示 2D）
    updateIcon();
    updateBoundingRect();

    // 平面模式才旋转图元
    if (!m_usePoseAtlas && platform.heading >= 0) {
        setRotation(platform.heading);
    }
}

/**
 * @brief 更新图标
 * @details 根据平台类型（type）选择 JB_Souce 中对应图标模板，
 *          再按阵营颜色 campColor(camp) 着色。所有平台（含导弹）
 *          统一以图标绘制；若模板缺失则回退到默认舰船图标，
 *          确保海图上始终显示图标而非圆点。
 */
void PlatformItem::updateIcon()
{
    // 保留导弹标记（供边界矩形等使用），但不再据此跳过取图
    m_isMissile = (m_platform.type == "missile");

    // ===== 3D 姿态图集模式（不回退：失败则留空，日志警告）=====
    if (m_usePoseAtlas) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const double roll = PoseAtlasManager::simulateRoll(m_platform.heading, m_lastHeading, now - m_lastUpdateMs);
        m_currentRoll = roll;
        m_poseImage = PoseAtlasManager::instance()->poseImage(
            m_platform.type, m_platform.heading, roll, m_campColor);
        m_lastHeading = m_platform.heading;
        m_lastUpdateMs = now;
        if (!m_poseImage.isNull()) {
            const int sz = PoseAtlasManager::instance()->iconSize();
            m_scaledIcon = m_poseImage.scaled(sz, sz, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            m_scaledIcon = QImage();
        }
        return;   // 3D 模式绝不回退到 2D
    }

    // 按平台类型解析图标文件名，并用阵营颜色着色
    const QString iconFile = iconFileForType(m_platform.type);
    m_currentIcon = IconManager::instance().getIcon(iconFile, campColor(m_platform.camp));

    // 若图标加载失败（模板缺失），回退到默认舰船图标，避免画成圆点
    if (m_currentIcon.isNull()) {
        m_currentIcon = IconManager::instance().getIcon(QStringLiteral("驱逐舰.png"),
                                                        campColor(m_platform.camp));
    }

    if (!m_currentIcon.isNull()) {
        int iconSize = 32;
        m_scaledIcon = m_currentIcon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        m_scaledIcon = QImage();
    }
}

/**
 * @brief 更新边界矩形
 * @details 根据平台类型和图标尺寸计算图元的边界矩形
 */
void PlatformItem::updateBoundingRect()
{
    // 边界矩形需覆盖“旋转后的图标 + 旋转到任意方位的标签”，
    // 否则 Qt 只重绘矩形区域，旧位置的标签不会被擦除，出现标签滞留残影。
    // 用足够大的正方形（以原点为中心）保证任意航向下都完整覆盖。
    qreal iconReach = m_scaledIcon.isNull() ? 15.0
                                            : qMax(m_scaledIcon.width(), m_scaledIcon.height()) / 2.0;
    qreal labelReach = 45.0;   // 标签宽40、位于图标下方，反向旋转后最大半径约45
    qreal r = qMax(iconReach + 6.0, labelReach) + 4.0;
    // 3D 模式下为选中时的包围盒预留空间（包围盒半径约 iconSize*0.7）
    if (m_usePoseAtlas) r = qMax(r, PoseAtlasManager::instance()->iconSize() * 1.3);
    m_boundingRect = QRectF(-r, -r, 2 * r, 2 * r);
}

/**
 * @brief 更新平台数据
 * @param platform 新的平台数据
 */
void PlatformItem::updateData(const PlatformData &platform)
{
    const bool headingChanged = (m_platform.heading != platform.heading && platform.heading >= 0);
    const bool typeChanged = (m_platform.type != platform.type || m_platform.camp != platform.camp);
    if (!headingChanged && !typeChanged &&
        m_platform.lon == platform.lon && m_platform.lat == platform.lat) return;

    prepareGeometryChange();
    m_platform = platform;
    m_campColor = campColor(platform.camp);

    if (m_usePoseAtlas) {
        // 3D：重新取姿态图（图集已含航向）
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const double roll = PoseAtlasManager::simulateRoll(platform.heading, m_lastHeading, now - m_lastUpdateMs);
        m_currentRoll = roll;
        m_poseImage = PoseAtlasManager::instance()->poseImage(
            platform.type, platform.heading, roll, m_campColor);
        m_lastHeading = platform.heading;
        m_lastUpdateMs = now;
        if (!m_poseImage.isNull()) {
            m_scaledIcon = m_poseImage.scaled(PoseAtlasManager::instance()->iconSize(),
                                              PoseAtlasManager::instance()->iconSize(),
                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            m_scaledIcon = QImage();   // 3D 模式不回退
        }
        setRotation(0);   // 图集已含航向，不旋转图元
    } else {
        // 平面：旋转图元 + 军标图标
        if (headingChanged) setRotation(platform.heading);
        if (typeChanged) updateIcon();
    }
    updateBoundingRect();
    update();
}

/**
 * @brief 更新显示状态
 * @param state 显示状态
 */
void PlatformItem::updateDisplayState(const PlatformDisplayState &state)
{
    m_displayState = state;
    update();
}

/**
 * @brief 获取边界矩形
 * @return 图元的边界矩形
 */
QRectF PlatformItem::boundingRect() const
{
    return m_boundingRect;
}

/**
 * @brief 绘制平台图元（图标/形状及名称）
 * @param painter 绘制器
 * @param option 样式选项
 * @param widget 父界面
 */
void PlatformItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_displayState.showShip) {
        return;
    }

    // 统一以图标绘制所有平台（含导弹）；图标已在 updateIcon() 中
    // 按 type 取图并按阵营着色，且对缺失模板做了默认图标回退，
    // 因此正常情况下 m_scaledIcon 始终有效，不再出现圆点兜底。
    if (!m_scaledIcon.isNull()) {
        painter->drawImage(-m_scaledIcon.width()/2, -m_scaledIcon.height()/2, m_scaledIcon);
    } else {
        // 兜底：图标未匹配/加载失败时，用阵营色小圆圈占位（按用户要求用圈代替，而非方块）
        painter->setPen(QPen(m_campColor, 2));
        painter->setBrush(m_campColor);
        painter->drawEllipse(-6, -6, 12, 12);
    }

    if (m_displayState.showName) {
        painter->save();
        QFont font("Arial", 8);
        painter->setFont(font);
        painter->setPen(Qt::white);

        if (m_usePoseAtlas && !m_poseImage.isNull()) {
            // 3D 模式：图元不旋转，标签直接水平画在图标下方
            const qreal y = m_scaledIcon.height()/2.0 + 4.0;
            painter->drawText(QRectF(-24, y, 48, 15), Qt::AlignCenter, m_platform.id);
        } else {
            // 平面模式：图元已旋转 heading，抵消旋转让文字水平
            const qreal screenLabelY = m_scaledIcon.isNull()
                                       ? 12.0 : (m_scaledIcon.height() / 2.0 + 6.0);
            const qreal h = m_platform.heading >= 0 ? m_platform.heading : 0.0;
            const qreal rad = -h * 3.14159265358979323846 / 180.0;
            QPointF labelPos(-screenLabelY * std::sin(rad), screenLabelY * std::cos(rad));
            painter->translate(labelPos);
            painter->rotate(-h);
            painter->drawText(QRectF(-20, 0, 40, 15), Qt::AlignCenter, m_platform.id);
        }
        painter->restore();
    }
}
