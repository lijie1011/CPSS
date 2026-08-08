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

#include "common/IconManager.h"

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
    updateIcon();
    updateBoundingRect();

    if (platform.heading >= 0) {
        setRotation(platform.heading - 90);
    }

    // 阵营颜色统一由公共映射 campColor() 提供（我红敌蓝）
    m_campColor = campColor(platform.camp);
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
    int textHeight = 15;
    // 所有平台（含导弹）均以图标绘制，优先按图标尺寸计算边界矩形
    if (!m_scaledIcon.isNull()) {
        m_boundingRect = QRectF(-m_scaledIcon.width()/2 - 5, -m_scaledIcon.height()/2 - 5,
                                m_scaledIcon.width() + 10, m_scaledIcon.height() + 10 + textHeight);
    } else {
        m_boundingRect = QRectF(-15, -15, 30, 45);
    }
}

/**
 * @brief 更新平台数据
 * @param platform 新的平台数据
 */
void PlatformItem::updateData(const PlatformData &platform)
{
    bool needUpdate = false;

    if (m_platform.lon != platform.lon || m_platform.lat != platform.lat) {
        prepareGeometryChange();
        needUpdate = true;
    }

    if (m_platform.heading != platform.heading && platform.heading >= 0) {
        setRotation(platform.heading - 90);
        needUpdate = true;
    }

    if (m_platform.type != platform.type || m_platform.camp != platform.camp) {
        prepareGeometryChange();
        m_platform = platform;
        updateIcon();
        updateBoundingRect();
        m_campColor = campColor(platform.camp);  // 阵营变化时同步更新绘制颜色
        needUpdate = true;
    }

    m_platform = platform;

    if (needUpdate) {
        update();
    }
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
        
        if (m_platform.heading >= 0) {
            painter->rotate(-(m_platform.heading - 90));
        }
        
        painter->setFont(QFont("Arial", 8));
        painter->setPen(Qt::white);
        
        QRectF textRect;
        // 名称统一显示在图标下方
        if (!m_scaledIcon.isNull()) {
            textRect = QRectF(-20, m_scaledIcon.height()/2 + 5, 40, 15);
        } else {
            textRect = QRectF(-15, 10, 30, 15);
        }
        
        painter->drawText(textRect, Qt::AlignCenter, m_platform.id);
        
        painter->restore();
    }
}
