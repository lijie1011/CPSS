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

/**
 * @brief 构造函数
 * @param platform 平台数据
 * @param redBoatIcon 红方舰船图标
 * @param redPlaneIcon 红方飞机图标
 * @param purpleBoatIcon 紫方舰船图标
 * @param purplePlaneIcon 紫方飞机图标
 * @param parent 父图元
 */
PlatformItem::PlatformItem(const PlatformData &platform, const QImage &redBoatIcon,
                           const QImage &redPlaneIcon, const QImage &purpleBoatIcon,
                           const QImage &purplePlaneIcon, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_platform(platform),
      m_redBoatIcon(redBoatIcon),
      m_redPlaneIcon(redPlaneIcon),
      m_purpleBoatIcon(purpleBoatIcon),
      m_purplePlaneIcon(purplePlaneIcon)
{
    updateIcon();
    updateBoundingRect();

    if (platform.heading >= 0) {
        setRotation(platform.heading - 90);
    }

    switch (platform.camp) {
    case Camp_Friendly: m_campColor = Qt::green; break;
    case Camp_Red: m_campColor = Qt::red; break;
    case Camp_Purple: m_campColor = QColor(148, 0, 211); break;
    case Camp_Enemy: m_campColor = Qt::darkRed; break;
    case Camp_Neutral: m_campColor = Qt::yellow; break;
    default: m_campColor = Qt::gray; break;
    }
}

/**
 * @brief 更新图标
 * @details 根据平台类型和阵营选择合适的显示图标
 */
void PlatformItem::updateIcon()
{
    m_isMissile = (m_platform.type == "missile");

    if (!m_isMissile) {
        bool isAircraft = (m_platform.type == "aircraft");
        bool isRed = (m_platform.camp == Camp_Red);
        bool isPurple = (m_platform.camp == Camp_Purple);

        if (isRed && isAircraft) {
            m_currentIcon = m_redPlaneIcon;
        } else if (isRed && !isAircraft) {
            m_currentIcon = m_redBoatIcon;
        } else if (isPurple && isAircraft) {
            m_currentIcon = m_purplePlaneIcon;
        } else if (isPurple && !isAircraft) {
            m_currentIcon = m_purpleBoatIcon;
        } else {
            m_currentIcon = QImage();
        }

        if (!m_currentIcon.isNull()) {
            int iconSize = 32;
            m_scaledIcon = m_currentIcon.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
}

/**
 * @brief 更新边界矩形
 * @details 根据平台类型和图标尺寸计算图元的边界矩形
 */
void PlatformItem::updateBoundingRect()
{
    int textHeight = 15;
    if (m_isMissile) {
        m_boundingRect = QRectF(-15, -10, 30, 35);
    } else if (!m_scaledIcon.isNull()) {
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

    if (m_isMissile) {
        QColor missileColor(255, 165, 0);
        painter->setPen(QPen(missileColor, 2));
        painter->setBrush(missileColor);
        painter->drawEllipse(-5, -5, 10, 10);
        painter->setFont(QFont("Arial", 7));
        painter->setPen(Qt::white);
        painter->drawText(QRectF(-15, -15, 30, 10), Qt::AlignCenter, "M");
    } else if (!m_scaledIcon.isNull()) {
        painter->drawImage(-m_scaledIcon.width()/2, -m_scaledIcon.height()/2, m_scaledIcon);
    } else {
        bool isOwnShip = (m_platform.id == "1");

        if (isOwnShip) {
            painter->setPen(QPen(m_campColor, 2));
            QPolygonF shipShape;
            shipShape << QPointF(15, 0)
                      << QPointF(-10, -8)
                      << QPointF(-5, 0)
                      << QPointF(-10, 8);
            painter->drawPolygon(shipShape);
        } else {
            painter->setPen(QPen(m_campColor, 2));
            painter->setBrush(m_campColor);
            painter->drawEllipse(-8, -8, 16, 16);
        }
    }

    if (m_displayState.showName) {
        painter->save();
        
        if (m_platform.heading >= 0) {
            painter->rotate(-(m_platform.heading - 90));
        }
        
        painter->setFont(QFont("Arial", 8));
        painter->setPen(Qt::white);
        
        QRectF textRect;
        if (m_isMissile) {
            textRect = QRectF(-15, 8, 30, 15);
        } else if (!m_scaledIcon.isNull()) {
            textRect = QRectF(-20, m_scaledIcon.height()/2 + 5, 40, 15);
        } else {
            textRect = QRectF(-15, 10, 30, 15);
        }
        
        painter->drawText(textRect, Qt::AlignCenter, m_platform.id);
        
        painter->restore();
    }
}
